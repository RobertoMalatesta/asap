//    Copyright The Blocxxi Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <iostream>

#include <boost/program_options.hpp>

#include <common/logging.h>
#include <console_runner.h>
#include <imgui_runner.h>

namespace bpo = boost::program_options;

using asap::ConsoleRunner;
using asap::ImGuiRunner;
using asap::RunnerBase;

void Shutdown() {
  auto &logger = asap::logging::Registry::GetLogger(asap::logging::Id::MAIN);
  // Shutdown
  ASLOG_TO_LOGGER(logger, info, "Shutting down...");
}

int main(int argc, char **argv) {
  auto &logger = asap::logging::Registry::GetLogger(asap::logging::Id::MAIN);

  bool show_debug_gui{false};
  try {
    // Command line arguments
    bpo::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()
      ("help", "show the help message")
      ("debug-ui,d", bpo::value<bool>(&show_debug_gui)->default_value(false),
        "show the debug UI");
    // clang-format on

    bpo::variables_map bpo_vm;
    bpo::store(bpo::parse_command_line(argc, argv, desc), bpo_vm);

    if (bpo_vm.count("help")) {
      std::cout << desc << std::endl;
      return 0;
    }

    bpo::notify(bpo_vm);

    ASLOG_TO_LOGGER(logger, info, "Application is starting...");

    if (!show_debug_gui) {
      //
      // Start the console runner
      //
      ConsoleRunner runner(Shutdown);
      runner.AwaitStop();
    } else {
      //
      // Start the ImGui runner
      //
      ImGuiRunner runner(Shutdown);
      runner.AwaitStop();
    }
  } catch (std::exception &e) {
    // Restore the original log sink
    if (show_debug_gui) asap::logging::Registry::PopSink();
    ASLOG_TO_LOGGER(logger, error, "Error: {}", e.what());
    return -1;
  } catch (...) {
    // Restore the original log sink
    if (show_debug_gui) asap::logging::Registry::PopSink();
    ASLOG_TO_LOGGER(logger, error, "Unknown error!");
    return -1;
  }

  return 0;
}
