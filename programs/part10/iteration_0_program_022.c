/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
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
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    printf("ASAN Test Constructor: Initializing...\n");
    /* Force early initialization of memory functions */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("ASAN Test Destructor: Cleaning up...\n");
}

/* Function with goto statements for control flow testing */
static void test_memmove_with_goto(char* dest, const char* src, size_t n) {
    int use_fast_path = 0;
    
    if (n > 128) {
        use_fast_path = 1;
        goto fast_path;
    }
    
    /* Normal path with memmove */
    __builtin_memmove(dest, src, n);
    goto end;
    
fast_path:
    /* Alternative path with different memmove usage */
    char temp[256];
    __builtin_memcpy(temp, src, n > 256 ? 256 : n);
    __builtin_memmove(dest, temp, n > 256 ? 256 : n);
    
end:
    /* Final touch with memset */
    __builtin_memset(dest + n - 1, '\0', 1);
}

/* Recursive function operating on AST structure */
static ASTNode* create_ast_recursive(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for data initialization */
    size_t data_len = strlen(base_data) + 1;
    __builtin_memcpy(node->data, base_data, data_len > 256 ? 256 : data_len);
    node->size = data_len;
    
    /* Recursive creation with different memory operations */
    char child_data[256];
    __builtin_memset(child_data, 'A' + depth, 255);
    child_data[255] = '\0';
    
    node->left = create_ast_recursive(depth - 1, child_data);
    
    __builtin_memset(child_data, 'Z' - depth, 255);
    child_data[255] = '\0';
    
    node->right = create_ast_recursive(depth - 1, child_data);
    
    return node;
}

/* Function that copies between AST nodes */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Direct memcpy between structures */
    __builtin_memcpy(dest->data, src->data, 
                    src->size < sizeof(dest->data) ? src->size : sizeof(dest->data));
    
    /* Recursive copying */
    if (src->left && dest->left) {
        copy_ast_data(dest->left, src->left);
    }
    if (src->right && dest->right) {
        copy_ast_data(dest->right, src->right);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    const int array_size = 1024;
    char* buffers[4];
    
    #pragma omp parallel for
    for (int i = 0; i < 4; i++) {
        buffers[i] = (char*)malloc(array_size);
        if (buffers[i]) {
            /* Each thread uses different memory builtins */
            switch (i % 3) {
                case 0:
                    __builtin_memset(buffers[i], i, array_size);
                    break;
                case 1:
                    __builtin_memcpy(buffers[i], buffers[(i + 1) % 4], 
                                   array_size > 512 ? 512 : array_size);
                    break;
                case 2:
                    __builtin_memmove(buffers[i], buffers[(i + 2) % 4], 
                                    array_size > 256 ? 256 : array_size);
                    break;
            }
        }
    }
    
    /* Cleanup */
    #pragma omp parallel for
    for (int i = 0; i < 4; i++) {
        if (buffers[i]) {
            free(buffers[i]);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Basic built-in usage */
    char buffer1[512];
    char buffer2[512];
    
    /* Force all three builtins to be used */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 128, buffer1, 256);
    
    /* Phase 2: Control flow with goto */
    test_memmove_with_goto(buffer2, buffer1, g_mem_size);
    
    /* Phase 3: Recursive AST operations */
    ASTNode* ast1 = create_ast_recursive(3, "Base AST Node Data");
    ASTNode* ast2 = create_ast_recursive(3, "Another AST Node");
    
    if (ast1 && ast2) {
        copy_ast_data(ast2, ast1);
        
        /* Additional memory operations on AST */
        __builtin_memmove(ast1->data + 10, ast2->data + 5, 100);
    }
    
    /* Phase 4: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 5: Variable-sized operations */
    volatile size_t dynamic_size = g_mem_size * 2;
    char* dynamic_buf = (char*)malloc(dynamic_size);
    if (dynamic_buf) {
        __builtin_memset(dynamic_buf, 0xCC, dynamic_size);
        __builtin_memcpy(dynamic_buf + 32, buffer1, 128);
        __builtin_memmove(dynamic_buf, dynamic_buf + 64, 64);
        free(dynamic_buf);
    }
    
    /* Verification phase */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash = hash * 31 + buffer1[i];
    }
    
    printf("Test completed. Buffer hash: %lu\n", hash);
    printf("All memory built-ins should have been redirected through ASAN/HWASAN\n");
    
    /* Cleanup */
    /* Note: In real usage, you'd need proper AST cleanup */
    
    return 0;
}
