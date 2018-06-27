//    Copyright The Blocxxi Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <fstream>  // to check if a file exists
#include <iostream>

#include <yaml-cpp/yaml.h>
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

  static char const *DEFAULT_CONFIG_FILE_NAME = "settings.yaml";
  std::string config_file_name;
  bool show_debug_gui{false};
  try {
    // Command line arguments
    bpo::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()
        ("help", "show the help message")
        ("config,c",
         bpo::value<std::string>(&config_file_name)->default_value(
             DEFAULT_CONFIG_FILE_NAME), "show the debug UI")
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

    // -------------------------------------------------------------------------
    // Load settings
    // -------------------------------------------------------------------------
    YAML::Node config;
    auto has_config{false};
    std::ifstream file(config_file_name);
    if (file) {
      try {
        config = YAML::LoadFile(config_file_name);
        ASLOG_TO_LOGGER(logger, info, "settings loaded from {}",
                        config_file_name);
        has_config = true;
      } catch (std::exception const &ex) {
        ASLOG_TO_LOGGER(logger, error,
                        "error () while loading settings from {}", ex.what(),
                        config_file_name);
      }
    } else {
      ASLOG_TO_LOGGER(logger, info, "no {} in current directory",
                      config_file_name);
    }

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

      try {
        auto display = config["settings"]["graphics"]["display"];
        if (has_config && !display) {
          ASLOG_TO_LOGGER(logger, warn,
                          "missing settings/graphics/display in config");
        }

        if (has_config && !display["mode"]) {
          ASLOG_TO_LOGGER(logger, warn, "missing 'display/mode' in config");
        }

        if (display["multi-sampling"]) {
          runner.MultiSample(display["multi-sampling"].as<int>());
        }
        if (display["vsync"]) {
          runner.EnableVsync(display["vsync"].as<bool>());
        }

        auto mode = display["mode"].as<std::string>();
        if (mode == "Full Screen") {
          if (!display["size"]["width"]) {
            ASLOG_TO_LOGGER(logger, warn,
                            "missing 'display/size/width' in config");
          }
          if (!display["size"]["height"]) {
            ASLOG_TO_LOGGER(logger, warn,
                            "missing 'display/size/height' in config");
          }
          if (!display["title"]) {
            ASLOG_TO_LOGGER(logger, warn, "missing 'display/title' in config");
          }
          if (!display["monitor"]) {
            ASLOG_TO_LOGGER(logger, warn,
                            "missing 'display/monitor' in config");
          }
          if (!display["refresh-rate"]) {
            ASLOG_TO_LOGGER(logger, warn,
                            "missing 'display/refresh-rate' in config");
          }

          runner.FullScreen(display["size"]["width"].as<int>(),
                            display["size"]["height"].as<int>(),
                            display["title"].as<std::string>().c_str(),
                            display["monitor"].as<int>(),
                            display["refresh-rate"].as<int>());
        } else if (mode == "Full Screen Windowed") {
          if (!display["title"]) {
            ASLOG_TO_LOGGER(logger, warn, "missing 'display/title' in config");
          }
          if (!display["monitor"]) {
            ASLOG_TO_LOGGER(logger, warn,
                            "missing 'display/monitor' in config");
          }
          runner.FullScreenWindowed(display["title"].as<std::string>().c_str(),
                                    display["monitor"].as<int>());
        } else if (mode == "Windowed") {
          if (!display["size"]["width"]) {
            ASLOG_TO_LOGGER(logger, warn,
                            "missing 'display/size/width' in config");
          }
          if (!display["size"]["height"]) {
            ASLOG_TO_LOGGER(logger, warn,
                            "missing 'display/size/height' in config");
          }
          if (!display["title"]) {
            ASLOG_TO_LOGGER(logger, warn, "missing 'display/title' in config");
          }
          runner.Windowed(display["size"]["width"].as<int>(),
                          display["size"]["height"].as<int>(),
                          display["title"].as<std::string>().c_str());
        } else {
          ASLOG_TO_LOGGER(logger, error, "invalid 'display/mode' ({})", mode);
          runner.Windowed(900, 700, "ASAP Application");
        }
      } catch (std::exception const &ex) {
        if (has_config) {
          ASLOG_TO_LOGGER(logger, warn,
                          "configuration error; set all missing entries and "
                          "use the correct types");
        }
        // Use defaults
        runner.Windowed(900, 700, "ASAP Application");
      }

      runner.Run();
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
