#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};
#define KEYSCOUNT ((sizeof keyname) / (sizeof keyname[0]))
static uint8_t keystate[KEYSCOUNT];

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char buf[64], type[3], key[32];
  if (!NDL_PollEvent(buf, 64)) { return 0; }
  sscanf(buf, "%s %s", type, key);
  for (int i = 0; i < KEYSCOUNT; i++) {
    if (!strcmp(key, keyname[i])) { ev->key.keysym.sym = i; break; }
  }
  if (!strcmp(type, "kd")) {
    ev->key.type = SDL_KEYDOWN;
    ev->type = SDL_KEYDOWN;
    keystate[ev->key.keysym.sym] = 1;
  }
  else {
    ev->key.type = SDL_KEYUP;
    ev->type = SDL_KEYUP;
    keystate[ev->key.keysym.sym] = 0;
  }
  return 1;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[64], type[3], key[32];
  while (!NDL_PollEvent(buf, 64));
  sscanf(buf, "%s %s", type, key);
  for (int i = 0; i < KEYSCOUNT; i++) {
    if (!strcmp(key, keyname[i])) { event->key.keysym.sym = i; break; }
  }
  if (!strcmp(type, "kd")) {
    event->key.type = SDL_KEYDOWN;
    event->type = SDL_KEYDOWN;
    keystate[event->key.keysym.sym] = 1;
  }
  else {
    event->key.type = SDL_KEYUP;
    event->type = SDL_KEYUP;
    keystate[event->key.keysym.sym] = 0;
  }
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  if (numkeys) {
    *numkeys = KEYSCOUNT;
  }
  return (uint8_t*)&keystate;
}
