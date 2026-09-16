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
  state->profile.miiSkinColor = 0;
  state->profile.miiFaceShape = 0;
  state->profile.miiHairStyle = 0;
  state->profile.miiHairColor = 0;
  state->profile.miiEyeStyle = 0;
  state->profile.miiEyeColor = 0;
  state->profile.miiEyeScale = 4;
  state->profile.miiEyeYScale = 3;
  state->profile.miiEyeSpacing = 3;
  state->profile.miiEyeYPosition = 12;
  state->profile.miiEyebrowStyle = 0;
  state->profile.miiEyebrowColor = 0;
  state->profile.miiEyebrowSpacing = 3;
  state->profile.miiEyebrowYPosition = 10;
  state->profile.miiNoseStyle = 0;
  state->profile.miiNoseScale = 4;
  state->profile.miiNoseYPosition = 10;
  state->profile.miiMouthStyle = 0;
  state->profile.miiMouthColor = 0;
  state->profile.miiMouthScale = 4;
  state->profile.miiMouthYScale = 3;
  state->profile.miiMustacheStyle = 0;
  state->profile.miiBeardStyle = 0;
  state->profile.miiBeardColor = 0;
  state->profile.miiGlassesStyle = 0;
  state->profile.miiGlassesColor = 0;
  state->profile.miiGlassesScale = 4;
  state->profile.miiGlassesYPosition = 10;
  state->profile.miiMoleEnabled = false;
  state->profile.miiMoleScale = 0;
  state->profile.miiMoleXPosition = 0;
  state->profile.miiMoleYPosition = 0;
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
