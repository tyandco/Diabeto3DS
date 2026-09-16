#include "storage.h"

#include <stdio.h>
#include <sys/stat.h>

static const char* SAVE_DIR = "sdmc:/3ds/Diabeto3DS";
static const char* SAVE_PATH = "sdmc:/3ds/Diabeto3DS/save.bin";

typedef struct {
  uint32_t version;
  DiabetoProfile profile;
  DailyLog logs[DIABETO_LOG_DAYS];
  int currentLogIndex;
  int selectedField;
  int tipIndex;
  Screen screen;
} StoredAppState;

static void apply_stored_state(AppState* loaded, const StoredAppState* stored) {
  app_state_init(loaded);
  loaded->profile = stored->profile;
  for (int index = 0; index < DIABETO_LOG_DAYS; index++) {
    loaded->logs[index] = stored->logs[index];
  }
  loaded->currentLogIndex = stored->currentLogIndex;
  loaded->selectedField = stored->selectedField;
  loaded->tipIndex = stored->tipIndex;
  loaded->screen = stored->screen;
}

bool load_app_state(AppState* state) {
  FILE* file = fopen(SAVE_PATH, "rb");

  if (!file) {
    return false;
  }

  uint32_t version = 0;
  const size_t versionRead = fread(&version, sizeof(version), 1, file);
  rewind(file);

  if (versionRead != 1) {
    fclose(file);
    return false;
  }

  StoredAppState stored;
  if (version == DIABETO_SAVE_VERSION || version == 4) {
    const size_t readCount = fread(&stored, sizeof(StoredAppState), 1, file);
    fclose(file);

    if (readCount != 1) {
      return false;
    }
  } else {
    fclose(file);
    return false;
  }

  AppState loaded;
  apply_stored_state(&loaded, &stored);
  *state = loaded;
  state->version = DIABETO_SAVE_VERSION;
  state->screen = SCREEN_DASHBOARD;
  state->currentLogIndex = 0;
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

  StoredAppState saved;
  saved.version = DIABETO_SAVE_VERSION;
  saved.profile = state->profile;
  for (int index = 0; index < DIABETO_LOG_DAYS; index++) {
    saved.logs[index] = state->logs[index];
  }
  saved.currentLogIndex = state->currentLogIndex;
  saved.screen = SCREEN_DASHBOARD;
  saved.selectedField = 0;
  saved.tipIndex = state->tipIndex;

  const size_t writeCount = fwrite(&saved, sizeof(StoredAppState), 1, file);
  fclose(file);

  return writeCount == 1;
}
