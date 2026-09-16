#include "storage.h"

#include <stdio.h>
#include <sys/stat.h>

static const char* SAVE_DIR = "sdmc:/3ds/Diabeto3DS";
static const char* SAVE_PATH = "sdmc:/3ds/Diabeto3DS/save.bin";

bool load_app_state(AppState* state) {
  FILE* file = fopen(SAVE_PATH, "rb");

  if (!file) {
    return false;
  }

  AppState loaded;
  const size_t readCount = fread(&loaded, sizeof(AppState), 1, file);
  fclose(file);

  if (readCount != 1 || loaded.version != DIABETO_SAVE_VERSION) {
    return false;
  }

  *state = loaded;
  state->screen = SCREEN_DASHBOARD;
  state->selectedField = 0;
  return true;
}

bool save_app_state(const AppState* state) {
  mkdir("sdmc:/3ds", 0777);
  mkdir(SAVE_DIR, 0777);

  FILE* file = fopen(SAVE_PATH, "wb");

  if (!file) {
    return false;
  }

  AppState saved = *state;
  saved.version = DIABETO_SAVE_VERSION;
  saved.screen = SCREEN_DASHBOARD;
  saved.selectedField = 0;

  const size_t writeCount = fwrite(&saved, sizeof(AppState), 1, file);
  fclose(file);

  return writeCount == 1;
}

