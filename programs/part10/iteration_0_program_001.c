/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {
    "memcpy", "memset", "memmove",
    "asan", "hwasan", "instrumentation",
    "redzone", "shadow", "poison"
};
static const int token_count = sizeof(tokens) / sizeof(tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_test(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < 256; i++) {
        volatile_src[i] = (char)(i % 26 + 'A');
    }
    
    /* Force early built-in usage in constructor */
    __builtin_memset(volatile_dest, 0, sizeof(volatile_dest));
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    /* Final built-in usage in destructor */
    __builtin_memset(volatile_dest, 0xFF, 16);
}

/* Recursive AST creation and manipulation */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use built-ins to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy token into node data based on ID */
    const char* token = tokens[node->id % token_count];
    size_t len = strlen(token);
    if (len > sizeof(node->data) - 1) len = sizeof(node->data) - 1;
    __builtin_memcpy(node->data, token, len);
    node->data[len] = '\0';
    
    /* Recursive creation with goto for control flow */
    int use_goto = (depth % 2 == 0);
    
    if (use_goto) {
        goto create_left;
    }
    
    node->left = create_ast(depth - 1, counter);
    
create_left:
    if (!use_goto) {
        node->right = create_ast(depth - 1, counter);
    } else {
        node->left = create_ast(depth - 1, counter);
        node->right = NULL;
        goto skip_right;
    }
    
    node->right = create_ast(depth - 1, counter);
    
skip_right:
    return node;
}

/* Complex memory operation with goto jumps */
static void complex_memory_ops(void) {
    char buffer1[128];
    char buffer2[128];
    char buffer3[128];
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 'A', sizeof(buffer1));
    __builtin_memset(buffer2, 'B', sizeof(buffer2));
    __builtin_memset(buffer3, 'C', sizeof(buffer3));
    
    int mode = 0;
    
    /* Goto-based control flow around memmove */
    if (volatile_len > 32) {
        mode = 1;
        goto memmove_block;
    }
    
    /* Regular memcpy */
    __builtin_memcpy(buffer2, buffer1, volatile_len % 64);
    goto end_ops;
    
memmove_block:
    /* This tests the memmove redirection */
    __builtin_memmove(buffer3, buffer1, volatile_len % 64);
    
    /* Jump out of block */
    if (mode == 1) {
        goto final_copy;
    }
    
    __builtin_memcpy(buffer1, buffer2, 32);
    
final_copy:
    /* Final overlapping copy */
    __builtin_memmove(buffer1 + 16, buffer1, 48);
    
end_ops:
    /* Verify by copying back to volatile */
    __builtin_memcpy((void*)volatile_dest, buffer3, 32);
}

/* OpenMP parallel memory operations */
static void parallel_memory_dispatch(void) {
    const int num_workers = 4;
    char worker_buffers[4][256];
    int results[4] = {0};
    
    #pragma omp parallel num_threads(num_workers)
    {
        int tid = 0;
        #ifdef _OPENMP
        tid = omp_get_thread_num();
        #endif
        
        /* Each thread uses different built-ins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(worker_buffers[tid], tid + '0', 128);
                __builtin_memcpy(worker_buffers[tid] + 64, volatile_src, 64);
                break;
            case 1:
                __builtin_memcpy(worker_buffers[tid], volatile_src, 128);
                __builtin_memset(worker_buffers[tid] + 96, 0, 32);
                break;
            case 2:
                __builtin_memmove(worker_buffers[tid], worker_buffers[(tid + 1) % 4], 96);
                __builtin_memcpy(worker_buffers[tid] + 96, "TEST", 4);
                break;
        }
        
        /* Compute simple hash */
        for (int i = 0; i < 128; i++) {
            results[tid] += worker_buffers[tid][i];
        }
    }
    
    /* Combine results */
    int total = 0;
    for (int i = 0; i < 4; i++) {
        total += results[i];
    }
    
    /* Store in volatile for verification */
    volatile_dest[0] = (char)(total % 256);
}

/* Main test execution */
int main(void) {
    int counter = 0;
    int hash_result = 0;
    
    printf("Starting ASAN/HWASAN built-in redirection test...\n");
    
    /* Phase 1: Create and manipulate AST */
    ASTNode* root = create_ast(4, &counter);
    
    if (root) {
        /* Copy between AST nodes using built-ins */
        ASTNode* current = root;
        while (current) {
            if (current->left && current->right) {
                /* Test memcpy between child nodes */
                __builtin_memcpy(current->left->data + 16, 
                               current->right->data, 
                               16);
                
                /* Test memmove with overlap */
                __builtin_memmove(current->data + 8, 
                                current->data, 
                                24);
            }
            
            /* Add to hash */
            for (int i = 0; i < 32 && current->data[i]; i++) {
                hash_result += current->data[i];
            }
            
            current = current->left;
        }
    }
    
    /* Phase 2: Complex memory operations with goto */
    complex_memory_ops();
    
    /* Phase 3: OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_dispatch();
    #endif
    
    /* Phase 4: Final built-in stress test */
    char final_buffer[512];
    
    /* Chain of built-in operations */
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, volatile_src, volatile_len % 128);
    __builtin_memmove(final_buffer + 64, final_buffer, 64);
    __builtin_memset(final_buffer + 128, 0xAA, 64);
    __builtin_memcpy(final_buffer + 192, final_buffer + 64, 64);
    __builtin_memmove(final_buffer + 256, final_buffer, 128);
    
    /* Add final buffer to hash */
    for (int i = 0; i < 256; i++) {
        hash_result += final_buffer[i];
    }
    
    /* Add volatile dest */
    for (int i = 0; i < 32; i++) {
        hash_result += volatile_dest[i];
    }
    
    printf("Test completed. Hash result: %d\n", hash_result);
    printf("Built-in functions tested: memcpy, memset, memmove\n");
    
    /* Cleanup */
    /* Note: In real usage, would need proper AST cleanup */
    
    return 0;
}
