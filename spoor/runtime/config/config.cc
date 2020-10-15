#include "spoor/runtime/config/config.h"

namespace spoor::runtime::config {

using util::env::GetEnvOrDefault;

auto UserOptions::FromEnv(const util::env::GetEnv& get_env) -> UserOptions {
  return {.trace_file_path = GetEnvOrDefault(
              kTraceFilePathKey.data(), kTraceFilePathDefaultValue, get_env),
          .session_id = GetEnvOrDefault(kSessionIdKey.data(),
                                        kSessionIdDefaultValue(), get_env),
          .thread_event_buffer_capacity =
              GetEnvOrDefault(kThreadEventBufferCapacityKey.data(),
                              kThreadEventBufferCapacityDefaultValue, get_env),
          .max_reserved_event_buffer_slice_capacity = GetEnvOrDefault(
              kMaxReservedEventBufferSliceCapacityKey.data(),
              kMaxReservedEventBufferSliceCapacityDefaultValue, get_env),
          .max_dynamic_event_buffer_slice_capacity = GetEnvOrDefault(
              kMaxDynamicEventBufferSliceCapacityKey.data(),
              kMaxDynamicEventBufferSliceCapacityDefaultValue, get_env),
          .reserved_event_pool_capacity =
              GetEnvOrDefault(kReservedEventPoolCapacityKey.data(),
                              kReservedEventPoolCapacityDefaultValue, get_env),
          .dynamic_event_pool_capacity =
              GetEnvOrDefault(kDynamicEventPoolCapacityKey.data(),
                              kDynamicEventPoolCapacityDefaultValue, get_env),
          .dynamic_event_slice_borrow_cas_attempts = GetEnvOrDefault(
              kDynamicEventSliceBorrowCasAttemptsKey.data(),
              kDynamicEventSliceBorrowCasAttemptsDefaultValue, get_env),
          .event_buffer_retention_duration_nanoseconds = GetEnvOrDefault(
              kEventBufferRetentionDurationNanosecondsKey.data(),
              kEventBufferRetentionNanosecondsDefaultValue, get_env),
          .max_flush_buffer_to_file_attempts = GetEnvOrDefault(
              kMaxFlushBufferToFileAttemptsKey.data(),
              kMaxFlushBufferToFileAttemptsDefaultValue, get_env),
          .flush_event_buffer_when_full =
              GetEnvOrDefault(kFlushEventBufferWhenFullKey.data(),
                              kFlushEventBufferWhenFullDefaultValue, get_env),
          .flush_event_buffer_immediately_after_flush = GetEnvOrDefault(
              kFlushEventBufferImmediatelyAfterFlushKey.data(),
              kFlushEventBufferImmediatelyAfterFlushDefaultValue, get_env)};
}

auto operator==(const UserOptions& lhs, const UserOptions& rhs) -> bool {
  return lhs.trace_file_path == rhs.trace_file_path &&
         lhs.session_id == rhs.session_id &&
         lhs.thread_event_buffer_capacity == rhs.thread_event_buffer_capacity &&
         lhs.max_reserved_event_buffer_slice_capacity ==
             rhs.max_reserved_event_buffer_slice_capacity &&
         lhs.max_dynamic_event_buffer_slice_capacity ==
             rhs.max_dynamic_event_buffer_slice_capacity &&
         lhs.reserved_event_pool_capacity == rhs.reserved_event_pool_capacity &&
         lhs.dynamic_event_pool_capacity == rhs.dynamic_event_pool_capacity &&
         lhs.dynamic_event_slice_borrow_cas_attempts ==
             rhs.dynamic_event_slice_borrow_cas_attempts &&
         lhs.event_buffer_retention_duration_nanoseconds ==
             rhs.event_buffer_retention_duration_nanoseconds &&
         lhs.flush_event_buffer_when_full == rhs.flush_event_buffer_when_full &&
         lhs.flush_event_buffer_immediately_after_flush ==
             rhs.flush_event_buffer_immediately_after_flush;
}

};  // namespace spoor::runtime::config
