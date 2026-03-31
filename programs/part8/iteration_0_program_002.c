/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    g_init_flag = 1;
    printf("Constructor: Initializing ASAN test environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with volatile size */
    size_t copy_size = g_mem_size < sizeof(node->data) ? g_mem_size : sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_size);
    
    node->size = copy_size;
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = (depth % 2 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, base_data);
        node->right = NULL;
        return node;
        
    create_children:
        node->left = create_ast(depth - 1, base_data);
        node->right = create_ast(depth - 1, base_data);
    }
    
    return node;
}

/* Function with __builtin_memmove and goto */
static void manipulate_ast(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int do_move = 1;
    
    if (do_move) {
        goto perform_move;
    }
    
    /* This won't execute due to goto */
    __builtin_memcpy(dst->data, src->data, src->size);
    return;
    
perform_move:
    /* Use __builtin_memmove for overlapping regions */
    size_t move_size = src->size;
    if (move_size > sizeof(src->data)) move_size = sizeof(src->data);
    
    __builtin_memmove(dst->data, src->data, move_size);
    
    /* Jump back */
    if (dst->left) {
        goto process_child;
    }
    return;
    
process_child:
    manipulate_ast(src->left, dst->left);
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(void) {
    const int num_arrays = 8;
    char* arrays[num_arrays];
    volatile size_t local_size = g_mem_size;
    
    #pragma omp parallel for
    for (int i = 0; i < num_arrays; i++) {
        arrays[i] = (char*)malloc(local_size * sizeof(char));
        if (arrays[i]) {
            /* Force ASAN to intercept these builtins */
            __builtin_memset(arrays[i], i, local_size);
            
            if (i > 0) {
                /* Use memcpy between arrays */
                __builtin_memcpy(arrays[i], arrays[i-1], local_size / 2);
            }
            
            /* Use memmove for potential overlap */
            if (i < num_arrays - 1 && arrays[i+1]) {
                __builtin_memmove(arrays[i] + local_size/4, 
                                 arrays[i], 
                                 local_size/4);
            }
        }
    }
    
    /* Cleanup */
    #pragma omp parallel for
    for (int i = 0; i < num_arrays; i++) {
        if (arrays[i]) free(arrays[i]);
    }
}

/* Multi-stage initialization */
static int initialize_token_array(char tokens[][32], int count) {
    volatile int stage = 0;
    int result = 0;
    
stage1:
    /* Stage 1: Initialize with memset */
    for (int i = 0; i < count; i++) {
        __builtin_memset(tokens[i], 0, 32);
    }
    stage = 1;
    
    if (stage == 1) {
        goto stage2;
    }
    
stage2:
    /* Stage 2: Fill with pattern using memcpy */
    char pattern[32];
    __builtin_memset(pattern, 'A', 32);
    
    for (int i = 0; i < count; i++) {
        __builtin_memcpy(tokens[i], pattern, 32);
        /* Modify with memmove */
        if (i > 0) {
            __builtin_memmove(tokens[i] + 16, tokens[i-1], 16);
        }
    }
    stage = 2;
    
    /* Calculate checksum */
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 32; j++) {
            result += tokens[i][j];
        }
    }
    
    return result;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Test 1: Recursive AST operations */
    ASTNode* ast1 = create_ast(3, "AST_Test_Data");
    ASTNode* ast2 = create_ast(3, "Another_AST");
    
    if (ast1 && ast2) {
        manipulate_ast(ast1, ast2);
        
        /* Verify with memcmp */
        int cmp = __builtin_memcmp(ast1->data, ast2->data, 
                                  ast1->size < ast2->size ? ast1->size : ast2->size);
        printf("AST comparison result: %d\n", cmp);
    }
    
    /* Test 2: Parallel memory operations */
    parallel_memory_ops();
    
    /* Test 3: Token array with goto control flow */
    char tokens[8][32];
    int checksum = initialize_token_array(tokens, 8);
    printf("Token array checksum: %d\n", checksum);
    
    /* Test 4: Direct built-in calls in varied contexts */
    volatile char buffer1[128];
    volatile char buffer2[128];
    
    /* Chain of memory operations */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 64, buffer1, 64);
    
    /* Complex expression with builtins */
    size_t dynamic_size = g_mem_size;
    char* dyn_buf = (char*)malloc(dynamic_size);
    if (dyn_buf) {
        __builtin_memset(dyn_buf, 0xCC, dynamic_size);
        __builtin_memcpy(dyn_buf, buffer1, dynamic_size < 128 ? dynamic_size : 128);
        free(dyn_buf);
    }
    
    /* Cleanup */
    /* Recursive free function would go here */
    
    printf("ASAN test completed successfully\n");
    return 0;
}
