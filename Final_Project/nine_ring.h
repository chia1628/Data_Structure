#ifndef NINE_RING_H
#define NINE_RING_H

#include <stdio.h>

void print_ring(int cnt_print, int X, int Xring[]);
int find_lead(int cnt_lead, int Xring[]);
int on_ring(int X, int Xring[], int on_num);
void solve_Xring(char cmd, int X, int Xring[], int cnt_move);

#endif
