/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force memcpy redirection early */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    g_use_hwasan = (buffer[0] == 0xAA) ? 1 : 0;
}

/* Destructor to test cleanup paths */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile int dummy[16];
    __builtin_memset(dummy, 0xFF, sizeof(dummy));
}

/* Complex token array for parser */
typedef struct {
    char tokens[8][32];
    size_t count;
} TokenArray;

/* Recursive parser with memory operations */
static ASTNode* parse_expression(TokenArray* tokens, size_t* index) {
    if (*index >= tokens->count) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy with volatile size */
    size_t copy_size = g_mem_size % 64;
    __builtin_memcpy(node->data, tokens->tokens[*index], copy_size);
    node->size = copy_size;
    (*index)++;
    
    /* Jump logic to test flow sensitivity */
    if (node->data[0] == '[') {
        goto parse_left;
    } else if (node->data[0] == ']') {
        goto parse_right;
    }
    
    node->left = parse_expression(tokens, index);
    parse_left:
    node->right = parse_expression(tokens, index);
    parse_right:
    
    return node;
}

/* Memory operation dispatcher with OpenMP */
static void dispatch_memory_ops(ASTNode* nodes[], size_t count) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->left && nodes[i]->right) {
                volatile char temp[128];
                size_t op_size = (g_mem_size + thread_id) % 128;
                
                /* Test all three builtins with goto edge cases */
                if (i % 3 == 0) {
                    __builtin_memcpy(temp, nodes[i]->data, op_size);
                    goto copy_done;
                } else if (i % 3 == 1) {
                    __builtin_memset(temp, thread_id, op_size);
                    goto memset_done;
                } else {
                    /* memmove with overlapping regions */
                    char overlap[256];
                    __builtin_memcpy(overlap, nodes[i]->data, nodes[i]->size);
                    __builtin_memmove(overlap + 32, overlap, nodes[i]->size);
                    goto memmove_done;
                }
                
                copy_done:
                memset_done:
                memmove_done:
                
                /* Cross-node memory operations */
                if (nodes[i]->left && nodes[i]->right) {
                    size_t min_size = nodes[i]->left->size < nodes[i]->right->size ? 
                                     nodes[i]->left->size : nodes[i]->right->size;
                    __builtin_memcpy(nodes[i]->left->data, 
                                   nodes[i]->right->data, 
                                   min_size);
                }
            }
        }
    }
}

/* Calculate hash from AST */
static size_t calculate_ast_hash(ASTNode* root) {
    if (!root) return 0;
    
    size_t hash = 5381;
    for (size_t i = 0; i < root->size; i++) {
        hash = ((hash << 5) + hash) + root->data[i];
    }
    
    /* Recursive with goto for flow testing */
    if (root->left) {
        hash ^= calculate_ast_hash(root->left);
        goto hash_left_done;
    }
    hash_left_done:
    
    if (root->right) {
        hash ^= calculate_ast_hash(root->right);
        goto hash_right_done;
    }
    hash_right_done:
    
    return hash;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode* root) {
    if (!root) return;
    
    /* Clear data before free */
    __builtin_memset(root->data, 0, sizeof(root->data));
    free_ast(root->left);
    free_ast(root->right);
    free(root);
}

int main(void) {
    /* Initialize token array */
    TokenArray tokens = {
        .tokens = {
            "[expr1]", "[expr2]", "value1", "value2",
            "[expr3]", "value3", "[expr4]", "value4"
        },
        .count = 8
    };
    
    /* Parse into AST */
    size_t index = 0;
    ASTNode* root = parse_expression(&tokens, &index);
    
    /* Create node array for parallel processing */
    ASTNode* nodes[16];
    nodes[0] = root;
    
    /* Build tree structure */
    for (int i = 1; i < 16; i++) {
        nodes[i] = malloc(sizeof(ASTNode));
        if (nodes[i]) {
            size_t fill_size = (g_mem_size + i) % 64;
            __builtin_memset(nodes[i]->data, i, fill_size);
            nodes[i]->size = fill_size;
            nodes[i]->left = NULL;
            nodes[i]->right = NULL;
        }
    }
    
    /* Dispatch memory operations with OpenMP */
    dispatch_memory_ops(nodes, 16);
    
    /* Calculate and print verification hash */
    size_t total_hash = 0;
    for (int i = 0; i < 16; i++) {
        if (nodes[i]) {
            total_hash ^= calculate_ast_hash(nodes[i]);
        }
    }
    
    printf("Verification hash: 0x%08zx\n", total_hash);
    
    /* Cleanup */
    for (int i = 1; i < 16; i++) {
        if (nodes[i]) {
            __builtin_memset(nodes[i]->data, 0, sizeof(nodes[i]->data));
            free(nodes[i]);
        }
    }
    free_ast(root);
    
    /* Final builtin calls to ensure coverage */
    volatile char final_buf[512];
    __builtin_memset(final_buf, 0xCC, sizeof(final_buf));
    __builtin_memcpy(final_buf + 128, final_buf, 256);
    __builtin_memmove(final_buf + 256, final_buf + 128, 128);
    
    return (int)(total_hash & 0xFF);
}
