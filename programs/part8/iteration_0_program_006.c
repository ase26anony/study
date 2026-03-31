/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive function using builtin memcpy */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with volatile size */
    size_t copy_size = g_mem_size % 64;
    if (copy_size > 63) copy_size = 63;
    
    __builtin_memcpy(node->data, base_data, copy_size);
    node->size = copy_size;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    if (depth > 3) {
        create_left = 0;
        goto skip_left;
    }
    
    node->left = create_ast(depth - 1, base_data);
    
skip_left:
    node->right = create_ast(depth - 2, base_data);
    
    return node;
}

/* Function with goto jumping into memmove block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int use_memmove = 1;
    
    if (src == NULL || dst == NULL) {
        use_memmove = 0;
        goto cleanup;
    }
    
    /* Jump into memmove block */
    if (use_memmove) {
        goto do_memmove;
    }
    
    /* This label creates the goto edge case */
do_memmove:
    /* Use __builtin_memmove with overlapping regions */
    if (src->size > 0 && dst->size > 0) {
        size_t move_size = (src->size < dst->size) ? src->size : dst->size;
        __builtin_memmove(dst->data, src->data, move_size);
    }
    
    /* Jump out of the block */
    goto after_memmove;
    
cleanup:
    printf("Skipping memmove due to null pointers\n");
    
after_memmove:
    return;
}

/* Function using all three builtins in sequence */
static void memory_operations_sequence(char* buffer1, char* buffer2, size_t size) {
    volatile size_t op_size = size;
    
    /* 1. memset */
    __builtin_memset(buffer1, 0xAA, op_size);
    
    /* 2. memcpy */
    __builtin_memcpy(buffer2, buffer1, op_size);
    
    /* 3. memmove with overlap */
    size_t overlap = op_size / 2;
    if (overlap > 0) {
        __builtin_memmove(buffer1 + overlap, buffer1, op_size - overlap);
    }
}

/* OpenMP parallel section */
static void parallel_memory_operations(void) {
    const int num_buffers = 8;
    char* buffers[num_buffers];
    size_t sizes[num_buffers];
    
    #pragma omp parallel for
    for (int i = 0; i < num_buffers; i++) {
        sizes[i] = (g_mem_size * (i + 1)) % 256 + 16;
        buffers[i] = (char*)malloc(sizes[i]);
        
        if (buffers[i]) {
            /* Use all three builtins in parallel */
            __builtin_memset(buffers[i], i, sizes[i]);
            
            if (i > 0) {
                size_t copy_size = (sizes[i] < sizes[i-1]) ? sizes[i] : sizes[i-1];
                __builtin_memcpy(buffers[i], buffers[i-1], copy_size);
            }
            
            /* Create overlap for memmove */
            if (sizes[i] > 32) {
                __builtin_memmove(buffers[i] + 16, buffers[i], sizes[i] - 16);
            }
        }
    }
    
    /* Cleanup */
    #pragma omp parallel for
    for (int i = 0; i < num_buffers; i++) {
        free(buffers[i]);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Test 1: Recursive AST operations */
    ASTNode* root = create_ast(5, "TestDataForAST");
    if (root) {
        ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
        if (copy) {
            /* Force memcpy initialization */
            __builtin_memcpy(copy, root, sizeof(ASTNode));
            
            /* Test goto edge case */
            process_with_goto(root, copy);
            
            free(copy);
        }
        
        /* Cleanup AST */
        free(root);
    }
    
    /* Test 2: Buffer operations */
    char buffer1[256];
    char buffer2[256];
    
    memory_operations_sequence(buffer1, buffer2, sizeof(buffer1));
    
    /* Test 3: OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_operations();
    #endif
    
    /* Test 4: Direct builtin calls with volatile sizes */
    volatile size_t dynamic_size = g_mem_size % 128;
    char* dyn_buf1 = (char*)malloc(dynamic_size + 16);
    char* dyn_buf2 = (char*)malloc(dynamic_size + 16);
    
    if (dyn_buf1 && dyn_buf2) {
        /* Ensure all three builtins are called */
        __builtin_memset(dyn_buf1, 0xCC, dynamic_size);
        __builtin_memcpy(dyn_buf2, dyn_buf1, dynamic_size);
        __builtin_memmove(dyn_buf1 + 8, dyn_buf1, dynamic_size - 8);
        
        free(dyn_buf1);
        free(dyn_buf2);
    }
    
    /* Calculate and print verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash = (hash * 31) + (unsigned char)buffer1[i];
    }
    
    printf("Test completed. Verification hash: %lu\n", hash);
    printf("ASAN built-in redirection should be fully exercised\n");
    
    return 0;
}
