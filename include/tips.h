#pragma once

#include "app_state.h"

const char* pick_tip(const AppState* state, const Prediction* prediction);
void advance_tip(AppState* state);

