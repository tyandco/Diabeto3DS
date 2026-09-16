#include <3ds.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
  switch (field) {
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

static void select_profile_mii(AppState* state, char* status, size_t statusSize) {
  MiiSelectorConf conf;
  MiiSelectorReturn result;

  miiSelectorInit(&conf);
  miiSelectorSetTitle(&conf, "Choose your Diabeto Mii");
  miiSelectorSetOptions(&conf, MIISELECTOR_CANCEL | MIISELECTOR_GUESTS | MIISELECTOR_TOP);
  miiSelectorLaunch(&conf, &result);

  if (!miiSelectorChecksumIsValid(&result)) {
    snprintf(status, statusSize, "Mii selection failed.");
    return;
  }

  if (result.no_mii_selected) {
    snprintf(status, statusSize, "No Mii selected.");
    return;
  }

  miiSelectorReturnGetName(&result, state->profile.miiName, sizeof(state->profile.miiName));
  state->profile.hasMii = true;
  state->profile.miiShirtColor = result.mii.mii_details.shirt_color;
  snprintf(status, statusSize, "Mii linked: %s", state->profile.miiName);
}

static DailyLog* current_log(AppState* state) {
  return &state->logs[state->currentLogIndex];
}

static int prompt_number(const char* hint, int currentValue, int minValue, int maxValue, bool* changed) {
  SwkbdState keyboard;
  char buffer[16];

  snprintf(buffer, sizeof(buffer), "%d", currentValue);
  swkbdInit(&keyboard, SWKBD_TYPE_NUMPAD, 2, 6);
  swkbdSetInitialText(&keyboard, buffer);
  swkbdSetHintText(&keyboard, hint);
  swkbdSetValidation(&keyboard, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
  swkbdSetButton(&keyboard, SWKBD_BUTTON_LEFT, "Cancel", false);
  swkbdSetButton(&keyboard, SWKBD_BUTTON_RIGHT, "OK", true);

  *changed = false;
  const SwkbdButton button = swkbdInputText(&keyboard, buffer, sizeof(buffer));

  if (button != SWKBD_BUTTON_RIGHT) {
    return currentValue;
  }

  int value = atoi(buffer);
  if (value < minValue) value = minValue;
  if (value > maxValue) value = maxValue;

  *changed = true;
  return value;
}

static void edit_profile_field(AppState* state, char* status, size_t statusSize) {
  bool changed = false;
  DiabetoProfile* profile = &state->profile;

  switch (state->selectedField) {
    case 0:
      profile->age = prompt_number("Age in years", profile->age, 1, 120, &changed);
      break;
    case 1:
      profile->heightCm = prompt_number("Height in centimeters", profile->heightCm, 80, 230, &changed);
      break;
    case 2:
      profile->weightKg = prompt_number("Weight in kilograms", profile->weightKg, 20, 250, &changed);
      break;
    case 3:
      profile->glucoseMgDl = prompt_number("Glucose mg/dL", profile->glucoseMgDl, 50, 300, &changed);
      break;
    case 4:
    case 5:
    case 6:
      adjust_profile(profile, state->selectedField, 1);
      changed = true;
      break;
    case 7:
      select_profile_mii(state, status, statusSize);
      return;
  }

  if (changed) {
    snprintf(status, statusSize, "Profile updated.");
  }
}

static void edit_log_field(AppState* state, char* status, size_t statusSize) {
  bool changed = false;
  DailyLog* log = current_log(state);

  switch (state->selectedField) {
    case 0:
      log->glucoseMgDl = prompt_number("Glucose mg/dL", log->glucoseMgDl, 50, 300, &changed);
      break;
    case 1:
      log->activityMinutes = prompt_number("Activity minutes", log->activityMinutes, 0, 240, &changed);
      break;
    case 2:
      log->sleepHours = prompt_number("Sleep hours", log->sleepHours, 0, 16, &changed);
      break;
    case 3:
      log->waterCups = prompt_number("Water cups", log->waterCups, 0, 20, &changed);
      break;
    case 4:
      log->balancedMeals = prompt_number("Balanced meals", log->balancedMeals, 0, 6, &changed);
      break;
  }

  if (changed) {
    snprintf(status, statusSize, "Log updated.");
  }
}

static void change_log_day(AppState* state, int delta, char* status, size_t statusSize) {
  state->currentLogIndex += delta;

  if (state->currentLogIndex < 0) {
    state->currentLogIndex = 0;
  }

  if (state->currentLogIndex >= DIABETO_LOG_DAYS) {
    state->currentLogIndex = DIABETO_LOG_DAYS - 1;
  }

  if (state->currentLogIndex == 0) {
    snprintf(status, statusSize, "Editing today.");
  } else {
    snprintf(status, statusSize, "Editing %d day%s ago.", state->currentLogIndex, state->currentLogIndex == 1 ? "" : "s");
  }
}

static void handle_field_input(AppState* state, u32 down) {
  int delta = 0;

  if (down & KEY_UP) state->selectedField--;
  if (down & KEY_DOWN) state->selectedField++;
  if (down & KEY_LEFT) delta = -1;
  if (down & KEY_RIGHT) delta = 1;

  if (state->screen == SCREEN_PROFILE) {
    if (state->selectedField < 0) state->selectedField = 7;
    if (state->selectedField > 7) state->selectedField = 0;
    if (delta != 0 && state->selectedField < 7) adjust_profile(&state->profile, state->selectedField, delta);
  } else if (state->screen == SCREEN_LOG) {
    if (state->selectedField < 0) state->selectedField = 4;
    if (state->selectedField > 4) state->selectedField = 0;
    if (delta != 0) adjust_log(current_log(state), state->selectedField, delta);
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
      } else if (state.screen == SCREEN_PROFILE) {
        edit_profile_field(&state, status, sizeof(status));
      } else if (state.screen == SCREEN_LOG) {
        edit_log_field(&state, status, sizeof(status));
      } else {
        state.screen = SCREEN_LOG;
        state.selectedField = 0;
      }
    }

    if (state.screen == SCREEN_LOG && (down & KEY_L)) {
      change_log_day(&state, 1, status, sizeof(status));
    }

    if (state.screen == SCREEN_LOG && (down & KEY_R)) {
      change_log_day(&state, -1, status, sizeof(status));
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
  }

  ui_exit();
  gfxExit();
  return 0;
}
