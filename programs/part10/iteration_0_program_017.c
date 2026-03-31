/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
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
    int id;
} ASTNode;

/* Token array for parser simulation */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "data", "test", "asan", "hwasan", "coverage"
};
static const int token_count = 8;

/* Global buffer for memory operations */
static char global_buffer[4096];
static char shadow_buffer[4096];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize global buffers with pattern */
    __builtin_memset(global_buffer, 0xAA, sizeof(global_buffer));
    __builtin_memset(shadow_buffer, 0x55, sizeof(shadow_buffer));
    
    printf("Constructor: Initialized ASAN test buffers\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    /* Verify buffers were modified */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += global_buffer[i];
    }
    printf("Destructor: Buffer checksum = %d\n", sum);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* token) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data using builtins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy token into node data with volatile length control */
    int len = volatile_len % 128;
    if (len > 255) len = 255;
    
    /* Force memcpy redirection with goto */
    if (volatile_flag) {
        goto do_copy;
    } else {
        /* Alternative path */
        __builtin_memset(node->data, 'X', len);
        goto skip_copy;
    }
    
do_copy:
    /* This goto tests flow sensitivity */
    __builtin_memcpy(node->data, token, len);
    
skip_copy:
    node->id = depth;
    
    /* Recursive creation with different memory operations */
    if (depth > 1) {
        node->left = create_ast(depth - 1, tokens[(depth * 2) % token_count]);
        node->right = create_ast(depth - 2, tokens[(depth * 3) % token_count]);
        
        /* Copy between nodes using memmove */
        if (node->left && node->right) {
            __builtin_memmove(node->right->data, 
                            node->left->data, 
                            volatile_len % 64);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with complex control flow and goto */
static void process_with_goto(ASTNode* node) {
    if (!node) return;
    
    int use_memmove = 0;
    
    /* Goto into block with memory operation */
    if (node->id % 3 == 0) {
        goto use_memmove_block;
    } else if (node->id % 3 == 1) {
        goto use_memcpy_block;
    } else {
        goto use_memset_block;
    }
    
use_memmove_block:
    /* This tests the memmove redirection */
    __builtin_memmove(node->data + 32, node->data, 32);
    use_memmove = 1;
    goto continue_processing;
    
use_memcpy_block:
    __builtin_memcpy(global_buffer + node->id * 64, 
                    node->data, 
                    volatile_len % 128);
    goto continue_processing;
    
use_memset_block:
    __builtin_memset(node->data, node->id, 64);
    goto continue_processing;
    
continue_processing:
    /* Process children */
    process_with_goto(node->left);
    process_with_goto(node->right);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    int i;
    
    #pragma omp parallel private(i)
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buf[512];
        
        /* Each thread performs different memory operations */
        #pragma omp for
        for (i = 0; i < 100; i++) {
            /* Mix of memory operations */
            switch (i % 3) {
                case 0:
                    __builtin_memset(local_buf, thread_id, 
                                   volatile_len % 256);
                    __builtin_memcpy(global_buffer + i * 8, 
                                   local_buf, 8);
                    break;
                case 1:
                    __builtin_memcpy(local_buf, 
                                   shadow_buffer + i * 8, 8);
                    __builtin_memmove(global_buffer + i * 8 + 4,
                                    global_buffer + i * 8, 4);
                    break;
                case 2:
                    __builtin_memset(global_buffer + i * 8, 
                                   i, 8);
                    __builtin_memcpy(shadow_buffer + i * 8,
                                   global_buffer + i * 8, 8);
                    break;
            }
        }
        
        /* Barrier to ensure all memory ops complete */
        #pragma omp barrier
        
        /* Final memory operation after barrier */
        #pragma omp single
        {
            __builtin_memmove(global_buffer, 
                            global_buffer + 256, 256);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create recursive AST structure */
    ASTNode* root = create_ast(5, tokens[0]);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto control flow */
    process_with_goto(root);
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Additional memory operations in main */
    char temp[1024];
    
    /* Chain of memory operations */
    __builtin_memset(temp, 0, sizeof(temp));
    __builtin_memcpy(temp, root->data, 128);
    __builtin_memmove(temp + 128, temp, 128);
    __builtin_memset(temp + 256, 0xFF, 128);
    
    /* Copy back to global buffer */
    __builtin_memcpy(global_buffer + 1024, temp, 384);
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 2000; i++) {
        hash = hash * 31 + global_buffer[i];
    }
    
    printf("Test completed. Verification hash: %lu\n", hash);
    
    /* Cleanup */
    /* Note: In real ASAN, this would detect leaks */
    
    return 0;
}
