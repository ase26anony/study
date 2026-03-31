/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_memcpy_len = 64;
static volatile size_t g_memset_len = 128;
static volatile size_t g_memmove_len = 96;

/* Recursive AST-like structure */
struct ast_node {
    int type;
    char data[256];
    struct ast_node *left;
    struct ast_node *right;
};

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_globals(void) {
    /* Force initialization before main */
    printf("Constructor: Initializing ASAN/HWASAN environment\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor)) 
static void cleanup_globals(void) {
    printf("Destructor: ASAN/HWASAN cleanup complete\n");
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    struct ast_node *node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Use builtin memset to initialize node */
    __builtin_memset(node, 0, sizeof(struct ast_node));
    
    node->type = depth;
    
    /* Use volatile length for builtin memcpy */
    size_t copy_len = g_memcpy_len % 256;
    __builtin_memcpy(node->data, "AST_NODE_DATA", copy_len);
    
    /* Recursive construction */
    node->left = build_ast(depth + 1, max_depth);
    node->right = build_ast(depth + 1, max_depth);
    
    return node;
}

/* Function with goto jumps around memory operations */
static void process_with_goto(struct ast_node *src, struct ast_node *dst) {
    int state = 0;
    
    /* Jump into memory operation block */
    goto entry_point;
    
memory_operation:
    /* This block contains the builtin memmove */
    if (src && dst) {
        size_t move_len = g_memmove_len % 256;
        __builtin_memmove(dst->data, src->data, move_len);
    }
    state = 1;
    goto exit_point;
    
entry_point:
    /* Jump to memory operation */
    goto memory_operation;
    
exit_point:
    /* Jump out of block */
    if (state) {
        /* Additional memory operation after goto */
        size_t set_len = g_memset_len % 256;
        __builtin_memset(dst->data + 128, 0xAA, set_len);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(struct ast_node **nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Mix of builtin memory operations */
            size_t len = (g_memcpy_len + i) % 256;
            
            /* Force all three builtins to be called */
            __builtin_memset(nodes[i]->data, i, len);
            
            if (i > 0 && nodes[i-1]) {
                __builtin_memcpy(nodes[i]->data, nodes[i-1]->data, len);
            }
            
            if (i > 1 && nodes[i-2]) {
                __builtin_memmove(nodes[i]->data + 32, 
                                 nodes[i-2]->data, 
                                 len % 224);
            }
        }
    }
}

/* Complex token processing with memory operations */
static unsigned long process_tokens(const char **tokens, int token_count) {
    unsigned long hash = 0;
    char buffer[1024];
    int i;
    
    /* Initialize buffer with builtin memset */
    __builtin_memset(buffer, 0, sizeof(buffer));
    
    for (i = 0; i < token_count; i++) {
        size_t token_len = strlen(tokens[i]);
        size_t copy_len = token_len < 256 ? token_len : 255;
        
        /* Use builtin memcpy for token copying */
        __builtin_memcpy(buffer + (i * 64), tokens[i], copy_len);
        
        /* Use builtin memmove for overlapping regions */
        if (i > 0) {
            __builtin_memmove(buffer + (i * 64) - 16, 
                             buffer + (i * 64), 
                             copy_len < 16 ? copy_len : 16);
        }
        
        /* Update hash */
        for (size_t j = 0; j < copy_len; j++) {
            hash = hash * 31 + buffer[(i * 64) + j];
        }
    }
    
    return hash;
}

int main(void) {
    const char *tokens[] = {
        "TOKEN_ALPHA", "TOKEN_BETA", "TOKEN_GAMMA",
        "TOKEN_DELTA", "TOKEN_EPSILON", "TOKEN_ZETA",
        "TOKEN_ETA", "TOKEN_THETA", "TOKEN_IOTA"
    };
    const int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Build AST structure */
    struct ast_node *root = build_ast(0, 4);
    
    /* Create array of nodes for parallel processing */
    struct ast_node *node_array[8];
    for (int i = 0; i < 8; i++) {
        node_array[i] = build_ast(i % 3, 3);
    }
    
    /* Process with goto jumps */
    if (root && node_array[0]) {
        process_with_goto(root, node_array[0]);
    }
    
    /* Parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Process tokens with memory operations */
    unsigned long final_hash = process_tokens(tokens, token_count);
    
    /* Verify operations by printing hash */
    printf("Result hash: %lu\n", final_hash);
    
    /* Cleanup */
    /* Note: In real ASAN, memory would be automatically checked */
    for (int i = 0; i < 8; i++) {
        free(node_array[i]);
    }
    free(root);
    
    return 0;
}
