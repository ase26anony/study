#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char *data;
    size_t data_len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Constructor function */
__attribute__((constructor))
static void init_constructor(void) {
    printf("Constructor: Initializing ASAN/HWASAN redirection cache\n");
}

/* Destructor function */
__attribute__((destructor))
static void cleanup_destructor(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive function that builds and manipulates AST */
static ASTNode* build_ast(int depth, int max_depth) {
    if (depth >= max_depth) {
        return NULL;
    }
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = depth % 3;
    node->value = depth * 10;
    node->data_len = volatile_len / (depth + 1);
    node->data = (char*)malloc(node->data_len);
    node->left = build_ast(depth + 1, max_depth);
    node->right = build_ast(depth + 2, max_depth);
    
    /* Use __builtin_memset to initialize data */
    if (node->data) {
        __builtin_memset(node->data, depth, node->data_len);
    }
    
    return node;
}

/* Function with goto statements for flow control */
static void process_with_goto(ASTNode *src, ASTNode *dst) {
    if (!src || !dst || !src->data || !dst->data) return;
    
    size_t copy_len = src->data_len < dst->data_len ? src->data_len : dst->data_len;
    
    /* Jump into memory operation block */
    goto start_copy;
    
    /* This label should be jumped into */
    start_copy:
    if (copy_len > 0) {
        /* Use __builtin_memmove with goto flow */
        __builtin_memmove(dst->data, src->data, copy_len);
    }
    
    /* Jump out of block */
    goto after_copy;
    
    after_copy:
    if (volatile_flag) {
        /* Use __builtin_memcpy after goto */
        char temp[128];
        __builtin_memcpy(temp, dst->data, copy_len < 128 ? copy_len : 128);
    }
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_ops(ASTNode **nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i] && nodes[i]->data) {
            /* Force redirection of builtins in parallel context */
            char buffer[256];
            size_t len = nodes[i]->data_len < 256 ? nodes[i]->data_len : 256;
            
            /* Use all three builtins in parallel region */
            __builtin_memset(buffer, 0, sizeof(buffer));
            __builtin_memcpy(buffer, nodes[i]->data, len);
            
            /* Create overlapping memory regions for memmove */
            if (len > 16) {
                __builtin_memmove(buffer + 8, buffer, len - 8);
            }
            
            /* Copy back to node */
            __builtin_memcpy(nodes[i]->data, buffer, len);
        }
    }
}

/* Complex token processing with varied memory operations */
static uint32_t process_tokens(const char *tokens[], int token_count) {
    uint32_t hash = 0x811C9DC5; /* FNV-1a basis */
    char buffer[512];
    char temp_buffer[512];
    
    for (int i = 0; i < token_count; i++) {
        size_t token_len = strlen(tokens[i]);
        
        /* Clear buffer using __builtin_memset */
        __builtin_memset(buffer, 0, sizeof(buffer));
        
        /* Copy token using __builtin_memcpy */
        __builtin_memcpy(buffer, tokens[i], token_len);
        
        /* Process with overlapping memmove */
        if (token_len > 32) {
            __builtin_memmove(buffer + 16, buffer, token_len - 16);
        }
        
        /* Mix into hash */
        for (size_t j = 0; j < token_len && j < sizeof(buffer); j++) {
            hash ^= buffer[j];
            hash *= 0x01000193;
        }
        
        /* Use volatile to control operation */
        if (volatile_flag) {
            __builtin_memcpy(temp_buffer, buffer, sizeof(buffer));
        }
    }
    
    return hash;
}

/* Main execution flow */
int main(void) {
    const char *tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "instrumentation", "redzone", "builtin", "optimization",
        "parallel", "volatile", "recursive", "AST", "node"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Build recursive AST structure */
    ASTNode *root = build_ast(0, 5);
    if (!root) {
        fprintf(stderr, "Failed to build AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    ASTNode *nodes[10];
    ASTNode *current = root;
    for (int i = 0; i < 10 && current; i++) {
        nodes[i] = current;
        current = current->left ? current->left : current->right;
    }
    
    /* Process with goto statements */
    if (root->left && root->right) {
        process_with_goto(root->left, root->right);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(nodes, 10);
    
    /* Process tokens with memory builtins */
    uint32_t final_hash = process_tokens(tokens, token_count);
    
    printf("Final hash: 0x%08X\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup would go here in a real implementation */
    
    return 0;
}
