/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations for attribute functions */
void __attribute__((constructor)) init_asan_early(void);
void __attribute__((destructor)) cleanup_asan_late(void);

/* Complex recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* volatile to prevent optimization */
    struct ASTNode* left;
    struct ASTNode* right;
    unsigned char data[256];
} ASTNode;

/* Global volatile variables to prevent constant folding */
volatile size_t g_memcpy_len = 128;
volatile size_t g_memset_len = 64;
volatile size_t g_memmove_len = 96;

/* Token array for parser simulation */
static unsigned char token_buffer[4096];

/* Constructor - forces early initialization */
void __attribute__((constructor)) init_asan_early(void) {
    /* Initialize token buffer with pattern */
    for (size_t i = 0; i < sizeof(token_buffer); i++) {
        token_buffer[i] = (unsigned char)(i % 256);
    }
    
    /* Early built-in usage in constructor */
    volatile unsigned char local_buf[128];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    __builtin_memcpy(token_buffer, local_buf, 64);
}

/* Recursive AST manipulation with memory operations */
ASTNode* create_ast_node(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = depth;
    node->value = depth * 17;
    node->size = (size_t)(depth * 32);
    node->left = create_ast_node(depth - 1);
    node->right = create_ast_node(depth - 1);
    
    /* Use all three built-ins in AST creation */
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, token_buffer, node->size % 256);
    
    if (node->left && node->right) {
        /* memmove between child nodes */
        size_t move_len = (node->left->size < node->right->size) ? 
                          node->left->size : node->right->size;
        __builtin_memmove(node->right->data, node->left->data, move_len);
    }
    
    return node;
}

/* Function with goto jumps around memmove */
void process_with_goto(ASTNode* node) {
    if (!node) return;
    
    volatile unsigned char temp[512];
    int state = 0;
    
    goto start_block;
    
memmove_block:
    /* This block contains the critical memmove */
    __builtin_memmove(node->data, temp, g_memmove_len);
    state = 1;
    goto after_block;
    
start_block:
    /* Setup before jump */
    __builtin_memset(temp, 0xCC, sizeof(temp));
    __builtin_memcpy(temp, token_buffer, 256);
    
    if (node->type % 2 == 0) {
        goto memmove_block;
    } else {
        /* Alternative path */
        __builtin_memcpy(node->data, temp, g_memcpy_len);
    }
    
after_block:
    /* Post-processing */
    if (state) {
        __builtin_memset(temp + 200, 0xFF, 100);
    }
}

/* OpenMP parallel memory operations */
void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        volatile unsigned char local_buf[1024];
        
        /* Each thread uses all three built-ins */
        __builtin_memset(local_buf, i, sizeof(local_buf));
        
        if (nodes[i]) {
            __builtin_memcpy(nodes[i]->data, local_buf, 
                           g_memcpy_len % sizeof(nodes[i]->data));
            
            /* Conditional memmove with volatile length */
            if (i > 0 && nodes[i-1]) {
                size_t len = g_memmove_len;
                if (len > sizeof(nodes[i]->data)) len = sizeof(nodes[i]->data);
                __builtin_memmove(nodes[i]->data + 64, 
                                nodes[i-1]->data, len);
            }
        }
    }
}

/* Compute hash from AST structure */
unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    unsigned char* ptr = node->data;
    
    /* Process data with volatile counter */
    volatile size_t count = node->size % 256;
    for (size_t i = 0; i < count; i++) {
        hash = ((hash << 5) + hash) + ptr[i];
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

/* Destructor */
void __attribute__((destructor)) cleanup_asan_late(void) {
    /* Final built-in usage in destructor */
    volatile unsigned char final_buf[256];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf, "ASAN_TEST_COMPLETE", 19);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create AST structure */
    ASTNode* root = create_ast_node(4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto jumps */
    process_with_goto(root);
    
    /* Create array of nodes for parallel processing */
    ASTNode* nodes[8];
    nodes[0] = root;
    for (int i = 1; i < 8; i++) {
        nodes[i] = create_ast_node(3);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(nodes, 8);
    
    /* Compute and print verification hash */
    unsigned long total_hash = 0;
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            total_hash ^= compute_ast_hash(nodes[i]);
        }
    }
    
    printf("Verification hash: 0x%08lx\n", total_hash);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free(nodes[i]);
    }
    
    return 0;
}
