#pragma once

#include <stdbool.h>
#include <stdint.h>

#define DIABETO_SAVE_VERSION 1

typedef enum {
  SCREEN_DASHBOARD = 0,
  SCREEN_PROFILE,
  SCREEN_LOG,
  SCREEN_TIPS,
} Screen;

typedef enum {
  ACTIVITY_LOW = 0,
  ACTIVITY_MEDIUM,
  ACTIVITY_HIGH,
} ActivityLevel;

typedef enum {
  SUGAR_RARE = 0,
  SUGAR_SOMETIMES,
  SUGAR_OFTEN,
} SugarLevel;

typedef enum {
  RISK_LOW = 0,
  RISK_MODERATE,
  RISK_HIGH,
} RiskLevel;

typedef struct {
  int age;
  int heightCm;
  int weightKg;
  int glucoseMgDl;
  ActivityLevel activity;
  SugarLevel sugar;
  bool familyHistory;
} DiabetoProfile;

typedef struct {
  int glucoseMgDl;
  int activityMinutes;
  int sleepHours;
  int waterCups;
  int balancedMeals;
  int streakDays;
  uint32_t lastLogDay;
} DailyLog;

typedef struct {
  int score;
  float bmi;
  RiskLevel level;
} Prediction;

typedef struct {
  uint32_t version;
  DiabetoProfile profile;
  DailyLog log;
  int selectedField;
  int tipIndex;
  Screen screen;
} AppState;

void app_state_init(AppState* state);
const char* activity_label(ActivityLevel level);
const char* sugar_label(SugarLevel level);
const char* risk_label(RiskLevel level);
