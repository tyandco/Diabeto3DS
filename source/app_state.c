#include "app_state.h"

void app_state_init(AppState* state) {
  state->version = DIABETO_SAVE_VERSION;
  state->profile.age = 16;
  state->profile.heightCm = 170;
  state->profile.weightKg = 65;
  state->profile.glucoseMgDl = 95;
  state->profile.activity = ACTIVITY_MEDIUM;
  state->profile.sugar = SUGAR_SOMETIMES;
  state->profile.familyHistory = false;
  state->profile.hasMii = false;
  state->profile.miiShirtColor = 0;
  state->profile.miiName[0] = '\0';

  for (int index = 0; index < DIABETO_LOG_DAYS; index++) {
    state->logs[index].glucoseMgDl = 95;
    state->logs[index].activityMinutes = 30;
    state->logs[index].sleepHours = 8;
    state->logs[index].waterCups = 6;
    state->logs[index].balancedMeals = 2;
    state->logs[index].streakDays = 0;
    state->logs[index].lastLogDay = index;
  }

  state->currentLogIndex = 0;
  state->selectedField = 0;
  state->tipIndex = 0;
  state->screen = SCREEN_DASHBOARD;
}

const char* activity_label(ActivityLevel level) {
  switch (level) {
    case ACTIVITY_LOW:
      return "Low";
    case ACTIVITY_HIGH:
      return "High";
    case ACTIVITY_MEDIUM:
    default:
      return "Medium";
  }
}

const char* sugar_label(SugarLevel level) {
  switch (level) {
    case SUGAR_RARE:
      return "Rare";
    case SUGAR_OFTEN:
      return "Often";
    case SUGAR_SOMETIMES:
    default:
      return "Sometimes";
  }
}

const char* risk_label(RiskLevel level) {
  switch (level) {
    case RISK_HIGH:
      return "High";
    case RISK_MODERATE:
      return "Moderate";
    case RISK_LOW:
    default:
      return "Low";
  }
}
