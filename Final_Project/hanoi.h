#ifndef HANOI_H
#define HANOI_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h> // 為了使用 Sleep


// 定義堆疊結構 (代表柱子)
typedef struct {
    int *data;      // 盤子資料陣列
    int top;        // 堆疊頂端索引
    int capacity;   // 容量 (N)
    char name;      // 柱子名稱 'A', 'B', 'C'
} Stack;

// 定義河內塔執行環境 (包含三根柱子與步數)
typedef struct {
    Stack A;
    Stack B;
    Stack C;
    unsigned long long stepCount;
} HanoiContext;

/**
 * 核心解題函式
 * @param n 盤子的數量
 * 執行後會自動印出過程與總步數，並釋放記憶體。
 */
void gotoxy(int x, int y);
void wait_for_a_while();
void solve_hanoi(int n);
void clear_screen();

// 如果主程式需要單獨控制 Stack 或其他細節，
// 可以選擇性公開以下函式 (若不需要可保留在 .c 內當作內部函式)：

/* void initStack(Stack *s, char name, int capacity);
int isEmpty(Stack *s);
int isFull(Stack *s);
void push(Stack *s, int val);
int popStack(Stack *s);
void printTowers(HanoiContext *ctx);
*/

#endif