/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    int type;
    char data[64];
    struct ast_node* left;
    struct ast_node* right;
    struct ast_node* parent;
} ast_node_t;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive tree manipulation with memory operations */
static ast_node_t* create_ast_node(int depth) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    node->type = depth;
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) {
        pattern[i] = (char)((i + depth) & 0xFF);
    }
    __builtin_memcpy(node->data, pattern, 64);
    
    /* Recursive creation */
    node->left = create_ast_node(depth - 1);
    node->right = create_ast_node(depth - 1);
    
    return node;
}

/* Complex memory operation with goto flow control */
static void complex_memory_ops(void* dest, void* src, size_t n) {
    volatile int use_memmove = 1;
    void* temp_buffer = malloc(n * 2);
    
    if (!temp_buffer) return;
    
    /* Label for goto jumps */
    memmove_block:
    if (use_memmove) {
        /* Force __builtin_memmove usage */
        __builtin_memmove(temp_buffer, src, n);
        use_memmove = 0;
        goto copy_block;  /* Jump to different block */
    }
    
    copy_block:
    /* Use __builtin_memcpy */
    __builtin_memcpy(dest, temp_buffer, n);
    
    /* Jump back with different condition */
    if (g_init_flag) {
        goto cleanup_block;
    } else {
        goto memmove_block;
    }
    
    cleanup_block:
    /* Final __builtin_memset */
    __builtin_memset(temp_buffer, 0, n * 2);
    free(temp_buffer);
}

/* OpenMP parallel memory operations */
static void parallel_memory_dispatch(void) {
    const int num_threads = 4;
    const size_t block_size = g_mem_size / num_threads;
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        char* local_buf = (char*)malloc(block_size);
        char* shared_buf = (char*)malloc(block_size);
        
        if (local_buf && shared_buf) {
            /* Each thread uses different builtins */
            switch (tid % 3) {
                case 0:
                    __builtin_memset(local_buf, tid, block_size);
                    break;
                case 1:
                    __builtin_memcpy(shared_buf, local_buf, block_size);
                    break;
                case 2:
                    __builtin_memmove(local_buf, shared_buf, block_size);
                    break;
            }
            
            /* Barrier to ensure all threads complete */
            #pragma omp barrier
            
            /* Cross-thread memory operation */
            if (tid == 0) {
                for (int i = 1; i < num_threads; i++) {
                    /* This should trigger ASAN instrumentation */
                    __builtin_memcpy(shared_buf, local_buf, block_size);
                }
            }
        }
        
        free(local_buf);
        free(shared_buf);
    }
}

/* Token processing with varied memory operations */
static unsigned long process_tokens(char** tokens, int count) {
    unsigned long hash = 0;
    char buffer[512];
    volatile size_t buf_size = sizeof(buffer);
    
    for (int i = 0; i < count; i++) {
        size_t token_len = strlen(tokens[i]);
        
        /* Mix of memory operations based on conditions */
        if (i % 3 == 0) {
            __builtin_memset(buffer, 0, buf_size);
            __builtin_memcpy(buffer, tokens[i], token_len);
        } else if (i % 3 == 1) {
            /* Overlapping memory regions to force memmove */
            char* mid_point = buffer + (buf_size / 2);
            __builtin_memcpy(mid_point, tokens[i], token_len);
            __builtin_memmove(buffer, mid_point, token_len);
        } else {
            /* Direct copy */
            __builtin_memcpy(buffer + i * 16, tokens[i], token_len);
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < token_len && j < buf_size; j++) {
            hash = (hash * 31) + buffer[j];
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Initialize complex token array */
    char* tokens[] = {
        "ASAN", "BUILTIN", "MEMCPY", "MEMSET", "MEMMOVE",
        "REDIRECTION", "COVERAGE", "TEST", "PROGRAM"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Phase 2: Create recursive AST structure */
    ast_node_t* ast_root = create_ast_node(3);
    if (ast_root) {
        /* Copy between AST nodes */
        ast_node_t node_copy;
        __builtin_memcpy(&node_copy, ast_root, sizeof(ast_node_t));
        
        /* Move data within node */
        __builtin_memmove(ast_root->data + 16, ast_root->data, 32);
        
        /* Cleanup */
        free(ast_root);
    }
    
    /* Phase 3: Execute complex memory operations with goto */
    char src_data[256];
    char dst_data[256];
    
    /* Initialize source with pattern */
    for (int i = 0; i < 256; i++) {
        src_data[i] = (char)(i & 0xFF);
    }
    
    complex_memory_ops(dst_data, src_data, 256);
    
    /* Phase 4: Parallel memory dispatch */
    parallel_memory_dispatch();
    
    /* Phase 5: Process tokens and compute result */
    unsigned long final_hash = process_tokens(tokens, token_count);
    
    /* Verify operations by printing hash */
    printf("Final hash: %lu\n", final_hash);
    printf("Test completed successfully\n");
    
    return 0;
}
