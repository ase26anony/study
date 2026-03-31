/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[128];
static volatile char volatile_src[128];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {
    "memcpy_test", "memset_test", "memmove_test",
    "recursive", "parallel", "asan", "hwasan"
};
static const int token_count = 7;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < sizeof(volatile_src); i++) {
        volatile_src[i] = (char)(i % 256);
    }
    
    /* Force early built-in usage in constructor */
    __builtin_memset(volatile_dest, 0xAA, sizeof(volatile_dest));
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operations in destructor */
    __builtin_memset(volatile_dest, 0, sizeof(volatile_dest));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use built-ins with volatile lengths */
    int copy_len = volatile_len % 32;
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy token data using built-in */
    const char* token = tokens[node->id % token_count];
    __builtin_memcpy(node->data, token, strlen(token));
    
    /* Create children recursively */
    node->left = create_ast(depth - 1, counter);
    node->right = create_ast(depth - 1, counter);
    
    return node;
}

/* Function with goto jumps around memmove */
static void goto_memmove_test(char* dest, char* src, int len) {
    int use_memmove = 0;
    
    /* Jump into block containing memmove */
    if (len > 16) {
        goto do_memmove;
    }
    
    /* Normal path */
    __builtin_memcpy(dest, src, len);
    return;
    
do_memmove:
    /* This block will be entered via goto */
    use_memmove = 1;
    
memmove_block:
    /* Perform memmove with overlap */
    __builtin_memmove(dest + 8, dest, len - 8);
    
    /* Jump out of block */
    if (use_memmove) {
        goto memmove_done;
    }
    
    /* Unreachable but tests flow */
    __builtin_memset(dest, 0, len);
    
memmove_done:
    return;
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    char buffer1[256];
    char buffer2[256];
    char buffer3[256];
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 0x11, sizeof(buffer1));
    __builtin_memset(buffer2, 0x22, sizeof(buffer2));
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                __builtin_memcpy(buffer3, buffer1, volatile_len % 128);
                break;
            case 1:
                __builtin_memset(buffer3 + 64, thread_id, 32);
                break;
            case 2:
                __builtin_memmove(buffer3 + 32, buffer3, 64);
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Collective operation */
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            int offset = i * 32;
            __builtin_memcpy(buffer2 + offset, buffer3 + offset, 32);
        }
    }
    
    /* Verify with final memcpy */
    __builtin_memcpy(buffer1, buffer2, 128);
}

/* Multi-stage processing */
static unsigned long process_tokens(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 0;
    char temp_buffer[64];
    
    /* Process node data */
    for (int i = 0; i < sizeof(node->data); i++) {
        hash = hash * 31 + (unsigned char)node->data[i];
    }
    
    /* Copy to temp buffer using built-in */
    __builtin_memcpy(temp_buffer, node->data, sizeof(node->data));
    
    /* Overlapping memmove within the buffer */
    __builtin_memmove(temp_buffer + 16, temp_buffer, 16);
    
    /* Recursively process children */
    hash += process_tokens(node->left);
    hash += process_tokens(node->right);
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize recursive structures */
    int counter = 0;
    ASTNode* root = create_ast(4, &counter);
    printf("Created AST with %d nodes\n", counter);
    
    /* Phase 2: Perform goto-based memmove tests */
    char test_buffer[128];
    for (int i = 0; i < sizeof(test_buffer); i++) {
        test_buffer[i] = (char)i;
    }
    
    goto_memmove_test(test_buffer, test_buffer + 32, 96);
    
    /* Phase 3: Execute parallel memory operations */
    #ifdef _OPENMP
    printf("Running parallel memory operations...\n");
    #endif
    parallel_memory_ops();
    
    /* Phase 4: Process AST and compute verification hash */
    unsigned long final_hash = process_tokens(root);
    
    /* Phase 5: Additional built-in calls with volatile control */
    int dynamic_len = volatile_len;
    char* dyn_buffer = (char*)malloc(dynamic_len * 2);
    if (dyn_buffer) {
        __builtin_memset(dyn_buffer, 0xCC, dynamic_len);
        __builtin_memcpy(dyn_buffer + dynamic_len, dyn_buffer, dynamic_len);
        __builtin_memmove(dyn_buffer, dyn_buffer + dynamic_len/2, dynamic_len/2);
        
        /* Use result to affect final output */
        for (int i = 0; i < dynamic_len; i++) {
            final_hash += (unsigned char)dyn_buffer[i];
        }
        free(dyn_buffer);
    }
    
    /* Final verification output */
    printf("Test completed. Verification hash: %lu\n", final_hash);
    
    /* Cleanup */
    /* Note: In real code, would need recursive free function */
    
    return (final_hash != 0) ? 0 : 1;
}
