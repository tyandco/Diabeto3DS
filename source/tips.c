#include "tips.h"

typedef enum {
  TIP_GENERAL = 0,
  TIP_FOOD,
  TIP_ACTIVITY,
  TIP_SLEEP,
  TIP_WATER,
  TIP_GLUCOSE,
} TipKind;

typedef struct {
  TipKind kind;
  const char* text;
} Tip;

static const Tip TIPS[] = {
  {TIP_GENERAL, "Small steady habits beat one huge reset. Log one useful detail today."},
  {TIP_FOOD, "Pair carbs with protein or fiber to keep energy steadier."},
  {TIP_FOOD, "Try making water or unsweetened tea your default drink today."},
  {TIP_ACTIVITY, "A short walk after a meal can help your body use glucose."},
  {TIP_ACTIVITY, "If energy is low, start with ten easy minutes of movement."},
  {TIP_SLEEP, "Consistent sleep supports appetite, energy, and blood sugar balance."},
  {TIP_WATER, "Keep water visible. If you can see it, you are more likely to drink it."},
  {TIP_GLUCOSE, "A high glucose log is a signal to review food, sleep, and stress patterns."},
  {TIP_GENERAL, "Use trends, not one day, to judge progress."},
  {TIP_FOOD, "Build plates around vegetables, lean protein, and slow carbs."},
};

static int tip_matches(const Tip* tip, const AppState* state, const Prediction* prediction) {
  const DailyLog* log = &state->logs[state->currentLogIndex];

  switch (tip->kind) {
    case TIP_FOOD:
      return state->profile.sugar == SUGAR_OFTEN || log->balancedMeals < 2;
    case TIP_ACTIVITY:
      return state->profile.activity == ACTIVITY_LOW || log->activityMinutes < 25;
    case TIP_SLEEP:
      return log->sleepHours > 0 && log->sleepHours < 7;
    case TIP_WATER:
      return log->waterCups > 0 && log->waterCups < 6;
    case TIP_GLUCOSE:
      return state->profile.glucoseMgDl >= 100 || log->glucoseMgDl >= 100;
    case TIP_GENERAL:
    default:
      return prediction->score < 100;
  }
}

const char* pick_tip(const AppState* state, const Prediction* prediction) {
  const int count = (int)(sizeof(TIPS) / sizeof(TIPS[0]));

  for (int offset = 0; offset < count; offset++) {
    const int index = (state->tipIndex + offset) % count;

    if (tip_matches(&TIPS[index], state, prediction)) {
      return TIPS[index].text;
    }
  }

  return TIPS[0].text;
}

void advance_tip(AppState* state) {
  const int count = (int)(sizeof(TIPS) / sizeof(TIPS[0]));
  state->tipIndex = (state->tipIndex + 1) % count;
}
