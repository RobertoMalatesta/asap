//    Copyright The Blocxxi Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#pragma once

#include <common/logging.h>

#include <ui/abstract_application.h>
#include <ui/log/sink.h>

namespace asap {
class ImGuiRunner;

namespace debug {
namespace ui {


class ApplicationBase : public AbstractApplication,
                        asap::NonCopiable,
                        asap::logging::Loggable<asap::logging::Id::MAIN> {
 public:
  explicit ApplicationBase(ImGuiRunner& runner);

  /// Not move constructible
  ApplicationBase(ApplicationBase &&) = delete;
  /// Not move assignable
  ApplicationBase &operator=(ApplicationBase &&) = delete;

  ~ApplicationBase() override = default;

  void Init() final;
  bool Draw() override;
  void ShutDown() final;

 protected:
  virtual void AfterInit() {}
  virtual void DrawInsideMainMenu() {}
  virtual void DrawInsideStatusBar(float /*width*/, float /*height*/) {}
  virtual void BeforeShutDown() {}

 private:
  float DrawMainMenu();
  void DrawStatusBar(float width, float height, float pos_x, float pos_y);
  void DrawLogView();
  void DrawSettings();
  void DrawDocksDebug();

 private:
  bool show_docks_debug_{true};
  bool show_logs_{true};
  bool show_settings_{true};

  std::shared_ptr<ImGuiLogSink> sink_;
  ImGuiRunner &runner_;
};

}  // namespace ui
}  // namespace debug
}  // namespace asap
