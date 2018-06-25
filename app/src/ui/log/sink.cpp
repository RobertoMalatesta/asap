//    Copyright The asap Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <array>
#include <sstream>      // for log record formatting

#include <date/date.h>  // for time formatting

#include <ui/fonts/material_design_icons.h>
#include <ui/log/sink.h>

#include <common/assert.h>
#include <common/logging.h>

namespace asap {
namespace debug {
namespace ui {

const ImVec4 ImGuiLogSink::COLOR_WARN{0.9f, 0.7f, 0.0f, 1.0f};
const ImVec4 ImGuiLogSink::COLOR_ERROR{1.0f, 0.0f, 0.0f, 1.0f};

void ImGuiLogSink::Clear() {
  std::unique_lock<std::shared_timed_mutex> lock(records_mutex_);
  records_.clear();
}

void ImGuiLogSink::ShowLogLevelsPopup() {
  static constexpr int IDS_COUNT = 4;
  // clang-format off
  static std::array<asap::logging::Id, IDS_COUNT> logger_ids{{
    asap::logging::Id::MISC,
    asap::logging::Id::TESTING,
    asap::logging::Id::COMMON,
    asap::logging::Id::MAIN
  }};
  // clang-format on

  ImGui::MenuItem("Logging Levels", nullptr, false, false);

  for (auto id : logger_ids) {
    auto &the_logger = asap::logging::Registry::GetLogger(id);
    static int levels[IDS_COUNT];
    auto id_value =
        static_cast<typename std::underlying_type<asap::logging::Id>::type>(
            id);
    levels[id_value] = the_logger.level();
    auto format = std::string("%u (")
                      .append(spdlog::level::to_str(
                          spdlog::level::level_enum(levels[id_value])))
                      .append(")");
    if (ImGui::SliderInt(the_logger.name().c_str(), &levels[id_value], 0, 6,
                         format.c_str())) {
      the_logger.set_level(spdlog::level::level_enum(levels[id_value]));
    }
  }
}

void ImGuiLogSink::ShowLogFormatPopup() {
  ImGui::MenuItem("Logging Format", nullptr, false, false);
  ImGui::Checkbox("Time", &show_time_);
  ImGui::SameLine();
  ImGui::Checkbox("Thread", &show_thread_);
  ImGui::SameLine();
  ImGui::Checkbox("Level", &show_level_);
  ImGui::SameLine();
  ImGui::Checkbox("Logger", &show_logger_);
}

void ImGuiLogSink::Draw(const char *title, bool *open) {
  ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

  // This is the case when the log viewer is supposed to open in its own ImGui
  // window (not docked).
  if (open) {
    ASAP_ASSERT(title != nullptr);
    ImGui::Begin(title, open);
  }

  /*
  if (ImGui::Button("Log something...")) {
    BXLOG_MISC(trace, "TRACE");
    BXLOG_MISC(debug, "DEBUG");
    BXLOG_MISC(info, "INFO");
    BXLOG_MISC(warn, "WARN");
    BXLOG_MISC(error, "ERROR");
    BXLOG_MISC(critical, "CRITICAL");
  }
  */

  // -------------------------------------------------------------------------
  // Toolbar
  // -------------------------------------------------------------------------

  {
    // Make all buttons transparent in the toolbar
    auto button_color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
    button_color.w = 0.0f;
    ImGui::PushStyleColor(ImGuiCol_Button, button_color);

    if (ImGui::Button(ICON_MDI_SETTINGS " Levels")) {
      ImGui::OpenPopup("LogLevelsPopup");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Change the logging levels");
    }
    if (ImGui::BeginPopup("LogLevelsPopup")) {
      ShowLogLevelsPopup();
      ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_MDI_VIEW_COLUMN " Format")) {
      ImGui::OpenPopup("LogFormatPopup");
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Chose what information to show");
    }
    if (ImGui::BeginPopup("LogFormatPopup")) {
      ShowLogFormatPopup();
      ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_MDI_NOTIFICATION_CLEAR_ALL " Clear")) Clear();
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Discard all messages");
    }

    ImGui::SameLine();
    bool need_pop_style_var = false;
    if (wrap_) {
      // Highlight the button
      ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
      ImGui::PushStyleColor(ImGuiCol_Border,
                            ImGui::GetStyleColorVec4(ImGuiCol_TextSelectedBg));
      need_pop_style_var = true;
    }
    if (ImGui::Button(ICON_MDI_WRAP)) ToggleWrap();
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Toggle soft wraps");
    }
    if (need_pop_style_var) {
      ImGui::PopStyleColor();
      ImGui::PopStyleVar();
    }

    ImGui::SameLine();
    need_pop_style_var = false;
    if (scroll_lock_) {
      // Highlight the button
      ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
      ImGui::PushStyleColor(ImGuiCol_Border,
                            ImGui::GetStyleColorVec4(ImGuiCol_TextSelectedBg));
      need_pop_style_var = true;
    }
    if (ImGui::Button(ICON_MDI_LOCK)) ToggleScrollLock();
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Toggle automatic scrolling to the bottom");
    }
    if (need_pop_style_var) {
      ImGui::PopStyleColor();
      ImGui::PopStyleVar();
    }

    ImGui::SameLine();
    display_filter_.Draw(ICON_MDI_FILTER " Filter", -100.0f);
  }
  // Restore the button color
  ImGui::PopStyleColor();

  // -------------------------------------------------------------------------
  // Log records
  // -------------------------------------------------------------------------

  ImGui::Separator();
  ImGui::BeginChild("scrolling", ImVec2(0, 0), false,
                    ImGuiWindowFlags_HorizontalScrollbar);

  {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));

    Font font("Inconsolata");
    font.MediumSize();

    std::shared_lock<std::shared_timed_mutex> lock(records_mutex_);
    for (auto const &record : records_) {
      if (!display_filter_.IsActive() ||
          display_filter_.PassFilter(
              record.properties_.c_str(),
              record.properties_.c_str() + record.properties_.size()) ||
          display_filter_.PassFilter(
              record.source_.c_str(),
              record.source_.c_str() + record.source_.size()) ||
          display_filter_.PassFilter(
              record.message_.c_str(),
              record.message_.c_str() + record.message_.size())) {
        ImGui::BeginGroup();
        if (record.emphasis_) {
          font.Bold();
        } else {
          font.Regular();
        }
        ImGui::PushFont(font.ImGuiFont());

        if (record.color_range_start_ > 0) {
          auto props_len = record.properties_.size();

          ASAP_ASSERT_VAL(record.color_range_start_ < props_len,
                             record.color_range_start_);
          ASAP_ASSERT(record.color_range_end_ > record.color_range_start_);
          ASAP_ASSERT_VAL(record.color_range_end_ < props_len,
                             record.color_range_end_);

          std::string part =
              record.properties_.substr(0, record.color_range_start_);
          ImGui::TextUnformatted(part.c_str());
          ImGui::SameLine();

          part = record.properties_.substr(
              record.color_range_start_,
              record.color_range_end_ - record.color_range_start_);
          ImGui::TextColored(record.color_, "%s", part.c_str());

          part = record.properties_.substr(record.color_range_end_,
                                           props_len - record.color_range_end_);
          ImGui::SameLine();
          ImGui::TextUnformatted(part.c_str());

        } else {
          if (record.color_range_end_ == 1) {
            ImGui::TextColored(record.color_, "%s", record.properties_.c_str());
          } else {
            ImGui::TextUnformatted(record.properties_.c_str());
          }
        }

        if (record.color_range_end_ == 1) {
          ImGui::SameLine();
          if (wrap_) ImGui::PushTextWrapPos(0.0f);
          ImGui::TextColored(record.color_, "%s", record.message_.c_str());
          if (wrap_) ImGui::PopTextWrapPos();
        } else {
          ImGui::SameLine();
          if (wrap_) ImGui::PushTextWrapPos(0.0f);
          ImGui::TextUnformatted(record.message_.c_str());
          if (wrap_) ImGui::PopTextWrapPos();
        }
        ImGui::EndGroup();
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("%s", record.source_.c_str());
        }

        ImGui::PopFont();
      }
    }
  }
  ImGui::PopStyleVar();

  if (!scroll_lock_ && scroll_to_bottom_) ImGui::SetScrollHere(1.0f);
  scroll_to_bottom_ = false;
  ImGui::EndChild();

  // The case of the log viewer in its own ImGui window (not docked)
  if (open) {
    ImGui::End();
  }
}

void ImGuiLogSink::_sink_it(const spdlog::details::log_msg &msg) {
  auto ostr = std::ostringstream();
  std::size_t color_range_start = 0;
  std::size_t color_range_end = 0;
  ImVec4 const *color = nullptr;
  auto emphasis = false;

  if (show_time_) {
    ostr << "[" << date::format("%D %T %Z", msg.time) << "] ";
  }
  if (show_thread_) {
    ostr << "[" << msg.thread_id << "] ";
  }
  if (show_level_) {
    color_range_start = static_cast<std::size_t>(ostr.tellp());
    ostr << "[" << spdlog::level::to_short_str(msg.level) << "] ";
    color_range_end = static_cast<std::size_t>(ostr.tellp());
  }
  if (show_logger_) {
    ostr << "[" << *msg.logger_name << "] ";
  }
  auto properties = ostr.str();

  // Strip the filename:line from the message and put it in a separate string
  auto msg_str = msg.raw.str();
  auto skip_to = msg_str.begin();
  if (*skip_to == '[') {
    // skip spaces
    while (skip_to != msg_str.end() && *skip_to != ']') ++skip_to;
    if (skip_to == msg_str.end()) {
      skip_to = msg_str.begin();
    } else {
      auto saved_skip_to = skip_to + 1;
      while (skip_to != msg_str.begin()) {
        --skip_to;
        if (*skip_to == ':' || !std::isdigit(*skip_to)) break;
      }
      if (*skip_to == ':') skip_to = ++saved_skip_to;  // Add one space
    }
  }
  // Ignore the '[' and ']'
  auto source = msg_str.substr(1, skip_to - msg_str.begin() - 3);

  // Select display color and colored text range based on level
  switch (msg.level) {
    case spdlog::level::trace:
      color = &ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
      // the entire message
      color_range_start = 0;
      color_range_end = 1;
      break;

    case spdlog::level::debug:
      color = &ImGui::GetStyleColorVec4(ImGuiCol_Text);
      // The level part if show, otherwise no coloring
      break;

    case spdlog::level::info:
      color = &ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight);
      // The level part if show, otherwise no coloring
      break;

    case spdlog::level::warn:
      color = &COLOR_WARN;
      // the entire message
      color_range_start = 0;
      color_range_end = 1;
      break;

    case spdlog::level::err:
      color = &COLOR_ERROR;
      // the entire message
      color_range_start = 0;
      color_range_end = 1;
      break;

    case spdlog::level::critical:
      color = &COLOR_ERROR;
      emphasis = true;
      // the entire message
      color_range_start = 0;
      color_range_end = 1;
      break;

    default:
        // Nothing
        ;
  }

  auto record = LogRecord{properties,
                          source,
                          msg_str.substr(skip_to - msg_str.begin()),
                          color_range_start,
                          color_range_end,
                          *color,
                          emphasis};
  {
    std::unique_lock<std::shared_timed_mutex> lock(records_mutex_);
    records_.push_back(std::move(record));
  }
  scroll_to_bottom_ = true;
}

void ImGuiLogSink::_flush() {
  // Your code here
}

}  // namespace ui
}  // namespace debug
}  // namespace asap
