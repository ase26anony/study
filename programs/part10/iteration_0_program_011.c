#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* volatile to prevent optimization */
    struct ASTNode *left;
    struct ASTNode *right;
    char padding[32];      /* Ensure size for memcpy operations */
} ASTNode;

/* Global volatile variables to control memory operations */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_hooks(void) {
    /* Force initialization of ASAN runtime */
    volatile char buffer[128];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_hooks(void) {
    /* Final memory operation to ensure coverage */
    volatile char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function that builds and manipulates AST */
static ASTNode* build_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = (*counter)++;
    node->size = g_mem_size + depth;  /* volatile size */
    
    /* Create child nodes recursively */
    node->left = build_ast(depth - 1, counter);
    node->right = build_ast(depth - 1, counter);
    
    return node;
}

/* Function with goto statements for control flow testing */
static void test_memmove_with_goto(ASTNode* src, ASTNode* dst) {
    int use_memmove = g_use_memmove;
    
    if (use_memmove) {
        goto use_memmove_block;
    } else {
        goto use_memcpy_block;
    }
    
use_memmove_block:
    {
        /* This block tests __builtin_memmove with goto entry */
        volatile size_t copy_size = src->size;
        
        /* Force memmove to be called */
        __builtin_memmove(dst, src, copy_size);
        
        /* Jump out of block */
        goto after_copy;
    }
    
use_memcpy_block:
    {
        /* Alternative path with memcpy */
        __builtin_memcpy(dst, src, src->size);
        goto after_copy;
    }
    
after_copy:
    /* Verify the copy */
    if (dst->type != src->type) {
        __builtin_memset(dst, 0, sizeof(ASTNode));
    }
}

/* Function that processes AST nodes with memory operations */
static uint64_t process_ast(ASTNode* root) {
    if (!root) return 0;
    
    uint64_t hash = 0;
    ASTNode temp_node;
    
    /* Copy node to temporary buffer using builtins */
    volatile size_t copy_size = root->size % sizeof(ASTNode);
    if (copy_size == 0) copy_size = sizeof(ASTNode);
    
    /* Test all three builtins in different contexts */
    __builtin_memset(&temp_node, 0, sizeof(temp_node));
    __builtin_memcpy(&temp_node, root, copy_size);
    
    /* Conditional memmove based on node type */
    if (root->type % 2 == 0) {
        ASTNode backup;
        __builtin_memmove(&backup, &temp_node, sizeof(ASTNode));
        __builtin_memcpy(&temp_node, &backup, sizeof(ASTNode));
    }
    
    /* Calculate hash from node data */
    hash = (uint64_t)root->type * 31 + root->value;
    hash ^= (uint64_t)process_ast(root->left) * 17;
    hash ^= (uint64_t)process_ast(root->right) * 23;
    
    return hash;
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char buffer1[256];
        char buffer2[256];
        volatile size_t op_size = g_mem_size + thread_id;
        
        /* Each thread uses different memory builtins */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(buffer1, thread_id, op_size);
                __builtin_memcpy(buffer2, buffer1, op_size);
                break;
            case 1:
                __builtin_memmove(buffer1, buffer2, op_size);
                __builtin_memset(buffer2, 0xFF, op_size);
                break;
            case 2:
                __builtin_memcpy(buffer1, buffer2, op_size);
                __builtin_memmove(buffer2, buffer1, op_size);
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Final memory operation in parallel region */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            volatile char temp[64];
            __builtin_memset(temp, i, sizeof(temp));
        }
    }
}

/* Main test driver */
int main(void) {
    int counter = 1;
    uint64_t total_hash = 0;
    
    printf("Starting ASAN memory operation tests...\n");
    
    /* Build complex AST structure */
    ASTNode* ast_root = build_ast(4, &counter);
    if (!ast_root) {
        fprintf(stderr, "Failed to build AST\n");
        return 1;
    }
    
    /* Create destination node for memmove tests */
    ASTNode* dst_node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!dst_node) {
        free(ast_root);
        return 1;
    }
    
    /* Test 1: Basic memory operations */
    __builtin_memset(dst_node, 0, sizeof(ASTNode));
    __builtin_memcpy(dst_node, ast_root, sizeof(ASTNode));
    
    /* Test 2: Control flow with goto and memmove */
    test_memmove_with_goto(ast_root, dst_node);
    
    /* Test 3: Process AST recursively */
    total_hash = process_ast(ast_root);
    
    /* Test 4: Parallel memory operations */
    parallel_memory_operations();
    
    /* Test 5: Additional builtin calls in different scopes */
    {
        char buffer1[128], buffer2[128];
        volatile size_t size = g_mem_size;
        
        __builtin_memset(buffer1, 0xAA, size);
        __builtin_memcpy(buffer2, buffer1, size);
        __builtin_memmove(buffer1, buffer2, size);
        
        /* Nested scope with more operations */
        {
            char inner_buf[64];
            __builtin_memset(inner_buf, 0x55, sizeof(inner_buf));
            __builtin_memcpy(buffer2 + 32, inner_buf, 32);
        }
    }
    
    /* Print verification result */
    printf("Total hash: %llu\n", (unsigned long long)total_hash);
    printf("Memory operations completed successfully.\n");
    
    /* Cleanup */
    free(ast_root);
    free(dst_node);
    
    return 0;
}
