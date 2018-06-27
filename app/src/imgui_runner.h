//    Copyright The asap Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#pragma once

#include <runner_base.h>

namespace boost {
namespace asio {
class io_context;
class signal_set;
}  // namespace asio
}  // namespace boost

struct GLFWwindow;
struct GLFWmonitor;

namespace asap {

class ImGuiRunner : public RunnerBase {
 public:
  explicit ImGuiRunner(shutdown_function_type f);

  ~ImGuiRunner() override;
  ImGuiRunner(const ImGuiRunner &) = delete;
  ImGuiRunner &operator=(const ImGuiRunner &) = delete;

  void Windowed(int width, int height, char const *title);
  void FullScreenWindowed(char const *title, int monitor);
  void FullScreen(int width, int height, char const *title, int monitor,
                  int refresh_rate);

  void EnableVsync(bool state = true) const;
  void MultiSample(int samples) const;

  void Run() override;

 private:
  void SetupSignalHandler();
  void InitGraphics();
  void SetupContext();
  void SetupFrameBuffer();
  void InitImGui();
  void CleanUp();

  GLFWwindow *window{nullptr};

  boost::asio::io_context *io_context_;
  /// The signal_set is used to register for process termination notifications.
  boost::asio::signal_set *signals_;
};

}  // namespace asap
