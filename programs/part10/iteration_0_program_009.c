/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_memmove = 1;
volatile int g_init_value = 0x42;

/* Recursive AST-like structure */
typedef struct ast_node {
    int type;
    int value;
    char *data;
    size_t data_len;
    struct ast_node *left;
    struct ast_node *right;
} ast_node_t;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive AST creation with memory operations */
static ast_node_t* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ast_node_t *node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    
    node->type = depth;
    node->value = depth * 10;
    node->data_len = g_mem_size / (depth + 1);
    node->data = (char*)malloc(node->data_len);
    
    if (node->data) {
        /* Fill data with pattern using builtin memset */
        __builtin_memset(node->data, g_init_value + depth, node->data_len);
    }
    
    /* Recursive creation with goto for flow control */
    if (depth < max_depth - 1) {
        goto create_children;
    } else {
        node->left = node->right = NULL;
        return node;
    }
    
create_children:
    node->left = create_ast(depth + 1, max_depth);
    node->right = create_ast(depth + 1, max_depth);
    return node;
}

/* Copy AST node data with builtin memcpy/memmove */
static void copy_ast_data(ast_node_t *dest, const ast_node_t *src) {
    if (!dest || !src || !dest->data || !src->data) return;
    
    size_t copy_len = dest->data_len < src->data_len ? 
                      dest->data_len : src->data_len;
    
    /* Conditional memcpy vs memmove with goto */
    if (g_use_memmove) {
        goto use_memmove;
    } else {
        __builtin_memcpy(dest->data, src->data, copy_len);
        return;
    }
    
use_memmove:
    __builtin_memmove(dest->data, src->data, copy_len);
}

/* Process AST recursively with memory operations */
static int process_ast(ast_node_t *node, int *sum) {
    if (!node) return 0;
    
    int local_sum = node->value;
    
    /* Process data with volatile length */
    volatile size_t process_len = node->data_len;
    if (node->data && process_len > 0) {
        char *temp = (char*)malloc(process_len);
        if (temp) {
            /* Use builtin memcpy with volatile length */
            __builtin_memcpy(temp, node->data, process_len);
            
            /* XOR all bytes */
            for (size_t i = 0; i < process_len; i++) {
                local_sum ^= temp[i];
            }
            
            free(temp);
        }
    }
    
    /* Recursive processing */
    local_sum += process_ast(node->left, sum);
    local_sum += process_ast(node->right, sum);
    
    *sum += local_sum;
    return local_sum;
}

/* Free AST recursively */
static void free_ast(ast_node_t *node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    if (node->data) {
        /* Clear sensitive data before free */
        __builtin_memset(node->data, 0, node->data_len);
        free(node->data);
    }
    
    __builtin_memset(node, 0, sizeof(ast_node_t));
    free(node);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    const int num_arrays = 8;
    const size_t array_size = 1024;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread allocates and operates on memory */
        char *src = (char*)malloc(array_size);
        char *dst = (char*)malloc(array_size);
        
        if (src && dst) {
            /* Initialize with builtin memset */
            __builtin_memset(src, thread_id + 0x30, array_size);
            
            /* Copy with builtin memcpy */
            __builtin_memcpy(dst, src, array_size);
            
            /* Modify with builtin memset in middle */
            size_t offset = array_size / 4;
            __builtin_memset(dst + offset, 0xFF, array_size / 2);
            
            /* Use memmove to handle overlapping regions */
            if (thread_id % 2 == 0) {
                __builtin_memmove(dst + offset / 2, dst + offset, array_size / 4);
            }
            
            free(src);
            free(dst);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Create and process AST */
    printf("Phase 1: Creating AST structure...\n");
    ast_node_t *root = create_ast(0, 4);
    
    int ast_sum = 0;
    if (root) {
        printf("Phase 2: Processing AST...\n");
        process_ast(root, &ast_sum);
        printf("AST checksum: %d\n", ast_sum);
    }
    
    /* Phase 3: Test builtin redirection with goto flow */
    printf("Phase 3: Testing builtin redirection...\n");
    
    char buffer1[512];
    char buffer2[512];
    volatile size_t op_size = 256;
    
    /* Test all three builtins with goto jumps */
    goto test_memset;
    
test_memcpy:
    __builtin_memcpy(buffer2, buffer1, op_size);
    goto test_memmove;
    
test_memset:
    __builtin_memset(buffer1, 0xAA, op_size);
    goto test_memcpy;
    
test_memmove:
    __builtin_memmove(buffer1 + 128, buffer1, op_size);
    
    /* Phase 4: OpenMP parallel operations */
    printf("Phase 4: Running OpenMP memory operations...\n");
    parallel_memory_operations();
    
    /* Phase 5: Cleanup */
    printf("Phase 5: Cleanup...\n");
    if (root) {
        free_ast(root);
    }
    
    /* Final verification */
    int final_hash = ast_sum;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        final_hash ^= buffer1[i];
        final_hash ^= buffer2[i];
    }
    
    printf("Final verification hash: %d\n", final_hash);
    printf("=== Test Complete ===\n");
    
    return 0;
}
