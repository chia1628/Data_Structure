#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

// 定義可以執行的模式
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

// 建立碼表
typedef struct {
    unsigned int code;
    unsigned char length;
    unsigned char symbol;
} CodeEntry;

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

//建立Huffman_tree
HuffmanNode* build_huffman_tree(int freq[MAX_SYMBOLS]) { 
    int n = 0; // 葉子數量
    HuffmanNode* nodes[MAX_SYMBOLS];
    // 生成初始節點，使用n個符號就有n個點
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (freq[i] > 0) {
            nodes[n] = (HuffmanNode*)malloc(sizeof(HuffmanNode));
            nodes[n]->symbol = (unsigned char)i;
            nodes[n]->freq = freq[i];
            nodes[n]->left = nodes[n]->right = NULL; //左右都接地
            n++;
        }
    }
    // 合併節點
    while (n > 1) {
        // 找出最小兩個節點
        int min1 = -1, min2 = -1;
        for (int i = 0; i < n; i++) {
            if (min1 == -1 || nodes[i]->freq < nodes[min1]->freq) { // 先找第一個最小的
                min2 = min1; // 讓第二個不用從頭找起
                min1 = i;
            } 
            else if (min2 == -1 || nodes[i]->freq < nodes[min2]->freq) { // 再找第一個最小的
                min2 = i;
            }
        }
        // 找好兩個小的後建立新父節點
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

// 輔助函式：將二進位字串轉換為無符號整數
unsigned int code_to_uint(const char* code_str) {
    unsigned int acc = 0;
    for (int k = 0; code_str[k]; k++) {
        acc = (acc << 1) | (code_str[k] == '1');
    }
    return acc;
}

// 產生編碼
void generate_codes(HuffmanNode* node, char* code, int depth, char codes[MAX_SYMBOLS][MAX_CODE_LEN]) {
    printf("check01\n");
    if (!node){
        printf("check02\n");
        return;
    } 
    printf("check03\n");
    if (!node->left && !node->right) { // 是葉子 -> 加上結尾符號
        printf("leaves\n");
        code[depth] = '\0';
        printf("Symbol 0x%02X, Freq: %u, Depth: %d\n", node->symbol, node->freq, depth);
        for(int i=0; i<depth; i++){
            printf("%c", code[i]);
        }
        printf("\n");
        strncpy(codes[(unsigned char)node->symbol], code, MAX_CODE_LEN-1); // 該symbol在codes的編號存入 code
        return;
    }
    if (node->left) { // 是左邊 -> code 加上0
        printf("left\n");
        code[depth] = '0';
        
        generate_codes(node->left, code, depth + 1, codes);
    }
    if (node->right) { // 是左邊 -> code 加上1
        printf("right\n");
        code[depth] = '1';
        generate_codes(node->right, code, depth + 1, codes);
    }
}

// 產生有限制長度的編碼
void generate_limited_codes(int lengths[MAX_SYMBOLS], char codes[MAX_SYMBOLS][MAX_CODE_LEN]) {
    // 確定最大碼字長度
    int max_len = 0;
    for (int i = 0; i < MAX_SYMBOLS; i++) { 
        if (lengths[i] > max_len) {
            max_len = lengths[i];
        }
    }
        
    // 計算每種長度的數量
    int bl_count[MAX_CODE_LEN + 1] = {0};
    for (int i = 0; i < MAX_SYMBOLS; i++){
        if (lengths[i] > 0){
            bl_count[lengths[i]]++;
        }
    }
        
    // 計算每個長度的起始碼值
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

// 釋放 Huffman Tree 記憶體
void free_tree(HuffmanNode* node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

//  計算現在的每個Huffman code的長度
void calculate_code_lengths(HuffmanNode* node, int depth, int lengths[MAX_SYMBOLS]) {
    if (!node) return;

    if (!node->left && !node->right) {
        lengths[node->symbol] =  depth;
        return;
    }
    if (node->left) calculate_code_lengths(node->left, depth + 1, lengths);
    if (node->right) calculate_code_lengths(node->right, depth + 1, lengths);
}

// 只針對「出現過的符號」(lengths[i] > 0) 做事
// 只在超過 limit_length 時才套用長度限制
// 將每個符號的長度都改成 limit_length
// 否則不更動
int fix_code_lengths(int lengths[MAX_SYMBOLS], int limit_length, char codes[MAX_SYMBOLS][MAX_CODE_LEN]) {
    if (limit_length <= 0) { // 沒有限制，直接用原碼長
        return 0; 
    }
    int present[MAX_SYMBOLS];
    int k = 0, max_len = 0;

    // 蒐集「真的出現過」的符號，並找原本的最大碼長
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (lengths[i] > 0) {
            present[k++] = i;
            if (lengths[i] > max_len) max_len = lengths[i];
        }
    }
    if (k == 0) return 0; // 空檔案不處理
    if (max_len <= limit_length){  // 最大長度小於限制長度
        return 0; 
    }
    else{ // 最長 code 超過 limit_length 時
        if (limit_length < 31 && k > (1 << limit_length)) { // 要表示的符號多於2^L個則告訴使用者做不到
            fprintf(stderr, "Error: L=%d CAN'T ENCODE %d SYMBOLS NEED L >= ceil(log2(%d)\n", limit_length, k, k);
            exit(1);
        }
        // 將每個 code 全部設為 L，確保可解
        for (int i = 0; i < k; i++) {
            lengths[present[i]] = limit_length;
        }
        return 1;     // 沒超過限制：完全不更動
    }
}

// 寫入符號、長度、碼值 並印出
int write_full_header(FILE* fout, uint32_t original_size, const int lengths[256], char codes[256][MAX_CODE_LEN]) {
    // 寫入原始資料大小 original_size
    if (fwrite(&original_size, sizeof(uint32_t), 1, fout) != 1) { 
        return -1;
    }
    // 計算有效符號數量
    uint16_t num = 0;
    for (int i = 0; i < 256; i++) {
        if (lengths[i] > 0) {
            num++;
        }
    }
    // 寫入有效符號數量 num
    if (fwrite(&num, sizeof(uint16_t), 1, fout) != 1) { 
        return -1;
    }
    printf("| Sym | Len | Code (Binary) | Code (Dec) |\n");
    printf("----------------------------------------\n");
    for (int i = 0; i < 256; i++) { 
        if (lengths[i] > 0) {
            unsigned char s = (unsigned char)i;
            unsigned char L  = (unsigned char)lengths[i];
            // 將二進位碼字串轉換為整數碼值
            unsigned int code_value = code_to_uint(codes[i]);
            printf("| 0x%02X | %3d | %-13s | %10u |\n", s, L, codes[i], code_value);
            
            // 寫入符號 (1 byte)
            fwrite(&s, 1, 1, fout);
            // 寫入長度 (1 byte)
            fwrite(&L, 1, 1, fout);
            fwrite(&code_value, sizeof(unsigned int), 1, fout);
        }
    }
    printf("----------------------------------------\n");
    return 0;
}

// 產生壓縮檔案
void compress_file_bin(FILE* fin, FILE* fout, char codes[MAX_SYMBOLS][MAX_CODE_LEN], uint32_t original_size, int lengths[256], int limit_length) {
    uint8_t Lhdr = (limit_length > 0) ? (uint8_t)limit_length : 0;
    // 寫Header
    if ( write_full_header(fout, original_size, lengths, codes)  != 0) {
        fprintf(stderr, "write header failed\n");
        exit(1);
    }
    // 寫 bitstream
    fseek(fin, 0, SEEK_SET);
    unsigned char buffer = 0;
    int bit_count = 0;
    int c;
    while ((c = fgetc(fin)) != EOF) { // 逐字去尋找CODE
        const char* code = codes[(unsigned char)c];
        for (int i = 0; code[i]; i++) {
            buffer <<= 1;
            if (code[i] == '1'){
                buffer |= 1;
            }
            bit_count++;
            if (bit_count == 8) {// 一個BTYE才可以寫入bin file
                fputc(buffer, fout);
                buffer = 0;
                bit_count = 0;
            }
        }
    }
    if (bit_count > 0) {  // 不足八個則補0  ex: 110 -> 110 00000
        buffer <<= (8 - bit_count);
        fputc(buffer, fout);
    }
}

// 進到壓縮模式
void compress(FILE* fin, FILE* fout, int limit_length){
    int freq[MAX_SYMBOLS] = {0};
    count_frequency(fin, freq);

    // 把 freq 加總算原始長度
    uint32_t original_size = 0;
    for (int i = 0; i < MAX_SYMBOLS; i++) {
       original_size += (uint32_t)freq[i]; 
    } 

    int lengths[MAX_SYMBOLS] = {0};
    char codes[MAX_SYMBOLS][MAX_CODE_LEN];
    for (int i = 0; i < MAX_SYMBOLS; i++){ // 初始化所有code
       codes[i][0] = '\0'; 
    } 
    char code_buffer[MAX_CODE_LEN];
    HuffmanNode* root = build_huffman_tree(freq);
    calculate_code_lengths(root, 0, lengths);
    int gen_mode = fix_code_lengths(lengths, limit_length, codes);
    printf("gen_mode: %d\n" , gen_mode);
    generate_limited_codes(lengths, codes);
    compress_file_bin(fin, fout, codes, original_size, lengths, limit_length);
    free_tree(root);
}

// 讀取完整碼表並填充 CodeEntry 陣列
int read_full_header_table(FILE* fbin, uint32_t* original_size, CodeEntry table[256]) {
    // 讀取原始大小 (4 bytes)
    if (fread(original_size, sizeof(uint32_t), 1, fbin) != 1) { 
        return -1;
    } 
    // 讀取有效符號數量 (2 bytes)
    uint16_t num = 0;
    if (fread(&num, sizeof(uint16_t), 1, fbin) != 1) { 
        return -2;
    }

    // 讀取符號、長度、碼值
    for (uint16_t i = 0; i < num; i++) {
        unsigned char sym, len;
        unsigned int code_value;
        
        // 讀取符號 (1 byte)
        if (fread(&sym, 1, 1, fbin) != 1) return -3;
        
        // 讀取長度 (1 byte)
        if (fread(&len, 1, 1, fbin) != 1) return -4;
        
        // 讀取碼值 (4 bytes)
        if (fread(&code_value, sizeof(unsigned int), 1, fbin) != 1) return -5;
        
        // 填充 CodeEntry 結構 去建立symbol code table
        table[i].symbol = sym;
        table[i].length = len;
        table[i].code = code_value;
    }
    return (int)num;
}

// 解壓縮檔案
void decompress_file_bin(FILE* fin, FILE* fout) {
    uint32_t original_size = 0;
    
    // 讀取完整碼表
    CodeEntry table[MAX_SYMBOLS];
    int num_symbols = read_full_header_table(fin, &original_size, table);
    
    if (num_symbols < 0) {
        fprintf(stderr, "decode header error\n");
        return;
    }

    // 逐位元解碼，直到寫滿 original_size(符合原本的長度)
    uint32_t remain = original_size;
    unsigned int bit_acc = 0;
    int bit_count = 0;
    int c;

    while (remain > 0 && (c = fgetc(fin)) != EOF) {
        unsigned char byte = (unsigned char)c;
        for (int i = 7; i >= 0 && remain > 0; i--) {
            int bit = (byte >> i) & 1;
            bit_acc = (bit_acc << 1) | bit;
            bit_count++;

            // 嘗試匹配
            for (int j = 0; j < num_symbols; j++) {
                if (bit_count == table[j].length && bit_acc == table[j].code) {
                    fputc(table[j].symbol, fout);
                    bit_acc = 0;
                    bit_count = 0;
                    remain--;// 解到一個就扣，remain==0 立刻停(符合原本的長度)
                    break;
                }
            }
        }
    }
    if (remain != 0) {
        fprintf(stderr, "ERROR: unexpected EOF, still need %u bytes\n", remain);
    }
}

// 主程式
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
    // 確保進到其中一種模式
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

