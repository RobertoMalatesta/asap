//    Copyright The Blocxxi Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <fstream>  // to check if a file exists
#include <iostream>

#include <boost/program_options.hpp>

#include <common/logging.h>
#include <console_runner.h>
#include <imgui_runner.h>
#include <config.h>

namespace bpo = boost::program_options;


using asap::ConsoleRunner;
using asap::ImGuiRunner;
using asap::RunnerBase;

void Shutdown() {
  auto &logger = asap::logging::Registry::GetLogger(asap::logging::Id::MAIN);
  // Shutdown
  ASLOG_TO_LOGGER(logger, info, "Shutdown complete");
}

int main(int argc, char **argv) {
  auto &logger = asap::logging::Registry::GetLogger(asap::logging::Id::MAIN);

  asap::fs::CreateDirectories();


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

    if (!show_debug_gui) {
      ASLOG_TO_LOGGER(logger, info, "starting in console mode...");
      //
      // Start the console runner
      //
      ConsoleRunner runner(Shutdown);
      runner.Run();
    } else {
      ASLOG_TO_LOGGER(logger, info, "starting in GUI mode...");
      //
      // Start the ImGui runner
      //
      ImGuiRunner runner(Shutdown);
      runner.LoadSetting();
      runner.Run();
    }
  } catch (std::exception &e) {
    ASLOG_TO_LOGGER(logger, error, "Error: {}", e.what());
    return -1;
  } catch (...) {
    ASLOG_TO_LOGGER(logger, error, "Unknown error!");
    return -1;
  }

  return 0;
}
