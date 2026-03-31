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
static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with __builtin_memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) 
        copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Create children with goto-based control flow */
    int create_left = 1;
    int create_right = 1;
    
    /* Goto into memory operation block */
    if (depth > 2) {
        goto create_children;
    }
    
    skip_children:
    node->left = NULL;
    node->right = NULL;
    node->size = copy_len;
    return node;
    
    create_children:
    /* Use volatile to control flow */
    volatile int should_create = depth % 2;
    if (should_create) {
        char left_data[64];
        char right_data[64];
        
        /* Prepare data with __builtin_memcpy */
        __builtin_memcpy(left_data, "LEFT_", 5);
        __builtin_memcpy(left_data + 5, base_data, copy_len);
        
        __builtin_memcpy(right_data, "RIGHT_", 6);
        __builtin_memcpy(right_data + 6, base_data, copy_len);
        
        node->left = create_ast(depth - 1, left_data);
        node->right = create_ast(depth - 1, right_data);
        
        /* Move data between nodes with __builtin_memmove */
        if (node->left && node->right) {
            size_t move_size = node->left->size;
            if (move_size > sizeof(node->right->data))
                move_size = sizeof(node->right->data);
            __builtin_memmove(node->right->data, node->left->data, move_size);
        }
    }
    goto skip_children;
}

/* Parallel memory operations with OpenMP */
static void parallel_mem_operations(void) {
    const int num_arrays = 8;
    char* arrays[num_arrays];
    
    #pragma omp parallel for
    for (int i = 0; i < num_arrays; i++) {
        /* Allocate with volatile size */
        size_t size = g_mem_size + i * 16;
        arrays[i] = malloc(size);
        
        if (arrays[i]) {
            /* Initialize with __builtin_memset */
            __builtin_memset(arrays[i], i, size);
            
            /* Copy between arrays with __builtin_memcpy */
            if (i > 0) {
                size_t copy_size = (size < g_mem_size) ? size : g_mem_size;
                __builtin_memcpy(arrays[i], arrays[i-1], copy_size);
            }
            
            /* Move data within array with __builtin_memmove */
            if (size > 32) {
                __builtin_memmove(arrays[i] + 16, arrays[i], 16);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_arrays; i++) {
        free(arrays[i]);
    }
}

/* Complex token processing with control flow */
static size_t process_tokens(const char** tokens, int count) {
    size_t total_hash = 0;
    char buffer[512];
    
    for (int i = 0; i < count; i++) {
        /* Clear buffer with __builtin_memset */
        __builtin_memset(buffer, 0, sizeof(buffer));
        
        /* Copy token with goto-based control flow */
        if (i % 3 == 0) {
            goto copy_token;
        } else if (i % 3 == 1) {
            goto move_token;
        } else {
            goto process_directly;
        }
        
        copy_token:
        __builtin_memcpy(buffer, tokens[i], strlen(tokens[i]));
        goto compute_hash;
        
        move_token:
        /* First copy, then move */
        __builtin_memcpy(buffer + 128, tokens[i], strlen(tokens[i]));
        __builtin_memmove(buffer, buffer + 128, strlen(tokens[i]));
        goto compute_hash;
        
        process_directly:
        /* Direct processing */
        size_t len = strlen(tokens[i]);
        __builtin_memset(buffer, 'X', len);
        __builtin_memcpy(buffer, tokens[i], len);
        
        compute_hash:
        /* Compute simple hash */
        for (int j = 0; buffer[j] && j < 64; j++) {
            total_hash += buffer[j] * (i + 1);
        }
    }
    
    return total_hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast(4, "ROOT_NODE");
    
    /* Phase 2: Parallel memory operations */
    parallel_mem_operations();
    
    /* Phase 3: Token processing */
    const char* tokens[] = {
        "MEMCPY_TEST_123",
        "MEMSET_OPERATION",
        "MEMMOVE_EXAMPLE",
        "ASAN_INSTRUMENT",
        "HWASAN_CHECK",
        "BUILTIN_REDIRECT"
    };
    
    size_t token_hash = process_tokens(tokens, 
                                      sizeof(tokens)/sizeof(tokens[0]));
    
    /* Phase 4: Direct built-in calls with volatile control */
    char final_buffer[1024];
    volatile int use_memcpy = 1;
    
    if (use_memcpy) {
        __builtin_memset(final_buffer, 0, sizeof(final_buffer));
        __builtin_memcpy(final_buffer, "FINAL_RESULT_", 13);
        __builtin_memcpy(final_buffer + 13, &token_hash, sizeof(token_hash));
        
        /* Move to different location */
        __builtin_memmove(final_buffer + 500, final_buffer, 100);
    }
    
    /* Print verification result */
    printf("Test completed. Token hash: %zu\n", token_hash);
    printf("Buffer[0] = %c, Buffer[500] = %c\n", 
           final_buffer[0], final_buffer[500]);
    
    /* Cleanup */
    /* Note: In real code, would need recursive free function for AST */
    
    return 0;
}
