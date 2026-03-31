/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force builtin initialization early */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    printf("[Constructor] Initialized ASAN early buffer\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan_late(void) {
    printf("[Destructor] ASAN cleanup completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_tree(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to prevent folding */
    volatile size_t local_size = g_mem_size / (depth + 1);
    if (local_size > sizeof(node->data)) local_size = sizeof(node->data);
    
    /* Builtin memset with non-constant size */
    __builtin_memset(node->data, depth, (size_t)local_size);
    node->size = (size_t)local_size;
    
    /* Create children with goto for flow control */
    if (depth > 1) {
        int use_left = 1;
        
        /* Jump into memory operation block */
        if (use_left) goto create_left;
        
        create_left:
        node->left = create_tree(depth - 1);
        
        /* Jump out of block */
        if (node->left) goto skip_right;
        
        skip_right:
        node->right = create_tree(depth - 2);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with memcpy between AST nodes */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    volatile size_t copy_size = dest->size < src->size ? dest->size : src->size;
    
    /* Builtin memcpy with volatile size */
    __builtin_memcpy(dest->data, src->data, (size_t)copy_size);
    
    /* Conditional memmove with goto */
    if (g_use_memmove && dest->data + 16 < src->data + sizeof(src->data)) {
        int do_memmove = 1;
        if (do_memmove) goto perform_memmove;
        
        perform_memmove:
        /* Overlapping memory regions */
        __builtin_memmove(dest->data + 8, dest->data, 32);
        goto after_memmove;
        
        after_memmove:
        /* Additional operation after jump */
        __builtin_memset(dest->data + 40, 0xFF, 16);
    }
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[256];
        char shared_buf[512];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            volatile size_t op_size = g_mem_size / (i + 2);
            
            /* Mix of memory operations */
            if (i % 3 == 0) {
                __builtin_memcpy(shared_buf + i * 64, local_buf, op_size);
            } else if (i % 3 == 1) {
                __builtin_memset(shared_buf + i * 64, i, op_size);
            } else {
                __builtin_memmove(shared_buf + i * 64, 
                                 shared_buf + (i-1) * 64, 
                                 op_size);
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Initialize AST structures */
    ASTNode* tree1 = create_tree(4);
    ASTNode* tree2 = create_tree(3);
    
    if (!tree1 || !tree2) {
        fprintf(stderr, "Failed to create AST trees\n");
        return 1;
    }
    
    /* Phase 2: Perform memory operations between trees */
    copy_ast_data(tree1, tree2);
    copy_ast_data(tree2, tree1);
    
    /* Phase 3: Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Phase 4: Complex memory pattern with gotos */
    char pattern_buf[1024];
    volatile int pattern = 0xCC;
    
    for (int i = 0; i < 3; i++) {
        if (i == 0) goto do_memcpy;
        if (i == 1) goto do_memset;
        if (i == 2) goto do_memmove;
        
        do_memcpy:
        __builtin_memcpy(pattern_buf + 256, pattern_buf, 128);
        continue;
        
        do_memset:
        __builtin_memset(pattern_buf + 384, pattern, 192);
        continue;
        
        do_memmove:
        __builtin_memmove(pattern_buf + 512, pattern_buf + 256, 256);
        continue;
    }
    
    /* Phase 5: Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(pattern_buf); i++) {
        hash = (hash * 31) + pattern_buf[i];
    }
    
    /* Also hash tree data */
    for (int i = 0; i < 64 && i < (int)tree1->size; i++) {
        hash = (hash * 17) + tree1->data[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(tree1);
    free(tree2);
    
    return 0;
}
