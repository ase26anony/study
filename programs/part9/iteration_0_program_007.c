/* asan_coverage.c - Comprehensive test for ASAN memory builtin redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    char data[256];
    struct ast_node *left;
    struct ast_node *right;
    int id;
};

/* Global token array */
static char token_pool[4096];
volatile char *volatile_token_ptr = token_pool;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    /* Force initialization of memory functions */
    volatile char buffer[128];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    
    /* Copy between volatile pointers */
    volatile char *src = (volatile char *)token_pool;
    volatile char *dst = (volatile char *)buffer;
    __builtin_memcpy((void *)dst, (const void *)src, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    struct ast_node *node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize node data with builtins */
    __builtin_memset(node->data, id, sizeof(node->data));
    
    /* Copy pattern using volatile size */
    volatile size_t copy_size = g_mem_size % 128;
    __builtin_memcpy(node->data + 64, token_pool + id * 16, copy_size);
    
    /* Recursive calls */
    node->left = build_ast(depth - 1, id * 2);
    node->right = build_ast(depth - 1, id * 2 + 1);
    node->id = id;
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(struct ast_node *a, struct ast_node *b) {
    volatile int use_memmove = 0;
    
    if (a && b) {
        use_memmove = 1;
        goto perform_copy;
    }
    
    /* Normal path */
    if (a) {
        __builtin_memset(a->data, 0xFF, 128);
    }
    return;
    
perform_copy:
    /* Jumped-to block with memmove */
    volatile size_t move_len = g_mem_size % 256;
    __builtin_memmove(b->data, a->data, move_len);
    
    /* Jump back */
    if (b->id > 100) {
        goto cleanup;
    }
    
    /* Another memcpy after goto */
    __builtin_memcpy(a->data + 128, b->data + 64, 32);
    
cleanup:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(struct ast_node **nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        volatile char local_buf[512];
        volatile size_t op_size = (g_mem_size + i * 16) % 512;
        
        /* Mix of memory builtins */
        __builtin_memset(local_buf, i, op_size);
        
        if (nodes[i]) {
            __builtin_memcpy(nodes[i]->data, local_buf, op_size);
            
            /* Conditional memmove */
            if (i > 0 && nodes[i-1]) {
                volatile size_t move_size = (op_size / 2) + 1;
                __builtin_memmove(nodes[i]->data + 128, 
                                 nodes[i-1]->data + 64, 
                                 move_size);
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    int i, result_hash = 0;
    struct ast_node *ast_root;
    struct ast_node *node_array[8];
    
    /* Initialize token pool with pattern */
    for (i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (i % 256);
    }
    
    /* Build recursive structure */
    ast_root = build_ast(4, 1);
    
    /* Create node array for parallel processing */
    for (i = 0; i < 8; i++) {
        node_array[i] = build_ast(3, i + 100);
    }
    
    /* Test goto flow with memory operations */
    process_with_goto(ast_root, node_array[0]);
    
    /* OpenMP parallel section */
    #pragma omp parallel
    {
        #pragma omp single
        {
            parallel_memory_ops(node_array, 8);
        }
    }
    
    /* Additional memory operations in main */
    volatile char main_buf[1024];
    volatile size_t final_size = g_mem_size % 1024;
    
    __builtin_memset(main_buf, 0xCC, final_size);
    __builtin_memcpy(main_buf + 512, ast_root->data, 256);
    __builtin_memmove(main_buf + 256, main_buf + 512, 128);
    
    /* Compute verification hash */
    for (i = 0; i < 256; i++) {
        result_hash += main_buf[i];
        if (ast_root) result_hash += ast_root->data[i % 128];
    }
    
    for (i = 0; i < 8; i++) {
        if (node_array[i]) {
            result_hash += node_array[i]->id;
        }
    }
    
    printf("Result hash: %d\n", result_hash);
    
    /* Cleanup */
    /* ... (free allocated memory) ... */
    
    return result_hash != 0 ? 0 : 1;
}
