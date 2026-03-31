/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i * 7) & 0xFF);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = create_ast_node(depth - 1, id * 2);
    node->right = create_ast_node(depth - 1, id * 2 + 1);
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill with pattern using volatile size */
    for (volatile int i = 0; i < (int)(g_mem_size % 256); i++) {
        node->data[i] = (char)((id + i) & 0xFF);
    }
    
    return node;
}

/* Complex function with goto and memory operations */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    volatile int condition = src->id % 3;
    
    if (condition == 0) {
        goto copy_block;
    } else if (condition == 1) {
        goto move_block;
    } else {
        goto set_block;
    }
    
copy_block:
    /* Force __builtin_memcpy redirection */
    __builtin_memcpy(dst->data, src->data, 
                     (size_t)(g_mem_size % sizeof(src->data)));
    goto next_op;
    
move_block:
    /* Force __builtin_memmove redirection with overlap */
    {
        volatile size_t move_len = (size_t)(g_mem_size % 128);
        __builtin_memmove(dst->data + 32, dst->data, move_len);
    }
    goto next_op;
    
set_block:
    /* Force __builtin_memset redirection */
    __builtin_memset(dst->data, 0xFF, 
                     (size_t)(g_mem_size % sizeof(dst->data)));
    goto next_op;
    
next_op:
    /* Additional operation after goto */
    if (dst->id % 2 == 0) {
        __builtin_memcpy(dst->data + 64, src->data, 32);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    volatile char buffer1[512];
    volatile char buffer2[512];
    volatile char buffer3[512];
    
    #pragma omp parallel
    {
        int tid = 0;
        #ifdef _OPENMP
        tid = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        switch (tid % 3) {
            case 0:
                __builtin_memset((void*)buffer1, tid, 
                                (size_t)(g_mem_size % 512));
                __builtin_memcpy((void*)buffer2, (void*)buffer1, 256);
                break;
            case 1:
                __builtin_memmove((void*)(buffer3 + 128), 
                                 (void*)buffer3, 384);
                break;
            case 2:
                __builtin_memcpy((void*)buffer1, (void*)buffer3, 
                                (size_t)(g_mem_size % 256));
                __builtin_memset((void*)buffer2, 0xAA, 128);
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Collective operation */
        #pragma omp single
        {
            __builtin_memcpy((void*)buffer1, (void*)buffer2, 128);
        }
    }
}

/* Multi-stage initialization function */
static void initialize_test_environment(void) {
    volatile static int initialized = 0;
    
    if (!initialized) {
        /* Force multiple memory built-in calls */
        char init_buf[1024];
        __builtin_memset(init_buf, 0, sizeof(init_buf));
        
        /* Copy from token pool using volatile index */
        __builtin_memcpy(init_buf, g_token_pool + g_token_index, 256);
        
        /* Move within buffer */
        __builtin_memmove(init_buf + 512, init_buf, 256);
        
        initialized = 1;
    }
}

/* Main test execution */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Stage 1: Environment setup */
    initialize_test_environment();
    
    /* Stage 2: Create and manipulate AST */
    ASTNode* root = create_ast_node(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
    if (!copy) {
        free(root);
        return 1;
    }
    
    /* Stage 3: Goto-based processing */
    process_with_goto(root, copy);
    
    /* Stage 4: Parallel operations */
    parallel_memory_ops();
    
    /* Stage 5: Additional built-in calls in loop */
    volatile char final_buffer[1024];
    for (volatile int i = 0; i < 10; i++) {
        switch (i % 3) {
            case 0:
                __builtin_memset(final_buffer, i, 
                                (size_t)(g_mem_size % 1024));
                break;
            case 1:
                __builtin_memcpy(final_buffer + i * 64, 
                                g_token_pool, 64);
                break;
            case 2:
                __builtin_memmove(final_buffer, 
                                 final_buffer + 256, 512);
                break;
        }
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 1024; i++) {
        hash = (hash * 31) + (unsigned char)final_buffer[i];
    }
    
    printf("Test completed. Verification hash: %lu\n", hash);
    
    /* Cleanup */
    free(root);
    free(copy);
    
    return 0;
}
