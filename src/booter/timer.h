#ifndef TIMER_H
#define TIMER_H


void init_timer(void);
void sleep_cs(unsigned cs);

extern void timer_event(void);

#endif /* TIMER_H */

