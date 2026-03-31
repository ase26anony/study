/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
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
static void init_asan_test(void) {
    printf("ASAN Test Constructor: Initializing...\n");
    g_mem_size = 256 + (rand() % 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("ASAN Test Destructor: Cleaning up...\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) {
        pattern[i] = (char)((depth * 17 + i) % 256);
    }
    __builtin_memcpy(node->data, pattern, 64);
    
    node->size = g_mem_size;
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    return node;
}

/* Function with goto jumps around memory operations */
static void test_goto_memmove(void* dest, const void* src, size_t n) {
    int use_memmove = 1;
    
    if (n == 0) goto skip_op;
    
    /* Jump into memory operation block */
    goto perform_op;
    
perform_op_back:
    if (use_memmove) {
        /* This should trigger ASAN memmove redirection */
        __builtin_memmove(dest, src, n);
    }
    goto after_op;
    
perform_op:
    /* Alternative path with different size */
    n = n > 32 ? 32 : n;
    goto perform_op_back;
    
skip_op:
    /* Small memset when n=0 */
    __builtin_memset(dest, 0, 16);
    
after_op:
    return;
}

/* Parallel memory operations using OpenMP */
static void parallel_mem_ops(void) {
    const int num_arrays = 8;
    char* arrays[num_arrays];
    size_t sizes[num_arrays];
    
    /* Initialize arrays with different sizes */
    for (int i = 0; i < num_arrays; i++) {
        sizes[i] = (g_mem_size * (i + 1)) / 4;
        arrays[i] = (char*)malloc(sizes[i]);
        if (arrays[i]) {
            __builtin_memset(arrays[i], i, sizes[i]);
        }
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < num_arrays; i++) {
            if (arrays[i]) {
                /* Create temporary buffer and copy data */
                char* temp = (char*)malloc(sizes[i]);
                if (temp) {
                    /* Use __builtin_memcpy in parallel region */
                    __builtin_memcpy(temp, arrays[i], sizes[i]);
                    
                    /* Modify and copy back with memmove */
                    temp[0] = (char)thread_id;
                    __builtin_memmove(arrays[i], temp, sizes[i]);
                    
                    free(temp);
                }
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_arrays; i++) {
        free(arrays[i]);
    }
}

/* Complex memory operation with control flow */
static size_t process_ast(ASTNode* root) {
    if (!root) return 0;
    
    size_t total = 0;
    char buffer[256];
    volatile size_t buf_size = g_mem_size;
    
    /* Copy node data to buffer */
    __builtin_memcpy(buffer, root->data, 64);
    
    /* Process with goto jumps */
    int stage = 0;
    
stage1:
    if (stage == 0) {
        __builtin_memset(buffer + 64, 0xA5, buf_size - 64);
        stage = 1;
        goto stage2;
    }
    
stage2:
    if (stage == 1) {
        /* Move data around */
        __builtin_memmove(buffer + 128, buffer, 64);
        stage = 2;
        goto stage3;
    }
    
stage3:
    if (stage == 2) {
        /* Final copy */
        __builtin_memcpy(root->data, buffer + 128, 64);
    }
    
    /* Calculate hash */
    for (size_t i = 0; i < 64; i++) {
        total += (size_t)root->data[i];
    }
    
    total += process_ast(root->left);
    total += process_ast(root->right);
    
    return total;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create recursive AST structure */
    ASTNode* root = create_ast(4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Test goto with memmove */
    char src_data[128];
    char dest_data[128];
    
    for (int i = 0; i < 128; i++) {
        src_data[i] = (char)(i * 3);
    }
    
    test_goto_memmove(dest_data, src_data, 64);
    test_goto_memmove(dest_data + 32, dest_data, 32);
    
    /* Execute parallel memory operations */
    parallel_mem_ops();
    
    /* Process AST with memory operations */
    size_t result = process_ast(root);
    printf("AST processing result: %zu\n", result);
    
    /* Additional built-in calls in different contexts */
    {
        char temp1[100], temp2[100];
        volatile int size_var = 50;
        
        __builtin_memset(temp1, 0xCC, sizeof(temp1));
        __builtin_memcpy(temp2, temp1, size_var);
        __builtin_memmove(temp1 + 10, temp1, 40);
    }
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully.\n");
    return 0;
}
