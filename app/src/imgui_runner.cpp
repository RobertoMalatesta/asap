//    Copyright The asap Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <imgui_runner.h>

#include <boost/asio.hpp>

// clang-format off
// Include order is important
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
// clang-format on

#include <ui/application.h>

namespace {
void glfw_error_callback(int error, const char *description) {
  auto &logger = asap::logging::Registry::GetLogger(asap::logging::Id::MAIN);
  ASLOG_TO_LOGGER(logger, critical, "Glfw Error {}: {}", error, description);
}
}  // namespace

namespace asap {

ImGuiRunner::ImGuiRunner(RunnerBase::shutdown_function_type f)
    : RunnerBase(std::move(f)) {
  Init();
}

ImGuiRunner::~ImGuiRunner() {
  if (!io_context_->stopped()) io_context_->stop();
  delete signals_;
  delete io_context_;
}

void ImGuiRunner::Init() {
  ASLOG(info, "Setup termination signal handlers");
  // Set signal handler
  io_context_ = new boost::asio::io_context(1);
  signals_ = new boost::asio::signal_set(*io_context_);
  // Register to handle the signals that indicate when the server should exit.
  // It is safe to register for the same signal multiple times in a program,
  // provided all registration for the specified signal is made through Asio.
  signals_->add(SIGINT);
  signals_->add(SIGTERM);

  ASLOG(info, "Initialize graphical subsystem...");
  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  window = glfwCreateWindow(960, 600, "Debug Console", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  gladLoadGL((GLADloadfunc)glfwGetProcAddress);
  glfwSwapInterval(1);  // Enable vsync
  ASLOG(debug, "  GLFW init done");

  // Setup Dear ImGui binding
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable
  // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  // // Enable Gamepad Controls
  ASLOG(debug, "  ImGui context init done");

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init();
  ASLOG(debug, "  OpenGL3 init done");
}

void ImGuiRunner::CleanUp() {
  // Restore the original log sink
  ASLOG(debug, "  restore original logging sink");
  asap::logging::Registry::PopSink();

  ASLOG(info, "Cleanup graphical subsystem...");

  // Cleanup ImGui
  ASLOG(debug, "  shutdown OpenGL3");
  ImGui_ImplOpenGL3_Shutdown();
  ASLOG(debug, "  shutdown GLFW");
  ImGui_ImplGlfw_Shutdown();
  ASLOG(debug, "  destroy ImGui context");
  ImGui::DestroyContext();

  ASLOG(debug, "  destroy window");
  glfwDestroyWindow(window);
  ASLOG(debug, "  terminate GLFW");
  glfwTerminate();

  // Call the shutdown hook
  ASLOG(debug, "  call shutdown handler");
  shutdown_function_();
}

void ImGuiRunner::Run() {
  // Create the Application GUI
  asap::debug::ui::Application app;
  app.Init();

  // Main loop
  bool interrupted = false;
  bool sleep_when_inactive = true;
  while (!glfwWindowShouldClose(window) && !interrupted) {
    signals_->async_wait(
        [this, &interrupted](boost::system::error_code /*ec*/, int /*signo*/) {
          ASLOG(info, "Signal caught");
          // Once all operations have finished the io_context::run() call will
          // exit.
          interrupted = true;
          io_context_->stop();
        });

    if (sleep_when_inactive && !glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
      static float wanted_fps = 5.0f;
      float current_fps = ImGui::GetIO().Framerate;
      float frame_time = 1000 / current_fps;
      auto wait_time = std::lround(1000 / wanted_fps - frame_time);
      if (wanted_fps < current_fps) {
        io_context_->run_for(std::chrono::milliseconds(wait_time));
      }
    } else {
      io_context_->poll_one();
    }

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

    // Draw the Application
    sleep_when_inactive = app.Draw();

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0, 0, 0, 255);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwMakeContextCurrent(window);
    glfwSwapBuffers(window);
  }

  app.ShutDown();

  CleanUp();
}

}  // namespace asap