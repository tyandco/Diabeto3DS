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

static u32 mii_shirt_color(int color) {
  switch (color < 0 ? 0 : color % 12) {
    case 0: return C2D_Color32(0xD7, 0x45, 0x45, 0xFF);
    case 1: return C2D_Color32(0xFF, 0x7A, 0x59, 0xFF);
    case 2: return C2D_Color32(0xF1, 0xC2, 0x40, 0xFF);
    case 3: return C2D_Color32(0x45, 0xAD, 0x6A, 0xFF);
    case 4: return C2D_Color32(0x0F, 0x9F, 0x9A, 0xFF);
    case 5: return C2D_Color32(0x35, 0x79, 0xC9, 0xFF);
    case 6: return C2D_Color32(0x7E, 0x5A, 0xC7, 0xFF);
    case 7: return C2D_Color32(0xE5, 0x7B, 0xB8, 0xFF);
    case 8: return C2D_Color32(0x8D, 0x6A, 0x4E, 0xFF);
    case 9: return C2D_Color32(0x33, 0x38, 0x3F, 0xFF);
    case 10: return C2D_Color32(0xB9, 0xC2, 0xC7, 0xFF);
    default: return C2D_Color32(0xFF, 0xFF, 0xFA, 0xFF);
  }
}

static u32 mii_skin_color(int color) {
  switch (color < 0 ? 0 : color % 6) {
    case 0: return C2D_Color32(0xF6, 0xD3, 0xA5, 0xFF);
    case 1: return C2D_Color32(0xF0, 0xBD, 0x83, 0xFF);
    case 2: return C2D_Color32(0xD9, 0x98, 0x61, 0xFF);
    case 3: return C2D_Color32(0xB8, 0x73, 0x49, 0xFF);
    case 4: return C2D_Color32(0x8F, 0x58, 0x3B, 0xFF);
    default: return C2D_Color32(0x62, 0x3B, 0x2C, 0xFF);
  }
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

static void format_log_day(const AppState* state, char* buffer, size_t size) {
  if (state->currentLogIndex == 0) {
    snprintf(buffer, size, "Today");
    return;
  }

  if (state->currentLogIndex == 1) {
    snprintf(buffer, size, "Yesterday");
    return;
  }

  snprintf(buffer, size, "%d days ago", state->currentLogIndex);
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

static void draw_mii_badge(const DiabetoProfile* profile, float x, float y, float radius) {
  const u32 shirtColor = profile->hasMii ? mii_shirt_color(profile->miiShirtColor) : COLOR_PRIMARY;
  const u32 skinColor = profile->hasMii ? mii_skin_color(profile->miiSkinColor) : C2D_Color32(0xF2, 0xC9, 0x9B, 0xFF);

  C2D_DrawCircleSolid(x + radius * 0.08f, y + radius * 0.12f, 0.4f, radius * 1.03f, C2D_Color32(0xD8, 0xD3, 0xC7, 0x70));
  C2D_DrawCircleSolid(x, y, 0.6f, radius, shirtColor);
  C2D_DrawCircleSolid(x, y - radius * 0.05f, 0.7f, radius * 0.72f, skinColor);
  C2D_DrawCircleSolid(x - radius * 0.23f, y - radius * 0.18f, 0.8f, radius * 0.055f, COLOR_TEXT);
  C2D_DrawCircleSolid(x + radius * 0.23f, y - radius * 0.18f, 0.8f, radius * 0.055f, COLOR_TEXT);
  C2D_DrawRectSolid(x - radius * 0.21f, y + radius * 0.2f, 0.8f, radius * 0.42f, radius * 0.06f, COLOR_TEXT);
  C2D_DrawCircleSolid(x - radius * 0.21f, y + radius * 0.17f, 0.85f, radius * 0.075f, skinColor);
  C2D_DrawCircleSolid(x + radius * 0.21f, y + radius * 0.17f, 0.85f, radius * 0.075f, skinColor);
}

static void draw_top_dashboard(const AppState* state, const Prediction* prediction, const char* tip) {
  char buffer[96];
  const u32 riskColor = risk_color(prediction->level);
  const DiabetoProfile* profile = &state->profile;
  const DailyLog* log = &state->logs[state->currentLogIndex];
  char dayLabel[32];

  format_log_day(state, dayLabel, sizeof(dayLabel));

  draw_text("Diabeto", 18.0f, 14.0f, 0.95f, COLOR_TEXT);
  draw_text("3DS offline companion", 20.0f, 43.0f, 0.45f, COLOR_MUTED);
  if (profile->hasMii) {
    draw_mii_badge(profile, 358.0f, 35.0f, 20.0f);
    draw_text(profile->miiName, 306.0f, 59.0f, 0.38f, COLOR_MUTED);
  } else {
    draw_ribbon_face(358.0f, 35.0f);
  }

  draw_card(18.0f, 72.0f, 364.0f, 72.0f);
  draw_text("Risk prediction", 34.0f, 84.0f, 0.52f, COLOR_MUTED);
  snprintf(buffer, sizeof(buffer), "%s  %d/100", risk_label(prediction->level), prediction->score);
  draw_text(buffer, 34.0f, 103.0f, 0.72f, riskColor);
  snprintf(buffer, sizeof(buffer), "BMI %.1f", prediction->bmi);
  draw_text(buffer, 280.0f, 105.0f, 0.55f, COLOR_TEXT);
  draw_progress(34.0f, 130.0f, 316.0f, 7.0f, prediction->score, riskColor);

  draw_card(18.0f, 154.0f, 175.0f, 66.0f);
  draw_text(dayLabel, 34.0f, 164.0f, 0.52f, COLOR_MUTED);
  snprintf(buffer, sizeof(buffer), "%d mg/dL  %d min", log->glucoseMgDl, log->activityMinutes);
  draw_text(buffer, 34.0f, 184.0f, 0.52f, COLOR_TEXT);
  snprintf(buffer, sizeof(buffer), "%d h sleep  %d cups", log->sleepHours, log->waterCups);
  draw_text(buffer, 34.0f, 202.0f, 0.48f, COLOR_TEXT);

  draw_card(205.0f, 154.0f, 177.0f, 66.0f);
  draw_text("Ribbon tip", 221.0f, 164.0f, 0.52f, COLOR_MUTED);
  draw_text_wrap(tip, 221.0f, 183.0f, 0.42f, COLOR_TEXT, 145.0f);
}

static void draw_top_profile(const AppState* state, const Prediction* prediction) {
  char buffer[96];
  const u32 riskColor = risk_color(prediction->level);
  const DiabetoProfile* profile = &state->profile;

  draw_text("Risk Setup", 18.0f, 14.0f, 0.9f, COLOR_TEXT);
  draw_text("Link a Mii, then tune your risk factors.", 20.0f, 45.0f, 0.46f, COLOR_MUTED);

  draw_card(20.0f, 76.0f, 132.0f, 122.0f);
  draw_mii_badge(profile, 86.0f, 119.0f, 28.0f);
  draw_text(profile->hasMii ? profile->miiName : "No Mii linked", 38.0f, 158.0f, 0.48f, COLOR_TEXT);
  draw_text("Press A to choose", 39.0f, 178.0f, 0.38f, COLOR_MUTED);

  draw_card(166.0f, 76.0f, 214.0f, 122.0f);
  snprintf(buffer, sizeof(buffer), "%s risk", risk_label(prediction->level));
  draw_text(buffer, 184.0f, 92.0f, 0.64f, riskColor);
  snprintf(buffer, sizeof(buffer), "%d / 100", prediction->score);
  draw_text(buffer, 184.0f, 121.0f, 0.74f, COLOR_TEXT);
  snprintf(buffer, sizeof(buffer), "BMI %.1f", prediction->bmi);
  draw_text(buffer, 294.0f, 125.0f, 0.5f, COLOR_TEXT);
  draw_progress(184.0f, 155.0f, 164.0f, 7.0f, prediction->score, riskColor);

  draw_text("Bottom screen: Up/Down selects, Left/Right changes.", 34.0f, 212.0f, 0.43f, COLOR_MUTED);
}

static void draw_top_log(const AppState* state) {
  char buffer[80];
  const int selected = state->selectedField % 5;
  const DailyLog* log = &state->logs[state->currentLogIndex];
  char dayLabel[32];

  format_log_day(state, dayLabel, sizeof(dayLabel));

  draw_text("Daily Log", 18.0f, 14.0f, 0.9f, COLOR_TEXT);
  snprintf(buffer, sizeof(buffer), "%s  -  L/R switches days, A edits.", dayLabel);
  draw_text(buffer, 20.0f, 45.0f, 0.44f, COLOR_MUTED);

  draw_card(20.0f, 76.0f, 170.0f, 68.0f);
  draw_card(210.0f, 76.0f, 170.0f, 68.0f);
  draw_card(20.0f, 156.0f, 170.0f, 58.0f);
  draw_card(210.0f, 156.0f, 170.0f, 58.0f);

  snprintf(buffer, sizeof(buffer), "%d mg/dL", log->glucoseMgDl);
  draw_label_value(selected == 0 ? "> Glucose" : "Glucose", buffer, 38.0f, 92.0f);
  snprintf(buffer, sizeof(buffer), "%d min", log->activityMinutes);
  draw_label_value(selected == 1 ? "> Activity" : "Activity", buffer, 228.0f, 92.0f);
  snprintf(buffer, sizeof(buffer), "%d h", log->sleepHours);
  draw_label_value(selected == 2 ? "> Sleep" : "Sleep", buffer, 38.0f, 172.0f);
  snprintf(buffer, sizeof(buffer), "%d cups", log->waterCups);
  draw_label_value(selected == 3 ? "> Water" : "Water", buffer, 228.0f, 172.0f);
  snprintf(buffer, sizeof(buffer), "%d balanced meals", log->balancedMeals);
  draw_text(buffer, 38.0f, 132.0f, 0.4f, selected == 4 ? COLOR_PRIMARY_DARK : COLOR_MUTED);
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
  C2D_DrawRectSolid(18.0f, y, 0.25f, 284.0f, 20.0f, selected ? COLOR_CARD_ALT : COLOR_CARD);
  snprintf(line, sizeof(line), "%s  %s", label, value);
  draw_text(line, 27.0f, y + 4.0f, 0.4f, selected ? COLOR_PRIMARY_DARK : COLOR_TEXT);
}

static void draw_profile_fields(const AppState* state) {
  char value[32];
  const DiabetoProfile* profile = &state->profile;
  const int selected = state->selectedField % 8;

  draw_field(42.0f, "Mii", profile->hasMii ? profile->miiName : "Press A to choose", selected == 7);
  snprintf(value, sizeof(value), "%d years", profile->age);
  draw_field(65.0f, "Age", value, selected == 0);
  snprintf(value, sizeof(value), "%d cm", profile->heightCm);
  draw_field(88.0f, "Height", value, selected == 1);
  snprintf(value, sizeof(value), "%d kg", profile->weightKg);
  draw_field(111.0f, "Weight", value, selected == 2);
  snprintf(value, sizeof(value), "%d mg/dL", profile->glucoseMgDl);
  draw_field(134.0f, "Glucose", value, selected == 3);
  draw_field(157.0f, "Activity", activity_label(profile->activity), selected == 4);
  draw_field(180.0f, "Sugar", sugar_label(profile->sugar), selected == 5);
  draw_field(203.0f, "Family history", profile->familyHistory ? "Yes" : "No", selected == 6);
}

static void draw_log_fields(const AppState* state) {
  char value[32];
  const DailyLog* log = &state->logs[state->currentLogIndex];
  const int selected = state->selectedField % 5;
  char dayLabel[32];

  format_log_day(state, dayLabel, sizeof(dayLabel));
  draw_text(dayLabel, 224.0f, 13.0f, 0.42f, COLOR_PRIMARY_DARK);
  snprintf(value, sizeof(value), "%d mg/dL", log->glucoseMgDl);
  draw_text("Use this after meals, exercise, or before bed.", 18.0f, 43.0f, 0.37f, COLOR_MUTED);
  draw_field(66.0f, "Glucose", value, selected == 0);
  snprintf(value, sizeof(value), "%d min", log->activityMinutes);
  draw_field(92.0f, "Activity", value, selected == 1);
  snprintf(value, sizeof(value), "%d h", log->sleepHours);
  draw_field(118.0f, "Sleep", value, selected == 2);
  snprintf(value, sizeof(value), "%d cups", log->waterCups);
  draw_field(144.0f, "Water", value, selected == 3);
  snprintf(value, sizeof(value), "%d", log->balancedMeals);
  draw_field(170.0f, "Balanced meals", value, selected == 4);
  draw_text("Y saves this snapshot.", 23.0f, 197.0f, 0.37f, COLOR_MUTED);
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
      draw_top_profile(state, prediction);
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
