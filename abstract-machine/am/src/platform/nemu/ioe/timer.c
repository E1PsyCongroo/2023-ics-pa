#include <am.h>
#include <nemu.h>
static uint64_t init_us;
void __am_timer_init() {
  init_us = inl(RTC_ADDR + 4);
  init_us = (init_us << 32) + inl(RTC_ADDR);
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uint64_t current_us = inl(RTC_ADDR + 4);
  uptime->us = (current_us << 32) + inl(RTC_ADDR) - init_us;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
