#include <algorithm>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <variant>

#include "absl/flags/flag.h"
#include "absl/flags/usage.h"
#include "google/protobuf/stubs/common.h"
#include "gsl/gsl"
#include "toolchain/compilation_database/compilation_database_util.h"
#include "toolchain/compilation_database/compile_commands.pb.h"
#include "util/flags.h"
#include "util/result.h"

ABSL_FLAG(util::flags::OutputPath, compile_command_directory, {},
          "Directory for each compile command.");

ABSL_FLAG(util::flags::OutputPath, output_compilation_database, {},
          "Path to the output compile_commands.json file.");

namespace {

using toolchain::compilation_database::ConcatenateCompileCommandsError;

const char* kUsage =
    "Concatenate compile command intermediate protocol buffers generated by "
    "the `extract_compile_command` action listener, inject the execution "
    "directory, and output the generated JSON compilation database. The "
    "intermediate file names are read from stdin.";

}  // namespace

auto main(int argc, char** argv) -> int {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  auto _ = gsl::finally([] { google::protobuf::ShutdownProtobufLibrary(); });

  absl::SetProgramUsageMessage(kUsage);
  absl::ParseCommandLine(argc, argv);

  std::vector<std::string> input_files{};
  std::copy(std::istream_iterator<std::string>(std::cin),
            std::istream_iterator<std::string>(),
            std::back_inserter(input_files));
  const auto compile_command_directory =
      absl::GetFlag(FLAGS_compile_command_directory).output_path;
  const auto make_input_stream = [](const std::string_view input_file)
      -> std::variant<std::ifstream, std::istringstream> {
    return std::ifstream{input_file, std::ios::in | std::ios::binary};
  };
  const auto compile_commands_result =
      toolchain::compilation_database::ConcatenateCompileCommands(
          input_files, compile_command_directory, make_input_stream);
  if (compile_commands_result.IsErr()) {
    switch (compile_commands_result.Err()) {
      case ConcatenateCompileCommandsError::kParsingError:
        std::cerr << "Failed to parse the extra action info file.\n";
        break;
    }
    return EXIT_FAILURE;
  }
  const auto& compile_commands = compile_commands_result.Ok();

  const auto output_compilation_database =
      absl::GetFlag(FLAGS_output_compilation_database).output_path;
  std::ofstream output_stream{
      output_compilation_database,
      std::ios::out | std::ios::trunc | std::ios::binary};
  const auto write_result =
      toolchain::compilation_database::SerializeCompileCommandsToOutputStream(
          compile_commands, &output_stream);
  if (write_result.IsErr()) {
    std::cerr << "Failed to concatenate the compile commands: "
              << write_result.Err().message() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
