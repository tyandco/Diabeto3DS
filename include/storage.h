#pragma once

#include <stdbool.h>

#include "app_state.h"

bool load_app_state(AppState* state);
bool save_app_state(const AppState* state);

