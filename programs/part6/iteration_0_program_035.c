/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Recursive AST-like structure */
struct ast_node {
    int type;
    char data[256];
    struct ast_node *left;
    struct ast_node *right;
    struct ast_node *next;
};

/* Constructor function to force early initialization */
__attribute__((constructor)) static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 8);
}

/* Destructor for cleanup coordination */
__attribute__((destructor)) static void cleanup_asan(void) {
    volatile char final_check[16];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive tree manipulation with memory operations */
static struct ast_node* build_tree(int depth, int index) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(*node));
    node->type = index;
    
    /* Fill data with pattern */
    for (int i = 0; i < 255; i++) {
        node->data[i] = (char)((index + i) % 256);
    }
    node->data[255] = '\0';
    
    /* Recursive construction */
    node->left = build_tree(depth - 1, index * 2);
    node->right = build_tree(depth - 1, index * 2 + 1);
    
    /* Copy data between nodes if siblings exist */
    if (node->left && node->right) {
        size_t copy_len = (size_t)(g_memcpy_len % 128);
        if (copy_len > 255) copy_len = 255;
        __builtin_memcpy(node->right->data, node->left->data, copy_len);
    }
    
    return node;
}

/* Function with goto jumps around memmove */
static void goto_memmove_test(char *dest, char *src, size_t len) {
    int use_memmove = 1;
    
    if (len > 100) {
        goto skip_memmove;
    }
    
    /* Jump target with memmove */
memmove_block:
    __builtin_memmove(dest, src, len);
    goto after_memmove;
    
skip_memmove:
    if (use_memmove) {
        use_memmove = 0;
        goto memmove_block;
    }
    
after_memmove:
    /* Additional operation */
    __builtin_memset(dest + len/2, 0xCC, len/4);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[512];
        char shared_buf[1024];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Collective memcpy to shared buffer */
        size_t offset = (size_t)(tid * 64) % 512;
        size_t copy_len = (size_t)(g_memcpy_len % 128);
        __builtin_memcpy(&shared_buf[offset], local_buf, copy_len);
        
        /* Memmove within shared buffer */
        if (tid % 2 == 0) {
            size_t move_len = (size_t)(g_memmove_len % 96);
            __builtin_memmove(&shared_buf[offset + 32], 
                            &shared_buf[offset], 
                            move_len);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Tree construction with recursive memory ops */
    struct ast_node *root = build_tree(4, 1);
    
    /* Phase 2: Goto-based memmove testing */
    char src_buffer[256];
    char dst_buffer[256];
    
    for (int i = 0; i < 256; i++) {
        src_buffer[i] = (char)i;
    }
    
    goto_memmove_test(dst_buffer, src_buffer, g_memmove_len);
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Phase 4: Direct builtin calls with volatile control */
    volatile char final_buffer[1024];
    volatile size_t final_len = g_memset_len;
    
    __builtin_memset(final_buffer, 0x55, final_len);
    __builtin_memcpy(final_buffer + 256, src_buffer, g_memcpy_len);
    __builtin_memmove(final_buffer + 512, final_buffer + 256, g_memmove_len);
    
    /* Verification: Compute simple hash */
    unsigned long long hash = 0;
    for (int i = 0; i < 1024; i++) {
        hash = (hash * 31) + (unsigned char)final_buffer[i];
    }
    
    printf("Final hash: %llu\n", hash);
    
    /* Cleanup */
    /* Note: In real usage, would need proper tree freeing */
    
    return (hash != 0) ? 0 : 1;
}
