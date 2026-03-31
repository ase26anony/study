/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Global token array */
static const char *tokens[] = {"memcpy", "memset", "memmove", "test", "data"};
static const size_t num_tokens = sizeof(tokens)/sizeof(tokens[0]);

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
}

__attribute__((destructor)) static void cleanup_globals(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, const char **token_ptr) {
    if (depth <= 0 || *token_ptr >= tokens + num_tokens)
        return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    const char *current = *token_ptr;
    (*token_ptr)++;
    
    /* Use volatile to control allocation size */
    node->len = g_mem_size % 128 + 64;
    node->data = malloc(node->len);
    
    if (!node->data) {
        free(node);
        return NULL;
    }
    
    /* Force __builtin_memset call with non-foldable size */
    __builtin_memset(node->data, 0xAA, node->len);
    
    /* Copy token string using __builtin_memcpy */
    size_t token_len = strlen(*current);
    if (token_len < node->len) {
        __builtin_memcpy(node->data, *current, token_len);
    }
    
    /* Recursive calls with goto for flow control */
    int use_goto = (depth % 2 == 0);
    
    if (use_goto) {
        goto parse_left;
    } else {
        node->left = parse_expression(depth - 1, token_ptr);
        goto parse_right;
    }
    
parse_left:
    node->left = parse_expression(depth - 2, token_ptr);
    
parse_right:
    node->right = parse_expression(depth - 1, token_ptr);
    
    return node;
}

/* Function with goto jumping into memmove block */
static void process_with_goto(ASTNode *src, ASTNode *dst) {
    if (!src || !dst || !src->data || !dst->data) return;
    
    int condition = src->len > dst->len;
    
    if (condition) {
        goto use_memmove;
    } else {
        /* Regular path */
        size_t copy_len = src->len < dst->len ? src->len : dst->len;
        __builtin_memcpy(dst->data, src->data, copy_len);
        return;
    }
    
use_memmove:
    /* Jump target with __builtin_memmove */
    if (g_use_memmove) {
        size_t move_len = src->len < dst->len ? src->len : dst->len;
        __builtin_memmove(dst->data, src->data, move_len);
    }
    
    /* Jump out */
    goto cleanup;
    
cleanup:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode **nodes, size_t count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->data) {
                /* Thread-specific memory operations */
                volatile size_t local_size = g_mem_size + tid;
                
                /* Force all three builtins in parallel region */
                __builtin_memset(nodes[i]->data, tid, 
                    nodes[i]->len < local_size ? nodes[i]->len : local_size);
                
                if (i > 0 && nodes[i-1]) {
                    size_t copy_len = nodes[i]->len < nodes[i-1]->len ? 
                                     nodes[i]->len : nodes[i-1]->len;
                    __builtin_memcpy(nodes[i]->data, nodes[i-1]->data, copy_len);
                }
                
                /* Conditional memmove */
                if (tid % 3 == 0 && i + 1 < count && nodes[i+1]) {
                    size_t move_len = nodes[i]->len < nodes[i+1]->len ? 
                                     nodes[i]->len : nodes[i+1]->len;
                    __builtin_memmove(nodes[i+1]->data, nodes[i]->data, move_len);
                }
            }
        }
    }
}

/* Calculate hash from AST */
static size_t compute_ast_hash(ASTNode *node) {
    if (!node) return 0;
    
    size_t hash = 0;
    for (size_t i = 0; i < node->len && i < 64; i++) {
        hash = hash * 31 + (unsigned char)node->data[i];
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode *node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    if (node->data) {
        /* Clear before free */
        __builtin_memset(node->data, 0, node->len);
        free(node->data);
    }
    
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Initialize token pointer */
    const char **token_ptr = tokens;
    
    /* Create recursive AST */
    ASTNode *root = parse_expression(4, &token_ptr);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    ASTNode *node_array[8] = {0};
    
    /* Build node array with some overlapping data */
    ASTNode *current = root;
    for (int i = 0; i < 8 && current; i++) {
        node_array[i] = current;
        current = current->left ? current->left : current->right;
    }
    
    /* Test goto with memmove */
    if (root->left && root->right) {
        process_with_goto(root->left, root->right);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Compute verification hash */
    size_t final_hash = compute_ast_hash(root);
    printf("Result hash: %zu\n", final_hash);
    
    /* Additional builtin calls in main */
    char buffer1[256], buffer2[256];
    volatile size_t op_size = g_mem_size % 128;
    
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, op_size);
    
    if (g_use_memmove) {
        __builtin_memmove(buffer1 + 64, buffer1, op_size);
    }
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
