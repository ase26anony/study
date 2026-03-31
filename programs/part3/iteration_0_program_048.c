/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Global token array */
static volatile char g_token_buffer[4096];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize token buffer with pattern */
    for (int i = 0; i < sizeof(g_token_buffer); i++) {
        g_token_buffer[i] = (i % 256);
    }
    
    /* Force early built-in usage in constructor */
    char local_buf[128];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    __builtin_memcpy((void*)&g_token_buffer[1024], local_buf, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    /* Final memory operation in destructor */
    char final_buf[32];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with built-ins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    node->value = depth * 100;
    
    /* Copy data with volatile size */
    size_t copy_size = g_mem_size % 64;
    __builtin_memcpy(node->data, (void*)&g_token_buffer[g_token_index], copy_size);
    g_token_index = (g_token_index + copy_size) % 4096;
    
    /* Recursive calls */
    node->left = parse_expression(depth - 1);
    
    /* Jump label for goto testing */
    process_right:
    node->right = parse_expression(depth - 2);
    
    /* Memory move between nodes if both children exist */
    if (node->left && node->right) {
        __builtin_memmove(node->left->data, node->right->data, 32);
    }
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char thread_buffer[512];
        volatile int local_size = 128 + (thread_id * 64);
        
        /* Each thread uses built-ins independently */
        __builtin_memset(thread_buffer, thread_id, local_size);
        
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            char temp[256];
            
            /* Mixed built-in usage in loop */
            __builtin_memset(temp, i, sizeof(temp));
            __builtin_memcpy(&thread_buffer[i * 2], temp, 128);
            
            /* Conditional goto to test flow sensitivity */
            if (i % 7 == 0) {
                goto memmove_block;
            }
            
            continue;
            
            memmove_block:
            __builtin_memmove(&thread_buffer[0], &thread_buffer[128], 64);
        }
        
        /* Final memory operation per thread */
        __builtin_memset(&thread_buffer[local_size - 64], 0xCC, 64);
    }
}

/* Function with goto jumping into memory operation block */
static void test_flow_sensitivity(void) {
    char buffer_a[256], buffer_b[256];
    volatile int use_memmove = 1;
    
    __builtin_memset(buffer_a, 0x11, sizeof(buffer_a));
    __builtin_memset(buffer_b, 0x22, sizeof(buffer_b));
    
    if (use_memmove) {
        goto jump_into_memmove;
    }
    
    normal_path:
    __builtin_memcpy(buffer_a, buffer_b, 128);
    return;
    
    jump_into_memmove:
    __builtin_memmove(buffer_a, buffer_b, 192);
    goto normal_path;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = parse_expression(5);
    
    /* Phase 2: Parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 3: Flow sensitivity testing */
    test_flow_sensitivity();
    
    /* Phase 4: Direct built-in calls with volatile sizes */
    char final_buffer[1024];
    volatile size_t sizes[] = {32, 64, 128, 256};
    
    for (int i = 0; i < 4; i++) {
        volatile size_t current_size = sizes[i];
        __builtin_memset(final_buffer, i, current_size);
        __builtin_memcpy(&final_buffer[current_size], final_buffer, current_size / 2);
        __builtin_memmove(&final_buffer[0], &final_buffer[current_size / 4], current_size / 2);
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 1024; i++) {
        hash = (hash * 31) + final_buffer[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    
    /* Cleanup */
    free(root);
    
    return 0;
}
