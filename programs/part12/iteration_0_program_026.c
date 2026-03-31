/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    struct ast_node* left;
    struct ast_node* right;
    char data[64];
    uint32_t hash;
} ast_node_t;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    printf("ASAN Test Constructor: Initializing test environment\n");
    g_mem_size = 256 + (rand() % 768);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("ASAN Test Destructor: Cleaning up\n");
}

/* Recursive function with memory operations */
static ast_node_t* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    
    /* Copy data with builtin memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) 
        copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Create hash using volatile size */
    uint32_t hash = 0;
    for (size_t i = 0; i < copy_len; i++) {
        hash = (hash * 31) + node->data[i];
    }
    node->hash = hash;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        char left_data[32], right_data[32];
        snprintf(left_data, sizeof(left_data), "%s_L%d", base_data, depth);
        snprintf(right_data, sizeof(right_data), "%s_R%d", base_data, depth);
        
        /* Use goto to create unusual control flow */
        create_left:
        node->left = create_ast(depth - 1, left_data);
        
        /* Jump over right creation in some cases */
        if (depth % 2 == 0) goto skip_right;
        
        create_right:
        node->right = create_ast(depth - 1, right_data);
        goto done;
        
        skip_right:
        node->right = NULL;
        done:;
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with complex memory movement using builtins */
static void process_ast(ast_node_t* node, char* buffer, size_t buf_size) {
    if (!node || !buffer) return;
    
    /* Use volatile to prevent constant folding */
    volatile size_t local_size = buf_size;
    
    /* Clear buffer with builtin memset */
    __builtin_memset(buffer, 0, local_size);
    
    /* Copy node data with builtin memcpy */
    size_t data_len = strlen(node->data);
    if (data_len > local_size - 1) 
        data_len = local_size - 1;
    
    __builtin_memcpy(buffer, node->data, data_len);
    
    /* Move data around with builtin memmove */
    if (data_len > 16) {
        __builtin_memmove(buffer + 8, buffer, data_len - 8);
        __builtin_memset(buffer, 'X', 8);
    }
    
    /* Process children recursively */
    if (node->left) {
        char left_buf[128];
        process_ast(node->left, left_buf, sizeof(left_buf));
        
        /* Combine buffers */
        if (strlen(left_buf) > 0) {
            size_t offset = strlen(buffer);
            size_t remaining = local_size - offset - 1;
            if (remaining > 0) {
                size_t copy_len = strlen(left_buf);
                if (copy_len > remaining) copy_len = remaining;
                __builtin_memcpy(buffer + offset, left_buf, copy_len);
            }
        }
    }
    
    if (node->right) {
        char right_buf[128];
        process_ast(node->right, right_buf, sizeof(right_buf));
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_arrays = 8;
    char* arrays[num_arrays];
    volatile size_t block_size = g_mem_size / num_arrays;
    
    #pragma omp parallel for
    for (int i = 0; i < num_arrays; i++) {
        arrays[i] = malloc(block_size);
        if (arrays[i]) {
            /* Each thread uses builtins independently */
            __builtin_memset(arrays[i], i + 'A', block_size);
            
            /* Create pattern with memcpy */
            if (i > 0) {
                size_t copy_size = (block_size > 64) ? 64 : block_size;
                __builtin_memcpy(arrays[i], arrays[i-1], copy_size);
            }
            
            /* Move data within array */
            if (block_size > 128) {
                __builtin_memmove(arrays[i] + 32, arrays[i], block_size - 32);
            }
        }
    }
    
    /* Cleanup */
    #pragma omp parallel for
    for (int i = 0; i < num_arrays; i++) {
        if (arrays[i]) free(arrays[i]);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: AST operations */
    ast_node_t* root = create_ast(4, "ROOT");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    char result_buffer[512];
    process_ast(root, result_buffer, sizeof(result_buffer));
    
    /* Calculate verification hash */
    uint32_t final_hash = 0;
    for (size_t i = 0; i < strlen(result_buffer); i++) {
        final_hash = (final_hash * 31) + result_buffer[i];
    }
    printf("AST Processing Hash: 0x%08X\n", final_hash);
    
    /* Phase 2: Parallel memory operations */
    printf("Starting parallel memory operations\n");
    parallel_memory_ops();
    
    /* Phase 3: Direct builtin calls with goto control flow */
    {
        char src[256], dst[256];
        volatile size_t test_size = 128;
        
        __builtin_memset(src, 'S', sizeof(src));
        
        /* Complex goto pattern around memmove */
        if (final_hash & 1) {
            goto use_memcpy;
        }
        
        use_memmove:
        __builtin_memmove(dst, src, test_size);
        goto verify;
        
        use_memcpy:
        __builtin_memcpy(dst, src, test_size);
        goto use_memmove;  /* Jump back */
        
        verify:
        /* Verify copy */
        int match = 1;
        for (size_t i = 0; i < test_size; i++) {
            if (dst[i] != src[i]) {
                match = 0;
                break;
            }
        }
        printf("Memory verification: %s\n", match ? "PASS" : "FAIL");
    }
    
    /* Cleanup */
    /* Note: In real code, you'd need to properly free the AST */
    
    printf("Test completed successfully\n");
    return 0;
}
