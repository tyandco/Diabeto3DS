#include "ui.h"

#include <stdio.h>
#include <string.h>

static PrintConsole topScreen;
static PrintConsole bottomScreen;

static void print_wrapped_tip(const char* tip, int y) {
  const size_t length = strlen(tip);

  printf("\x1b[%d;1H%.38s", y, tip);
  if (length > 38) {
    printf("\x1b[%d;1H%.38s", y + 1, tip + 38);
  }
}

static void draw_top_dashboard(const AppState* state, const Prediction* prediction, const char* tip) {
  consoleSelect(&topScreen);
  consoleClear();
  printf("\x1b[1;1HDiabeto 3DS");
  printf("\x1b[3;1HRisk: %s  %d/100", risk_label(prediction->level), prediction->score);
  printf("\x1b[4;1HBMI: %.1f", prediction->bmi);
  printf("\x1b[6;1HStreak: %d days", state->log.streakDays);
  printf("\x1b[8;1HToday");
  printf("\x1b[9;1HGlucose %d mg/dL", state->log.glucoseMgDl);
  printf("\x1b[10;1HMove %d min  Sleep %d h", state->log.activityMinutes, state->log.sleepHours);
  printf("\x1b[12;1HTip:");
  print_wrapped_tip(tip, 13);
}

static void draw_top_profile(const Prediction* prediction) {
  consoleSelect(&topScreen);
  consoleClear();
  printf("\x1b[1;1HProfile / Risk");
  printf("\x1b[3;1HScore: %s %d/100", risk_label(prediction->level), prediction->score);
  printf("\x1b[4;1HBMI: %.1f", prediction->bmi);
  printf("\x1b[6;1HUse Up/Down to select.");
  printf("\x1b[7;1HUse Left/Right to change.");
}

static void draw_top_log(void) {
  consoleSelect(&topScreen);
  consoleClear();
  printf("\x1b[1;1HDaily Log");
  printf("\x1b[3;1HLog today's snapshot.");
  printf("\x1b[5;1HY saves. B returns.");
  printf("\x1b[7;1HThis stays offline on SD.");
}

static void draw_top_tips(const char* tip) {
  consoleSelect(&topScreen);
  consoleClear();
  printf("\x1b[1;1HRibbon's Offline Tip");
  print_wrapped_tip(tip, 4);
  printf("\x1b[9;1HPress A for another tip.");
}

static void draw_bottom_nav(const char* status) {
  printf("\x1b[26;1H[A] Log  [X] Profile  [Y] Save");
  printf("\x1b[28;1H[Touch] Dashboard Log Risk Tips");

  if (status && status[0]) {
    printf("\x1b[2;1H%s", status);
  }
}

static void draw_profile_fields(const AppState* state) {
  const Profile* profile = &state->profile;
  const char* cursor[7] = {" ", " ", " ", " ", " ", " ", " "};
  cursor[state->selectedField % 7] = ">";

  printf("\x1b[4;1H%s Age: %d", cursor[0], profile->age);
  printf("\x1b[6;1H%s Height: %d cm", cursor[1], profile->heightCm);
  printf("\x1b[8;1H%s Weight: %d kg", cursor[2], profile->weightKg);
  printf("\x1b[10;1H%s Glucose: %d mg/dL", cursor[3], profile->glucoseMgDl);
  printf("\x1b[12;1H%s Activity: %s", cursor[4], activity_label(profile->activity));
  printf("\x1b[14;1H%s Sugar: %s", cursor[5], sugar_label(profile->sugar));
  printf("\x1b[16;1H%s Family history: %s", cursor[6], profile->familyHistory ? "Yes" : "No");
}

static void draw_log_fields(const AppState* state) {
  const DailyLog* log = &state->log;
  const char* cursor[5] = {" ", " ", " ", " ", " "};
  cursor[state->selectedField % 5] = ">";

  printf("\x1b[4;1H%s Glucose: %d mg/dL", cursor[0], log->glucoseMgDl);
  printf("\x1b[6;1H%s Activity: %d min", cursor[1], log->activityMinutes);
  printf("\x1b[8;1H%s Sleep: %d h", cursor[2], log->sleepHours);
  printf("\x1b[10;1H%s Water: %d cups", cursor[3], log->waterCups);
  printf("\x1b[12;1H%s Balanced meals: %d", cursor[4], log->balancedMeals);
}

void ui_init(void) {
  consoleInit(GFX_TOP, &topScreen);
  consoleInit(GFX_BOTTOM, &bottomScreen);
}

void ui_render(const AppState* state, const Prediction* prediction, const char* tip, const char* status) {
  switch (state->screen) {
    case SCREEN_PROFILE:
      draw_top_profile(prediction);
      break;
    case SCREEN_LOG:
      draw_top_log();
      break;
    case SCREEN_TIPS:
      draw_top_tips(tip);
      break;
    case SCREEN_DASHBOARD:
    default:
      draw_top_dashboard(state, prediction, tip);
      break;
  }

  consoleSelect(&bottomScreen);
  consoleClear();
  printf("\x1b[1;1H%s", state->screen == SCREEN_DASHBOARD ? "Dashboard" : state->screen == SCREEN_LOG ? "Log" : state->screen == SCREEN_PROFILE ? "Risk Setup" : "Tips");

  if (state->screen == SCREEN_PROFILE) {
    draw_profile_fields(state);
  } else if (state->screen == SCREEN_LOG) {
    draw_log_fields(state);
  } else {
    printf("\x1b[4;1HTap a button below or use");
    printf("\x1b[5;1Hthe face buttons.");
  }

  draw_bottom_nav(status);
}

void ui_handle_touch(AppState* state, touchPosition touch) {
  if (touch.py < 205) {
    return;
  }

  if (touch.px < 80) {
    state->screen = SCREEN_DASHBOARD;
  } else if (touch.px < 160) {
    state->screen = SCREEN_LOG;
  } else if (touch.px < 240) {
    state->screen = SCREEN_PROFILE;
  } else {
    state->screen = SCREEN_TIPS;
  }

  state->selectedField = 0;
}
