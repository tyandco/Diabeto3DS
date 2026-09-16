#include "ui.h"

#include <citro2d.h>
#include <stdbool.h>
#include <stdio.h>

static C3D_RenderTarget* topTarget;
static C3D_RenderTarget* bottomTarget;
static C2D_TextBuf textBuffer;

#define COLOR_BG C2D_Color32(0xF8, 0xF5, 0xED, 0xFF)
#define COLOR_BOTTOM_BG C2D_Color32(0xEF, 0xF8, 0xF6, 0xFF)
#define COLOR_CARD C2D_Color32(0xFF, 0xFF, 0xFA, 0xFF)
#define COLOR_CARD_ALT C2D_Color32(0xE0, 0xF3, 0xEF, 0xFF)
#define COLOR_TEXT C2D_Color32(0x13, 0x2B, 0x27, 0xFF)
#define COLOR_MUTED C2D_Color32(0x5A, 0x70, 0x6C, 0xFF)
#define COLOR_PRIMARY C2D_Color32(0x0F, 0x9F, 0x9A, 0xFF)
#define COLOR_PRIMARY_DARK C2D_Color32(0x07, 0x69, 0x67, 0xFF)
#define COLOR_ACCENT C2D_Color32(0xFF, 0x7A, 0x59, 0xFF)
#define COLOR_WARNING C2D_Color32(0xF1, 0x8B, 0x22, 0xFF)
#define COLOR_DANGER C2D_Color32(0xD7, 0x45, 0x45, 0xFF)
#define COLOR_WHITE C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF)

static u32 risk_color(RiskLevel level) {
  if (level == RISK_HIGH) return COLOR_DANGER;
  if (level == RISK_MODERATE) return COLOR_WARNING;
  return COLOR_PRIMARY;
}

static void draw_text(const char* value, float x, float y, float scale, u32 color) {
  C2D_Text text;
  C2D_TextParse(&text, textBuffer, value);
  C2D_TextOptimize(&text);
  C2D_DrawText(&text, C2D_WithColor, x, y, 0.5f, scale, scale, color);
}

static void draw_text_wrap(const char* value, float x, float y, float scale, u32 color, float width) {
  C2D_Text text;
  C2D_TextParse(&text, textBuffer, value);
  C2D_TextOptimize(&text);
  C2D_DrawText(&text, C2D_WithColor | C2D_WordWrap, x, y, 0.5f, scale, scale, color, width);
}

static void draw_label_value(const char* label, const char* value, float x, float y) {
  draw_text(label, x, y, 0.46f, COLOR_MUTED);
  draw_text(value, x, y + 15.0f, 0.58f, COLOR_TEXT);
}

static void draw_card(float x, float y, float w, float h) {
  C2D_DrawRectSolid(x + 2.0f, y + 3.0f, 0.1f, w, h, C2D_Color32(0xD8, 0xD3, 0xC7, 0x70));
  C2D_DrawRectSolid(x, y, 0.2f, w, h, COLOR_CARD);
}

static void draw_progress(float x, float y, float w, float h, int score, u32 color) {
  const float fill = (w * score) / 100.0f;
  C2D_DrawRectSolid(x, y, 0.3f, w, h, C2D_Color32(0xD8, 0xEA, 0xE6, 0xFF));
  C2D_DrawRectSolid(x, y, 0.4f, fill, h, color);
}

static void draw_ribbon_face(float x, float y) {
  C2D_DrawCircleSolid(x, y, 0.6f, 20.0f, COLOR_PRIMARY);
  C2D_DrawCircleSolid(x - 7.0f, y - 4.0f, 0.7f, 3.0f, COLOR_TEXT);
  C2D_DrawCircleSolid(x + 7.0f, y - 4.0f, 0.7f, 3.0f, COLOR_TEXT);
  C2D_DrawRectSolid(x - 8.0f, y + 7.0f, 0.7f, 16.0f, 3.0f, COLOR_TEXT);
  C2D_DrawCircleSolid(x - 15.0f, y + 13.0f, 0.5f, 5.0f, COLOR_ACCENT);
  C2D_DrawCircleSolid(x + 15.0f, y + 13.0f, 0.5f, 5.0f, COLOR_ACCENT);
}

static void draw_top_dashboard(const AppState* state, const Prediction* prediction, const char* tip) {
  char buffer[96];
  const u32 riskColor = risk_color(prediction->level);

  draw_text("Diabeto", 18.0f, 14.0f, 0.95f, COLOR_TEXT);
  draw_text("3DS offline companion", 20.0f, 43.0f, 0.45f, COLOR_MUTED);
  draw_ribbon_face(358.0f, 35.0f);

  draw_card(18.0f, 72.0f, 364.0f, 72.0f);
  draw_text("Risk prediction", 34.0f, 84.0f, 0.52f, COLOR_MUTED);
  snprintf(buffer, sizeof(buffer), "%s  %d/100", risk_label(prediction->level), prediction->score);
  draw_text(buffer, 34.0f, 103.0f, 0.72f, riskColor);
  snprintf(buffer, sizeof(buffer), "BMI %.1f", prediction->bmi);
  draw_text(buffer, 280.0f, 105.0f, 0.55f, COLOR_TEXT);
  draw_progress(34.0f, 130.0f, 316.0f, 7.0f, prediction->score, riskColor);

  draw_card(18.0f, 154.0f, 175.0f, 66.0f);
  draw_text("Today", 34.0f, 164.0f, 0.52f, COLOR_MUTED);
  snprintf(buffer, sizeof(buffer), "%d mg/dL  %d min", state->log.glucoseMgDl, state->log.activityMinutes);
  draw_text(buffer, 34.0f, 184.0f, 0.52f, COLOR_TEXT);
  snprintf(buffer, sizeof(buffer), "%d h sleep  %d cups", state->log.sleepHours, state->log.waterCups);
  draw_text(buffer, 34.0f, 202.0f, 0.48f, COLOR_TEXT);

  draw_card(205.0f, 154.0f, 177.0f, 66.0f);
  draw_text("Ribbon tip", 221.0f, 164.0f, 0.52f, COLOR_MUTED);
  draw_text_wrap(tip, 221.0f, 183.0f, 0.42f, COLOR_TEXT, 145.0f);
}

static void draw_top_profile(const Prediction* prediction) {
  char buffer[96];
  const u32 riskColor = risk_color(prediction->level);

  draw_text("Risk Setup", 18.0f, 14.0f, 0.9f, COLOR_TEXT);
  draw_text("Tune profile values with the D-Pad.", 20.0f, 45.0f, 0.46f, COLOR_MUTED);

  draw_card(24.0f, 76.0f, 352.0f, 92.0f);
  snprintf(buffer, sizeof(buffer), "%s risk", risk_label(prediction->level));
  draw_text(buffer, 42.0f, 92.0f, 0.72f, riskColor);
  snprintf(buffer, sizeof(buffer), "%d / 100", prediction->score);
  draw_text(buffer, 42.0f, 122.0f, 0.72f, COLOR_TEXT);
  snprintf(buffer, sizeof(buffer), "BMI %.1f", prediction->bmi);
  draw_text(buffer, 262.0f, 122.0f, 0.58f, COLOR_TEXT);
  draw_progress(42.0f, 150.0f, 292.0f, 7.0f, prediction->score, riskColor);

  draw_text("Up/Down selects. Left/Right changes.", 34.0f, 194.0f, 0.47f, COLOR_MUTED);
}

static void draw_top_log(const AppState* state) {
  char buffer[80];

  draw_text("Daily Log", 18.0f, 14.0f, 0.9f, COLOR_TEXT);
  draw_text("Today's health snapshot stays on your SD card.", 20.0f, 45.0f, 0.44f, COLOR_MUTED);
  draw_card(28.0f, 80.0f, 344.0f, 112.0f);

  snprintf(buffer, sizeof(buffer), "%d mg/dL", state->log.glucoseMgDl);
  draw_label_value("Glucose", buffer, 50.0f, 98.0f);
  snprintf(buffer, sizeof(buffer), "%d min", state->log.activityMinutes);
  draw_label_value("Activity", buffer, 190.0f, 98.0f);
  snprintf(buffer, sizeof(buffer), "%d h", state->log.sleepHours);
  draw_label_value("Sleep", buffer, 50.0f, 146.0f);
  snprintf(buffer, sizeof(buffer), "%d cups", state->log.waterCups);
  draw_label_value("Water", buffer, 190.0f, 146.0f);
}

static void draw_top_tips(const char* tip) {
  draw_text("Ribbon's Tips", 18.0f, 14.0f, 0.9f, COLOR_TEXT);
  draw_text("Offline suggestions based on your logs.", 20.0f, 45.0f, 0.45f, COLOR_MUTED);
  draw_card(28.0f, 82.0f, 344.0f, 110.0f);
  draw_ribbon_face(62.0f, 118.0f);
  draw_text_wrap(tip, 94.0f, 100.0f, 0.52f, COLOR_TEXT, 245.0f);
  draw_text("Press A for another tip.", 82.0f, 204.0f, 0.48f, COLOR_MUTED);
}

static void draw_button(float x, float y, float w, const char* label, bool active) {
  C2D_DrawRectSolid(x, y, 0.3f, w, 30.0f, active ? COLOR_PRIMARY : COLOR_CARD);
  draw_text(label, x + 10.0f, y + 8.0f, 0.43f, active ? COLOR_WHITE : COLOR_TEXT);
}

static void draw_field(float y, const char* label, const char* value, bool selected) {
  char line[80];
  C2D_DrawRectSolid(18.0f, y, 0.25f, 284.0f, 22.0f, selected ? COLOR_CARD_ALT : COLOR_CARD);
  snprintf(line, sizeof(line), "%s  %s", label, value);
  draw_text(line, 27.0f, y + 5.0f, 0.43f, selected ? COLOR_PRIMARY_DARK : COLOR_TEXT);
}

static void draw_profile_fields(const AppState* state) {
  char value[32];
  const DiabetoProfile* profile = &state->profile;
  const int selected = state->selectedField % 7;

  snprintf(value, sizeof(value), "%d", profile->age);
  draw_field(43.0f, "Age", value, selected == 0);
  snprintf(value, sizeof(value), "%d cm", profile->heightCm);
  draw_field(68.0f, "Height", value, selected == 1);
  snprintf(value, sizeof(value), "%d kg", profile->weightKg);
  draw_field(93.0f, "Weight", value, selected == 2);
  snprintf(value, sizeof(value), "%d mg/dL", profile->glucoseMgDl);
  draw_field(118.0f, "Glucose", value, selected == 3);
  draw_field(143.0f, "Activity", activity_label(profile->activity), selected == 4);
  draw_field(168.0f, "Sugar", sugar_label(profile->sugar), selected == 5);
  draw_field(193.0f, "Family", profile->familyHistory ? "Yes" : "No", selected == 6);
}

static void draw_log_fields(const AppState* state) {
  char value[32];
  const DailyLog* log = &state->log;
  const int selected = state->selectedField % 5;

  snprintf(value, sizeof(value), "%d mg/dL", log->glucoseMgDl);
  draw_field(55.0f, "Glucose", value, selected == 0);
  snprintf(value, sizeof(value), "%d min", log->activityMinutes);
  draw_field(84.0f, "Activity", value, selected == 1);
  snprintf(value, sizeof(value), "%d h", log->sleepHours);
  draw_field(113.0f, "Sleep", value, selected == 2);
  snprintf(value, sizeof(value), "%d cups", log->waterCups);
  draw_field(142.0f, "Water", value, selected == 3);
  snprintf(value, sizeof(value), "%d", log->balancedMeals);
  draw_field(171.0f, "Meals", value, selected == 4);
}

static void draw_bottom(const AppState* state, const char* status) {
  const char* title = "Dashboard";

  if (state->screen == SCREEN_LOG) title = "Log";
  if (state->screen == SCREEN_PROFILE) title = "Risk Setup";
  if (state->screen == SCREEN_TIPS) title = "Tips";

  draw_text(title, 18.0f, 12.0f, 0.62f, COLOR_TEXT);
  if (status && status[0]) {
    draw_text(status, 18.0f, 31.0f, 0.38f, COLOR_MUTED);
  }

  if (state->screen == SCREEN_PROFILE) {
    draw_profile_fields(state);
  } else if (state->screen == SCREEN_LOG) {
    draw_log_fields(state);
  } else {
    draw_card(18.0f, 62.0f, 284.0f, 92.0f);
    draw_text("A opens Log. X opens Risk Setup.", 35.0f, 82.0f, 0.43f, COLOR_TEXT);
    draw_text("Y saves. START exits.", 35.0f, 104.0f, 0.43f, COLOR_TEXT);
    draw_text("Touch the tabs below to switch.", 35.0f, 126.0f, 0.43f, COLOR_MUTED);
  }

  draw_button(10.0f, 204.0f, 72.0f, "Home", state->screen == SCREEN_DASHBOARD);
  draw_button(86.0f, 204.0f, 72.0f, "Log", state->screen == SCREEN_LOG);
  draw_button(162.0f, 204.0f, 72.0f, "Risk", state->screen == SCREEN_PROFILE);
  draw_button(238.0f, 204.0f, 72.0f, "Tips", state->screen == SCREEN_TIPS);
}

void ui_init(void) {
  C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
  C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
  C2D_Prepare();

  topTarget = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
  bottomTarget = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
  textBuffer = C2D_TextBufNew(4096);
}

void ui_exit(void) {
  C2D_TextBufDelete(textBuffer);
  C2D_Fini();
  C3D_Fini();
}

void ui_render(const AppState* state, const Prediction* prediction, const char* tip, const char* status) {
  C2D_TextBufClear(textBuffer);

  C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

  C2D_TargetClear(topTarget, COLOR_BG);
  C2D_SceneBegin(topTarget);

  switch (state->screen) {
    case SCREEN_PROFILE:
      draw_top_profile(prediction);
      break;
    case SCREEN_LOG:
      draw_top_log(state);
      break;
    case SCREEN_TIPS:
      draw_top_tips(tip);
      break;
    case SCREEN_DASHBOARD:
    default:
      draw_top_dashboard(state, prediction, tip);
      break;
  }

  C2D_TargetClear(bottomTarget, COLOR_BOTTOM_BG);
  C2D_SceneBegin(bottomTarget);
  draw_bottom(state, status);

  C3D_FrameEnd(0);
}

void ui_handle_touch(AppState* state, touchPosition touch) {
  if (touch.py < 204) {
    return;
  }

  if (touch.px < 82) {
    state->screen = SCREEN_DASHBOARD;
  } else if (touch.px < 160) {
    state->screen = SCREEN_LOG;
  } else if (touch.px < 238) {
    state->screen = SCREEN_PROFILE;
  } else {
    state->screen = SCREEN_TIPS;
  }

  state->selectedField = 0;
}
