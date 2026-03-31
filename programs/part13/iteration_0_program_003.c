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

/* Global token array */
static char g_token_array[4096];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Force initialization of memory functions */
    char buffer[256];
    volatile char* volatile_ptr = buffer;
    
    /* Call all three builtins in constructor */
    __builtin_memset(volatile_ptr, 0xAA, sizeof(buffer));
    __builtin_memcpy(g_token_array, volatile_ptr, 128);
    __builtin_memmove(g_token_array + 128, g_token_array, 128);
    
    g_init_flag = 1;
    printf("[Constructor] ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("[Destructor] Cleaning up ASAN environment\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* data, size_t depth) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with builtin memcpy */
    size_t copy_len = strlen(data);
    if (copy_len > sizeof(node->data) - 1)
        copy_len = sizeof(node->data) - 1;
    
    __builtin_memcpy(node->data, data, copy_len);
    node->data[copy_len] = '\0';
    node->size = copy_len;
    
    /* Recursive creation with goto for flow control */
    if (depth > 0) {
        char child_data[64];
        volatile size_t v_depth = depth;
        
        /* Complex goto pattern around memmove */
        if (v_depth % 2 == 0) {
            goto create_left;
        } else {
            goto create_right;
        }
        
    create_left:
        snprintf(child_data, sizeof(child_data), "%s-L%d", data, (int)depth);
        node->left = create_ast_node(child_data, depth - 1);
        goto after_left;
        
    create_right:
        snprintf(child_data, sizeof(child_data), "%s-R%d", data, (int)depth);
        node->right = create_ast_node(child_data, depth - 1);
        goto after_right;
        
    after_left:
    after_right:
        /* Use memmove between nodes if both exist */
        if (node->left && node->right) {
            volatile size_t move_size = sizeof(node->left->data);
            if (move_size > sizeof(node->right->data))
                move_size = sizeof(node->right->data);
            
            __builtin_memmove(node->right->data, node->left->data, move_size);
        }
    }
    
    return node;
}

/* Calculate hash from AST */
static size_t ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 5381;
    volatile char* ptr = node->data;
    
    /* Process string with volatile pointer */
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr;
        ptr++;
    }
    
    /* Recursive hash calculation */
    size_t left_hash = ast_hash(node->left);
    size_t right_hash = ast_hash(node->right);
    
    /* Combine hashes */
    volatile size_t combined = hash ^ (left_hash << 1) ^ (right_hash >> 1);
    return combined;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    volatile char* data_ptr = node->data;
    __builtin_memset(data_ptr, 0, sizeof(node->data));
    free(node);
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[512];
        char src_buf[512];
        
        /* Initialize source with builtin */
        volatile int init_val = thread_id * 0x11;
        __builtin_memset(src_buf, init_val, sizeof(src_buf));
        
        /* Copy to local buffer */
        volatile size_t copy_size = sizeof(local_buf);
        if (copy_size > sizeof(src_buf))
            copy_size = sizeof(src_buf);
        
        __builtin_memcpy(local_buf, src_buf, copy_size);
        
        /* Move data around */
        volatile size_t move_offset = thread_id * 16;
        if (move_offset + 256 <= sizeof(local_buf)) {
            __builtin_memmove(local_buf + move_offset, 
                            local_buf, 256);
        }
        
        /* Update global token array */
        #pragma omp critical
        {
            size_t idx = g_token_idx;
            if (idx + copy_size < sizeof(g_token_array)) {
                __builtin_memcpy(g_token_array + idx, 
                               local_buf, copy_size);
                g_token_idx += copy_size;
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Initialize complex data structures */
    ASTNode* root = create_ast_node("ROOT", 3);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Perform parallel memory operations */
    printf("Executing parallel memory operations...\n");
    parallel_memory_operations();
    
    /* Phase 3: Process data with builtins */
    char processing_buffer[1024];
    volatile size_t process_size = g_token_idx;
    if (process_size > sizeof(processing_buffer))
        process_size = sizeof(processing_buffer);
    
    /* Chain of memory operations */
    __builtin_memset(processing_buffer, 0, sizeof(processing_buffer));
    __builtin_memcpy(processing_buffer, g_token_array, process_size);
    
    /* Self-overlapping memmove */
    if (process_size > 256) {
        __builtin_memmove(processing_buffer + 128, 
                         processing_buffer, 256);
    }
    
    /* Phase 4: Calculate and verify results */
    size_t ast_hash_value = ast_hash(root);
    size_t buffer_hash = 0;
    
    volatile char* buf_ptr = processing_buffer;
    for (size_t i = 0; i < process_size; i++) {
        buffer_hash = ((buffer_hash << 5) + buffer_hash) + buf_ptr[i];
    }
    
    /* Final memory operation with goto */
    volatile int use_memmove = (ast_hash_value % 2 == 0);
    char final_buffer[64];
    
    if (use_memmove) {
        goto use_memmove_path;
    } else {
        goto use_memcpy_path;
    }
    
use_memmove_path:
    __builtin_memmove(final_buffer, processing_buffer, 
                     sizeof(final_buffer));
    goto after_final_op;
    
use_memcpy_path:
    __builtin_memcpy(final_buffer, processing_buffer, 
                    sizeof(final_buffer));
    goto after_final_op;
    
after_final_op:
    /* Calculate final verification value */
    size_t final_hash = ast_hash_value ^ buffer_hash;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        final_hash = ((final_hash << 3) + final_hash) + final_buffer[i];
    }
    
    printf("Verification hash: 0x%08zx\n", final_hash);
    printf("AST nodes processed: Hash=0x%08zx\n", ast_hash_value);
    printf("Buffer hash: 0x%08zx\n", buffer_hash);
    printf("Total memory operations: %zu bytes\n", 
           g_token_idx + process_size);
    
    /* Cleanup */
    free_ast(root);
    
    printf("ASAN test completed successfully\n");
    return 0;
}
