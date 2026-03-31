/* ISO C99-compliant test program for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    char data[64];
    struct ast_node *left;
    struct ast_node *right;
    int id;
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    /* Force initialization of ASAN runtime */
    volatile char init_buf[16];
    __builtin_memset(init_buf, 0, sizeof(init_buf));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    /* Final memory operation to ensure cleanup paths */
    volatile char cleanup_buf[8];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Complex recursive parser with memory operations */
static struct ast_node* parse_recursive(char **tokens, int depth) {
    if (depth <= 0 || **tokens == '\0') {
        return NULL;
    }
    
    struct ast_node *node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Use builtin memcpy for node initialization */
    __builtin_memset(node, 0, sizeof(*node));
    node->id = depth;
    
    /* Copy token data with volatile length */
    volatile size_t copy_len = 32;
    if (copy_len > 63) copy_len = 63;
    
    __builtin_memcpy(node->data, *tokens, copy_len);
    node->data[copy_len] = '\0';
    
    /* Move token pointer */
    (*tokens) += copy_len;
    
    /* Recursive calls with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto parse_left;
        }
        
        node->left = parse_recursive(tokens, depth - 1);
        
        parse_left:
        node->right = parse_recursive(tokens, depth - 2);
        
        /* Memory move between child nodes if both exist */
        if (node->left && node->right) {
            volatile size_t move_size = 16;
            __builtin_memmove(node->left->data + 16, 
                             node->right->data, 
                             move_size);
        }
    }
    
    return node;
}

/* Parallel memory operations using OpenMP */
static void parallel_mem_operations(struct ast_node **nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Varied memory operations in parallel context */
            volatile char temp_buf[128];
            volatile size_t op_size = g_mem_size % 128;
            
            /* Force all three builtins to be called */
            __builtin_memset(temp_buf, i, op_size);
            __builtin_memcpy(nodes[i]->data, temp_buf, op_size > 64 ? 64 : op_size);
            
            /* Conditional memmove with goto */
            if (i > 0 && nodes[i-1]) {
                int do_move = (i % 2 == 0);
                
                if (do_move) {
                    goto perform_move;
                }
                
                /* Alternative path */
                __builtin_memcpy(nodes[i]->data + 32, 
                               nodes[i-1]->data, 
                               16);
                goto skip_move;
                
                perform_move:
                __builtin_memmove(nodes[i]->data, 
                                nodes[i-1]->data + 16, 
                                24);
                
                skip_move:;
            }
        }
    }
}

/* Calculate hash from AST structure */
static unsigned long calculate_ast_hash(struct ast_node *node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char *ptr = node->data;
    
    /* DJB2 hash algorithm */
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    /* Recursive hash combination */
    hash ^= calculate_ast_hash(node->left);
    hash ^= calculate_ast_hash(node->right);
    hash ^= node->id;
    
    return hash;
}

/* Free AST recursively */
static void free_ast(struct ast_node *node) {
    if (!node) return;
    
    /* Clear data before freeing */
    volatile size_t clear_size = sizeof(node->data);
    __builtin_memset(node->data, 0, clear_size);
    
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    /* Initialize complex token array */
    char tokens[1024];
    for (int i = 0; i < (int)sizeof(tokens) - 1; i++) {
        tokens[i] = 'A' + (i % 26);
    }
    tokens[sizeof(tokens) - 1] = '\0';
    
    char *token_ptr = tokens;
    
    /* Create recursive AST structure */
    struct ast_node *root = parse_recursive(&token_ptr, 5);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    struct ast_node *nodes[8];
    nodes[0] = root;
    
    /* Build additional nodes */
    for (int i = 1; i < 8; i++) {
        char local_tokens[128];
        __builtin_memset(local_tokens, '0' + i, sizeof(local_tokens) - 1);
        local_tokens[sizeof(local_tokens) - 1] = '\0';
        
        char *ptr = local_tokens;
        nodes[i] = parse_recursive(&ptr, 3);
    }
    
    /* Execute parallelized memory operations */
    parallel_mem_operations(nodes, 8);
    
    /* Calculate and print verification result */
    unsigned long total_hash = 0;
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            total_hash ^= calculate_ast_hash(nodes[i]);
        }
    }
    
    printf("Result hash: 0x%08lx\n", total_hash);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            free_ast(nodes[i]);
        }
    }
    
    /* Final memory operation to ensure all paths are taken */
    volatile char final_buf[256];
    volatile size_t final_size = g_mem_size;
    
    __builtin_memset(final_buf, 0xAA, final_size % 256);
    __builtin_memcpy(final_buf + 128, final_buf, 64);
    __builtin_memmove(final_buf, final_buf + 64, 32);
    
    return 0;
}
