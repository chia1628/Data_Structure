#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 定義可以執行的模式種類
#define MODE_NONE 0
#define MODE_C    1
#define MODE_D    2
#define MAX_SYMBOLS   256
#define MAX_CODE_LEN 256


// 定義鏈結串列結構
typedef struct HuffmanNode {
    unsigned char symbol; // byte
    unsigned int freq; // 出現頻率
    struct HuffmanNode *left; // 左子節點
    struct HuffmanNode *right; // 右子節點
} HuffmanNode;

int count_frequency(FILE* fin, int * fre_array){
    if (!fin) {
        perror("Cannot open input.txt");
        return 1;
    }

    
    int c;
    while ((c = fgetc(fin)) != EOF) { // 一個字一個字讀取
        fre_array[c]++;
    }

    fclose(fin);

    // 印出頻率
    for (int i = 0; i < 256; i++) {
        if (fre_array[i] > 0) {
            printf("Char 0x%02X ('%c') : %d\n", i, (i >= 32 && i <= 126) ? i : '.', fre_array[i]);
        }
    }
    HuffmanNode* parent = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    return 0;
}

HuffmanNode* build_huffman_tree(int freq[MAX_SYMBOLS]) {
    int n = 0; // 葉子數量
    HuffmanNode* nodes[MAX_SYMBOLS];

    // 1. 生成初始節點
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (freq[i] > 0) {
            nodes[n] = (HuffmanNode*)malloc(sizeof(HuffmanNode));
            nodes[n]->symbol = (unsigned char)i;
            nodes[n]->freq = freq[i];
            nodes[n]->left = nodes[n]->right = NULL; //左右接地
            n++;
        }
    }

    // 2. 合併節點
    while (n > 1) {
        // 找出最小兩個節點
        int min1 = -1, min2 = -1;
        for (int i = 0; i < n; i++) {
            if (min1 == -1 || nodes[i]->freq < nodes[min1]->freq) { // 先找第一個最小的
                min2 = min1; // 不用從頭找起
                min1 = i;
            } 
            else if (min2 == -1 || nodes[i]->freq < nodes[min2]->freq) { // 再找第一個最小的
                min2 = i;
            }
        }
        // 找好兩個小的
        // 建立新父節點
        HuffmanNode* parent = (HuffmanNode*)malloc(sizeof(HuffmanNode));
        parent->symbol = 0; // internal node 沒有符號
        parent->freq = nodes[min1]->freq + nodes[min2]->freq;
        parent->left = nodes[min1]; // 小的放左邊
        parent->right = nodes[min2]; // 大的放右邊

        // 用新節點替換 min1，刪除 min2
        if (min2 < min1) { 
            int tmp = min1; 
            min1 = min2; 
            min2 = tmp; 
        } 
        nodes[min1] = parent;
        nodes[min2] = nodes[n-1]; // 移除 min2
        n--;
    }

    return nodes[0]; // 根節點
}

void display_huffman_tree(HuffmanNode* node, int level) {
    if (!node) return;

    // 先顯示右子節點（這樣顯示時，樹的右邊在上）
    display_huffman_tree(node->right, level + 1);

    // 縮排
    for (int i = 0; i < level; i++){
        printf("    ");
    } 

    // 如果是葉節點，顯示符號和頻率
    if (!node->left && !node->right) { //兩個都是null
        if (node->symbol >= 32 && node->symbol <= 126) {
            printf("'%c' (%d)\n", node->symbol, node->freq);
        } 
        else {
            printf("0x%02X (%d)\n", node->symbol, node->freq);
        }
    } 
    else {
        printf("* (%d)\n", node->freq); // internal node
    }

    // 左子節點
    display_huffman_tree(node->left, level + 1);
}

void encode(HuffmanNode* node, int level) {
    if (!node) return;
        // 先顯示右子節點（這樣顯示時，樹的右邊在上）
    
    if(node->right){
        printf("%d", 1);
        encode(node->right, level + 1);
    }
    


    // 縮排
    for (int i = 0; i < level; i++) printf("    ");

    // 如果是葉節點，顯示符號和頻率
    if (!node->left && !node->right) { //兩個都是null
        if (node->symbol >= 32 && node->symbol <= 126) {
            printf("'%c' (%d)\n", node->symbol, node->freq);
        } else {
            printf("0x%02X (%d)\n", node->symbol, node->freq);
        }
    } else {
        printf("* (%d)\n", node->freq); // internal node
    }

    // 左子節點
    encode(node->left, level + 1);
}

// 產生編碼
void generate_codes(HuffmanNode* node, char* code, int depth, char codes[MAX_SYMBOLS][MAX_CODE_LEN]) {
    if (!node) return;

    if (!node->left && !node->right) { //如果是葉子
        code[depth] = '\0';
        strcpy(codes[node->symbol], code);
        return;
    }

    code[depth] = '0';
    generate_codes(node->left, code, depth + 1, codes); // 先往左放0
    code[depth] = '1';
    generate_codes(node->right, code, depth + 1, codes); // 再往右放1
}

// ---------------- Free Huffman Tree ----------------
void free_tree(HuffmanNode* node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

// ---------------- Write Compressed File ----------------
void compress_file(FILE* fin, FILE* fout, char codes[MAX_SYMBOLS][MAX_CODE_LEN]) {
    fseek(fin, 0, SEEK_SET); // rewind input file
    int c;
    unsigned char buffer = 0;
    int bit_count = 0;

    while ((c = fgetc(fin)) != EOF) {
        char* code = codes[c];
        for (int i = 0; code[i]; i++) {
            buffer <<= 1;
            if (code[i] == '1') buffer |= 1;
            bit_count++;
            if (bit_count == 8) {
                fputc(buffer, fout);
                buffer = 0;
                bit_count = 0;
            }
        }
    }
    if (bit_count > 0) {
        buffer <<= (8 - bit_count);
        fputc(buffer, fout);
    }
}

void compress(){

}

void decompress(){
    
}

int main(int argc, char *argv[]) {
    int opt;
    int mode = MODE_NONE;
    char *inputFile = NULL;
    char *outputFile = NULL;
    int limit_length = -1;  // -1 表示沒限制
    int frequency_array[256] = {0};

    while ((opt = getopt(argc, argv, "cdi:o:l:")) != -1) {
        switch(opt) {
            case 'c':
                if (mode == MODE_NONE) mode = MODE_C;
                else fprintf(stderr, "Error: mode already specified\n");
                break;
            case 'd':
                if (mode == MODE_NONE) mode = MODE_D;
                else fprintf(stderr, "Error: mode already specified\n");
                break;
            case 'i':
                inputFile = optarg;
                break;
            case 'o':
                outputFile = optarg;
                break;
            case 'l':
                limit_length = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Unknown option\n");
                return 1;
        }
    }

    if (mode == MODE_NONE) {
        fprintf(stderr, "Error: -c or -d must be specified\n");
        return 1;
    }
    if (inputFile == NULL) {
        fprintf(stderr, "Error: -i <input file> is required\n");
        return 1;
    }
    if (outputFile == NULL) {
        fprintf(stderr, "Error: -o <output file> is required\n");
        return 1;
    }
     printf("mode=%d, input=%s, output=%s, limit=%d\n", mode, inputFile, outputFile, limit_length);
    // TODO: 根據 mode 做 Huffman 壓縮或解壓縮
    
    // 確定輸入檔案存在
    FILE *fin = fopen(inputFile, "r");
    if (fin == NULL) {
        perror("Error opening input file");
        return 1;
    }

    // 確定輸出檔案存在
    FILE* fout = fopen(outputFile, "w");
    if (fout == NULL) {
        perror("Error opening output file");
        fclose(fin);
        return 1;
    }
    int freq[MAX_SYMBOLS] = {0};

    int a = count_frequency(fin, freq);
    HuffmanNode* root = build_huffman_tree(freq);
    encode(root, 0);

    return 0;
}
