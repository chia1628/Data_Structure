#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MODE_NONE 0
#define MODE_C    1
#define MODE_D    2
#define MAX_SYMBOLS 256
#define MAX_CODE_LEN 255

#define DEBUG_HUF 1  // 啟用除錯輸出時打開

typedef struct Node {
    int symbol;               // 0..255 (leaf), -1 for internal
    uint64_t freq;            // frequency (weight)
    struct Node *left, *right;
} Node;

typedef struct {
    uint32_t code;            // canonical code bits (MSB-first in length)
    uint8_t  length;          // code length in bits
    int      valid;           // 0/1
} Code;

typedef struct {
    uint8_t symbol;
    uint8_t length;
} TableEntry;

// ---- Build info ----
static void print_build_info(void){
    fprintf(stderr, "[HUF build] %s %s\n", __DATE__, __TIME__);
}

// ---- Utils ----
static int cmp_freq_asc(const void *a, const void *b) {
    const Node *x = *(const Node* const*)a, *y = *(const Node* const*)b;
    if (x->freq < y->freq) return -1;
    if (x->freq > y->freq) return 1;
    return x->symbol - y->symbol;
}

static Node* make_leaf(int s, uint64_t f){
    Node* n = (Node*)malloc(sizeof(Node));
    n->symbol = s; n->freq = f; n->left = n->right = NULL;
    return n;
}

static Node* make_parent(Node* a, Node* b){
    Node* p = (Node*)malloc(sizeof(Node));
    p->symbol = -1; p->freq = a->freq + b->freq; p->left = a; p->right = b;
    return p;
}

static void free_tree(Node* n){
    if(!n) return;
    free_tree(n->left); free_tree(n->right); free(n);
}

static void count_freq(FILE* fin, uint64_t freq[256], uint64_t *total_bytes){
    int c;
    while((c = fgetc(fin)) != EOF){
        freq[(uint8_t)c]++; (*total_bytes)++;
    }
}

// 驗證 Kraft 不等式；若違反，回傳 0
static int kraft_ok(const int lengths[256]){
    double s = 0.0;
    for (int i = 0; i < 256; i++) {
        if (lengths[i] > 0) {
            int l = lengths[i];
            if (l <= 0 || l > 30) return 0;
            s += 1.0 / (1u << l);
            if (s > 1.0000001) return 0;
        }
    }
    return 1;
}

static Node* build_huffman_from_freq(uint64_t freq[256], int *unique){
    Node* pool[256];
    int n = 0;
    for(int i=0;i<256;i++){
        if(freq[i]>0){ pool[n++] = make_leaf(i, freq[i]); }
    }
    *unique = n;
    if(n==0) return NULL;
    if(n==1){
        // edge case: 單一符號 → 做個假父節點，碼長為 1
        Node* a = pool[0];
        Node* dummy = make_leaf(-2, 0);
        return make_parent(dummy, a);
    }
    // 正確的「取兩最小、合併回陣列」迴圈
    while (n > 1) {
        qsort(pool, n, sizeof(pool[0]), cmp_freq_asc);
        Node* a = pool[0];
        Node* b = pool[1];
        Node* p = make_parent(a, b);
        pool[0] = p;          // 放回 parent
        pool[1] = pool[n-1];  // 用最後一個補位
        n--;
    }
    return pool[0];
}

// 走樹得到初始碼長（未限制）
static void gather_lengths(Node* n, int depth, int lengths[256]){
    if(!n) return;
    if(!n->left && !n->right){
        if(n->symbol>=0 && n->symbol<256){
            lengths[n->symbol] = depth? depth:1; // 單一符號時給 1
        }
        return;
    }
    gather_lengths(n->left,  depth+1, lengths);
    gather_lengths(n->right, depth+1, lengths);
}

// --- 碼長限制（穩定修正 + 保底） ---
static uint64_t __freq_for_sort_[256];

static int cmp_freq_desc_then_symbol_asc_int(const void* A, const void* B){
    int x = *(const int*)A;
    int y = *(const int*)B;
    if (__freq_for_sort_[x] > __freq_for_sort_[y]) return -1;
    if (__freq_for_sort_[x] < __freq_for_sort_[y]) return 1;
    return x - y;
}

// 以現有初始碼長 lengths[] + 頻率 freq[] 為基礎，強制所有碼長落在 1..L
static void enforce_length_limit(int lengths[256], int L, uint64_t freq[256]){
    if(L<=0) return;

    int present[256], n=0;
    for(int i=0;i<256;i++){
        if(lengths[i]>0){ present[n++]=i; }
    }
    if(n==0) return;

    int bl_count[MAX_CODE_LEN+1]={0};
    int maxlen=0;
    for(int i=0;i<n;i++){
        int s=present[i];
        if(lengths[s]<1) lengths[s]=1;
        if(lengths[s]>MAX_CODE_LEN) lengths[s]=MAX_CODE_LEN;
        bl_count[lengths[s]]++;
        if(lengths[s]>maxlen) maxlen=lengths[s];
    }

    if(maxlen > L){
        // 把超過 L 的葉子往上搬：每搬 1 個 len=t 的葉子 → t-1 增加 2 個
        for(int t=maxlen; t>L; --t){
            while(bl_count[t] > 0){
                bl_count[t] -= 1;
                bl_count[t-1] += 2;
            }
        }
    }

    // 調整總葉數 = n
    int leaves=0; for(int t=1;t<=L;t++) leaves+=bl_count[t];

    while(leaves > n){
        // 從最深層往上合併兩葉→上一層 1 葉
        int t=L; while(t>1 && bl_count[t]<2) --t;
        if(t<=1) break;
        bl_count[t]   -= 2;
        bl_count[t-1] += 1;
        leaves -= 1;
    }
    while(leaves < n){
        // 從較淺處拆 1 葉 → 下一層 2 葉
        int t=2; while(t<=L && bl_count[t]==0) ++t;
        if(t>L){
            bl_count[L] += (n - leaves);
            leaves = n; break;
        }
        bl_count[t]   -= 1;
        bl_count[t+1] += 2;
        leaves += 1;
    }

    // 依頻率高→低，配發較短碼長
    for(int i=0;i<256;i++) __freq_for_sort_[i]=freq[i];
    int order[256];
    for(int i=0;i<n;i++) order[i]=present[i];
    qsort(order, n, sizeof(order[0]), cmp_freq_desc_then_symbol_asc_int);

    // 清空並重新賦碼長
    for(int i=0;i<n;i++) lengths[present[i]] = 0;

    int idx=0;
    for(int t=1;t<=L;t++){
        for(int k=0;k<bl_count[t] && idx<n; k++, idx++){
            lengths[order[idx]] = t;
        }
    }
    // 保底：若還有任何出現過的符號 length==0，且 k <= 2^L，全部改為長度 L
    int k=0, missing=0;
    for(int i=0;i<256;i++) if(freq[i]>0) k++;
    for(int i=0;i<256;i++) if(freq[i]>0 && lengths[i]==0){ missing=1; break; }
    if(missing){
        if(L < 31 && k <= (1<<L)){
            for(int i=0;i<256;i++) if(freq[i]>0) lengths[i] = (L>0?L:1);
        }
    }
}

// ---- Canonical code ----
static void build_canonical_codes(const int lengths[256], Code out[256]){
    int maxlen=0, bl_count[MAX_CODE_LEN+1]={0};
    for(int i=0;i<256;i++){
        if(lengths[i]>0){ bl_count[lengths[i]]++; if(lengths[i]>maxlen) maxlen=lengths[i]; }
        out[i].valid = 0;
    }
    uint32_t next_code[MAX_CODE_LEN+1]={0};
    uint32_t code=0;
    for(int len=1; len<=maxlen; ++len){
        code = (code + bl_count[len-1]) << 1;
        next_code[len] = code;
    }
    for(int s=0;s<256;s++){
        int l = lengths[s];
        if(l>0){
            out[s].length = (uint8_t)l;
            out[s].code   = next_code[l]++;
            out[s].valid  = 1;
        }
    }
}

// ---- Decode trie ----
typedef struct DNode {
    int symbol; // -1 internal, >=0 leaf
    struct DNode *child[2];
} DNode;

static DNode* dnode_new(void){ DNode* n=(DNode*)calloc(1,sizeof(DNode)); n->symbol=-1; return n; }

static DNode* build_decode_trie(const Code table[256]){
    DNode* root = dnode_new();
    for(int s=0;s<256;s++){
        if(!table[s].valid) continue;
        uint32_t code = table[s].code;
        int len = table[s].length;
        DNode* cur = root;
        for(int i=len-1;i>=0;i--){
            int bit = (code>>i)&1;
            if(!cur->child[bit]) cur->child[bit]=dnode_new();
            cur = cur->child[bit];
        }
        cur->symbol = s;
    }
    return root;
}

static void free_decode_trie(DNode* n){
    if(!n) return;
    free_decode_trie(n->child[0]);
    free_decode_trie(n->child[1]);
    free(n);
}

// ---- Bit IO ----
typedef struct {
    FILE* f;
    uint8_t buf;
    int bits;
} BitW;

static void bw_init(BitW* bw, FILE* f){ bw->f=f; bw->buf=0; bw->bits=0; }
static void bw_put(BitW* bw, uint32_t code, int len){
    for(int i=len-1;i>=0;i--){
        int b = (code>>i)&1;
        bw->buf = (bw->buf<<1) | (b&1);
        bw->bits++;
        if(bw->bits==8){ fputc(bw->buf, bw->f); bw->buf=0; bw->bits=0; }
    }
}
static void bw_flush(BitW* bw){
    if(bw->bits>0){
        bw->buf <<= (8-bw->bits);
        fputc(bw->buf, bw->f);
        bw->buf=0; bw->bits=0;
    }
}

typedef struct {
    FILE* f;
    uint8_t buf;
    int bits;
} BitR;

static void br_init(BitR* br, FILE* f){ br->f=f; br->buf=0; br->bits=0; }
static int br_get(BitR* br, int* out_bit){
    if(br->bits==0){
        int c=fgetc(br->f);
        if(c==EOF) return 0;
        br->buf=(uint8_t)c; br->bits=8;
    }
    int bit=(br->buf>>7)&1;
    br->buf<<=1; br->bits--;
    *out_bit=bit;
    return 1;
}

// ---- Header IO ----
static void dump_lengths(const int lengths[256]){
#ifdef DEBUG_HUF
    fprintf(stderr, "[DEBUG] code lengths (symbol:length):\n");
    for(int i=0;i<256;i++){
        if(lengths[i]>0) fprintf(stderr, "%d:%d ", i, lengths[i]);
    }
    fprintf(stderr, "\n");
#endif
}

static int write_header(FILE* fo, uint32_t original_size, uint8_t limit_L, const int lengths[256]){
    if(fwrite("HUF1",1,4,fo)!=4) return -1;
    if(fwrite(&original_size, sizeof(original_size),1,fo)!=1) return -1;
    if(fwrite(&limit_L, sizeof(limit_L),1,fo)!=1) return -1;
    TableEntry entries[256]; int n=0;
    for(int i=0;i<256;i++){
        if(lengths[i]>0){
            entries[n].symbol=(uint8_t)i;
            entries[n].length=(uint8_t)lengths[i];
            n++;
        }
    }
    uint16_t num = (uint16_t)n;
    if(fwrite(&num, sizeof(num),1,fo)!=1) return -1;
    if(n>0 && fwrite(entries, sizeof(entries[0]), n, fo)!=(size_t)n) return -1;
    return 0;
}

static int read_header(FILE* fi, uint32_t *original_size, uint8_t *limit_L, int lengths[256]){
    char magic[4];
    if (fread(magic,1,4,fi) != 4) return -1;

#ifdef DEBUG_HUF
    fprintf(stderr, "[HDR] magic = %c%c%c%c\n", magic[0], magic[1], magic[2], magic[3]);
#endif

    if (memcmp(magic, "HUF1", 4) != 0) return -2;

    if (fread(original_size, sizeof(*original_size), 1, fi) != 1) return -1;
#ifdef DEBUG_HUF
    fprintf(stderr, "[HDR] original_size = %u\n", *original_size);
#endif

    if (fread(limit_L, sizeof(*limit_L), 1, fi) != 1) return -1;
#ifdef DEBUG_HUF
    fprintf(stderr, "[HDR] limit_L = %u\n", *limit_L);
#endif

    uint16_t num = 0;
    if (fread(&num, sizeof(num), 1, fi) != 1) return -1;
#ifdef DEBUG_HUF
    fprintf(stderr, "[HDR] num_symbols = %u\n", num);
#endif

    memset(lengths, 0, 256 * sizeof(int));

    for (uint16_t i = 0; i < num; i++) {
        TableEntry te;
        if (fread(&te, sizeof(te), 1, fi) != 1) return -1;

        lengths[te.symbol] = te.length;

#ifdef DEBUG_HUF
        if (te.symbol >= 32 && te.symbol <= 126)
            fprintf(stderr, "[HDR] symbol '%c' (0x%02X), length = %u\n", 
                    te.symbol, te.symbol, te.length);
        else
            fprintf(stderr, "[HDR] symbol 0x%02X, length = %u\n", 
                    te.symbol, te.length);
#endif
    }

    return 0;
}


// ---- Build code from lengths ----
static void build_code_from_lengths(const int lengths[256], Code table[256]){
    build_canonical_codes(lengths, table);
}

// ---- Compress / Decompress ----
static int do_compress(const char* inpath, const char* outpath, int limit_L){
    FILE* fi=fopen(inpath,"rb"); if(!fi){ perror("open input"); return 1; }
    FILE* fo=fopen(outpath,"wb"); if(!fo){ perror("open output"); fclose(fi); return 1; }

    uint64_t freq[256]={0}, total=0;
    count_freq(fi, freq, &total);
    if(total==0){
        int lengths[256]={0};
        if(write_header(fo,0,(uint8_t)(limit_L>0?limit_L:0),lengths)!=0){
            fprintf(stderr,"write header failed\n");
        }
        fclose(fi); fclose(fo); return 0;
    }
    // L 可行性檢查：出現的符號數 k 是否 <= 2^L
    int k = 0; for(int i=0;i<256;i++) if(freq[i]>0) k++;
    if(limit_L>0 && limit_L<31 && k > (1<<limit_L)){
        fprintf(stderr, "Error: L=%d 不足以編碼 %d 種符號（需要 L >= ceil(log2(%d)) ）\n",
                limit_L, k, k);
        fclose(fi); fclose(fo); return 1;
    }

    int unique=0;
    Node* root = build_huffman_from_freq(freq, &unique);

    int lengths[256]={0};
    gather_lengths(root, 0, lengths);

    if(limit_L>0) enforce_length_limit(lengths, limit_L, freq);

#ifdef DEBUG_HUF
    {
        int ns=0, maxL=0;
        for (int i=0;i<256;i++){
            if (lengths[i]>0){ ns++; if (lengths[i]>maxL) maxL=lengths[i]; }
        }
        fprintf(stderr, "[DEBUG-C] about to write header: entries=%d, maxLen=%d\n", ns, maxL);
    }
#endif

    Code table[256]; build_code_from_lengths(lengths, table);

    if(write_header(fo, (uint32_t)total, (uint8_t)(limit_L>0?limit_L:0), lengths)!=0){
        fprintf(stderr,"write header failed\n");
        free_tree(root); fclose(fi); fclose(fo); return 1;
    }

    BitW bw; bw_init(&bw, fo);
    fseek(fi,0,SEEK_SET);
    for(uint64_t k2=0;k2<total;k2++){
        int c = fgetc(fi);
        Code cd = table[(uint8_t)c];
        bw_put(&bw, cd.code, cd.length);
    }
    bw_flush(&bw);

    free_tree(root);
    fclose(fi); fclose(fo);
    return 0;
}

static int do_decompress(const char* inpath, const char* outpath){
    FILE* fi=fopen(inpath,"rb"); if(!fi){ perror("open input"); return 1; }
    FILE* fo=fopen(outpath,"wb"); if(!fo){ perror("open output"); fclose(fi); return 1; }

    uint32_t original_size=0; uint8_t limit_L=0; int lengths[256]={0};
    if(read_header(fi,&original_size,&limit_L,lengths)!=0){
        fprintf(stderr,"invalid or corrupted header\n");
        fclose(fi); fclose(fo); return 1;
    }

#ifdef DEBUG_HUF
    {
        Code tmp[256]; build_code_from_lengths(lengths, tmp);
        int maxL=0, cnt=0;
        for(int i=0;i<256;i++){
            if(tmp[i].valid){
                if(tmp[i].length>maxL) maxL=tmp[i].length;
                cnt++;
            }
        }
        fprintf(stderr,"[DEBUG] entries=%d, maxLen=%d, original_size=%u, L=%u\n",
                cnt, maxL, (unsigned)original_size, (unsigned)limit_L);
    }
#endif

    Code table[256]; build_code_from_lengths(lengths, table);
    DNode* trie = build_decode_trie(table);

    BitR br; br_init(&br, fi);
    uint32_t written=0; DNode* cur=trie; int bit;
    while(written < original_size){
        if(!br_get(&br, &bit)){
            fprintf(stderr,"unexpected EOF in bitstream\n");
            free_decode_trie(trie); fclose(fi); fclose(fo); return 1;
        }
        cur = cur->child[bit];
        if(!cur){ fprintf(stderr,"decode trie error\n");
            free_decode_trie(trie); fclose(fi); fclose(fo); return 1;
        }
        if(cur->symbol>=0){
            fputc(cur->symbol, fo);
            written++;
            cur = trie;
        }
    }

    free_decode_trie(trie);
    fclose(fi); fclose(fo);
    return 0;
}

// ---- CLI ----
static void print_usage(const char* prog){
    fprintf(stderr,
        "Usage:\n"
        "  %s -c -i <input.txt> -o <output.bin> [-l L]\n"
        "  %s -d -i <input.bin> -o <output.txt>\n"
        "Note: -l only allowed with -c (L>0)\n", prog, prog);
}

int main(int argc, char* argv[]){
    print_build_info();
    int mode=MODE_NONE; char *infile=NULL, *outfile=NULL; int limit_L=0;
    int opt;
    while((opt=getopt(argc, argv, "cdi:o:l:"))!=-1){
        switch(opt){
            case 'c': if(mode!=MODE_NONE){ fprintf(stderr,"Error: mode already specified\n"); return 1;} mode=MODE_C; break;
            case 'd': if(mode!=MODE_NONE){ fprintf(stderr,"Error: mode already specified\n"); return 1;} mode=MODE_D; break;
            case 'i': infile=optarg; break;
            case 'o': outfile=optarg; break;
            case 'l': limit_L=atoi(optarg); break;
            default: print_usage(argv[0]); return 1;
        }
    }
    if(mode==MODE_NONE){ fprintf(stderr,"Error: -c or -d must be specified\n"); return 1; }
    if(!infile){ fprintf(stderr,"Error: -i <input file> is required\n"); return 1; }
    if(!outfile){ fprintf(stderr,"Error: -o <output file> is required\n"); return 1; }
    if(mode==MODE_D && limit_L>0){ fprintf(stderr,"Error: -l is only valid with -c\n"); return 1; }
    if(mode==MODE_C && limit_L<0){ fprintf(stderr,"Error: L must be positive\n"); return 1; }

    if(mode==MODE_C) return do_compress(infile, outfile, limit_L);
    else             return do_decompress(infile, outfile);
}
