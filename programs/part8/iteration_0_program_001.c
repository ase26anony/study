/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Recursive AST-like structure */
struct ast_node {
    char data[256];
    struct ast_node* left;
    struct ast_node* right;
    int id;
};

/* Global token array */
static char token_pool[4096];

/* Constructor function (runs before main) */
static void __attribute__((constructor)) init_tokens(void) {
    for (size_t i = 0; i < sizeof(token_pool); ++i) {
        token_pool[i] = (char)(i % 256);
    }
}

/* Destructor function (runs after main) */
static void __attribute__((destructor)) cleanup(void) {
    /* Force built-in usage in destructor */
    volatile char buf[32];
    __builtin_memset(buf, 0, sizeof(buf));
}

/* Recursive parser with memory operations */
static struct ast_node* parse_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    struct ast_node* node = (struct ast_node*)malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(*node));
    node->id = depth;
    
    /* Copy token data using builtin memcpy */
    size_t copy_len = g_memcpy_len % 256;
    __builtin_memcpy(node->data, token_pool + depth * 64, copy_len);
    
    /* Recursive construction */
    node->left = parse_ast(depth + 1, max_depth);
    node->right = parse_ast(depth + 2, max_depth);
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(struct ast_node* src, struct ast_node* dst) {
    volatile int use_builtin = 1;
    
    if (src && dst) {
        goto copy_block;
    } else {
        goto skip_copy;
    }
    
copy_block:
    /* This block tests flow sensitivity */
    __builtin_memmove(dst->data, src->data, g_memmove_len % 128);
    goto after_copy;
    
skip_copy:
    /* Alternative path */
    volatile char tmp[16];
    __builtin_memset(tmp, 0xFF, sizeof(tmp));
    
after_copy:
    /* Common continuation */
    if (use_builtin) {
        __builtin_memcpy(dst->data + 64, src->data + 64, 32);
    }
}

/* OpenMP parallel memory dispatcher */
static void parallel_memory_ops(struct ast_node** nodes, size_t count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (size_t i = 0; i < count; ++i) {
            if (nodes[i]) {
                /* Thread-specific memory operations */
                volatile char local_buf[128];
                
                /* Use all three builtins */
                __builtin_memset(local_buf, tid, sizeof(local_buf));
                __builtin_memcpy(nodes[i]->data + 128, local_buf, 64);
                
                if (i > 0 && nodes[i-1]) {
                    __builtin_memmove(nodes[i]->data, nodes[i-1]->data, 48);
                }
            }
        }
    }
}

/* Calculate hash from AST */
static unsigned long compute_ast_hash(struct ast_node* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    for (size_t i = 0; i < sizeof(node->data); ++i) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(struct ast_node* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create AST */
    struct ast_node* root = parse_ast(0, 4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    struct ast_node* nodes[8];
    nodes[0] = root;
    for (int i = 1; i < 8; ++i) {
        nodes[i] = parse_ast(i, 3);
    }
    
    /* Test goto edge cases */
    process_with_goto(nodes[0], nodes[1]);
    process_with_goto(nodes[2], nodes[3]);
    
    /* Force built-in usage in main */
    volatile char main_buf[256];
    __builtin_memset(main_buf, 0xAA, sizeof(main_buf));
    __builtin_memcpy(main_buf + 64, token_pool, 128);
    __builtin_memmove(main_buf + 128, main_buf, 64);
    
    /* Execute parallel operations */
    parallel_memory_ops(nodes, 8);
    
    /* Compute and print verification hash */
    unsigned long total_hash = 0;
    for (int i = 0; i < 8; ++i) {
        if (nodes[i]) {
            total_hash ^= compute_ast_hash(nodes[i]);
        }
    }
    
    printf("Verification hash: %lu\n", total_hash);
    printf("Built-in redirection test completed\n");
    
    /* Cleanup */
    for (int i = 0; i < 8; ++i) {
        free_ast(nodes[i]);
    }
    
    return 0;
}
