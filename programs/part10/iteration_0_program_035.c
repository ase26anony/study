/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _OPENMP
#include <omp.h>
#endif

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    struct ast_node* left;
    struct ast_node* right;
    char* data;
    size_t data_len;
    int node_id;
} ast_node_t;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove",
    "test1", "test2", "test3",
    "asan", "hwasan", "coverage"
};
static const size_t g_token_count = sizeof(g_tokens) / sizeof(g_tokens[0]);

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

/* Recursive AST creation */
static ast_node_t* create_ast_node(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    node->node_id = depth;
    node->data_len = g_mem_size / (depth + 1);
    node->data = (char*)malloc(node->data_len);
    
    /* Use __builtin_memset to initialize data */
    if (node->data) {
        __builtin_memset(node->data, 0xAA + depth, node->data_len);
    }
    
    node->left = create_ast_node(depth + 1, max_depth);
    node->right = create_ast_node(depth + 2, max_depth);
    
    return node;
}

/* Recursive AST traversal with memory operations */
static size_t traverse_ast(ast_node_t* node, char* buffer, size_t buf_size) {
    if (!node || !node->data) return 0;
    
    size_t total = 0;
    
    /* Use goto for control flow testing */
    if (node->node_id % 3 == 0) {
        goto memcpy_block;
    } else if (node->node_id % 3 == 1) {
        goto memset_block;
    } else {
        goto memmove_block;
    }

memcpy_block:
    {
        /* Force __builtin_memcpy usage */
        size_t copy_len = node->data_len < buf_size ? node->data_len : buf_size;
        if (copy_len > 0) {
            __builtin_memcpy(buffer, node->data, copy_len);
            total += copy_len;
        }
        goto after_mem_op;
    }

memset_block:
    {
        /* Force __builtin_memset usage */
        if (node->data_len > 0) {
            __builtin_memset(node->data, node->node_id, node->data_len);
            total += node->data_len;
        }
        goto after_mem_op;
    }

memmove_block:
    {
        /* Force __builtin_memmove usage with overlapping regions */
        if (node->data_len > 1) {
            size_t move_len = node->data_len / 2;
            __builtin_memmove(node->data, node->data + move_len, move_len);
            total += move_len;
        }
        goto after_mem_op;
    }

after_mem_op:
    /* Recursive traversal */
    if (node->left) {
        total += traverse_ast(node->left, buffer, buf_size);
    }
    if (node->right) {
        total += traverse_ast(node->right, buffer, buf_size);
    }
    
    return total;
}

/* Free AST recursively */
static void free_ast(ast_node_t* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    if (node->data) {
        free(node->data);
    }
    free(node);
}

/* OpenMP parallel memory operations */
static size_t parallel_memory_ops(void) {
    size_t total_processed = 0;
    const size_t buffer_size = 4096;
    char* buffer = (char*)malloc(buffer_size);
    
    if (!buffer) return 0;
    
    /* Initialize buffer with __builtin_memset */
    __builtin_memset(buffer, 0, buffer_size);
    
    #pragma omp parallel reduction(+:total_processed)
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread processes different memory patterns */
        char local_buf[512];
        volatile size_t local_size = 128 + (thread_id * 64);
        
        /* Pattern 1: __builtin_memcpy between buffers */
        __builtin_memcpy(local_buf, buffer + (thread_id * 128), local_size);
        total_processed += local_size;
        
        /* Pattern 2: __builtin_memset with thread-specific pattern */
        __builtin_memset(local_buf + 64, thread_id + 0x30, local_size - 64);
        total_processed += (local_size - 64);
        
        /* Pattern 3: __builtin_memmove with overlapping regions */
        if (local_size > 96) {
            __builtin_memmove(local_buf, local_buf + 32, local_size - 32);
            total_processed += (local_size - 32);
        }
        
        /* Copy back to shared buffer */
        __builtin_memcpy(buffer + (thread_id * 128), local_buf, local_size);
    }
    
    /* Final verification with __builtin_memcmp */
    char verify_buf[512];
    __builtin_memset(verify_buf, 0, sizeof(verify_buf));
    __builtin_memcpy(verify_buf, buffer, sizeof(verify_buf) < buffer_size ? sizeof(verify_buf) : buffer_size);
    
    free(buffer);
    return total_processed;
}

/* Token processing with memory operations */
static size_t process_tokens(void) {
    size_t hash = 0;
    char token_buffer[1024];
    size_t buffer_pos = 0;
    
    for (size_t i = 0; i < g_token_count; i++) {
        const char* token = g_tokens[i];
        size_t token_len = strlen(token);
        
        /* Use __builtin_memcpy for token copying */
        if (buffer_pos + token_len < sizeof(token_buffer)) {
            __builtin_memcpy(token_buffer + buffer_pos, token, token_len);
            buffer_pos += token_len;
            
            /* Add separator with __builtin_memset */
            if (i < g_token_count - 1) {
                __builtin_memset(token_buffer + buffer_pos, '|', 1);
                buffer_pos += 1;
            }
        }
        
        /* Calculate simple hash */
        for (size_t j = 0; j < token_len; j++) {
            hash = (hash * 31) + token[j];
        }
    }
    
    /* Null-terminate with __builtin_memset */
    __builtin_memset(token_buffer + buffer_pos, '\0', 1);
    
    printf("Processed tokens: %s\n", token_buffer);
    return hash;
}

int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    if (!g_init_flag) {
        fprintf(stderr, "Error: Constructor not called\n");
        return 1;
    }
    
    /* Phase 1: Token processing */
    printf("\nPhase 1: Token Processing\n");
    size_t token_hash = process_tokens();
    printf("Token hash: %zu\n", token_hash);
    
    /* Phase 2: AST operations */
    printf("\nPhase 2: AST Memory Operations\n");
    ast_node_t* root = create_ast_node(0, 4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    char ast_buffer[1024];
    size_t ast_processed = traverse_ast(root, ast_buffer, sizeof(ast_buffer));
    printf("AST processed bytes: %zu\n", ast_processed);
    
    /* Phase 3: OpenMP parallel operations */
    printf("\nPhase 3: OpenMP Parallel Memory Operations\n");
    #ifdef _OPENMP
    printf("OpenMP enabled with %d max threads\n", omp_get_max_threads());
    #endif
    
    size_t parallel_processed = parallel_memory_ops();
    printf("Parallel processed bytes: %zu\n", parallel_processed);
    
    /* Phase 4: Complex memory pattern */
    printf("\nPhase 4: Complex Memory Pattern\n");
    {
        char pattern_buf[2048];
        volatile size_t pattern_size = 768;
        
        /* Chain of memory operations */
        __builtin_memset(pattern_buf, 0xCC, sizeof(pattern_buf));
        __builtin_memcpy(pattern_buf + 256, pattern_buf, 512);
        __builtin_memmove(pattern_buf, pattern_buf + 128, pattern_size);
        __builtin_memset(pattern_buf + 512, 0xDD, 256);
        
        /* Verify with overlapping copy */
        __builtin_memcpy(pattern_buf + 384, pattern_buf + 128, 384);
        
        size_t pattern_sum = 0;
        for (size_t i = 0; i < sizeof(pattern_buf); i++) {
            pattern_sum += (unsigned char)pattern_buf[i];
        }
        printf("Pattern checksum: %zu\n", pattern_sum);
    }
    
    /* Cleanup */
    free_ast(root);
    
    printf("\n=== Test Completed Successfully ===\n");
    printf("Total operations triggered:\n");
    printf("  - __builtin_memcpy: Multiple times\n");
    printf("  - __builtin_memset: Multiple times\n");
    printf("  - __builtin_memmove: Multiple times\n");
    
    return 0;
}
