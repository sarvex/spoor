#include "spoor/runtime/flush_queue/flush_queue.h"

#include <chrono>
#include <filesystem>
#include <functional>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>

#include "absl/strings/str_format.h"
#include "gsl/gsl"
#include "spoor/runtime/trace/trace.h"

namespace spoor::runtime::flush_queue {

FlushQueue::FlushQueue(const Options& options)
    : options_{options},
      flush_thread_{},
      lock_{},
      queue_{},
      flush_timestamp_{options.steady_clock->Now()},
      running_{false},
      draining_{false} {}

FlushQueue::~FlushQueue() { DrainAndStop(); }

auto FlushQueue::Run() -> void {
  if (running_.exchange(true)) return;
  draining_ = false;
  flush_thread_ = std::thread{[&]() {
    while (!draining_.load() || !Empty()) {
      auto flush_info_optional = [&]() -> std::optional<FlushInfo> {
        std::unique_lock lock{lock_};
        if (queue_.empty()) return {};
        auto flush_info = std::move(queue_.back());
        queue_.pop();
        return flush_info;
      }();
      if (!flush_info_optional.has_value()) {
        std::this_thread::yield();
        continue;
      };
      auto flush_info = std::move(flush_info_optional.value());
      const bool retain{options_.steady_clock->Now() <
                        flush_info.flush_timestamp +
                            options_.buffer_retention_duration};
      const bool flush{flush_info.flush_timestamp <= flush_timestamp_};
      if (retain && !flush) {
        std::unique_lock lock{lock_};
        queue_.emplace(std::move(flush_info));
        continue;
      }
      const auto result = options_.trace_writer->Write(
          TraceFilePath(flush_info), TraceFileHeader(flush_info),
          flush_info.buffer, trace::Footer{});
      if (result.IsErr() && 0 < flush_info.remaining_flush_attempts) {
        --flush_info.remaining_flush_attempts;
        std::unique_lock lock{lock_};
        queue_.emplace(std::move(flush_info));
      }
    }
    draining_ = false;
  }};
}

auto FlushQueue::DrainAndStop() -> void {
  if (!running_.load() || draining_.exchange(true)) return;
  if (flush_thread_.joinable()) flush_thread_.join();
  running_ = false;
}

auto FlushQueue::Enqueue(Buffer&& buffer) -> void {
  const auto flush_timestamp = options_.steady_clock->Now();
  if (!running_.load() || draining_.load()) return;
  const auto thread_id = static_cast<trace::ThreadId>(
      std::hash<std::thread::id>{}(std::this_thread::get_id()));
  FlushInfo flush_info{
      .buffer = std::move(buffer),
      .flush_timestamp = flush_timestamp,
      .thread_id = thread_id,
      .remaining_flush_attempts = options_.max_buffer_flush_attempts};
  std::unique_lock lock{lock_};
  queue_.emplace(std::move(flush_info));
}

auto FlushQueue::Flush() -> void {
  std::unique_lock lock{lock_};
  flush_timestamp_ = options_.steady_clock->Now();
}

auto FlushQueue::Clear() -> void {
  std::queue<FlushInfo> empty{};
  std::unique_lock lock{lock_};
  std::swap(queue_, empty);
}

auto FlushQueue::GetState() const -> State {
  if (!running_.load()) return State::kStopped;
  if (draining_.load()) return State::kDraining;
  return State::kRunning;
}

auto FlushQueue::Size() const -> SizeType {
  std::shared_lock lock{lock_};
  return queue_.size();
}

auto FlushQueue::Empty() const -> bool {
  std::shared_lock lock{lock_};
  return queue_.empty();
}

auto FlushQueue::TraceFilePath(const FlushInfo& flush_info) const
    -> std::filesystem::path {
  const auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
                             flush_info.flush_timestamp.time_since_epoch())
                             .count();
  const auto file_name =
      absl::StrFormat("spoor-%016x-%016x-%016x.trace", options_.session_id,
                      flush_info.thread_id, timestamp);
  return options_.trace_file_path / file_name;
}

auto FlushQueue::TraceFileHeader(const FlushInfo& flush_info) const
    -> trace::Header {
  const auto system_clock_now = options_.system_clock->Now();
  const auto steady_clock_now = options_.steady_clock->Now();
  const auto system_clock_timestamp =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          system_clock_now.time_since_epoch())
          .count();
  const auto steady_clock_timestamp =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          steady_clock_now.time_since_epoch())
          .count();
  const auto event_count =
      gsl::narrow_cast<trace::EventCount>(flush_info.buffer.Size());
  return trace::Header{.version = trace::kTraceFileVersion,
                       .session_id = options_.session_id,
                       .process_id = options_.process_id,
                       .thread_id = flush_info.thread_id,
                       .system_clock_timestamp = system_clock_timestamp,
                       .steady_clock_timestamp = steady_clock_timestamp,
                       .event_count = event_count};
}

}  // namespace spoor::runtime::flush_queue
