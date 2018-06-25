//    Copyright The Blocxxi Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#pragma once

#include <ui/application_base.h>

namespace asap {
namespace debug {
namespace ui {

class Application final : public ApplicationBase {
 public:
  Application() = default;

  /// Not move constructible
  Application(Application &&) = delete;
  /// Not move assignable
  Application &operator=(Application &&) = delete;

  ~Application() override = default;

  bool Draw() final;

 protected:
  void AfterInit() override {}
  void DrawInsideMainMenu() override {}
  void DrawInsideStatusBar(float /*width*/, float /*height*/) override {}
  void BeforeShutDown() override {}
};

}  // namespace ui
}  // namespace debug
}  // namespace asap
