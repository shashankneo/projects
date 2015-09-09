#ifndef _K_CONSOLE_H_

#define _K_CONSOLE_H_
int kprintf(char *str);
int print_at(char *str, int x, int y);
int printch(char ch, int *x_pos, int *y_pos);
#define     STATUS_BAR  25
#endif //#ifndef _K_CONSOLE_H_