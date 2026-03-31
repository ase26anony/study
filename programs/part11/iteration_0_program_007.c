/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int depth;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 128;  /* Force non-constant size */
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with goto for flow control */
    size_t copy_len = g_mem_size % 256;
    if (copy_len > 255) copy_len = 255;
    
    /* Jump into memory operation block */
    goto mem_operation;
    
mem_operation:
    __builtin_memcpy(node->data, base_data, copy_len);
    node->depth = depth;
    
    /* Conditional jump out of block */
    if (depth > 1) {
        node->left = create_ast(depth - 1, base_data);
        node->right = create_ast(depth - 1, base_data);
        
        /* Move data between nodes using __builtin_memmove */
        if (node->left && node->right) {
            size_t move_size = (g_mem_size % 128) + 1;
            __builtin_memmove(node->right->data, node->left->data, move_size);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with complex control flow and builtins */
static void process_ast(ASTNode* root) {
    if (!root) return;
    
    volatile int use_memcpy = 1;
    char buffer[512];
    
    /* Multiple goto statements for control flow testing */
    if (root->depth % 2 == 0) {
        goto use_memset;
    } else {
        goto use_memcpy_block;
    }

use_memset:
    /* Force __builtin_memset call */
    __builtin_memset(buffer, root->depth, sizeof(buffer));
    goto continue_processing;

use_memcpy_block:
    /* Force __builtin_memcpy call */
    __builtin_memcpy(buffer, root->data, g_mem_size % 512);
    goto continue_processing;

continue_processing:
    /* Process children recursively */
    process_ast(root->left);
    process_ast(root->right);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        size_t size = (tid + 1) * g_mem_size;
        
        buffers[tid] = (char*)malloc(size);
        if (buffers[tid]) {
            /* Each thread uses different builtins */
            switch (tid % 3) {
                case 0:
                    __builtin_memset(buffers[tid], tid, size);
                    break;
                case 1:
                    __builtin_memcpy(buffers[tid], "ThreadData", 10);
                    break;
                case 2:
                    __builtin_memmove(buffers[tid] + 5, buffers[tid], size - 5);
                    break;
            }
        }
        
        #pragma omp barrier
        
        /* Cross-thread memory operations */
        if (tid > 0) {
            size_t copy_size = g_mem_size % 64;
            __builtin_memcpy(buffers[tid], buffers[tid-1], copy_size);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        if (buffers[i]) free(buffers[i]);
    }
}

/* Multi-stage initialization with memory builtins */
static void initialize_test_data(char* dest, const char* src, size_t size) {
    volatile size_t actual_size = size;
    
    /* Three-phase initialization using all three builtins */
    __builtin_memset(dest, 0xAA, actual_size);
    
    if (src) {
        __builtin_memcpy(dest, src, actual_size / 2);
        __builtin_memmove(dest + actual_size/2, dest, actual_size/2);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* ast = create_ast(4, "BaseASTData");
    if (ast) {
        process_ast(ast);
        
        /* Calculate hash/sum for verification */
        unsigned long sum = 0;
        ASTNode* stack[100];
        int top = 0;
        stack[top++] = ast;
        
        while (top > 0) {
            ASTNode* node = stack[--top];
            for (int i = 0; i < 256; i++) {
                sum += (unsigned char)node->data[i];
            }
            if (node->right) stack[top++] = node->right;
            if (node->left) stack[top++] = node->left;
        }
        
        printf("AST data sum: %lu\n", sum);
        
        /* Cleanup AST */
        /* ... (recursive free implementation omitted for brevity) */
        free(ast);
    }
    
    /* Phase 2: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Phase 3: Direct builtin calls with volatile control */
    char buffer1[1024], buffer2[1024];
    volatile int use_memmove = 1;
    
    initialize_test_data(buffer1, "InitializationString", 256);
    
    if (use_memmove) {
        __builtin_memmove(buffer2, buffer1, g_mem_size % 1024);
    } else {
        __builtin_memcpy(buffer2, buffer1, g_mem_size % 1024);
    }
    
    /* Final verification memset */
    __builtin_memset(buffer1, 0xFF, 512);
    
    printf("ASAN test completed successfully\n");
    return 0;
}
