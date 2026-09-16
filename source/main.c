#include <3ds.h>
#include <stdio.h>

#include "app_state.h"
#include "risk.h"
#include "storage.h"
#include "tips.h"
#include "ui.h"

static void clamp_profile(DiabetoProfile* profile) {
  if (profile->age < 1) profile->age = 1;
  if (profile->age > 120) profile->age = 120;
  if (profile->heightCm < 80) profile->heightCm = 80;
  if (profile->heightCm > 230) profile->heightCm = 230;
  if (profile->weightKg < 20) profile->weightKg = 20;
  if (profile->weightKg > 250) profile->weightKg = 250;
  if (profile->glucoseMgDl < 50) profile->glucoseMgDl = 50;
  if (profile->glucoseMgDl > 300) profile->glucoseMgDl = 300;
}

static void clamp_log(DailyLog* log) {
  if (log->glucoseMgDl < 50) log->glucoseMgDl = 50;
  if (log->glucoseMgDl > 300) log->glucoseMgDl = 300;
  if (log->activityMinutes < 0) log->activityMinutes = 0;
  if (log->activityMinutes > 240) log->activityMinutes = 240;
  if (log->sleepHours < 0) log->sleepHours = 0;
  if (log->sleepHours > 16) log->sleepHours = 16;
  if (log->waterCups < 0) log->waterCups = 0;
  if (log->waterCups > 20) log->waterCups = 20;
  if (log->balancedMeals < 0) log->balancedMeals = 0;
  if (log->balancedMeals > 6) log->balancedMeals = 6;
}

static void adjust_profile(DiabetoProfile* profile, int field, int delta) {
  switch (field % 7) {
    case 0:
      profile->age += delta;
      break;
    case 1:
      profile->heightCm += delta;
      break;
    case 2:
      profile->weightKg += delta;
      break;
    case 3:
      profile->glucoseMgDl += delta;
      break;
    case 4:
      profile->activity = (ActivityLevel)((profile->activity + delta + 3) % 3);
      break;
    case 5:
      profile->sugar = (SugarLevel)((profile->sugar + delta + 3) % 3);
      break;
    case 6:
      if (delta != 0) profile->familyHistory = !profile->familyHistory;
      break;
  }

  clamp_profile(profile);
}

static void adjust_log(DailyLog* log, int field, int delta) {
  switch (field % 5) {
    case 0:
      log->glucoseMgDl += delta;
      break;
    case 1:
      log->activityMinutes += delta * 5;
      break;
    case 2:
      log->sleepHours += delta;
      break;
    case 3:
      log->waterCups += delta;
      break;
    case 4:
      log->balancedMeals += delta;
      break;
  }

  clamp_log(log);
}

static void handle_field_input(AppState* state, u32 down) {
  int delta = 0;

  if (down & KEY_UP) state->selectedField--;
  if (down & KEY_DOWN) state->selectedField++;
  if (down & KEY_LEFT) delta = -1;
  if (down & KEY_RIGHT) delta = 1;

  if (state->screen == SCREEN_PROFILE) {
    if (state->selectedField < 0) state->selectedField = 6;
    if (state->selectedField > 6) state->selectedField = 0;
    if (delta != 0) adjust_profile(&state->profile, state->selectedField, delta);
  } else if (state->screen == SCREEN_LOG) {
    if (state->selectedField < 0) state->selectedField = 4;
    if (state->selectedField > 4) state->selectedField = 0;
    if (delta != 0) adjust_log(&state->log, state->selectedField, delta);
  }
}

int main(void) {
  gfxInitDefault();
  ui_init();

  AppState state;
  app_state_init(&state);

  char status[64] = "New save started.";
  if (load_app_state(&state)) {
    snprintf(status, sizeof(status), "Save loaded.");
  }

  while (aptMainLoop()) {
    hidScanInput();
    const u32 down = hidKeysDown();

    if (down & KEY_START) {
      break;
    }

    if (down & KEY_A) {
      if (state.screen == SCREEN_TIPS) {
        advance_tip(&state);
      } else {
        state.screen = SCREEN_LOG;
        state.selectedField = 0;
      }
    }

    if (down & KEY_X) {
      state.screen = SCREEN_PROFILE;
      state.selectedField = 0;
    }

    if (down & KEY_B) {
      state.screen = SCREEN_DASHBOARD;
      state.selectedField = 0;
    }

    if (down & KEY_Y) {
      if (save_app_state(&state)) {
        snprintf(status, sizeof(status), "Saved to SD.");
      } else {
        snprintf(status, sizeof(status), "Save failed.");
      }
    }

    if (down & KEY_TOUCH) {
      touchPosition touch;
      hidTouchRead(&touch);
      ui_handle_touch(&state, touch);
    }

    handle_field_input(&state, down);

    Prediction prediction = calculate_prediction(&state.profile);
    const char* tip = pick_tip(&state, &prediction);
    ui_render(&state, &prediction, tip, status);

    gfxFlushBuffers();
    gfxSwapBuffers();
    gspWaitForVBlank();
  }

  gfxExit();
  return 0;
}
