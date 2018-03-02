#ifndef __TIMER_H_
#define __TIMER_H_

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDT_TIMER 100

  typedef void (*TIMEFUNC)(void *arg);

  typedef struct stTimer {
    int ms;
    TIMEFUNC func;
    void *arg;
    int start;
  }stTimer_t;

  int timer_init();
  int timer_init_timer(stTimer_t *t, TIMEFUNC tf, void *arg);
  int timer_set_timer(stTimer_t *t, int ms);
  int timer_cancle_timer(stTimer_t *t);

  
#ifdef __cplusplus
}
#endif
#endif
