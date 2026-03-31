/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
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

/* Token array for parser simulation */
static const char* tokens[] = {"memcpy", "memset", "memmove", "data", "node", "end"};
static volatile int token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_test(void) {
    g_init_flag = 1;
    printf("Constructor: Initializing ASAN test environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth) {
    if (depth <= 0 || token_idx >= 6) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data using __builtin_memcpy */
    const char* current_token = tokens[token_idx++];
    size_t token_len = strlen(current_token) + 1;
    if (token_len > sizeof(node->data)) token_len = sizeof(node->data);
    
    __builtin_memcpy(node->data, current_token, token_len);
    node->size = token_len;
    
    /* Recursive parsing with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 2 == 0);
        
        if (use_goto) {
            goto parse_left;
        } else {
            node->left = parse_expression(depth - 1);
            goto parse_right;
        }
        
    parse_left:
        node->left = parse_expression(depth - 1);
        
    parse_right:
        node->right = parse_expression(depth - 1);
    }
    
    return node;
}

/* Memory operation dispatcher with OpenMP */
static void dispatch_memory_operations(ASTNode* nodes[], size_t count) {
    #pragma omp parallel
    {
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->left && nodes[i]->right) {
                /* Use __builtin_memmove between child nodes */
                volatile size_t move_size = nodes[i]->left->size;
                if (move_size > sizeof(nodes[i]->right->data))
                    move_size = sizeof(nodes[i]->right->data);
                
                __builtin_memmove(nodes[i]->right->data, 
                                 nodes[i]->left->data, 
                                 move_size);
                
                /* Additional memcpy with volatile size */
                volatile size_t copy_size = g_mem_size % 64;
                if (copy_size > 0 && (i + 1) < count && nodes[i + 1]) {
                    __builtin_memcpy(nodes[i + 1]->data,
                                    nodes[i]->data,
                                    copy_size);
                }
            }
        }
        
        /* Thread-local memset operation */
        #pragma omp single
        {
            char local_buf[256];
            volatile size_t local_size = g_mem_size % 256;
            __builtin_memset(local_buf, 0xAB, local_size);
        }
    }
}

/* Calculate hash from AST structure */
static size_t calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 0;
    for (size_t i = 0; i < node->size && i < sizeof(node->data); i++) {
        hash = hash * 31 + (size_t)node->data[i];
    }
    
    /* Use __builtin_memcpy in hash calculation path */
    char temp[64];
    __builtin_memcpy(temp, node->data, 
                    (node->size < sizeof(temp)) ? node->size : sizeof(temp));
    
    for (size_t i = 0; i < sizeof(temp); i++) {
        hash ^= (size_t)temp[i] << ((i % 8) * 8);
    }
    
    return hash + calculate_ast_hash(node->left) + calculate_ast_hash(node->right);
}

/* Main test driver */
int main(void) {
    if (!g_init_flag) {
        fprintf(stderr, "Error: Constructor not called\n");
        return 1;
    }
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create AST tree */
    token_idx = 0;
    ASTNode* root = parse_expression(4);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    ASTNode* node_array[8];
    node_array[0] = root;
    
    /* Build node array with goto-based initialization */
    int array_idx = 1;
    ASTNode* current = root;
    
fill_array:
    if (current->left && array_idx < 7) {
        node_array[array_idx++] = current->left;
        if (current->left->right) {
            node_array[array_idx++] = current->left->right;
            goto check_right;
        }
    }
    
check_right:
    if (current->right && array_idx < 8) {
        node_array[array_idx] = current->right;
        array_idx++;
    }
    
    /* Initialize remaining slots with memset */
    for (int i = array_idx; i < 8; i++) {
        node_array[i] = (ASTNode*)malloc(sizeof(ASTNode));
        if (node_array[i]) {
            __builtin_memset(node_array[i], 0, sizeof(ASTNode));
        }
    }
    
    /* Execute memory operations with OpenMP */
    dispatch_memory_operations(node_array, 8);
    
    /* Calculate and verify results */
    size_t total_hash = 0;
    for (int i = 0; i < 8; i++) {
        if (node_array[i]) {
            total_hash ^= calculate_ast_hash(node_array[i]);
            
            /* Final memmove before cleanup */
            if (i > 0 && node_array[i-1]) {
                volatile size_t final_size = g_mem_size % 32;
                __builtin_memmove(node_array[i]->data,
                                 node_array[i-1]->data,
                                 final_size);
            }
        }
    }
    
    printf("Test completed. Result hash: 0x%zx\n", total_hash);
    
    /* Cleanup */
    for (int i = 1; i < 8; i++) {
        if (node_array[i]) free(node_array[i]);
    }
    
    /* Recursive free function */
    void free_ast(ASTNode* node) {
        if (!node) return;
        free_ast(node->left);
        free_ast(node->right);
        free(node);
    }
    free_ast(root);
    
    return 0;
}
