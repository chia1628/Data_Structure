#ifndef NINE_RING_H
#define NINE_RING_H

#include <stdio.h>
#include <stdlib.h>

// ==========================================
// 資料結構定義 (Linked List)
// ==========================================

typedef struct RingNode {
    int id;                 // 環的編號 (1, 2, 3...)
    int state;              // 1: 在劍上 (On), 0: 在下方 (Down)
    struct RingNode *prev;  // 指向前一個 (更內層) 的環 (Left)
    struct RingNode *next;  // 指向後一個 (更外層) 的環 (Right)
} RingNode;

// ==========================================
// 函式宣告
// ==========================================

// --- 系統輔助 ---
void wait_for_a_while(int milliseconds);
void clear_screen(void);
void hide_cursor(void);
void gotoxy(int x, int y);

// --- 鏈結串列操作 ---
/* 建立 N 個節點的雙向鏈結串列，回傳頭指標 (Ring 1) */
RingNode* create_ring_list(int n);

/* 釋放記憶體 */
void free_ring_list(RingNode *head);

/* 取得最後一個節點 (Ring N) - 用於 R 規則 */
RingNode* get_tail(RingNode *head);

// --- 邏輯與視覺化 ---
/* 視覺化繪製 */
void print_visual_list(RingNode *head, int step);

/* 尋找最右邊 (最外層) 的 '1' 所在的節點 */
RingNode* find_lead_node(RingNode *tail);

/* 計算目前有多少環在劍上 */
int count_nodes_on_sword(RingNode *head);

/* 遞迴解題核心 */
void solve_recursive(char cmd, int n, RingNode *head, RingNode *tail, int *step_counter, int delay_ms);

int nine_custom(int n);
int nine_original(int n);

#endif // NINE_RING_H