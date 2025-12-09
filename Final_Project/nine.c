#include "nine.h"
#include "common.h" // 假設您有 header，或者如果這是單一檔案，請確保結構定義在上方

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

// ==========================================
// 2. 鏈結串列操作 (Linked List Ops)
// ==========================================

RingNode* create_ring_list(int n) {
    RingNode *head = NULL;
    RingNode *curr = NULL;

    for (int i = 1; i <= n; i++) {
        RingNode *newNode = (RingNode*)malloc(sizeof(RingNode));
        newNode->id = i;
        newNode->state = 1; // 預設全部在劍上
        newNode->next = NULL;
        newNode->prev = NULL;

        if (head == NULL) {
            head = newNode;
            curr = newNode;
        } else {
            curr->next = newNode;
            newNode->prev = curr;
            curr = newNode;
        }
    }
    return head;
}

void free_ring_list(RingNode *head) {
    RingNode *tmp;
    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp);
    }
}

RingNode* get_tail(RingNode *head) {
    if (!head) return NULL;
    RingNode *curr = head;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    return curr;
}

// ==========================================
// 3. 核心邏輯 (Logic with Pointers)
// ==========================================

// 從最外層 (Tail) 往回找，找到第一個狀態為 1 的節點
RingNode* find_lead_node(RingNode *tail) {
    RingNode *curr = tail;
    while (curr != NULL) {
        if (curr->state == 1) {
            return curr;
        }
        curr = curr->prev;
    }
    return NULL; // 全部都解開了 (全0)
}

int count_nodes_on_sword(RingNode *head) {
    int count = 0;
    RingNode *curr = head;
    while (curr != NULL) {
        if (curr->state == 1) count++;
        curr = curr->next;
    }
    return count;
}

// ==========================================
// 4. 視覺化 (Visualization)
// ==========================================

void print_visual_list(RingNode *head, int step) {
    gotoxy(0, 0); // 覆蓋輸出，避免閃爍

    // 為了知道總共有幾個環，先計算一下 (或傳入 n 亦可)
    // 但既然是 Linked List 練習，我們就遍歷
    int n = 0;
    RingNode *temp = head;
    while(temp) { n++; temp = temp->next; }

    printf("\n=== %d-Linked Rings (Linked List Ver.) Step: %d ===\n\n", n, step);

    // 1. 印出編號
    printf("Ring: ");
    temp = head;
    while (temp != NULL) {
        printf(" %2d  ", temp->id);
        temp = temp->next;
    }
    printf("\n");

    // 2. 印出劍身
    printf("Sword:");
    temp = head;
    while (temp != NULL) {
        if (temp->state == 1) printf("-(@)-");
        else                  printf("-----");
        temp = temp->next;
    }
    printf("-->>\n");

    // 3. 印出下方
    printf("Down: ");
    temp = head;
    while (temp != NULL) {
        if (temp->state == 0) printf("  |  ");
        else                  printf("     ");
        temp = temp->next;
    }
    printf("\n\n");
    
    // Debug 顯示狀態列
    printf("State List: [HEAD] ");
    temp = head;
    while (temp != NULL) {
        printf("%d%s", temp->state, (temp->next) ? "<->" : "");
        temp = temp->next;
    }
    printf(" [TAIL]\n");

    printf("==================================================\n");
}

// ==========================================
// 5. 遞迴解題 (Recursive Solver)
// ==========================================

void solve_recursive(char cmd, int n, RingNode *head, RingNode *tail, int *step_counter, int delay_ms) {
    // 檢查是否已完成 (找 Lead)
    RingNode *lead = find_lead_node(tail);
    
    // 更新畫面
    (*step_counter)++;
    print_visual_list(head, *step_counter);
    wait_for_a_while(delay_ms);

    if (lead == NULL) {
        return; // Done
    }

    // --- 執行動作 ---
    if (cmd == 'R') {
        // R 規則：翻轉最外層 (Tail)
        if (tail) {
            tail->state = !tail->state;
        }
    } 
    else if (cmd == 'S') {
        // S 規則：翻轉 Lead 的「前一個 (prev)」
        // 數學上這就是操作 Lead 左邊的那個環
        if (lead && lead->prev) {
            lead->prev->state = !lead->prev->state;
        }
    }

    // 動作完成後，更新畫面
    print_visual_list(head, *step_counter);
    wait_for_a_while(delay_ms);

    // --- 遞迴下一步 ---
    lead = find_lead_node(tail);
    if (lead != NULL) {
        // 交替規則
        if (cmd == 'R') {
            solve_recursive('S', n, head, tail, step_counter, delay_ms);
        } else {
            solve_recursive('R', n, head, tail, step_counter, delay_ms);
        }
    }
}

// ==========================================
// 6. 主程式 (Main)
// ==========================================

int nine_Custom(int n) {
    int step_count = 0;
    int delay = 150; //ms

    hide_cursor();
    clear_screen();

    // 建立鏈結串列
    RingNode *head = create_ring_list(n);
    RingNode *tail = get_tail(head); // 先抓好 tail，之後傳遞比較快

    printf("\nEnter state (1=On Sword, 0=Down):\n");
    RingNode *curr = head;
    while (curr != NULL) {
        int val;
        printf("Ring %d: ", curr->id);
        scanf("%d", &val);
        if (val != 0 && val != 1) val = 1;
        curr->state = val;
        curr = curr->next;
    }
    // Mode 1 不需要做動作，因為 create_ring_list 預設就是全 1

    clear_screen();
    print_visual_list(head, 0);

    // 等待使用者
    printf("Press Enter to start...");
    getchar(); getchar(); 

    // --- 判斷起始規則 ---
    int on_count = count_nodes_on_sword(head);
    if (on_count == 0) {
        printf("\nAlready solved!\n");
    } else {
        if (on_count % 2 == 1) {
            solve_recursive('R', n, head, tail, &step_count, delay);
        } else {
            solve_recursive('S', n, head, tail, &step_count, delay);
        }
        printf("\n\nSolved in %d steps!\n", step_count);
    }

    // 記得釋放記憶體
    free_ring_list(head);

    #ifndef _WIN32
    printf("\033[?25h"); // Restore cursor on Linux/Mac
    #endif
    return step_count;
}

int nine_original(int n) {
    int step_count = 0;
    int delay = 150; //ms

    hide_cursor();
    clear_screen();

    // 建立鏈結串列
    RingNode *head = create_ring_list(n);
    RingNode *tail = get_tail(head); // 先抓好 tail，之後傳遞比較快

    clear_screen();
    print_visual_list(head, 0);

    // 等待使用者
    printf("Press Enter to start...");
    getchar(); getchar(); 

    // --- 判斷起始規則 ---
    int on_count = count_nodes_on_sword(head);
    if (on_count == 0) {
        printf("\nAlready solved!\n");
    } else {
        if (on_count % 2 == 1) {
            solve_recursive('R', n, head, tail, &step_count, delay);
        } else {
            solve_recursive('S', n, head, tail, &step_count, delay);
        }
        printf("\n\nSolved in %d steps!\n", step_count);
    }

    // 記得釋放記憶體
    free_ring_list(head);

    #ifndef _WIN32
    printf("\033[?25h"); // Restore cursor on Linux/Mac
    #endif

    return step_count;
}