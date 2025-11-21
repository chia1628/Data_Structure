#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 定義鏈結串列結構
typedef struct node {
    int poly;           // 多項式項次（例如 x^n 的 n）
    struct node* next;  // 指向下一個節點
} NODE;

// 在節點 P 之後插入新節點
NODE* INSERT_after(NODE* P, int poly_index) {
    NODE* new_node = (NODE*)malloc(sizeof(NODE));
    new_node->poly = poly_index;
    new_node->next = P->next;
    P->next = new_node;
    return new_node;
}

// 在節點 P 之前插入新節點
NODE* INSERT_before(NODE* P, int poly_index) {
    NODE* new_node = (NODE*)malloc(sizeof(NODE));
    new_node->poly = poly_index;
    new_node->next = P;
    return new_node;
}

// 刪除節點 P_delete（其前一節點為 P_front）
NODE* DELETE(NODE* P_front, NODE* P_delete) {
    if (P_front == NULL) {  // 若刪除的是頭節點
        return P_delete->next;
    }
    P_front->next = P_delete->next;
    free(P_delete);
    return P_front;
}

// 將串列中每個節點的 poly 加上 number
NODE* UPDATE(NODE* LIST, int number) {
    NODE* tmp = LIST;
    while (tmp != NULL) {
        tmp->poly += number;
        tmp = tmp->next;
    }
    return LIST;
}

// 印出整個鏈結串列
void PRINT(NODE* LIST) {
    NODE* tmp = LIST;
    while (tmp != NULL) {
        printf("%d ", tmp->poly);
        tmp = tmp->next;
    }
}

// 釋放整個鏈結串列記憶體
void FREE_LIST(NODE* head) {
    NODE* tmp;
    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp);
    }
}

// 比較兩個多項式串列 G 與 D
// 功能：模擬 XOR 運算，將 G 的內容融合進 D
NODE* COMPARE(NODE* G, NODE* D) {
    NODE *pG = G, *pD = D, *pD_prev = NULL;
    NODE* NEW_HEAD = NULL;

    while (pG != NULL) {
        if (pD == NULL) {
            // 若 D 已結束，將 G 剩餘節點插入 D 尾端
            if (pD_prev != NULL) {
                INSERT_after(pD_prev, pG->poly);
                pG = pG->next;
                pD = pD_prev->next;
            } else {
                // 若 D 是空的，建立新頭節點
                NEW_HEAD = (NODE*)malloc(sizeof(NODE));
                NEW_HEAD->poly = pG->poly;
                NEW_HEAD->next = NULL;
                pG = pG->next;
            }
        }
        else if (pG->poly == pD->poly) {
            // 若兩邊 poly 值相同，表示 XOR 為 0，刪除此節點
            pG = pG->next;
            pD = DELETE(pD_prev, pD);
        }
        else if (pG->poly < pD->poly) {
            // G 的項次較小，繼續往下比
            if (NEW_HEAD == NULL) NEW_HEAD = pD;
            pD_prev = pD;
            pD = pD->next;
        }
        else {
            // G 的項次較大，插入至 D 中
            pD = INSERT_before(pD, pG->poly);
            if (NEW_HEAD == NULL) {
                NEW_HEAD = pD;
                pD_prev = NULL;
            } else {
                pD_prev->next = pD;
            }
            pG = pG->next;
        }
    }
    return NEW_HEAD;
}

// 將十六進位位元（以整數陣列表示）轉換為二進位字串
void HexToBin(int *ascii, char *binary_string) {
    long i = 0;
    while (ascii[i]) {
        switch (ascii[i]) {
            case 0: strcat(binary_string, "0000"); break;
            case 1: strcat(binary_string, "0001"); break;
            case 2: strcat(binary_string, "0010"); break;
            case 3: strcat(binary_string, "0011"); break;
            case 4: strcat(binary_string, "0100"); break;
            case 5: strcat(binary_string, "0101"); break;
            case 6: strcat(binary_string, "0110"); break;
            case 7: strcat(binary_string, "0111"); break;
            case 8: strcat(binary_string, "1000"); break;
            case 9: strcat(binary_string, "1001"); break;
            default:
                return; // 遇到非法輸入就停止
        }
        i++;
    }
}


// 將輸入字元轉換為 0~15 的數值（拆成高、低 nibble）
int hex_to_byte_value(char* hex, int* ascii) {
    int i = 0, index = 0;
    while (hex[i]) {
        int value = hex[i];
        ascii[index++] = value / 16;
        ascii[index++] = value % 16;
        i++;
    }
    return 0;
}

// 主程式：多項式除法運算（CRC）
int main() {
    NODE *G_ORI, *P, *D;
    char input_data[3];
    int ascii[5];
    char binary_string[16] = {0};
    int poly_ori[3] = {4, 1, 0};  // 原始多項式的項次

    // 建立初始空串列 D 和 G_ORI
    D = (NODE*)malloc(sizeof(NODE));
    D->next = NULL;
    G_ORI = (NODE*)malloc(sizeof(NODE));
    G_ORI->next = NULL;
    P = G_ORI;

    // 輸入資料（假設為 2 位數字元）
    scanf("%s", input_data);
    printf("Input data: %s\n", input_data);

    // 建立 G_ORI 多項式串列（例如 x^4 + x + 1）
    for (int i = 0; i < 3; i++) {
        P = INSERT_after(P, poly_ori[i]);
    }
    G_ORI = G_ORI->next; // 移至第一個實際節點

    // 將輸入轉換為二進位串
    hex_to_byte_value(input_data, ascii);
    HexToBin(ascii, binary_string);

    // 根據二進位字串建立 D 串列
    P = D;
    for (int i = 0; i < 16; i++) {
        if (binary_string[i] == '1') {
            P = INSERT_after(P, 15 - i);
        }
    }
    D = D->next;

    // 所有項次 +4（模擬左移）
    UPDATE(D, 4);

    int steps = 0;
    // 當 D 的最高項次仍大於 3，持續進行「XOR除法」步驟
    while (D->poly > 3) {
        steps++;
        printf("Step %d\n", steps);

        int shift = D->poly - 4;  // 計算偏移量
        NODE* G_new = (NODE*)malloc(sizeof(NODE));
        G_new->next = NULL;
        P = G_new;

        // 建立位移後的新多項式 G_new
        for (int i = 0; i < 3; i++) {
            P = INSERT_after(P, poly_ori[i] + shift);
        }
        G_new = G_new->next;

        // 顯示運算前後狀態
        printf("List G: ");
        PRINT(G_new);
        printf("\n");

        printf("List D Before XOR Subtraction: ");
        PRINT(D);
        printf("\n");

        // 進行 XOR 運算
        D = COMPARE(G_new, D);

        printf("List D After XOR Subtraction: ");
        PRINT(D);
        printf("\n");
    }

    // 輸出最終結果（低 4 位）
    printf("\nFinal Result: ");
    for (int i = 3; i >= 0; i--) {
        if (D != NULL && D->poly == i) {
            printf("1");
            D = D->next;
        } else {
            printf("0");
        }
    }
    printf("\n");

    // 釋放記憶體
    FREE_LIST(G_ORI);
    FREE_LIST(D);

    return 0;
}
