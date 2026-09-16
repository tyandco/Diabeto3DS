#include "risk.h"

static int clamp_int(int value, int min, int max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

Prediction calculate_prediction(const Profile* profile) {
  Prediction prediction;
  const float heightMeters = profile->heightCm > 0 ? profile->heightCm / 100.0f : 1.70f;
  prediction.bmi = profile->weightKg / (heightMeters * heightMeters);

  int score = 0;

  if (profile->age >= 45) score += 18;
  else if (profile->age >= 30) score += 10;
  else if (profile->age >= 18) score += 4;

  if (prediction.bmi >= 30.0f) score += 24;
  else if (prediction.bmi >= 25.0f) score += 15;
  else if (prediction.bmi < 18.5f) score += 4;

  if (profile->glucoseMgDl >= 126) score += 28;
  else if (profile->glucoseMgDl >= 100) score += 16;

  if (profile->familyHistory) score += 12;
  if (profile->activity == ACTIVITY_LOW) score += 14;
  else if (profile->activity == ACTIVITY_HIGH) score -= 6;

  if (profile->sugar == SUGAR_OFTEN) score += 12;
  else if (profile->sugar == SUGAR_RARE) score -= 4;

  prediction.score = clamp_int(score, 1, 100);

  if (prediction.score >= 60) {
    prediction.level = RISK_HIGH;
  } else if (prediction.score >= 30) {
    prediction.level = RISK_MODERATE;
  } else {
    prediction.level = RISK_LOW;
  }

  return prediction;
}

