/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* AST-like recursive structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_array[1024];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with built-in memset */
    __builtin_memset(token_array, 0xAA, sizeof(token_array));
    
    /* Initialize volatile buffers */
    for (int i = 0; i < 256; i++) {
        volatile_src[i] = (char)(i % 256);
    }
    
    /* Use built-in memcpy in constructor */
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Cleanup with built-in memset */
    __builtin_memset(token_array, 0, sizeof(token_array));
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Initialize node data with built-in memset */
    __builtin_memset(node->data, node->id, sizeof(node->data));
    
    /* Create left child with goto-controlled flow */
    int use_goto = (depth % 3 == 0);
    
    if (use_goto) {
        goto create_left;
    }
    
    node->left = create_ast(depth - 1, counter);
    goto skip_left;
    
create_left:
    node->left = create_ast(depth - 2, counter);
    
skip_left:
    /* Create right child */
    node->right = create_ast(depth - 1, counter);
    
    return node;
}

/* Copy AST data between nodes using built-in memcpy */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use volatile length to prevent folding */
    int len = volatile_len % 32;
    if (len <= 0) len = 16;
    
    /* Test all three built-ins with goto jumps */
    int mode = dest->id % 3;
    
    if (mode == 0) {
        /* Direct memcpy */
        __builtin_memcpy(dest->data, src->data, len);
    } else if (mode == 1) {
        /* memset then memmove with goto */
        __builtin_memset(dest->data, 0xFF, len);
        goto do_memmove;
    } else {
        /* Complex flow with goto into memmove block */
        if (dest->id % 2 == 0) {
            goto memmove_block;
        }
        __builtin_memset(dest->data, 0xAA, len);
        goto skip_memmove;
        
memmove_block:
        __builtin_memmove(dest->data, src->data, len);
        goto after_memmove;
        
do_memmove:
        __builtin_memmove(dest->data + 4, dest->data, len - 4);
        
after_memmove:
        /* Empty for label */
        ;
    }
    
skip_memmove:
    /* Recursive copy */
    copy_ast_data(dest->left, src->left);
    copy_ast_data(dest->right, src->right);
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(void) {
    char buffer1[512];
    char buffer2[512];
    char buffer3[512];
    
    /* Initialize buffers */
    for (int i = 0; i < 512; i++) {
        buffer1[i] = (char)(i % 128);
        buffer2[i] = (char)((i + 64) % 128);
    }
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                __builtin_memcpy(buffer3, buffer1, 256 + thread_id * 16);
                break;
            case 1:
                __builtin_memset(buffer3 + 128, thread_id, 128);
                break;
            case 2:
                __builtin_memmove(buffer3 + 64, buffer2, 192);
                break;
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* Combined operation */
        #pragma omp for
        for (int i = 0; i < 8; i++) {
            int offset = i * 64;
            __builtin_memcpy(buffer1 + offset, buffer3 + offset, 48);
            __builtin_memset(buffer2 + offset + 16, i, 32);
        }
    }
    
    /* Verify with volatile access */
    volatile char* vbuf = (volatile char*)buffer1;
    (void)vbuf[0];
}

/* Compute hash of AST for verification */
static unsigned long compute_ast_hash(const ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    
    /* Hash node data */
    for (int i = 0; i < 32; i++) {
        hash = ((hash << 5) + hash) + (unsigned long)node->data[i];
    }
    
    /* Combine with children hashes */
    hash ^= compute_ast_hash(node->left);
    hash = ((hash << 5) + hash) ^ compute_ast_hash(node->right);
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize and create AST */
    int counter = 1;
    ASTNode* ast1 = create_ast(4, &counter);
    ASTNode* ast2 = create_ast(3, &counter);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Copy data between ASTs (triggers built-ins) */
    copy_ast_data(ast2, ast1);
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_ops();
    
    /* Phase 4: Complex flow with goto and built-ins */
    char temp_buf[128];
    char result_buf[128];
    
    /* Initialize with volatile-controlled length */
    int len = volatile_len % 64;
    if (len < 16) len = 16;
    
    /* Test goto jumping into memmove block */
    int use_complex_flow = 1;
    
    if (use_complex_flow) {
        __builtin_memset(temp_buf, 0xCC, len);
        goto perform_memmove;
    }
    
    __builtin_memcpy(result_buf, temp_buf, len);
    goto skip_complex;
    
perform_memmove:
    __builtin_memmove(result_buf, temp_buf, len);
    /* Jump back */
    goto after_memmove;
    
after_memmove:
    /* Additional operation after memmove */
    __builtin_memset(result_buf + len/2, 0xDD, len/4);
    
skip_complex:
    /* Phase 5: Token array operations */
    for (int i = 0; i < 8; i++) {
        int offset = i * 64;
        __builtin_memcpy(token_array + offset, result_buf, 32);
        __builtin_memmove(token_array + offset + 32, temp_buf, 24);
    }
    
    /* Phase 6: Compute and verify results */
    unsigned long hash1 = compute_ast_hash(ast1);
    unsigned long hash2 = compute_ast_hash(ast2);
    
    printf("AST Hash 1: %lu\n", hash1);
    printf("AST Hash 2: %lu\n", hash2);
    
    /* Compute final verification sum */
    unsigned long final_sum = hash1 ^ hash2;
    for (int i = 0; i < 256; i += 16) {
        final_sum += (unsigned long)volatile_dest[i];
    }
    
    printf("Final verification sum: %lu\n", final_sum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    /* Note: In real ASAN, memory would be freed and checked */
    
    return 0;
}
