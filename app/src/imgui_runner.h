//    Copyright The asap Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#pragma once

#include <runner.h>

namespace boost {
namespace asio {
class io_context;
class signal_set;
}  // namespace asio
}  // namespace boost

struct GLFWwindow;

namespace asap {

class ImGuiRunner : public RunnerBase {
 public:
  explicit ImGuiRunner(shutdown_function_type f);

  ~ImGuiRunner() override;
  ImGuiRunner(const ImGuiRunner &) = delete;
  ImGuiRunner &operator=(const ImGuiRunner &) = delete;

  void AwaitStop() override;

 private:
  void Init();
  void CleanUp();

  GLFWwindow *window;

  boost::asio::io_context *io_context_;
  /// The signal_set is used to register for process termination notifications.
  boost::asio::signal_set *signals_;
};

}  // namespace asap
