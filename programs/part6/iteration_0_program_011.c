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
__attribute__((constructor))
static void init_asan_early(void) {
    volatile char buf1[256], buf2[256];
    
    /* Force memcpy redirection in constructor */
    __builtin_memcpy(buf1, "constructor_init", 16);
    __builtin_memset(buf2, 0xAA, sizeof(buf2));
    
    /* Use goto to create complex control flow */
    if (buf1[0] != 0) {
        goto mem_ops;
    }
    
    return;
    
mem_ops:
    __builtin_memmove(buf1 + 8, buf1, 8);
    return;
}

/* Destructor for cleanup coordination */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    struct ast_node *node = malloc(sizeof(*node));
    if (!node) return NULL;
    
    /* Use builtins with volatile lengths */
    __builtin_memset(node, 0, sizeof(*node));
    __builtin_memcpy(node->data, base_data, strlen(base_data) + 1);
    
    node->type = depth;
    
    /* Recursive construction with memory copies between nodes */
    node->left = build_ast(depth - 1, "left_branch");
    node->right = build_ast(depth - 1, "right_branch");
    
    if (node->left && node->right) {
        /* Copy data between child nodes */
        __builtin_memcpy(node->left->data + 32, 
                        node->right->data, 
                        g_memcpy_len % 64);
    }
    
    return node;
}

/* Function with goto jumping into memory operation blocks */
static void complex_control_flow(char *dest, const char *src, size_t len) {
    int use_memmove = 1;
    
    /* Jump into memory operation block */
    if (len > 50) {
        goto do_memmove;
    }
    
    __builtin_memcpy(dest, src, len);
    return;
    
do_memmove:
    /* This block should be reached via goto */
    volatile char temp[512];
    __builtin_memcpy(temp, src, len);
    
    if (use_memmove) {
        __builtin_memmove(dest, temp, len);
    } else {
        goto do_memset;
    }
    return;
    
do_memset:
    __builtin_memset(dest, 0xCC, len);
}

/* OpenMP parallel section with memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[1024];
        char shared_buf[1024];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            volatile size_t len = (i * 7 + tid) % 256;
            
            /* Mix of all three builtins */
            if (i % 3 == 0) {
                __builtin_memcpy(shared_buf + i * 8, 
                               local_buf + i * 4, 
                               len);
            } else if (i % 3 == 1) {
                __builtin_memset(shared_buf + i * 8, 
                               i, 
                               len);
            } else {
                __builtin_memmove(shared_buf + i * 8,
                                shared_buf + (i-1) * 8,
                                len);
            }
        }
    }
}

/* Multi-stage initialization with memory builtins */
static void initialize_token_array(char tokens[][256], int count) {
    for (int i = 0; i < count; i++) {
        char pattern[256];
        
        /* Create pattern using memset */
        __builtin_memset(pattern, 'A' + (i % 26), sizeof(pattern));
        
        /* Copy to token array */
        __builtin_memcpy(tokens[i], pattern, sizeof(pattern));
        
        /* Occasionally use memmove for overlapping regions */
        if (i > 0 && i % 5 == 0) {
            __builtin_memmove(tokens[i] + 64,
                            tokens[i-1] + 32,
                            g_memmove_len % 128);
        }
    }
}

int main(void) {
    char tokens[10][256];
    long long hash = 0;
    
    /* Stage 1: Initialize token array with builtins */
    initialize_token_array(tokens, 10);
    
    /* Stage 2: Build recursive AST structure */
    struct ast_node *root = build_ast(4, "root_node");
    
    /* Stage 3: Complex control flow with gotos */
    for (int i = 0; i < 5; i++) {
        complex_control_flow(tokens[i], tokens[9-i], 
                           g_memcpy_len % 200);
    }
    
    /* Stage 4: Parallel memory operations */
    parallel_mem_ops();
    
    /* Stage 5: Process results with more builtins */
    char result_buf[2048];
    __builtin_memset(result_buf, 0, sizeof(result_buf));
    
    /* Combine all tokens */
    for (int i = 0; i < 10; i++) {
        __builtin_memcpy(result_buf + i * 256,
                        tokens[i],
                        256);
    }
    
    /* Calculate verification hash */
    for (size_t i = 0; i < sizeof(result_buf); i++) {
        hash += (long long)result_buf[i] * (i + 1);
    }
    
    /* Final memory operations in main */
    volatile char final_check[512];
    __builtin_memset(final_check, 0x55, sizeof(final_check));
    __builtin_memcpy(final_check + 128, result_buf, 256);
    __builtin_memmove(final_check, final_check + 64, 128);
    
    printf("Verification hash: %lld\n", hash);
    
    /* Cleanup */
    free(root);
    
    return 0;
}
