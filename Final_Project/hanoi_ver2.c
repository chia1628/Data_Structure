#include <stdio.h>
#include <stdlib.h>
#include "hanoi.h" // 假設您有 header，或者如果這是單一檔案，請確保結構定義在上方
#include "common.h" // 假設您有 header，或者如果這是單一檔案，請確保結構定義在上方

// 跨平台支援 Sleep 與清畫面
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif


void initStack(Stack *s, char name, int capacity) {
    s->data = (Disk *)malloc(sizeof(Disk) * capacity);
    if (s->data == NULL) {
        fprintf(stderr, "Error: memory allocation failed for stack %c\n", name);
        exit(1);
    }
    s->top = -1;
    s->capacity = capacity;
    s->name = name;
}

void freeStack(Stack *s) {
    if (s->data) {
        free(s->data);
        s->data = NULL;
    }
}

void push(Stack *s, Disk d) {
    if (s->top >= s->capacity - 1) return;
    s->top++;
    s->data[s->top] = d;
}

Disk popStack(Stack *s) {
    if (s->top == -1) {
        Disk empty = {0, 0};
        return empty;
    }
    Disk d = s->data[s->top];
    s->top--;
    return d;
}

Stack* getStackByName(HanoiContext *ctx, char name) {
    if (name == 'A') return &ctx->A;
    if (name == 'B') return &ctx->B;
    return &ctx->C;
}

// ==========================================
// 視覺化核心 (Visual Core)
// ==========================================

void print_level(Stack *s, int level, int max_n) {
    int disk_size = 0;
    char disk_color = '='; 

    if (level <= s->top) {
        disk_size = s->data[level].size;
        char c = s->data[level].color;
        if (c == 'R' || c == 'B') {
            disk_color = c;
        }
    }

    int padding = max_n - disk_size;

    for (int i = 0; i < padding; i++) printf(" ");
    if (disk_size > 0) {
        for (int i = 0; i < disk_size; i++) printf("%c", disk_color);
    }
    printf("|");
    if (disk_size > 0) {
        for (int i = 0; i < disk_size; i++) printf("%c", disk_color);
    }
    for (int i = 0; i < padding; i++) printf(" ");
    printf("  ");
}

void printTowers_visual(HanoiContext *ctx) {
    clear_screen();

    int n = ctx->A.capacity; 

    printf("\n=== Step: %llu ======================\n", ctx->stepCount);

    for (int i = n - 1; i >= 0; i--) {
        print_level(&ctx->A, i, n);
        print_level(&ctx->B, i, n);
        print_level(&ctx->C, i, n);
        printf("\n");
    }

    for (int k = 0; k < 3; k++) {
        for (int i = 0; i < n; i++) printf(" ");
        if (k == 0) printf("A");
        if (k == 1) printf("B");
        if (k == 2) printf("C");
        for (int i = 0; i < n; i++) printf(" ");
        printf("  ");
    }
    printf("\n==================================\n");
    
    
}

// ==========================================
// Mode 1: 標準漢諾塔 (Standard)
// ==========================================

void moveDisk_standard(HanoiContext *ctx, Stack *from, Stack *to) {
    Disk d = popStack(from);
    push(to, d);
    ctx->stepCount++;
    
    printTowers_visual(ctx);
    printf("Move disk %d from %c to %c\n", d.size, from->name, to->name);
    wait_for_a_while(ctx->delay_ms);
}

void hanoi_rec_standard(HanoiContext *ctx, int n, Stack *from, Stack *aux, Stack *to) {
    if (n == 1) {
        moveDisk_standard(ctx, from, to);
        return;
    }
    hanoi_rec_standard(ctx, n - 1, from, to, aux);
    moveDisk_standard(ctx, from, to);
    hanoi_rec_standard(ctx, n - 1, aux, from, to);
}

void solve_standard(int n) {
    HanoiContext ctx;
    ctx.stepCount = 0;
    ctx.n = n;
    ctx.delay_ms = 500; 

    initStack(&ctx.A, 'A', n);
    initStack(&ctx.B, 'B', n);
    initStack(&ctx.C, 'C', n);

    for (int i = n; i >= 1; i--) {
        Disk d = {i, 'X'};
        push(&ctx.A, d);
    }

    // 1. 先顯示初始狀態
    printTowers_visual(&ctx);
    printf("Press Enter to start...");
    getchar(); // 吃掉 scanf 留下的 newline
    getchar(); // 等待使用者按 Enter

    // 3. 開始遞迴
    hanoi_rec_standard(&ctx, n, &ctx.A, &ctx.B, &ctx.C);

    printf("\nDone! Total steps: %llu\n", ctx.stepCount);
    
    freeStack(&ctx.A);
    freeStack(&ctx.B);
    freeStack(&ctx.C);
}

// ==========================================
// Mode 2: 雙色/奇偶漢諾塔 (Bi-Color)
// ==========================================

void moveDisk_bicolor(HanoiContext *ctx, char fromName, char toName) {
    Stack *from = getStackByName(ctx, fromName);
    Stack *to   = getStackByName(ctx, toName);
    
    Disk d = popStack(from);
    push(to, d);
    ctx->stepCount++;

    printTowers_visual(ctx);
    printf("Move %c%d from %c to %c\n", d.color, d.size, fromName, toName);
    wait_for_a_while(ctx->delay_ms);
}

char thirdPeg(char a, char b) {
    if ((a == 'A' && b == 'B') || (a == 'B' && b == 'A')) return 'C';
    if ((a == 'A' && b == 'C') || (a == 'C' && b == 'A')) return 'B';
    return 'A';
}

char target_of_size(int size) {
    return (size % 2 == 1) ? 'C' : 'B';
}

void hanoi_block(HanoiContext *ctx, int k, char fromName, char toName, char auxName) {
    if (k <= 0) return;
    if (k == 1) {
        moveDisk_bicolor(ctx, fromName, toName);
        return;
    }
    hanoi_block(ctx, k - 1, fromName, auxName, toName);
    moveDisk_bicolor(ctx, fromName, toName);
    hanoi_block(ctx, k - 1, auxName, toName, fromName);
}

void moveVar(HanoiContext *ctx, int n, char fromName) {
    if (n == 0) return;

    char target = target_of_size(n);

    if (target == fromName) {
        moveVar(ctx, n - 1, fromName);
    }
    else {
        char other = thirdPeg(fromName, target);
        hanoi_block(ctx, n - 1, fromName, other, target);
        moveDisk_bicolor(ctx, fromName, target);
        moveVar(ctx, n - 1, other);
    }
}

void solve_bicolor(int n) {
    HanoiContext ctx;
    ctx.n = n;
    ctx.stepCount = 0;
    ctx.delay_ms = 500;

    initStack(&ctx.A, 'A', n);
    initStack(&ctx.B, 'B', n);
    initStack(&ctx.C, 'C', n);

    for (int size = n; size >= 1; size--) {
        Disk d;
        d.size = size;
        d.color = (size % 2 == 1) ? 'R' : 'B';
        push(&ctx.A, d);
    }

    // 1. 先顯示初始狀態
    printTowers_visual(&ctx);
    printf("Press Enter to start...");
    getchar(); // 吃掉 scanf 留下的 newline
    getchar(); // 等待使用者按 Enter

    // 3. 開始執行
    moveVar(&ctx, n, 'A');

    printf("\nDone! Final state shown above.\n");
    printf("Total steps: %llu\n", ctx.stepCount);

    freeStack(&ctx.A);
    freeStack(&ctx.B);
    freeStack(&ctx.C);
}

// ==========================================
// 主程式 (Main)
// ==========================================

// int main(void) {
//     int choice, n;

//     clear_screen(); 

//     printf("=== Hanoi Tower Visualizer ===\n");
//     printf("1. Standard Hanoi (Recursive)\n");
//     printf("2. Bi-Color Sorting (Red->C, Blue->B)\n");
//     printf("------------------------------\n");
//     printf("Select Mode (1 or 2): ");
    
//     if (scanf("%d", &choice) != 1) return 1;
//     if (choice == 1) {
//         printf("Enter number of disks: ");
//         if (scanf("%d", &n) != 1 || n <= 0) return 1;
//         solve_standard(n);
//     }
//     else if (choice == 2) {
//         printf("Enter number of disks: ");
//         if (scanf("%d", &n) != 1 || n <= 0) return 1;
//         solve_bicolor(n);
//     }
//     else {
//         printf("Unknown choice.\n");
//     }
//     return 0;
// }