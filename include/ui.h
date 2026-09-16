#pragma once

#include <3ds.h>

#include "app_state.h"

void ui_init(void);
void ui_exit(void);
void ui_render(const AppState* state, const Prediction* prediction, const char* tip, const char* status);
void ui_handle_touch(AppState* state, touchPosition touch);
