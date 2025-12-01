#include <stdio.h>
#include <stdlib.h>
#include <windows.h> // 為了使用 Sleep
#include "common.h" 
#include "hanoi.h"

// 定義一個函式把游標移到 x, y
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void wait_for_a_while() {
    #ifdef _WIN32
        Sleep(500); // 暫停 200 毫秒 (Windows)
    #else
        usleep(200000); // 暫停 200,000 微秒 = 200 毫秒 (Linux/Mac)
    #endif
}
void clear_screen() {
    #ifdef _WIN32
        system("cls");   // Windows 用這個
    #else
        system("clear"); // Mac/Linux 用這個
    #endif
}

void print_level(Stack *s, int level, int max_n) {
    int disk_size = 0;
    
    // 檢查這一層是否有盤子
    // s->top 是目前最上方盤子的索引，如果 level <= top 表示這層有東西
    if (level <= s->top) {
        disk_size = s->data[level];
    }

    // 計算空白數量 (最大盤子 - 目前盤子)
    int padding = max_n - disk_size;

    // 1. 印出左側空白
    for (int i = 0; i < padding; i++) printf(" ");

    // 2. 印出盤子左半邊 (或是空氣)
    if (disk_size > 0) {
        for (int i = 0; i < disk_size; i++) printf("=");
    }

    // 3. 印出中間的柱子
    printf("|");

    // 4. 印出盤子右半邊
    if (disk_size > 0) {
        for (int i = 0; i < disk_size; i++) printf("=");
    }

    // 5. 印出右側空白
    for (int i = 0; i < padding; i++) printf(" ");

    // 6. 柱子之間的間隔 (例如空 2 格)
    printf("  "); 
}

/**
 * 主要函式：印出三根柱子的完整狀態
 */
void printTowers_visual(HanoiContext *ctx) {
    gotoxy(20, 12); // 回到左上角，而不是清除螢幕
    // 取得最大盤子數 N (假設 A 柱的 capacity 就是 N)
    int n = ctx->A.capacity;

    printf("\n=== Step: %llu ===\n", ctx->stepCount);

    // 從最高層 (n-1) 往下掃描到最底層 (0)
    for (int i = n - 1; i >= 0; i--) {
        print_level(&ctx->A, i, n);
        print_level(&ctx->B, i, n);
        print_level(&ctx->C, i, n);
        printf("\n"); // 這一層印完，換行
    }

    // 印出底部的柱子名稱
    // 為了置中，名字前面要補 N 個空白
    for (int k = 0; k < 3; k++) {
        for (int i = 0; i < n; i++) printf(" ");
        if (k == 0) printf("A");
        if (k == 1) printf("B");
        if (k == 2) printf("C");
        for (int i = 0; i < n; i++) printf(" ");
        printf("  ");
    }
    printf("\n==================\n");
}
void initStack(Stack *s, char name, int capacity) {
    s->data = (int *)malloc(sizeof(int) * capacity);
    if (s->data == NULL) {
        fprintf(stderr, "Error: memory allocation failed for stack %c\n", name);
        exit(1);
    }
    s->top = -1;
    s->capacity = capacity;
    s->name = name;
}

int isEmpty(Stack *s) {
    return s->top == -1;
}

int isFull(Stack *s) {
    return s->top == s->capacity - 1;
}

void push(Stack *s, int val) {
    /*if (isFull(s)) {
        fprintf(stderr, "Error: stack %c overflow\n", s->name);
        exit(1);
    }*/
    s->top = s->top + 1;
    s->data[s->top] = val;
}

int popStack(Stack *s) {
    /*if (isEmpty(s)) {
        fprintf(stderr, "Error: stack %c underflow\n", s->name);
        exit(1);
    }*/
    int val = s->data[s->top];
    s->top--;
    return val;
}

void printOneStack(Stack *s) {
    printf("%c: ", s->name);
    if (isEmpty(s)) {
        printf("(empty)");
    }
    else {
        for (int i = 0; i <= s->top; i++) {
            printf("%d ", s->data[i]);
        }
    }
    printf("\n");
}

void printTowers(HanoiContext *ctx) {
    printf("\n----- State after step %llu -----\n", ctx->stepCount);
    printOneStack(&ctx->A);
    printOneStack(&ctx->B);
    printOneStack(&ctx->C);
    printf("-------------------------------\n");
}

void moveDisk(HanoiContext *ctx, Stack *from, Stack *to) {
    int disk = popStack(from);
    push(to, disk);
    ctx->stepCount++;
    printTowers_visual(ctx);
    wait_for_a_while();
    increment_step();
    printf("Step %llu: move disk %d from %c to %c\n",
           ctx->stepCount, disk, from->name, to->name);
    printTowers_visual(ctx);
}

void hanoi_rec(HanoiContext *ctx, int n, Stack *from, Stack *aux, Stack *to) {
    if (n == 1) {
        moveDisk(ctx, from, to);
        return;
    }
    hanoi_rec(ctx, n - 1, from, to, aux);
    moveDisk(ctx, from, to);
    hanoi_rec(ctx, n - 1, aux, from, to);
}

void solve_hanoi(int n) {
    HanoiContext ctx;
    ctx.stepCount = 0;
    printf("\nWe got %d disks.\n", n);
    initStack(&ctx.A, 'A', n);
    initStack(&ctx.B, 'B', n);
    initStack(&ctx.C, 'C', n);
    
    for (int i = n; i >= 1; i--) {
        push(&ctx.A, i);
    }
    hanoi_rec(&ctx, n, &ctx.A, &ctx.B, &ctx.C);

    printf("\nTotal steps: %llu\n", ctx.stepCount);

    free(ctx.A.data);
    free(ctx.B.data);
    free(ctx.C.data);
}
