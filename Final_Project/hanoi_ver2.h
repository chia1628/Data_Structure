#ifndef HANOI_H
#define HANOI_H

#include <stdio.h>
#include <stdlib.h>

// ==========================================
// 資料結構定義 (Data Structures)
// ==========================================

// 盤子結構：包含大小與顏色
typedef struct {
    int size;
    char color; // 'R' (紅), 'B' (藍), 'X' (預設/無色)
} Disk;

// 堆疊結構：代表一根柱子
typedef struct {
    Disk *data;   // 動態陣列儲存盤子
    int top;      // 目前最上方盤子的索引
    int capacity; // 柱子最大容量
    char name;    // 柱子名稱 ('A', 'B', 'C')
} Stack;

// 漢諾塔環境結構：包含所有狀態與設定
typedef struct {
    Stack A;
    Stack B;
    Stack C;
    unsigned long long stepCount; // 目前步數
    int n;                        // 盤子總數
    int delay_ms;                 // 動畫延遲時間 (毫秒)
} HanoiContext;

// ==========================================
// 函式原型宣告 (Function Prototypes)
// ==========================================

// --- 系統與輔助函式 ---
void wait_for_a_while(int milliseconds);
void clear_screen(void);

// --- 堆疊操作 (Stack Operations) ---
void initStack(Stack *s, char name, int capacity);
void freeStack(Stack *s);
void push(Stack *s, Disk d);
Disk popStack(Stack *s);
Stack* getStackByName(HanoiContext *ctx, char name);

// --- 視覺化輸出 (Visual Output) ---
/* 繪製單一層的盤子狀態 */
void print_level(Stack *s, int level, int max_n);

/* 繪製完整的三根柱子狀態並處理動畫更新 */
void printTowers_visual(HanoiContext *ctx);

// --- 模式一：標準漢諾塔 (Standard Mode) ---
void moveDisk_standard(HanoiContext *ctx, Stack *from, Stack *to);
void hanoi_rec_standard(HanoiContext *ctx, int n, Stack *from, Stack *aux, Stack *to);
void solve_standard(int n);

// --- 模式二：雙色/奇偶漢諾塔 (Bi-Color Mode) ---
char thirdPeg(char a, char b);
char target_of_size(int size);

/* 雙色模式的移動函式 (含視覺化) */
void moveDisk_bicolor(HanoiContext *ctx, char fromName, char toName);

/* 區塊搬移邏輯 */
void hanoi_block(HanoiContext *ctx, int k, char fromName, char toName, char auxName);

/* 變形漢諾塔主要遞迴邏輯 */
void moveVar(HanoiContext *ctx, int n, char fromName);

/* 雙色模式主入口 */
void solve_bicolor(int n);

#endif // HANOI_H