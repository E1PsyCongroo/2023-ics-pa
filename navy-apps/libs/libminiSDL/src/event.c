#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};
#define KEYCOUNT ((sizeof keyname) / (sizeof keyname[0]))

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[64], type[3], key[32];
  while (!NDL_PollEvent(buf, 64));
  sscanf(buf, "%s %s", type, key);
  if (!strcmp(type, "kd")) { event->key.type = SDL_KEYDOWN; }
  else { event->key.type = SDL_KEYUP; }
  for (int i = 0; i < KEYCOUNT; i++) {
    if (!strcmp(key, keyname[i])) { event->key.keysym.sym = i; break; }
  }
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
