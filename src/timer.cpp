#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <iostream>
#include <list>

#include "timer.h"

using namespace std;

static HWND hwnd;


static list<stTimer_t *> tlist;

static int trun_flag = 0;


static int timer_search_timer(stTimer_t *t);
static int timer_append_timer(stTimer_t *t);
static int timer_remove_timer(stTimer_t *t);
static int util_get_current_time_ms();
static int timer_get_min_delt();
static void CALLBACK timer_proc(HWND hwnd, UINT uMsg,
			   UINT_PTR idEvent,
			   DWORD dwTime);

int timer_init() {
  tlist.clear();
  return 0;
}
int timer_init_timer(stTimer_t *t, TIMEFUNC tf, void *arg) {
  t->func = tf;
  t->arg = arg;
  return 0;
}

int timer_set_timer(stTimer_t *t, int ms) {
  t->ms = ms;
  t->start = util_get_current_time_ms();
  
  if (!timer_search_timer(t)) {
    timer_append_timer(t);
  } else {
    ; // allready exsit, do nothing.
  }

  if (trun_flag == 0) {
      int ms = timer_get_min_delt();

      if (ms >= 0) {
	if (ms== 0) {
	  ms = 10;
	}
	
	int iret = SetTimer(NULL, IDT_TIMER, ms,
			  (TIMERPROC)timer_proc);
	trun_flag = 1;
	
      }
  }
  
  return 0;
}
int timer_cancle_timer(stTimer_t *t) {
  if (!timer_search_timer(t)) {
    return 0;
  }
  timer_remove_timer(t);
  return 0;
}

//////////////////////////////////////////////////////////////
static int timer_search_timer(stTimer_t *t) {
  for (list<stTimer_t*>::iterator it = tlist.begin();
       it != tlist.end();
       it++) {
    stTimer_t *tt = *it;
    if (tt != NULL && tt == t) {
      return 1;
    }
  }
  return 0;
}
static int timer_append_timer(stTimer_t *t) {
  tlist.push_back(t);
  return 0;
}
static int timer_remove_timer(stTimer_t *t) {
  list<stTimer_t *>::iterator itt = tlist.end();
  for (list<stTimer_t*>::iterator it = tlist.begin();
       it != tlist.end();
       ) {
    stTimer_t *tt = *it;
    if (tt != NULL && tt ==t ) {
      itt = it;
    }
  }
  if (itt != tlist.end()) {
    tlist.erase(itt);
  }
  return 0;  
}
static int timer_dispatch() {
  list<stTimer_t *>::iterator itt = tlist.end();
  for (list<stTimer_t*>::iterator it = tlist.begin();
       it != tlist.end();
       it++) {
    stTimer_t *tt = *it;
    int ms = util_get_current_time_ms();
    if (tt !=NULL && ms >= tt->start + ms && tt->func != NULL) {
      itt= it;
      break;
    }
  }
  if (itt != tlist.end()) {
    stTimer_t *tt = *itt;
    tlist.erase(itt);

    if (tt != NULL && tt->func != NULL) {
      tt->func(tt->arg);
    }
  }
  return 0;
}
static void CALLBACK timer_proc(HWND hwnd, UINT uMsg,
			   UINT_PTR idEvent,
			   DWORD dwTime) {
  timer_dispatch();
  
  int ms = timer_get_min_delt();
  if (ms < 0) {
    trun_flag = 0;
    return;
  }
  
  if (ms== 0) {
    ms = 10;
  }

  printf("ms is %d\n", ms);
  int iret = SetTimer(NULL, IDT_TIMER, ms,
			(TIMERPROC)timer_proc);
  trun_flag = 1;
}

static int util_get_current_time_ms() {
  return GetTickCount();
}

static int timer_get_min_delt() {
  int min = 10000000;
    for (list<stTimer_t*>::iterator it = tlist.begin();
       it != tlist.end();
       it++) {
    stTimer_t *tt = *it;
    if (tt == NULL) {
      continue;
    }
    if (tt->start + tt->ms < min) {
      min = tt->start + tt->ms;
    }
  }
    
    return min;
}


