//    Copyright The Blocxxi Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <cmath>  // for rounding frame rate
#include <sstream>

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <imgui/imgui_dock.h>
#include <imgui_runner.h>
#include <ui/application_base.h>
#include <ui/fonts/material_design_icons.h>
#include <ui/log/sink.h>

namespace asap {
namespace debug {
namespace ui {

ApplicationBase::ApplicationBase(ImGuiRunner &runner) : runner_(runner) {}

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
    if (show_settings_) DrawSettings();
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
                          &show_settings_)) {
        DrawSettings();
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

namespace {

bool DrawDisplaySettingsTitle(ImGuiRunner &runner, char title[80]) {
  auto copied = runner.GetWindowTitle().copy(title, 79);
  title[copied] = '\0';
  if (ImGui::InputText("Window Title", title, 79,
                       ImGuiInputTextFlags_EnterReturnsTrue)) {
    runner.SetWindowTitle(title);
    return true;
  }
  return false;
}

bool DrawDisplaySettingsMode(int &display_mode) {
  static const char *mode_items[] = {"Windowed", "Full Screen",
                                     "Full Screen Windowed"};
  return ImGui::Combo("Display Mode", &display_mode, mode_items,
                      IM_ARRAYSIZE(mode_items));
}

bool DrawDisplaySettingsMonitor(ImGuiRunner &runner, GLFWmonitor *&monitor) {
  if (ImGui::BeginCombo("Monitor", glfwGetMonitorName(monitor))) {
    int count = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&count);
    for (int n = 0; n < count; n++) {
      bool is_selected = (monitors[n] == runner.GetMonitor());
      if (ImGui::Selectable(glfwGetMonitorName(monitors[n]), is_selected)) {
        monitor = monitors[n];
        return true;
      }
      if (is_selected) {
        // Set the initial focus when opening the combo (scrolling + for
        // keyboard navigation support in the upcoming navigation branch)
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  return false;
}

bool DrawDisplaySettingsWindowSize(int size[2]) {
  return ImGui::InputInt2("Size", size);
}

bool DrawDisplaySettingsResolution(GLFWmonitor *monitor,
                                   const GLFWvidmode *&resolution) {
  auto ostr = std::ostringstream();
  ostr << resolution->width << 'x' << resolution->height << " @ "
       << resolution->refreshRate << " Hz";
  auto current_mode = ostr.str();
  if (ImGui::BeginCombo("Resolution", ostr.str().c_str())) {
    int count = 0;
    const GLFWvidmode *video_modes = glfwGetVideoModes(monitor, &count);
    for (int n = 0; n < count; n++) {
      ostr = std::ostringstream();
      ostr << video_modes[n].width << 'x' << video_modes[n].height << " @ "
           << video_modes[n].refreshRate << " Hz";
      auto mode = ostr.str();
      bool is_selected = (mode == current_mode);
      if (ImGui::Selectable(mode.c_str(), is_selected)) {
        resolution = &video_modes[n];
        return true;
      }
      if (is_selected) {
        // Set the initial focus when opening the combo (scrolling + for
        // keyboard navigation support in the upcoming navigation branch)
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  return false;
}

int GetMonitorIndex(GLFWmonitor *monitor) {
  int count = 0;
  GLFWmonitor **monitors = glfwGetMonitors(&count);
  for (int n = 0; n < count; n++) {
    if (monitors[n] == monitor) return n;
  }
  return 0;
}
}  // namespace

void ApplicationBase::DrawSettings() {
  if (ImGui::BeginDock("Settings", &show_settings_)) {
    static bool first_time = true;
    if (first_time) {
      ImGui::SetNextTreeNodeOpen(true);
      first_time = false;
    }
    if (ImGui::CollapsingHeader("Display")) {
      static char title[80];
      static int display_mode = 0;
      static GLFWmonitor *monitor = nullptr;
      static int size[2] = {0, 0};
      static const GLFWvidmode *resolution = nullptr;
      static int samples = 0;
      static bool vsync = false;

      static const char *status = "";

      static bool reset_to_current = true;
      if (reset_to_current) {
        reset_to_current = false;
        if (runner_.IsWindowed() && runner_.IsFullScreen()) {
          display_mode = 2;
        } else if (runner_.IsFullScreen()) {
          display_mode = 1;
        } else {
          display_mode = 0;
        }

        runner_.GetWindowSize(size);

        monitor = runner_.GetMonitor();
        if (!monitor) monitor = glfwGetPrimaryMonitor();

        resolution = glfwGetVideoMode(monitor);

        vsync = runner_.Vsync();

        samples = runner_.MultiSample();

        status = "Active";
      }

      // Toolbar
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {5.0f, 0.0f});
      ImGui::PushStyleColor(ImGuiCol_WindowBg,
                            ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
      ImGui::BeginChild("Display Settings Toolbar",
                        {ImGui::GetContentRegionAvailWidth(), 24}, true,
                        ImGuiWindowFlags_NoTitleBar |
                            ImGuiWindowFlags_NoScrollbar |
                            ImGuiWindowFlags_NoScrollWithMouse);

      Font font(Font::FAMILY_PROPORTIONAL);
      font.Italic().Light().LargeSize();
      ImGui::PushFont(font.ImGuiFont());
      ImGui::TextUnformatted(status);
      ImGui::PopFont();

      auto button_color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
      button_color.w = 0.0f;
      ImGui::PushStyleColor(ImGuiCol_Button, button_color);

      ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - 55);
      if (ImGui::Button(ICON_MDI_CHECK_ALL, {20, 20})) {
        runner_.EnableVsync(vsync);
        runner_.MultiSample(samples);
        switch (display_mode) {
          // Windowed
          case 0:
            runner_.Windowed(size[0], size[1], title);
            break;

            // Full Screen
          case 1:
            runner_.FullScreen(resolution->width, resolution->height, title,
                               GetMonitorIndex(monitor),
                               resolution->refreshRate);
            break;

            // Full Screen Windowed
          case 2:
            runner_.FullScreenWindowed(title, GetMonitorIndex(monitor));
            break;

          default:;
        }
        status = "Active";
      }
      ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - 25);
      if (ImGui::Button(ICON_MDI_RESTORE, {20, 20})) {
        reset_to_current = true;
      }
      // Restore the button color
      ImGui::PopStyleColor();
      ImGui::EndChild();
      ImGui::PopStyleColor();
      ImGui::PopStyleVar();

      // Settings

      if (DrawDisplaySettingsTitle(runner_, title)) {
        status = "Changed...";
      };
      if (DrawDisplaySettingsMode(display_mode)) {
        status = "Changed...";
      };
      if (display_mode == 1 || display_mode == 2) {
        if (DrawDisplaySettingsMonitor(runner_, monitor)) {
          status = "Changed...";
        }
      }
      if (display_mode == 0) {
        if (DrawDisplaySettingsWindowSize(size)) {
          status = "Changed...";
        }
      }
      if (display_mode == 1) {
        if (DrawDisplaySettingsResolution(monitor, resolution)) {
          status = "Changed...";
        }
      }

      if (ImGui::Checkbox("V-Sync", &vsync)) {
        status = "Changed...";
      }
      if (ImGui::SliderInt("Multi-sampling", &samples, -1, 4, "%d")) {
        status = "Changed...";
      }
    }

    if (ImGui::CollapsingHeader("Style")) {
      ImGui::ShowStyleEditor();
    }
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
