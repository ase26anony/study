/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force initialization of ASAN runtime */
    volatile char buffer[64];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    volatile char cleanup_buf[32];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_recursive(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for node initialization */
    __builtin_memcpy(node->data, base_data, strlen(base_data) + 1);
    node->size = strlen(base_data) + 1;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node->data + node->size - 1, 0, 
                    sizeof(node->data) - node->size);
    
    /* Recursive creation with goto for control flow */
    int create_left = 1;
    
create_children:
    if (create_left) {
        node->left = create_ast_recursive(depth - 1, base_data);
        create_left = 0;
        goto create_children; /* Jump back */
    } else {
        node->right = create_ast_recursive(depth - 1, base_data);
    }
    
    return node;
}

/* Function with goto jumping into memmove block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    volatile int use_memmove = 1;
    
    if (use_memmove) {
        goto do_memmove;
    }
    
    /* Normal path */
    __builtin_memcpy(dst->data, src->data, src->size);
    return;
    
do_memmove:
    /* Jump into memmove block */
    __builtin_memmove(dst->data, src->data, src->size);
    
    /* Jump out */
    goto finish;
    
    /* Unreachable in normal flow */
    __builtin_memset(dst->data, 0, sizeof(dst->data));
    
finish:
    return;
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    volatile size_t local_size = g_mem_size;
    char* buffers[4];
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        /* Each thread allocates and operates */
        buffers[tid] = malloc(local_size);
        if (buffers[tid]) {
            /* Force built-in usage in parallel region */
            if (tid % 2 == 0) {
                __builtin_memset(buffers[tid], tid, local_size);
            } else {
                char pattern[64];
                __builtin_memset(pattern, 0xCC, sizeof(pattern));
                __builtin_memcpy(buffers[tid], pattern, 
                               sizeof(pattern) < local_size ? 
                               sizeof(pattern) : local_size);
            }
            
            /* Cross-thread memmove */
            #pragma omp barrier
            if (tid > 0) {
                __builtin_memmove(buffers[tid - 1], buffers[tid], 
                                 local_size / 2);
            }
        }
        
        #pragma omp barrier
        
        /* Verify and free */
        if (buffers[tid]) {
            free(buffers[tid]);
        }
    }
}

/* Multi-stage initialization */
static void initialize_token_pool(void) {
    const char* tokens[] = {"func", "var", "const", "ptr", "arr"};
    
    for (size_t i = 0; i < sizeof(tokens)/sizeof(tokens[0]); i++) {
        size_t len = strlen(tokens[i]) + 1;
        volatile size_t offset = g_token_idx;
        
        /* Use all three built-ins in sequence */
        __builtin_memset(g_token_pool + offset, 0, len);
        __builtin_memcpy(g_token_pool + offset, tokens[i], len - 1);
        __builtin_memmove(g_token_pool + offset + 256, 
                         g_token_pool + offset, len);
        
        g_token_idx += len;
    }
}

/* Main execution flow */
int main(void) {
    unsigned long hash = 0;
    
    /* Stage 1: Initialize token pool */
    initialize_token_pool();
    
    /* Stage 2: Create recursive AST */
    ASTNode* root = create_ast_recursive(3, "root_data");
    ASTNode* copy = malloc(sizeof(ASTNode));
    
    if (root && copy) {
        /* Stage 3: Process with goto jumps */
        process_with_goto(root, copy);
        
        /* Stage 4: Parallel operations */
        parallel_memory_ops();
        
        /* Stage 5: Compute verification hash */
        for (size_t i = 0; i < sizeof(copy->data); i++) {
            hash = (hash * 31) + copy->data[i];
        }
        
        /* Additional built-in calls for coverage */
        volatile char final_buf[128];
        __builtin_memset(final_buf, hash & 0xFF, sizeof(final_buf));
        __builtin_memcpy(final_buf + 64, final_buf, 32);
        __builtin_memmove(final_buf, final_buf + 32, 64);
        
        free(copy);
        free(root);
    }
    
    /* Print verification result */
    printf("Verification hash: %lu\n", hash);
    printf("Token pool size: %zu\n", g_token_idx);
    
    return (hash != 0) ? 0 : 1;
}
