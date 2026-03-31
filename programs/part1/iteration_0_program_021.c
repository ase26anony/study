/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan",
    "instrument", "redzone", "shadow", "coverage"
};
static const int token_count = sizeof(tokens) / sizeof(tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_test(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < 256; i++) {
        volatile_src[i] = (char)(i % 26 + 'A');
    }
    
    /* Force early built-in calls in constructor */
    __builtin_memset(volatile_dest, 0xAA, volatile_len);
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    /* Final built-in calls in destructor */
    __builtin_memset(volatile_dest, 0xFF, 16);
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use built-ins for node initialization */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy token data using memcpy */
    const char* token = tokens[node->id % token_count];
    __builtin_memcpy(node->data, token, strlen(token));
    
    /* Create children recursively */
    node->left = create_ast(depth - 1, counter);
    node->left = create_ast(depth - 1, counter);
    
    return node;
}

/* Complex memory operation with goto flow control */
static void complex_memory_ops(void) {
    char buffer1[128];
    char buffer2[128];
    char buffer3[128];
    int use_memmove = 0;
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 'X', sizeof(buffer1));
    __builtin_memset(buffer2, 'Y', sizeof(buffer2));
    
    /* Goto-based control flow */
    goto start_ops;
    
copy_block:
    /* This block contains built-in memmove */
    __builtin_memmove(buffer3, buffer1, volatile_len % 64);
    goto after_move;
    
start_ops:
    /* Regular memcpy */
    __builtin_memcpy(buffer2, buffer1, 32);
    
    /* Conditional jump to memmove block */
    if (volatile_len > 32) {
        use_memmove = 1;
        goto copy_block;
    }
    
after_move:
    /* More operations after goto */
    __builtin_memset(buffer1 + 16, 'Z', 16);
    
    /* Jump back for second pass */
    if (use_memmove) {
        goto copy_block;
    }
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_dispatch(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buf[64];
        char shared_buf[64];
        
        /* Thread-specific pattern */
        __builtin_memset(local_buf, 'A' + thread_id, sizeof(local_buf));
        
        /* Critical section for shared operations */
        #pragma omp critical
        {
            __builtin_memcpy(shared_buf, local_buf, 32);
            __builtin_memset(shared_buf + 32, thread_id, 16);
        }
        
        /* Potential overlapping copy */
        if (thread_id % 2 == 0) {
            __builtin_memmove(local_buf + 16, local_buf, 32);
        }
    }
}

/* Main test driver */
int main(void) {
    int counter = 0;
    int hash = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: AST operations */
    ASTNode* root = create_ast(4, &counter);
    
    if (root) {
        /* Copy between AST nodes */
        ASTNode* current = root;
        while (current && current->left) {
            __builtin_memcpy(current->right->data, 
                           current->left->data, 
                           sizeof(current->data));
            current = current->left;
        }
        
        /* Calculate hash from AST data */
        for (int i = 0; i < 32 && root->data[i]; i++) {
            hash += root->data[i];
        }
    }
    
    /* Phase 2: Complex control flow with memory ops */
    complex_memory_ops();
    
    /* Phase 3: OpenMP parallel section */
    #ifdef _OPENMP
    parallel_memory_dispatch();
    #endif
    
    /* Phase 4: Volatile-based operations */
    int len = volatile_len;
    for (int i = 0; i < 3; i++) {
        /* Vary operations based on iteration */
        switch (i) {
            case 0:
                __builtin_memcpy((void*)volatile_dest, 
                               (void*)volatile_src, 
                               len % 128);
                break;
            case 1:
                __builtin_memset((void*)(volatile_dest + 64), 
                               0xCC, 
                               len % 64);
                break;
            case 2:
                __builtin_memmove((void*)(volatile_dest + 32),
                                (void*)volatile_dest,
                                len % 96);
                break;
        }
        len = (len * 13 + 7) % 256; /* Change length */
    }
    
    /* Final verification */
    hash += volatile_dest[0] + volatile_dest[127];
    printf("Test completed. Hash: %d\n", hash);
    
    return 0;
}
