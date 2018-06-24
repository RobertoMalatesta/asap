//    Copyright The Blocxxi Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <iostream>

#include <boost/algorithm/string/join.hpp>  // for string join
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
//#include <boost/thread.hpp>

#include <common/logging.h>

#include <glad/gl.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
// This example is using gl3w to access OpenGL functions. You may freely use any
// other OpenGL loader such as: glew, glad, glLoadGen, etc.
#include <GLFW/glfw3.h>

#include <ui/debug_ui.h>

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

namespace bpo = boost::program_options;

class RunnerBase {
 public:
  using shutdown_function_type = std::function<void()>;

  explicit RunnerBase(const shutdown_function_type &func)
      : shutdown_function_(func) {}

  virtual ~RunnerBase() = default;

  virtual void AwaitStop() = 0;

 protected:
  shutdown_function_type shutdown_function_;
};

class ConsoleRunner : public RunnerBase {
 public:
  explicit ConsoleRunner(const shutdown_function_type &f)
      : RunnerBase(f), io_context_(1), signals_(io_context_) {
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
  }

  ~ConsoleRunner() {
    if (!io_context_.stopped()) io_context_.stop();
  }
  ConsoleRunner(const ConsoleRunner &) = delete;
  ConsoleRunner &operator=(const ConsoleRunner &) = delete;

  void AwaitStop() override {
    signals_.async_wait(
        [this](boost::system::error_code /*ec*/, int /*signo*/) {
          std::cout << "Signal caught" << std::endl;
          // The server is stopped by cancelling all outstanding asynchronous
          // operations.
          shutdown_function_();
          // Once all operations have finished the io_context::run() call will
          // exit.
          io_context_.stop();
        });
    io_context_.run();
  }

 private:
  boost::asio::io_context io_context_;
  /// The signal_set is used to register for process termination notifications.
  boost::asio::signal_set signals_;
};



int main(int argc, char **argv) {
  auto &logger =
      asap::logging::Registry::GetLogger(asap::logging::Id::MAIN);

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


    GLFWwindow *window = nullptr;
    if (!show_debug_gui) {
      //
      // Start the console runner
      //
      ConsoleRunner runner([]() {
        // server_thread.join();
      });
      runner.AwaitStop();
    } else {
      //
      // Use GUI
      //
      // Setup window
      glfwSetErrorCallback(glfw_error_callback);
      if (!glfwInit()) return 1;
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
      window = glfwCreateWindow(960, 600, "Debug Console", NULL, NULL);
      if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
      }

      glfwMakeContextCurrent(window);
      gladLoadGL((GLADloadfunc)glfwGetProcAddress);
      glfwSwapInterval(1);  // Enable vsync

      // Setup Dear ImGui binding
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImGuiIO &io = ImGui::GetIO();
      (void)io;
      // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable
      // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
      // // Enable Gamepad Controls

      ImGui_ImplGlfw_InitForOpenGL(window, true);
      ImGui_ImplOpenGL3_Init();

      // Setup style
      // ImGui::StyleColorsClassic();

      // Load Fonts
      // - If no fonts are loaded, dear imgui will use the default font. You can
      // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
      // them.
      // - AddFontFromFileTTF() will return the ImFont* so you can store it if
      // you need to select the font among multiple.
      // - If the file cannot be loaded, the function will return NULL. Please
      // handle those errors in your application (e.g. use an assertion, or
      // display an error and quit).
      // - The fonts will be rasterized at a given size (w/ oversampling) and
      // stored into a texture when calling
      // ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame
      // below will call.
      // - Read 'misc/fonts/README.txt' for more instructions and details.
      // - Remember that in C/C++ if you want to include a backslash \ in a
      // string literal you need to write a double backslash \\ !
      // io.Fonts->AddFontDefault();
      // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
      // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
      // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
      // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
      // ImFont* font =
      // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
      // NULL, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

      // const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges

      asap::debug::ui::Theme::Init();

      auto sink = std::make_shared<asap::debug::ui::ImGuiLogSink>();
      asap::logging::Registry::PushSink(sink);

      bool show_demo_window = false;
      bool show_log_messages_window = true;

      ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

      // Main loop
      while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
        // tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data
        // to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
        // data to your main application. Generally you may always pass all
        // inputs to dear imgui, and hide them from your application based on
        // those two flags.
        glfwPollEvents();

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show a simple window.
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets
        // automatically appears in a window called "Debug".
        {
          ImGui::ColorEdit3(
              "clear color",
              (float *)&clear_color);  // Edit 3 floats representing a color

          ImGui::Checkbox("Demo Window",
                          &show_demo_window);  // Edit bools storing our windows
          // open/close state
          ImGui::Checkbox("Log Messages Window", &show_log_messages_window);

          ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                      1000.0f / ImGui::GetIO().Framerate,
                      ImGui::GetIO().Framerate);
        }

        // 3. Show the ImGui demo window. Most of the sample code is in
        // ImGui::ShowDemoWindow(). Read its code to learn more about Dear
        // ImGui!
        if (show_demo_window) {
          ImGui::SetNextWindowPos(
              ImVec2(650, 20),
              ImGuiCond_FirstUseEver);  // Normally user code doesn't need/want
                                        // to
          // call this because positions are saved
          // in .ini file anyway. Here we just want
          // to make the demo initial state a bit
          // more friendly!
          ImGui::ShowDemoWindow(&show_demo_window);
        }

        if (show_log_messages_window) {
          sink->Draw("Log Messages", &show_log_messages_window);
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
      }
    }
    // Cleanup ImGui
    if (show_debug_gui) {
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImGui::DestroyContext();

      glfwDestroyWindow(window);
      glfwTerminate();

      // Restore the original log sink
      asap::logging::Registry::PopSink();
    }

    // Shutdown
    ASLOG_TO_LOGGER(logger, info, "Shutting down...");

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
