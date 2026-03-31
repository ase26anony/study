/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* volatile to prevent optimization */
    struct ASTNode* left;
    struct ASTNode* right;
    char padding[32];      /* Ensure size for memcpy operations */
} ASTNode;

/* Global volatile variables to prevent constant folding */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Token array for parser simulation */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "parse", "build", "traverse"
};
#define TOKEN_COUNT (sizeof(tokens)/sizeof(tokens[0]))

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_sanitizer_hook(void) {
    /* Force early initialization of sanitizer runtime */
    volatile char buffer[64];
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("Constructor: ASAN runtime initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_sanitizer_hook(void) {
    printf("Destructor: ASAN cleanup completed\n");
}

/* Recursive AST builder with memory operations */
static ASTNode* build_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = (*counter)++ % 3;
    node->value = depth * 100 + node->type;
    node->size = g_mem_size / (depth + 1);  /* volatile size */
    
    /* Create child nodes with goto-based control flow */
    int build_left = 1;
    
    /* Goto label before memory operation */
    build_left_node:
    if (build_left) {
        node->left = build_ast(depth - 1, counter);
        build_left = 0;
        goto build_left_node;  /* Jump back to test flow sensitivity */
    }
    
    /* Another goto jumping over memory operation */
    if (depth > 2) {
        goto skip_memmove;
    }
    
    /* __builtin_memmove with goto context */
    if (node->left && node->right) {
        char temp[sizeof(ASTNode)];
        __builtin_memmove(temp, node->left, sizeof(ASTNode));
        __builtin_memmove(node->left, node->right, sizeof(ASTNode));
        __builtin_memmove(node->right, temp, sizeof(ASTNode));
    }
    
    skip_memmove:
    node->right = build_ast(depth - 2, counter);
    
    return node;
}

/* AST traversal with memory copying between nodes */
static int traverse_ast(ASTNode* node, int* sum) {
    if (!node) return 0;
    
    int local_sum = node->value;
    
    /* Copy node data to temporary buffer using __builtin_memcpy */
    char node_copy[sizeof(ASTNode)];
    __builtin_memcpy(node_copy, node, sizeof(ASTNode));
    
    /* Verify copy by comparing values */
    ASTNode* temp = (ASTNode*)node_copy;
    if (temp->value != node->value) {
        __builtin_memset(node_copy, 0xFF, sizeof(ASTNode));
    }
    
    /* Recursive traversal */
    local_sum += traverse_ast(node->left, sum);
    
    /* Another memory operation in different control flow path */
    volatile int do_memmove = (node->type == 1);
    if (do_memmove && node->left && node->right) {
        char buffer[64];
        size_t move_size = node->size % 64;
        __builtin_memmove(buffer, node->left->padding, move_size);
        __builtin_memmove(node->left->padding, node->right->padding, move_size);
        __builtin_memmove(node->right->padding, buffer, move_size);
    }
    
    local_sum += traverse_ast(node->right, sum);
    
    *sum += local_sum;
    return local_sum;
}

/* Free AST with memory sanitization */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    const int buffer_count = 16;
    char* buffers[buffer_count];
    
    #pragma omp parallel for
    for (int i = 0; i < buffer_count; i++) {
        buffers[i] = (char*)malloc(g_mem_size);
        if (buffers[i]) {
            /* Use all three builtins in parallel regions */
            __builtin_memset(buffers[i], i, g_mem_size);
            
            if (i > 0) {
                __builtin_memcpy(buffers[i], buffers[i-1], g_mem_size / 2);
            }
            
            /* Conditional memmove with volatile control */
            volatile int should_move = (i % 3 == 0);
            if (should_move) {
                char temp[128];
                __builtin_memmove(temp, buffers[i], 128);
                __builtin_memmove(buffers[i] + 64, temp, 128);
            }
        }
    }
    
    /* Cleanup */
    #pragma omp parallel for
    for (int i = 0; i < buffer_count; i++) {
        if (buffers[i]) {
            __builtin_memset(buffers[i], 0, g_mem_size);
            free(buffers[i]);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Build and traverse recursive AST */
    int counter = 0;
    ASTNode* root = build_ast(5, &counter);
    
    int ast_sum = 0;
    traverse_ast(root, &ast_sum);
    printf("AST traversal sum: %d\n", ast_sum);
    
    /* Phase 2: Direct builtin calls with volatile parameters */
    char src_buffer[512];
    char dst_buffer[512];
    
    volatile size_t op_size = g_mem_size;
    
    /* Test all three builtins in sequence */
    __builtin_memset(src_buffer, 0xAA, op_size);
    __builtin_memcpy(dst_buffer, src_buffer, op_size);
    
    /* Overlapping memmove */
    __builtin_memmove(src_buffer + 128, src_buffer, 256);
    
    /* Verify with another memcpy */
    __builtin_memcpy(dst_buffer + 256, src_buffer + 128, 128);
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 4: Token processing with memory operations */
    char token_data[TOKEN_COUNT][64];
    for (size_t i = 0; i < TOKEN_COUNT; i++) {
        size_t len = strlen(tokens[i]) + 1;
        __builtin_memcpy(token_data[i], tokens[i], len);
        
        /* Conditional memset based on token content */
        if (strstr(tokens[i], "mem")) {
            __builtin_memset(token_data[i] + len, 0xFF, 32);
        }
    }
    
    /* Final verification hash */
    uint32_t hash = 0;
    for (size_t i = 0; i < TOKEN_COUNT; i++) {
        for (size_t j = 0; j < 64; j++) {
            hash = (hash * 31) + token_data[i][j];
        }
    }
    printf("Verification hash: 0x%08X\n", hash);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
