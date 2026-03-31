/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    int type;
    char *data;
    size_t data_len;
    struct ast_node *left;
    struct ast_node *right;
    struct ast_node *parent;
    unsigned char padding[16]; /* Ensure redzone creation */
} ast_node_t;

/* Global token array */
static const char *tokens[] = {
    "memcpy", "memset", "memmove", "test", "data", "asan", "hwasan"
};
static const int token_count = sizeof(tokens) / sizeof(tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Initializing ASAN environment...\n");
    /* Force initialization of memory function caches */
    char buffer[64];
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, "init", 5);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    printf("Cleaning up ASAN environment...\n");
    char buffer[32];
    __builtin_memset(buffer, 0xFF, sizeof(buffer));
}

/* Recursive parser with memory operations */
static ast_node_t* parse_expression(int depth, const char **token_ptr) {
    if (depth <= 0 || *token_ptr == NULL) {
        return NULL;
    }
    
    ast_node_t *node = malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    
    node->type = depth;
    const char *current_token = *token_ptr;
    size_t token_len = strlen(current_token);
    
    /* Allocate and copy token data */
    node->data = malloc(token_len + 1);
    if (node->data) {
        /* Use builtin memcpy for data transfer */
        __builtin_memcpy(node->data, current_token, token_len);
        node->data[token_len] = '\0';
        node->data_len = token_len;
    }
    
    /* Move to next token with wrap-around */
    *token_ptr = tokens[(*token_ptr - tokens[0] + 1) % token_count];
    
    /* Recursive parsing with goto for flow control */
    int parse_left = 1;
    
    if (depth > 2) {
        goto parse_children;
    }
    
parse_children:
    if (parse_left) {
        node->left = parse_expression(depth - 1, token_ptr);
        parse_left = 0;
        goto parse_children; /* Jump back to parse right child */
    } else {
        node->right = parse_expression(depth - 1, token_ptr);
    }
    
    return node;
}

/* Function with goto jumping into memmove block */
static void process_with_goto(ast_node_t *dest, ast_node_t *src) {
    int state = 0;
    
    if (src->data && dest->data) {
        state = 1;
        goto copy_block;
    }
    
    /* Normal path */
    dest->type = src->type;
    return;
    
copy_block:
    {
        /* This block contains the critical memmove */
        size_t copy_len = src->data_len < dest->data_len ? 
                         src->data_len : dest->data_len;
        
        /* Force builtin memmove with goto entry */
        __builtin_memmove(dest->data, src->data, copy_len);
        
        if (state == 1) {
            goto finish;
        }
    }
    
    /* Unreachable in normal flow */
    dest->type = -1;
    return;
    
finish:
    dest->type = src->type + 100;
}

/* Parallel memory dispatch logic */
static void parallel_memory_operations(void) {
    const int num_ops = 8;
    char *buffers[num_ops];
    size_t sizes[num_ops];
    
    /* Initialize buffers with volatile sizes */
    for (int i = 0; i < num_ops; i++) {
        sizes[i] = (g_mem_size + i * 64) % 512 + 64;
        buffers[i] = malloc(sizes[i]);
        if (buffers[i]) {
            __builtin_memset(buffers[i], i, sizes[i]);
        }
    }
    
    #pragma omp parallel for
    for (int i = 0; i < num_ops; i++) {
        if (buffers[i]) {
            /* Mix of memory operations in parallel region */
            char temp[256];
            volatile int use_memcpy = i % 3;
            
            if (use_memcpy == 0) {
                __builtin_memcpy(temp, buffers[i], 
                               sizes[i] > sizeof(temp) ? sizeof(temp) : sizes[i]);
            } else if (use_memcpy == 1) {
                __builtin_memset(buffers[i], 0xAA, sizes[i]);
            } else {
                /* Create overlapping regions for memmove */
                size_t move_size = sizes[i] / 2;
                if (move_size > 0) {
                    __builtin_memmove(buffers[i] + move_size/2, 
                                    buffers[i], move_size);
                }
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_ops; i++) {
        free(buffers[i]);
    }
}

/* Compute hash from AST */
static unsigned long compute_ast_hash(ast_node_t *node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char *ptr = node->data;
    
    /* Process data with builtin awareness */
    if (ptr) {
        for (size_t i = 0; i < node->data_len; i++) {
            hash = ((hash << 5) + hash) + ptr[i];
        }
    }
    
    /* Recursive hash computation */
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize token pointer */
    const char *token_ptr = tokens[0];
    
    /* Create recursive AST */
    ast_node_t *root = parse_expression(4, &token_ptr);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto flow control */
    if (root->left && root->right) {
        process_with_goto(root->left, root->right);
        
        /* Also test reverse direction */
        process_with_goto(root->right, root->left);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Additional memory operations in main */
    char main_buffer[128];
    volatile size_t op_size = 64;
    
    /* Test all three builtins in sequence */
    __builtin_memset(main_buffer, 0xCC, sizeof(main_buffer));
    __builtin_memcpy(main_buffer + 32, tokens[0], strlen(tokens[0]));
    __builtin_memmove(main_buffer, main_buffer + 16, op_size);
    
    /* Compute and print verification result */
    unsigned long final_hash = compute_ast_hash(root);
    printf("AST Hash: %lu\n", final_hash);
    
    /* Verify memory operations */
    int sum = 0;
    for (int i = 0; i < sizeof(main_buffer); i++) {
        sum += main_buffer[i];
    }
    printf("Buffer checksum: %d\n", sum);
    
    /* Cleanup AST (simplified - real code would need proper traversal) */
    free(root->data);
    free(root);
    
    printf("Test completed successfully\n");
    return 0;
}
