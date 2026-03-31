/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 1024;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
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
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i * 7) & 0xFF);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive function using memory builtins */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Use __builtin_memcpy to copy data with goto for flow control */
    volatile int copy_mode = depth % 3;
    
    /* Goto-based control flow around memmove */
    if (g_use_memmove && depth > 2) {
        goto use_memmove;
    }
    
    /* Normal memcpy path */
    __builtin_memcpy(node->data, base_data, 63);
    node->data[63] = '\0';
    goto after_copy;
    
use_memmove:
    /* This tests the memmove redirection with goto */
    char temp[64];
    __builtin_memcpy(temp, base_data, 63);
    temp[63] = '\0';
    __builtin_memmove(node->data, temp, 64);
    
after_copy:
    node->id = depth;
    
    /* Recursive creation with varied memory operations */
    if (depth > 1) {
        char child_data[64];
        __builtin_memset(child_data, 'A' + (depth % 26), 63);
        child_data[63] = '\0';
        
        node->left = create_ast(depth - 1, child_data);
        
        __builtin_memset(child_data, 'a' + (depth % 26), 63);
        node->right = create_ast(depth - 2, child_data);
    }
    
    return node;
}

/* Function that processes AST with memory operations */
static long process_ast(ASTNode* node) {
    if (!node) return 0;
    
    long sum = 0;
    char buffer[128];
    
    /* Complex memory operations on node data */
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, node->data, 64);
    
    /* Conditional memmove with goto */
    if (node->id % 4 == 0) {
        goto do_memmove;
    }
    
    /* Calculate hash/sum */
    for (int i = 0; i < 64; i++) {
        sum += buffer[i];
    }
    goto skip_memmove;
    
do_memmove:
    /* This forces memmove redirection */
    char temp[128];
    __builtin_memcpy(temp, buffer, 64);
    __builtin_memmove(buffer + 32, temp, 64);
    
    for (int i = 32; i < 96; i++) {
        sum += buffer[i];
    }
    
skip_memmove:
    sum += process_ast(node->left);
    sum += process_ast(node->right);
    
    return sum;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf1[256];
        char local_buf2[256];
        
        /* Initialize with builtins */
        __builtin_memset(local_buf1, thread_id, sizeof(local_buf1));
        
        /* Copy between buffers */
        __builtin_memcpy(local_buf2, local_buf1, sizeof(local_buf1));
        
        /* Conditional memmove based on thread ID */
        if (thread_id % 2 == 0) {
            __builtin_memmove(local_buf1 + 128, local_buf2, 128);
        }
        
        /* Use volatile to prevent optimization */
        volatile char* vptr = local_buf1;
        for (size_t i = 0; i < 256; i++) {
            vptr[i] = (char)((vptr[i] + i) & 0xFF);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Basic built-in usage */
    char buffer1[512];
    char buffer2[512];
    
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 256, buffer2, 256);
    
    /* Phase 2: Recursive AST operations */
    ASTNode* root = create_ast(5, "BaseASTNodeData");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    long ast_sum = process_ast(root);
    printf("AST processing sum: %ld\n", ast_sum);
    
    /* Phase 3: OpenMP parallel operations */
    #ifdef _OPENMP
    printf("Running OpenMP parallel memory operations\n");
    parallel_memory_ops();
    #endif
    
    /* Phase 4: Token pool operations with gotos */
    volatile size_t ops = g_mem_size;
    char* dynamic_buf = (char*)malloc(ops);
    if (dynamic_buf) {
        int i = 0;
        
    restart_loop:
        if (i < 10) {
            /* Mix of memory operations */
            if (i % 3 == 0) {
                __builtin_memset(dynamic_buf, i, ops);
                goto next_iter;
            } else if (i % 3 == 1) {
                __builtin_memcpy(dynamic_buf, g_token_pool, ops < 4096 ? ops : 4096);
                goto next_iter;
            } else {
                __builtin_memmove(dynamic_buf + ops/2, dynamic_buf, ops/2);
            }
            
        next_iter:
            i++;
            goto restart_loop;
        }
        
        free(dynamic_buf);
    }
    
    /* Cleanup */
    free_ast(root);
    
    /* Final verification */
    volatile int final_check = 0;
    for (size_t i = 0; i < 100; i++) {
        final_check += g_token_pool[i % 4096];
    }
    printf("Final check value: %d\n", final_check);
    
    return 0;
}
