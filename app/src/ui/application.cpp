//    Copyright The Blocxxi Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <ui/application.h>

#include <imgui.h>
#include <imgui/imgui_dock.h>

namespace asap {
namespace debug {
namespace ui {

bool Application::Draw() {
  bool sleep_when_inactive = ApplicationBase::Draw();

  if (ImGui::GetIO().DisplaySize.y > 0) {
    // TODO: Temporary until we link this to application config and dock layout
    if (ImGui::BeginDock("Dummy2")) {
      ImGui::Text("Placeholder!");
    }
    ImGui::EndDock();
  }

  return sleep_when_inactive;
}

}  // namespace ui
}  // namespace debug
}  // namespace asap

