#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* volatile to prevent optimization */
    struct ASTNode* left;
    struct ASTNode* right;
    char padding[32];      /* Ensure size for memcpy operations */
} ASTNode;

/* Global volatile variables to prevent constant folding */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Token array for parser simulation */
typedef struct {
    char* data;
    size_t len;
} Token;

/* Constructor function - forces early initialization */
__attribute__((constructor))
static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force memcpy initialization in constructor */
    __builtin_memcpy(buffer, "constructor_init", 16);
    printf("Constructor initialized\n");
}

/* Destructor function */
__attribute__((destructor))
static void cleanup_asan(void) {
    printf("Destructor cleaning up\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(Token* tokens, size_t* pos, size_t len) {
    if (*pos >= len) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = tokens[*pos].data[0];
    node->size = g_mem_size;
    (*pos)++;
    
    /* Conditional goto for flow control */
    if (node->type == '(') {
        node->left = parse_expression(tokens, pos, len);
        goto skip_right;  /* Jump to skip right child */
    }
    
    node->left = parse_expression(tokens, pos, len);
    node->right = parse_expression(tokens, pos, len);
    
skip_right:
    /* Copy node data using memmove (self-overlap case) */
    if (node->left) {
        __builtin_memmove(&node->padding[0], &node->left->padding[0], 
                         sizeof(node->padding) / 2);
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void process_with_goto(ASTNode* node, char* buffer) {
    if (!node) return;
    
    volatile int use_memmove = 1;
    
    /* Jump into memory operation block */
    if (node->type == '+') {
        goto do_memcpy;
    }
    
    /* Normal path */
    __builtin_memset(buffer, node->type, node->size);
    
do_memcpy:
    /* This label is jumped into */
    __builtin_memcpy(buffer + 16, &node->value, sizeof(node->value));
    
    if (use_memmove) {
        goto do_memmove;
    }
    
    return;
    
do_memmove:
    /* Self-overlapping memmove */
    __builtin_memmove(buffer, buffer + 8, 24);
    
    /* Jump out of the block */
    goto cleanup;
    
cleanup:
    /* Final memset */
    __builtin_memset(buffer + 48, 0xFF, 16);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, size_t count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buffer[256];
        volatile size_t local_size = g_mem_size + tid;
        
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Thread-specific memory operations */
                __builtin_memset(local_buffer, tid, local_size);
                
                /* Copy node data */
                __builtin_memcpy(local_buffer + 64, nodes[i], 
                               sizeof(ASTNode) > 128 ? 128 : sizeof(ASTNode));
                
                /* Move data around */
                __builtin_memmove(local_buffer + 128, local_buffer + 32, 64);
                
                /* Update node with result */
                __builtin_memcpy(&nodes[i]->value, local_buffer + 16, 
                               sizeof(nodes[i]->value));
            }
        }
    }
}

/* Calculate hash from AST */
static uint32_t ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    uint32_t hash = 0;
    char buffer[sizeof(ASTNode)];
    
    /* Copy node to buffer using all three builtins */
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, node, sizeof(ASTNode));
    
    /* Create overlapping copy */
    __builtin_memmove(buffer + 16, buffer, 48);
    
    /* Compute simple hash */
    for (size_t i = 0; i < sizeof(buffer); i++) {
        hash = (hash * 31) + buffer[i];
    }
    
    return hash + ast_hash(node->left) + ast_hash(node->right);
}

int main(void) {
    /* Initialize complex token array */
    Token tokens[] = {
        {"+", 1}, {"1", 1}, {"*", 1}, 
        {"2", 1}, {"3", 1}, {")", 1}
    };
    size_t token_count = sizeof(tokens) / sizeof(tokens[0]);
    size_t pos = 0;
    
    /* Parse expression tree */
    ASTNode* root = parse_expression(tokens, &pos, token_count);
    
    if (!root) {
        fprintf(stderr, "Failed to parse expression\n");
        return 1;
    }
    
    /* Process buffer with goto flow control */
    char main_buffer[512];
    process_with_goto(root, main_buffer);
    
    /* Create array of nodes for parallel processing */
    ASTNode* node_array[8];
    node_array[0] = root;
    
    for (int i = 1; i < 8; i++) {
        node_array[i] = (ASTNode*)malloc(sizeof(ASTNode));
        if (node_array[i]) {
            __builtin_memset(node_array[i], i, sizeof(ASTNode));
            __builtin_memcpy(&node_array[i]->value, &i, sizeof(i));
        }
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Calculate and print verification hash */
    uint32_t total_hash = 0;
    for (int i = 0; i < 8; i++) {
        if (node_array[i]) {
            total_hash ^= ast_hash(node_array[i]);
        }
    }
    
    printf("Verification hash: 0x%08X\n", total_hash);
    
    /* Cleanup */
    for (int i = 1; i < 8; i++) {
        free(node_array[i]);
    }
    
    /* Free AST recursively */
    /* ... (recursive free implementation omitted for brevity) */
    
    return 0;
}
