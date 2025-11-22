#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

// 定義可以執行的模式種類
#define MODE_NONE 0
#define MODE_C    1
#define MODE_D    2
#define MAX_SYMBOLS 256
#define MAX_CODE_LEN 256
#define MAX_PSEUDO 256


// 定義鏈結串列結構
typedef struct HuffmanNode {
    unsigned char symbol; // byte
    unsigned int freq; // 出現頻率
    struct HuffmanNode *left; // 左子節點
    struct HuffmanNode *right; // 右子節點
} HuffmanNode;

// 定義存在檔案中的樹資料
typedef struct {
    unsigned char symbol;
    unsigned char code_length;
    unsigned int code; // 用整數存 bit
} HuffmanCodeEntry;

typedef struct {
    int symbols[MAX_SYMBOLS]; // 包含的原符號索引
    int count;
} PseudoSymbol;

PseudoSymbol pseudo[MAX_PSEUDO]; // pseudo-symbol 映射表
int pseudo_count = 0;

// 計算出現頻率
int count_frequency(FILE* fin, int * fre_array){
    if (!fin) {
        perror("Cannot open input.txt");
        return 1;
    }
    int c;
    while ((c = fgetc(fin)) != EOF) { // 一個字一個字讀取
        fre_array[c]++;
    }

    // 印出頻率
    for (int i = 0; i < 256; i++) {
        if (fre_array[i] > 0) {
            printf("Char 0x%02X ('%c') : %d\n", i, (i >= 32 && i <= 126) ? i : '.', fre_array[i]);
        }
    }
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
// ---------------- Write Compressed File ----------------
void compress_file_txt(FILE* fin, FILE* fout, char codes[MAX_SYMBOLS][MAX_CODE_LEN]) {
    fseek(fin, 0, SEEK_SET); // rewind

    fprintf(fout, "Huffman Table:\n");
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (codes[i][0] != '\0') {
            if (i >= 32 && i <= 126)
                fprintf(fout, "Char '%c' (%d): %s\n", i, i, codes[i]);
            else
                fprintf(fout, "Char 0x%02X: %s\n", i, codes[i]);
        }
    }

    fprintf(fout, "\nCompressed Data (bits):\n");
    int c;
    while ((c = fgetc(fin)) != EOF) {
        fprintf(fout, "%s", codes[c]);
    }
    fprintf(fout, "\n");
}

void compress_file_bin(FILE* fin, FILE* fout, char codes[MAX_SYMBOLS][MAX_CODE_LEN]) {
    fseek(fin, 0, SEEK_SET); // rewind

    // 1. 寫 Huffman table
    int num_symbols = 0;
    for (int i = 0; i < MAX_SYMBOLS; i++)
        if (codes[i][0] != '\0') num_symbols++;
    
    fputc(num_symbols, fout); // 總共幾個 symbol
    
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (codes[i][0] == '\0') continue;

        unsigned char len = strlen(codes[i]);
        fputc(i, fout);    // symbol
        fputc(len, fout);  // code 長度

        uint32_t bits = 0;
        for (int j = 0; j < len; j++) {
            bits <<= 1;
            if (codes[i][j] == '1') bits |= 1;
        }
        int bytes_needed = (len + 7) / 8;
        for (int b = bytes_needed - 1; b >= 0; b--) {
            fputc((bits >> (8*b)) & 0xFF, fout);
        }
    }

    // 2. 寫壓縮資料
    unsigned char buffer = 0;
    int bit_count = 0;
    int c;
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


// 產生編碼
void generate_codes(HuffmanNode* node, char* code, int depth, char codes[MAX_SYMBOLS][MAX_CODE_LEN]) {
    if (!node) return;

    if (!node->left && !node->right) {
        code[depth] = '\0';
        strncpy(codes[(unsigned char)node->symbol], code, MAX_CODE_LEN-1);
        return;
    }
    if (node->left) {
        code[depth] = '0';
        generate_codes(node->left, code, depth + 1, codes);
    }
    if (node->right) {
        code[depth] = '1';
        generate_codes(node->right, code, depth + 1, codes);
    }
}

// ---------------- Free Huffman Tree ----------------
void free_tree(HuffmanNode* node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

// ----------------- 限制長度生成 Huffman code -----------------
void calculate_code_lengths(HuffmanNode* node, int depth, int lengths[MAX_SYMBOLS]) {
    if (!node) return;

    if (!node->left && !node->right) {
        lengths[node->symbol] =  depth;
        return;
    }
    if (node->left) calculate_code_lengths(node->left, depth + 1, lengths);
    if (node->right) calculate_code_lengths(node->right, depth + 1, lengths);
}

// 取代原本的 fix_code_lengths（簡單、穩定、合法）
// 規則：只針對「出現過的符號」(lengths[i] > 0) 做事；
//      若 k > 2^L → 直接報錯；否則全部設為 L（一定滿足 Kraft，不會解崩）
void fix_code_lengths(int lengths[MAX_SYMBOLS], int limit_L) {
    if (limit_L <= 0) return;

    // 收集「真的有出現」的符號
    int present[MAX_SYMBOLS];
    int k = 0;
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (lengths[i] > 0) {    // 只有葉節點(出現過的符號)會被 calculate_code_lengths() 設到 >0
            present[k++] = i;
        }
    }
    if (k == 0) return;          // 空檔案之類的，就不動作

    // 檢查 L 是否足夠：k <= 2^L
    if (limit_L < 31 && k > (1 << limit_L)) {
        fprintf(stderr, "Error: L=%d 無法編 %d 種符號（需要 L >= ceil(log2(%d))）\n", limit_L, k, k);
        exit(1);
    }

    // 全部設為 L（保證 Kraft，不再出現把 0/1/2 這些沒出現的索引塞進碼表）
    for (int i = 0; i < k; i++) {
        int s = present[i];
        lengths[s] = limit_L;
    }
}



void generate_limited_codes(int lengths[MAX_SYMBOLS],
                            char codes[MAX_SYMBOLS][MAX_CODE_LEN])
{
    int max_len = 0;
    for (int i = 0; i < MAX_SYMBOLS; i++)
        if (lengths[i] > max_len)
            max_len = lengths[i];

    // 計算每種長度的數量
    int bl_count[MAX_CODE_LEN + 1] = {0};
    for (int i = 0; i < MAX_SYMBOLS; i++)
        if (lengths[i] > 0)
            bl_count[lengths[i]]++;

    int next_code[MAX_CODE_LEN + 1] = {0};
    int code = 0;

    // Canonical: 計算每個長度的起始 code
    for (int len = 1; len <= max_len; len++) {
        code = (code + bl_count[len-1]) << 1;
        next_code[len] = code;
    }

    // 產生每個 symbol 的 code
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        int len = lengths[i];
        if (len == 0) {
            codes[i][0] = '\0';
            continue;
        }

        int c = next_code[len]++;

        // 變成字串
        for (int b = len - 1; b >= 0; b--)
            codes[i][len - 1 - b] = ((c >> b) & 1) ? '1' : '0';

        codes[i][len] = '\0';
    }
}


void compress(FILE* fin, FILE* fout, int limit_length){
    int freq[MAX_SYMBOLS] = {0};
    count_frequency(fin, freq);

    HuffmanNode* root = build_huffman_tree(freq);

    // 計算限制長度 code 長度
    int lengths[MAX_SYMBOLS] = {0};
    calculate_code_lengths(root, 0, lengths);
    fix_code_lengths(lengths, limit_length);
    // 生成 code
    char codes[MAX_SYMBOLS][MAX_CODE_LEN];
    for(int i=0;i<MAX_SYMBOLS;i++) codes[i][0] = '\0';
    generate_limited_codes(lengths, codes);

    compress_file_bin(fin, fout, codes);
    free_tree(root);
}

typedef struct {
    unsigned int code;
    unsigned char length;
    unsigned char symbol;
} CodeEntry;

int read_huffman_table(FILE* fin, CodeEntry table[MAX_SYMBOLS]) {
    int num_symbols = fgetc(fin);
    for (int i = 0; i < num_symbols; i++) {
        unsigned char symbol = fgetc(fin);
        unsigned char len = fgetc(fin);
        uint32_t bits = 0;
        int bytes_needed = (len + 7) / 8;
        for (int b = 0; b < bytes_needed; b++) {
            int byte = fgetc(fin);
            bits = (bits << 8) | (byte & 0xFF);
        }
        table[i].symbol = symbol;
        table[i].length = len;
        table[i].code = bits;
    }
    return num_symbols;
}
void print_huffman_table(CodeEntry table[MAX_SYMBOLS], int num_symbols) {
    printf("Huffman Table (%d symbols):\n", num_symbols);
    for (int i = 0; i < num_symbols; i++) {
        printf("Symbol %d ", table[i].symbol);
        if (table[i].symbol >= 32 && table[i].symbol <= 126)
            printf("('%c') ", table[i].symbol);
        printf(": Code length = %d, Code = ", table[i].length);

        // 印 bits
        for (int b = table[i].length - 1; b >= 0; b--) {
            printf("%d", (table[i].code >> b) & 1);
        }
        printf("\n");
    }
}


void decompress_file_bin(FILE* fin, FILE* fout) {
    CodeEntry table[MAX_SYMBOLS];
    int num_symbols = read_huffman_table(fin, table);
    print_huffman_table(table, num_symbols);
    printf("num_symbols: %d\n", num_symbols);

    unsigned char buffer = 0;
    int bits_in_buffer = 0;
    unsigned int bit_acc = 0;   // 暫存 bits
    int bit_count = 0;

    int c;
    while ((c = fgetc(fin)) != EOF) {
        buffer = (unsigned char)c;
        for (int i = 7; i >= 0; i--) {
            bit_acc = (bit_acc << 1) | ((buffer >> i) & 1);
            printf("bit_acc: %u\n", bit_acc);
            bit_count++;
            printf("bit_count: %d", bit_count);
            printf("\n");
            // 嘗試匹配 table
            for (int j = 0; j < num_symbols; j++) {
                uint32_t code_masked = table[j].code & ((1 << table[j].length) - 1);
                if (bit_count == table[j].length && bit_acc == table[j].code) {
                    fputc(table[j].symbol, fout);
                    bit_acc = 0;
                    bit_count = 0;
                    break;
                }
            }
        }
    }
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
    
   

    

    if(mode == MODE_C){
         // 確定輸入檔案存在
        FILE *fin = fopen(inputFile, "rb");
        if (fin == NULL) {
            perror("Error opening input file");
            return 1;
        }

        // 確定輸出檔案存在
        FILE* fout = fopen(outputFile, "wb");
        if (fout == NULL) {
            perror("Error opening output file");
            fclose(fin);
            return 1;
        }
        compress(fin, fout, limit_length);
    }

    else if(mode == MODE_D){
        // 確定輸出檔案存在
        FILE* fin = fopen(inputFile, "rb");
        if (fin == NULL) {
            perror("Error opening output file");
            fclose(fin);
            return 1;
        }
        // 確定輸出檔案存在
        FILE* fout = fopen(outputFile, "wb");
        if (fout == NULL) {
            perror("Error opening output file");
            fclose(fout);
            return 1;
        }
        decompress_file_bin(fin, fout);
    }


    

    return 0;
}
