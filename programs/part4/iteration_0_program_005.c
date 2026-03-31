/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char global_tokens[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    printf("Constructor: Initialized global tokens\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtin memcpy in destructor */
    char verify[16];
    __builtin_memcpy(verify, global_tokens, 16);
    printf("Destructor: Verified %d bytes\n", 16);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for node initialization */
    size_t copy_len = (size_t)(volatile_len % 128) + 1;
    __builtin_memcpy(node->data, base_data, copy_len);
    node->size = copy_len;
    
    /* Recursive creation with goto for flow control */
    int use_goto = volatile_flag & 1;
    
    if (use_goto) {
        goto recursive_branch;
    }
    
    node->left = create_ast(depth - 1, node->data);
    
recursive_branch:
    /* Jump target for goto */
    node->right = create_ast(depth - 1, node->data + 16);
    
    /* Memory move between nodes if right exists */
    if (node->right && node->left) {
        size_t move_len = node->size < node->right->size ? 
                         node->size : node->right->size;
        __builtin_memmove(node->right->data, node->left->data, move_len);
    }
    
    return node;
}

/* Complex memory operation with OpenMP */
static void parallel_memory_operations(void) {
    char buffer1[1024];
    char buffer2[1024];
    char buffer3[1024];
    
    /* Initialize with volatile-controlled lengths */
    int len1 = volatile_len % 512;
    int len2 = (volatile_len * 2) % 512;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread uses different builtins */
        #pragma omp sections
        {
            #pragma omp section
            {
                __builtin_memset(buffer1 + thread_id * 64, thread_id, len1);
            }
            
            #pragma omp section
            {
                if (thread_id > 0) {
                    __builtin_memcpy(buffer2, buffer1, len2);
                }
            }
            
            #pragma omp section
            {
                /* Use goto to jump around memmove */
                if (thread_id % 2 == 0) {
                    goto skip_memmove;
                }
                
                __builtin_memmove(buffer3, buffer1, len1);
                
            skip_memmove:
                /* Fill with alternative pattern */
                __builtin_memset(buffer3 + 128, 0xFF, 64);
            }
        }
        
        /* Verify with another memcpy */
        char verify[64];
        __builtin_memcpy(verify, buffer1 + thread_id * 64, 64);
    }
}

/* Multi-stage memory stress test */
static void memory_stress_test(void) {
    /* Stage 1: Direct builtin calls */
    char stage1[256];
    __builtin_memset(stage1, 0xAA, 256);
    __builtin_memcpy(stage1 + 128, stage1, 128);
    __builtin_memmove(stage1, stage1 + 64, 192);
    
    /* Stage 2: Volatile-controlled operations */
    volatile int stage2_len = 128;
    char* stage2 = malloc(stage2_len);
    if (stage2) {
        for (int i = 0; i < 10; i++) {
            __builtin_memset(stage2, i, stage2_len);
            __builtin_memcpy(stage1, stage2, stage2_len / 2);
        }
        free(stage2);
    }
    
    /* Stage 3: Nested calls with goto */
    char stage3[512];
    int use_complex = volatile_flag;
    
    if (use_complex) {
        complex_pattern:
        __builtin_memset(stage3, 0xCC, 256);
        __builtin_memcpy(stage3 + 256, stage3, 256);
        goto finish_stage;
    }
    
    __builtin_memset(stage3, 0xDD, 512);
    
finish_stage:
    __builtin_memmove(stage3, stage3 + 128, 384);
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Initialize and parse */
    ASTNode* root = create_ast(4, "BaseDataForAST");
    
    /* Phase 2: Parallel operations */
    #ifdef _OPENMP
    parallel_memory_operations();
    #endif
    
    /* Phase 3: Stress testing */
    memory_stress_test();
    
    /* Phase 4: Verify with all three builtins */
    char verify_buffer[1024];
    char source_buffer[1024];
    
    __builtin_memset(source_buffer, 0x42, sizeof(source_buffer));
    __builtin_memcpy(verify_buffer, source_buffer, sizeof(verify_buffer));
    __builtin_memmove(verify_buffer + 512, verify_buffer, 512);
    
    /* Calculate and print verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(verify_buffer); i++) {
        hash = (hash * 31) + verify_buffer[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real code, would need recursive free function for AST */
    
    return 0;
}
