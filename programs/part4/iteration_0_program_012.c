/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 256 + (rand() % 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) {
        pattern[i] = (char)((i + depth) % 256);
    }
    __builtin_memcpy(node->data, pattern, 64);
    
    node->size = g_mem_size;
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    return node;
}

/* Function with goto statements and __builtin_memmove */
static void process_ast_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int use_memmove = 1;
    
    /* Jump into memory operation block */
    if (use_memmove) goto do_memmove;
    
    normal_path:
    __builtin_memcpy(dst->data, src->data, 64);
    return;
    
    do_memmove:
    /* Use __builtin_memmove with overlapping regions */
    char temp[128];
    __builtin_memcpy(temp, src->data, 64);
    
    /* Jump out of block */
    if (dst->size > 64) goto skip_memmove;
    
    __builtin_memmove(dst->data, temp, 64);
    goto normal_path;
    
    skip_memmove:
    __builtin_memcpy(dst->data, temp, 32);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_arrays = 8;
    char* arrays[num_arrays];
    volatile size_t local_size = g_mem_size;
    
    #pragma omp parallel for
    for (int i = 0; i < num_arrays; i++) {
        arrays[i] = malloc(local_size);
        if (arrays[i]) {
            /* Force ASAN to intercept these builtins */
            __builtin_memset(arrays[i], i, local_size);
            
            if (i > 0) {
                /* Overlapping copy with __builtin_memmove */
                __builtin_memmove(arrays[i] + 32, arrays[i-1], 64);
            }
            
            /* Conditional __builtin_memcpy */
            if (i % 2 == 0) {
                char src[128];
                __builtin_memset(src, 0xFF, 128);
                __builtin_memcpy(arrays[i], src, 64);
            }
        }
    }
    
    /* Cleanup */
    #pragma omp parallel for
    for (int i = 0; i < num_arrays; i++) {
        if (arrays[i]) free(arrays[i]);
    }
}

/* Multi-stage initialization with varied memory operations */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    
    /* Process data with __builtin_memcpy to temporary buffer */
    char buffer[64];
    __builtin_memcpy(buffer, node->data, 64);
    
    for (int i = 0; i < 64; i++) {
        hash = ((hash << 5) + hash) + buffer[i];
    }
    
    /* Recursive processing */
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Create and process AST structures */
    ASTNode* ast1 = create_ast(3);
    ASTNode* ast2 = create_ast(3);
    
    if (ast1 && ast2) {
        /* Test goto flow with memory operations */
        process_ast_with_goto(ast1, ast2);
        
        /* Compute verification hash */
        unsigned long hash1 = compute_ast_hash(ast1);
        unsigned long hash2 = compute_ast_hash(ast2);
        printf("AST Hash 1: %lu\n", hash1);
        printf("AST Hash 2: %lu\n", hash2);
        
        /* Additional __builtin_memmove test with overlap */
        __builtin_memmove(ast1->data + 16, ast1->data, 32);
        __builtin_memmove(ast2->data, ast2->data + 8, 48);
    }
    
    /* Phase 2: OpenMP parallel operations */
    printf("Starting parallel memory operations\n");
    parallel_memory_ops();
    
    /* Phase 3: Direct built-in calls with volatile control */
    volatile char* dynamic_buf = malloc(g_mem_size);
    if (dynamic_buf) {
        volatile size_t op_size = g_mem_size / 2;
        
        /* Sequence of all three builtins */
        __builtin_memset(dynamic_buf, 0xAA, op_size);
        __builtin_memcpy(dynamic_buf + op_size/2, dynamic_buf, op_size/2);
        __builtin_memmove(dynamic_buf, dynamic_buf + op_size/4, op_size/2);
        
        /* Verify with simple checksum */
        unsigned char sum = 0;
        for (size_t i = 0; i < op_size; i++) {
            sum += dynamic_buf[i];
        }
        printf("Memory checksum: %u\n", (unsigned)sum);
        
        free((void*)dynamic_buf);
    }
    
    /* Cleanup */
    /* Helper function to free AST */
    void free_ast(ASTNode* node) {
        if (!node) return;
        free_ast(node->left);
        free_ast(node->right);
        free(node);
    }
    
    if (ast1) free_ast(ast1);
    if (ast2) free_ast(ast2);
    
    printf("ASAN test completed successfully\n");
    return 0;
}
