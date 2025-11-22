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
// ===== Header: "HUF1" + original_size + L + num_symbols + (symbol, length)*n =====
static int write_header(FILE* fo, uint32_t original_size, uint8_t limit_L, const int lengths[256]) {
    if (fwrite("HUF1", 1, 4, fo) != 4) return -1;
    if (fwrite(&original_size, sizeof(uint32_t), 1, fo) != 1) return -1;
    if (fwrite(&limit_L, 1, 1, fo) != 1) return -1;

    uint16_t num = 0;
    for (int i = 0; i < 256; i++) if (lengths[i] > 0) num++;
    if (fwrite(&num, sizeof(uint16_t), 1, fo) != 1) return -1;

    for (int i = 0; i < 256; i++) {
        if (lengths[i] > 0) {
            unsigned char s = (unsigned char)i;
            unsigned char L  = (unsigned char)lengths[i];
            if (fwrite(&s, 1, 1, fo) != 1) return -1;
            if (fwrite(&L, 1, 1, fo) != 1) return -1;
        }
    }
    return 0;
}


void compress_file_bin(FILE* fin, FILE* fout, char codes[MAX_SYMBOLS][MAX_CODE_LEN],
                       uint32_t original_size, int lengths[256], int limit_length) {
    uint8_t Lhdr = (limit_length > 0) ? (uint8_t)limit_length : 0;
    if (write_header(fout, original_size, Lhdr, lengths) != 0) {
        fprintf(stderr, "write header failed\n");
        exit(1);
    }

    // 寫 bitstream（沿用你原本的 byte 緩衝）
    fseek(fin, 0, SEEK_SET);
    unsigned char buffer = 0;
    int bit_count = 0;
    int c;
    while ((c = fgetc(fin)) != EOF) {
        const char* code = codes[(unsigned char)c];
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
// 只在超過 limit_L 時才套用長度限制；否則不更動
void fix_code_lengths(int lengths[MAX_SYMBOLS], int limit_L) {
    if (limit_L <= 0) return;   // 沒有限制，直接用原碼長

    int present[MAX_SYMBOLS];
    int k = 0, max_len = 0;

    // 蒐集「真的出現過」的符號，並找原本的最大碼長
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (lengths[i] > 0) {
            present[k++] = i;
            if (lengths[i] > max_len) max_len = lengths[i];
        }
    }
    if (k == 0) return;                 // 空檔案之類，不處理
    if (max_len <= limit_L) return;     // ✅ 沒超過限制：完全不更動

    // 底下這段只在「真的超過」時才執行
    if (limit_L < 31 && k > (1 << limit_L)) {
        fprintf(stderr, "Error: L=%d CAN'T ENCODE %d SYMBOLS NEED L >= ceil(log2(%d)\n",
                limit_L, k, k);
        exit(1);
    }

    // 最簡、穩定的保底作法：全部設為 L（一定滿足 Kraft，確保可解）
    for (int i = 0; i < k; i++) {
        lengths[present[i]] = limit_L;
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

    // 算原始長度（直接把 freq 加總）
    uint32_t original_size = 0;
    for (int i = 0; i < MAX_SYMBOLS; i++) original_size += (uint32_t)freq[i];

    HuffmanNode* root = build_huffman_tree(freq);

    int lengths[MAX_SYMBOLS] = {0};
    calculate_code_lengths(root, 0, lengths);
    fix_code_lengths(lengths, limit_length);

    char codes[MAX_SYMBOLS][MAX_CODE_LEN];
    for (int i = 0; i < MAX_SYMBOLS; i++) codes[i][0] = '\0';
    generate_limited_codes(lengths, codes);

    // ✅ 新版：帶 original_size + lengths
    compress_file_bin(fin, fout, codes, original_size, lengths, limit_length);
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
static int read_header(FILE* fi, uint32_t* original_size, uint8_t* limit_L, int lengths[256]) {
    char magic[4];
    if (fread(magic, 1, 4, fi) != 4) return -1;
    if (memcmp(magic, "HUF1", 4) != 0) return -2;
    if (fread(original_size, sizeof(uint32_t), 1, fi) != 1) return -3;
    if (fread(limit_L, 1, 1, fi) != 1) return -4;
    uint16_t num = 0;
    if (fread(&num, sizeof(uint16_t), 1, fi) != 1) return -5;

    memset(lengths, 0, 256 * sizeof(int));
    for (uint16_t i = 0; i < num; i++) {
        unsigned char sym, len;
        if (fread(&sym, 1, 1, fi) != 1) return -6;
        if (fread(&len, 1, 1, fi) != 1) return -7;
        lengths[sym] = (int)len;
    }
    return (int)num;
}

// 由 lengths 依 canonical 規則重建 bit pattern（用你現有的 generate_limited_codes）
static void build_codes_from_lengths(const int lengths[256], CodeEntry table[256], int* out_n) {
    char codes[256][MAX_CODE_LEN];
    for (int i = 0; i < 256; i++) codes[i][0] = '\0';
    generate_limited_codes((int*)lengths, codes);

    int n = 0;
    for (int s = 0; s < 256; s++) {
        if (lengths[s] > 0) {
            unsigned acc = 0;
            for (int k = 0; codes[s][k]; k++) {
                acc = (acc << 1) | (codes[s][k] == '1');
            }
            table[n].symbol = (unsigned char)s;
            table[n].length = (unsigned char)lengths[s];
            table[n].code   = acc;
            n++;
        }
    }
    *out_n = n;
}


void decompress_file_bin(FILE* fin, FILE* fout) {
    // 讀 Header
    uint32_t original_size = 0;
    uint8_t  limitL = 0;
    int lengths[MAX_SYMBOLS] = {0};
    int num_symbols = read_header(fin, &original_size, &limitL, lengths);
    if (num_symbols < 0) {
        fprintf(stderr, "decode header error\n");
        return;
    }

    // 用 lengths 重建 canonical codes
    CodeEntry table[MAX_SYMBOLS];
    build_codes_from_lengths(lengths, table, &num_symbols);

    // 逐位元解碼，直到寫滿 original_size
    uint32_t remain = original_size;
    unsigned int bit_acc = 0;
    int bit_count = 0;
    int c;

    while (remain > 0 && (c = fgetc(fin)) != EOF) {
        unsigned char byte = (unsigned char)c;

        // （可選）debug：看讀到的 byte
        // printf("byte: dec=%3d hex=0x%02X bin=", byte, byte);
        // for (int k = 7; k >= 0; k--) printf("%d", (byte >> k) & 1);
        // printf("\n");

        for (int i = 7; i >= 0 && remain > 0; i--) {
            int bit = (byte >> i) & 1;
            bit_acc = (bit_acc << 1) | bit;
            bit_count++;

            // （可選）debug：看現在 bit_acc（bit_count 位）
            // printf("bit_acc (%2d bits) = ", bit_count);
            // for (int k = bit_count - 1; k >= 0; k--) printf("%d", (bit_acc >> k) & 1);
            // printf("\n");

            // 嘗試匹配
            for (int j = 0; j < num_symbols; j++) {
                if (bit_count == table[j].length && bit_acc == table[j].code) {
                    fputc(table[j].symbol, fout);
                    bit_acc = 0;
                    bit_count = 0;
                    remain--;              // ✅ 解到一個就扣，remain==0 立刻停
                    break;
                }
            }
        }
    }

    if (remain != 0) {
        fprintf(stderr, "ERROR: unexpected EOF, still need %u bytes\n", remain);
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
