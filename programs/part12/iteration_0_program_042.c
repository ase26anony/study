/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array for parser */
static const char* tokens[] = {"memcpy", "memset", "memmove", "data", "node"};
static const size_t token_count = 5;

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_globals(void) {
    printf("Initializing ASAN test environment...\n");
}

__attribute__((destructor)) static void cleanup_globals(void) {
    printf("Cleaning up ASAN test environment...\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, const char** token_ptr) {
    if (depth <= 0 || *token_ptr >= tokens + token_count) {
        return NULL;
    }
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data using __builtin_memcpy */
    const char* current_token = *token_ptr;
    size_t token_len = strlen(current_token);
    __builtin_memcpy(node->data, current_token, token_len + 1);
    node->size = token_len;
    
    (*token_ptr)++;
    
    /* Recursive calls with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 2 == 0);
        
        if (use_goto) {
            goto parse_left;
        } else {
            node->left = parse_expression(depth - 1, token_ptr);
            goto parse_right;
        }
        
    parse_left:
        node->left = parse_expression(depth - 1, token_ptr);
        
    parse_right:
        node->right = parse_expression(depth - 1, token_ptr);
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
                volatile size_t copy_size = nodes[i]->left->size;
                if (copy_size > sizeof(nodes[i]->right->data)) {
                    copy_size = sizeof(nodes[i]->right->data);
                }
                
                __builtin_memmove(nodes[i]->right->data, 
                                 nodes[i]->left->data, 
                                 copy_size);
                
                /* Use __builtin_memcpy to backup data */
                char backup[256];
                __builtin_memcpy(backup, nodes[i]->data, 
                                sizeof(nodes[i]->data));
                
                /* Conditional goto for edge cases */
                if (i % 3 == 0) {
                    goto skip_operation;
                }
                
                /* Use __builtin_memset on right node */
                __builtin_memset(nodes[i]->right->data + copy_size, 
                               0xAA, 
                               sizeof(nodes[i]->right->data) - copy_size);
                
            skip_operation:
                /* Use volatile to prevent dead code elimination */
                volatile int dummy = (int)backup[0];
                (void)dummy;
            }
        }
    }
}

/* Calculate hash of AST structure */
static size_t calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 5381;
    for (size_t i = 0; i < node->size && i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Use __builtin_memcpy for temporary storage */
    char temp[256];
    __builtin_memcpy(temp, node->data, node->size);
    temp[node->size] = '\0';
    
    hash += calculate_ast_hash(node->left);
    hash += calculate_ast_hash(node->right);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Use __builtin_memset before free for security */
    __builtin_memset(node, 0xCC, sizeof(ASTNode));
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create multiple ASTs with different depths */
    const char* token_ptr = tokens;
    ASTNode* ast1 = parse_expression(3, &token_ptr);
    
    token_ptr = tokens;
    ASTNode* ast2 = parse_expression(4, &token_ptr);
    
    /* Array of nodes for OpenMP processing */
    ASTNode* nodes[] = {ast1, ast2, NULL, NULL, NULL};
    size_t node_count = sizeof(nodes) / sizeof(nodes[0]);
    
    /* Fill remaining slots with simple nodes */
    for (size_t i = 2; i < node_count; i++) {
        nodes[i] = malloc(sizeof(ASTNode));
        if (nodes[i]) {
            __builtin_memset(nodes[i], 0, sizeof(ASTNode));
            __builtin_memcpy(nodes[i]->data, "test", 5);
            nodes[i]->size = 4;
        }
    }
    
    /* Dispatch memory operations with OpenMP */
    dispatch_memory_operations(nodes, node_count);
    
    /* Calculate and print verification hash */
    size_t total_hash = 0;
    for (size_t i = 0; i < node_count; i++) {
        if (nodes[i]) {
            total_hash ^= calculate_ast_hash(nodes[i]);
            
            /* Additional __builtin_memmove test */
            if (i > 0 && nodes[i-1]) {
                size_t move_size = (nodes[i]->size < nodes[i-1]->size) ? 
                                  nodes[i]->size : nodes[i-1]->size;
                __builtin_memmove(nodes[i]->data, nodes[i-1]->data, move_size);
            }
        }
    }
    
    printf("Verification hash: 0x%08zx\n", total_hash);
    printf("Total operations completed.\n");
    
    /* Cleanup */
    for (size_t i = 0; i < node_count; i++) {
        if (nodes[i]) {
            free_ast(nodes[i]);
        }
    }
    
    return 0;
}
