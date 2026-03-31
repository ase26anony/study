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
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force early builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    volatile char final_check[16];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(*node));
    if (!node) return NULL;
    
    /* Use all three builtins with volatile lengths */
    __builtin_memset(node->data, depth, g_memset_len % 256);
    
    /* Create pattern in first half */
    char pattern[128];
    __builtin_memset(pattern, 0xCC, sizeof(pattern));
    __builtin_memcpy(node->data, pattern, g_memcpy_len % 128);
    
    node->type = depth;
    node->left = build_ast(depth - 1);
    node->right = build_ast(depth - 2);
    
    /* Copy between child nodes if they exist */
    if (node->left && node->right) {
        __builtin_memmove(node->left->data, 
                         node->right->data, 
                         g_memmove_len % 256);
    }
    
    return node;
}

/* Function with goto edge cases */
static void goto_memory_operations(void) {
    volatile char src[256], dst[256];
    int use_memmove = 1;
    
    __builtin_memset(src, 0x11, sizeof(src));
    
    /* Jump into block with builtin */
    if (use_memmove) goto do_memmove;
    
    normal_path:
    __builtin_memcpy(dst, src, g_memcpy_len);
    return;
    
    do_memmove:
    /* This tests flow sensitivity */
    __builtin_memmove(dst, src, g_memmove_len);
    goto normal_path;
}

/* OpenMP parallel section */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        volatile char private_buf[512];
        
        /* Each thread uses builtins */
        __builtin_memset(private_buf, tid, sizeof(private_buf));
        
        #pragma omp barrier
        
        /* Copy between staggered positions */
        if (tid % 2 == 0) {
            __builtin_memcpy(private_buf + 128, 
                           private_buf, 
                           g_memcpy_len);
        } else {
            __builtin_memmove(private_buf, 
                            private_buf + 64, 
                            g_memmove_len);
        }
    }
}

/* Complex token processing */
static unsigned long process_tokens(const char* tokens[], int count) {
    unsigned long hash = 0xDEADBEEF;
    volatile char accum[1024] = {0};
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        __builtin_memcpy(accum + (i * 64 % 1024), 
                        tokens[i], 
                        len < 64 ? len : 64);
        
        /* Overlapping move */
        if (i > 0) {
            __builtin_memmove(accum + 512, 
                            accum + 256, 
                            g_memmove_len % 256);
        }
        
        /* Compute hash with builtin help */
        char tmp[256];
        __builtin_memset(tmp, 0, sizeof(tmp));
        __builtin_memcpy(tmp, accum, sizeof(tmp) < 256 ? sizeof(tmp) : 256);
        
        for (int j = 0; j < 256; j++) {
            hash = (hash * 31) + tmp[j];
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* 1. Initialize complex token array */
    const char* tokens[] = {
        "memcpy_test_token_1",
        "memset_test_token_2", 
        "memmove_test_token_3",
        "asan_redirect_token_4",
        "hwasan_branch_token_5"
    };
    
    /* 2. Build recursive AST */
    struct ast_node* root = build_ast(5);
    
    /* 3. Execute goto edge cases */
    goto_memory_operations();
    
    /* 4. Parallel memory operations */
    parallel_memory_ops();
    
    /* 5. Process tokens with memory operations */
    unsigned long result = process_tokens(tokens, 5);
    
    /* 6. Additional direct builtin calls */
    volatile char final_buffer[2048];
    __builtin_memset(final_buffer, 0x55, sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 1024, final_buffer, 1024);
    __builtin_memmove(final_buffer + 512, final_buffer + 1536, 512);
    
    printf("Result hash: 0x%08lx\n", result);
    printf("Test completed - check ASAN/HWASAN instrumentation\n");
    
    /* Cleanup */
    free(root);
    
    return 0;
}
