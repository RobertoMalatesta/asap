//    Copyright The Blocxxi Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <cmath>  // for rounding frame rate

#include <imgui.h>

#include <imgui/imgui_dock.h>
#include <ui/application_base.h>
#include <ui/log/sink.h>

namespace asap {
namespace debug {
namespace ui {

void ApplicationBase::Init() {
  sink_ = std::make_shared<asap::debug::ui::ImGuiLogSink>();
  asap::logging::Registry::PushSink(sink_);

  ASLOG(debug, "Initializing UI theme");
  asap::debug::ui::Theme::Init();

  ImGui::LoadDock();

  // Call for custom init operations from derived class
  AfterInit();
}

void ApplicationBase::ShutDown() {
  // Call derived class for any custom shutdown logic before we shutdown the
  // app. We do this before to stay consistent with the initialization order.
  BeforeShutDown();

  ImGui::SaveDock();

  ImGui::ShutdownDock();
}

bool ApplicationBase::Draw() {
  auto menu_height = DrawMainMenu();

  if (ImGui::GetIO().DisplaySize.y > 0) {
    auto pos = ImVec2(0, menu_height);
    auto size = ImGui::GetIO().DisplaySize;
    size.y -= pos.y;
    ImGui::RootDock(pos, ImVec2(size.x, size.y - 16.0f));

    DrawStatusBar(size.x, 16.0f, 0.0f, size.y);

    if (show_logs_) DrawLogView();
    if (show_style_editor_) DrawStyleEditor();
    if (show_docks_debug_) DrawDocksDebug();
  }

  // Return true to indicate that we are not doing any calculation and we can
  // sleep if the application window is not focused.
  return true;
}

float ApplicationBase::DrawMainMenu() {
  auto menu_height = 0.0f;
  if (ImGui::BeginMainMenuBar()) {
    // Call the derived class to draw the application specific menus inside the
    // menu bar
    DrawInsideMainMenu();

    // Last menu is the Debug menu
    if (ImGui::BeginMenu("Debug")) {
      if (ImGui::MenuItem("Show Logs", "CTRL+SHIFT+L", &show_logs_)) {
        DrawLogView();
      }
      if (ImGui::MenuItem("Show Style Editor", "CTRL+SHIFT+S",
                          &show_style_editor_)) {
        DrawStyleEditor();
      }
      ImGui::EndMenu();
    }
    menu_height = ImGui::GetWindowSize().y;

    ImGui::EndMainMenuBar();
  }

  return menu_height;
}

void ApplicationBase::DrawStatusBar(float width, float height, float pos_x,
                                    float pos_y) {
  // Draw status bar (no docking)
  ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiSetCond_Always);
  ImGui::SetNextWindowPos(ImVec2(pos_x, pos_y), ImGuiSetCond_Always);
  ImGui::Begin("statusbar", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_NoBringToFrontOnFocus |
                   ImGuiWindowFlags_NoResize);

  // Call the derived class to add stuff to the status bar
  DrawInsideStatusBar(width - 45.0f, height);

  // Draw the common stuff
  ImGui::SameLine(width - 45.0f);
  Font font(Font::FAMILY_PROPORTIONAL);
  font.Normal().Regular().SmallSize();
  ImGui::PushFont(font.ImGuiFont());
  ImGui::Text("FPS: %ld", std::lround(ImGui::GetIO().Framerate));
  ImGui::PopFont();
  ImGui::End();
}

void ApplicationBase::DrawLogView() {
  if (ImGui::BeginDock("Logs", &show_logs_)) {
    // Draw the log view docked
    sink_->Draw();
  }
  ImGui::EndDock();
}

void ApplicationBase::DrawStyleEditor() {
  if (ImGui::BeginDock("StyleEditor", &show_style_editor_)) {
    ImGui::ShowStyleEditor();
  }
  ImGui::EndDock();
}
void ApplicationBase::DrawDocksDebug() {
  if (ImGui::BeginDock("Docks", &show_docks_debug_)) {
    ImGui::PrintDocks();  // print docking information
  }
  ImGui::EndDock();
}

#if 0
// 1. Show a simple window.
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets
    // automatically appears in a window called "Debug".
    {
      ImGui::ColorEdit3(
          "clear color",
          (float *) &clear_color);  // Edit 3 floats representing a color

      ImGui::Checkbox("Demo Window",
                      &show_demo_window);  // Edit bools storing our windows
      // open/close state
      ImGui::Checkbox("Log Messages Window", &show_log_messages_window);

      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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

#endif

}  // namespace ui
}  // namespace debug
}  // namespace asap
