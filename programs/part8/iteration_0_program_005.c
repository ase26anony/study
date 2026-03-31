/* ISO C99-compliant test program for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations for attribute functions */
void __attribute__((constructor)) init_asan_test(void);
void __attribute__((destructor)) cleanup_asan_test(void);

/* Complex AST-like node structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    volatile int data;
    char buffer[64];
    volatile size_t size;
} ASTNode;

/* Global volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_init_value = 0x42;
volatile int g_control_flag = 1;

/* Token array for parser simulation */
static volatile char token_array[256];

/* Recursive AST builder with memory operations */
ASTNode* build_ast(int depth, volatile int* counter) {
    if (depth <= 0 || *counter >= 100) {
        ASTNode* leaf = (ASTNode*)malloc(sizeof(ASTNode));
        if (!leaf) return NULL;
        
        /* Use builtin memset for initialization */
        __builtin_memset(leaf, 0, sizeof(ASTNode));
        leaf->data = (*counter)++;
        leaf->size = g_mem_size;
        
        /* Initialize buffer with builtin memcpy */
        char init_pattern[64];
        __builtin_memset(init_pattern, g_init_value, sizeof(init_pattern));
        __builtin_memcpy(leaf->buffer, init_pattern, leaf->size);
        
        return leaf;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->data = (*counter)++;
    node->size = g_mem_size;
    
    /* Build children recursively */
    node->left = build_ast(depth - 1, counter);
    node->right = build_ast(depth - 1, counter);
    
    /* Copy data between nodes using builtin memcpy */
    if (node->left && node->right) {
        __builtin_memcpy(node->buffer, node->left->buffer, 
                        node->size < sizeof(node->buffer) ? node->size : sizeof(node->buffer));
    }
    
    return node;
}

/* Parser with goto flow control */
void parse_tokens_with_goto(volatile char* tokens, size_t len) {
    volatile int state = 0;
    volatile char* current = tokens;
    
    /* Label for goto jumps */
    process_block:
    
    if (state == 0 && current < tokens + len) {
        /* Use builtin memmove with goto jumping into block */
        char temp[32];
        __builtin_memcpy(temp, current, 32);
        
        if (g_control_flag) {
            goto memmove_operation;
        }
        
        current += 32;
        state = 1;
        goto process_block;
    }
    
    if (state == 1) {
        memmove_operation:
        /* This tests flow sensitivity of asan_memfn_rtls retrieval */
        __builtin_memmove(current - 16, current, 16);
        
        if (current + 16 < tokens + len) {
            current += 16;
            state = 0;
            goto process_block;
        }
    }
}

/* OpenMP parallel memory operations */
void parallel_memory_operations(ASTNode** nodes, int count) {
    volatile int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Force builtin usage in parallel context */
            char temp_buffer[64];
            
            /* Use all three builtins */
            __builtin_memset(temp_buffer, i, sizeof(temp_buffer));
            __builtin_memcpy(nodes[i]->buffer, temp_buffer, 
                           nodes[i]->size < sizeof(temp_buffer) ? nodes[i]->size : sizeof(temp_buffer));
            
            /* Conditional memmove */
            if (i > 0 && nodes[i-1]) {
                __builtin_memmove(nodes[i-1]->buffer + 16, nodes[i]->buffer, 32);
            }
        }
    }
}

/* Constructor function for initialization */
void __attribute__((constructor)) init_asan_test(void) {
    /* Initialize token array with pattern */
    for (volatile int i = 0; i < 256; i++) {
        token_array[i] = (char)(i % 256);
    }
    
    /* Force early builtin usage in constructor */
    char init_buf[128];
    __builtin_memset(init_buf, 0xAA, sizeof(init_buf));
    __builtin_memcpy((void*)token_array, init_buf, 128);
}

/* Destructor for cleanup */
void __attribute__((destructor)) cleanup_asan_test(void) {
    /* Empty - exists to force linking logic */
}

/* Main execution flow */
int main(void) {
    volatile int counter = 0;
    volatile int result_hash = 0;
    
    /* Build AST structure */
    ASTNode* root = build_ast(4, &counter);
    
    if (!root) {
        fprintf(stderr, "Failed to build AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    ASTNode* nodes[8];
    nodes[0] = root;
    
    /* Build additional nodes */
    for (volatile int i = 1; i < 8; i++) {
        nodes[i] = build_ast(3, &counter);
    }
    
    /* Parse tokens with goto flow control */
    parse_tokens_with_goto(token_array, sizeof(token_array));
    
    /* Execute parallel memory operations */
    parallel_memory_operations(nodes, 8);
    
    /* Compute verification hash */
    for (volatile int i = 0; i < 8; i++) {
        if (nodes[i]) {
            for (volatile int j = 0; j < 64 && j < (int)nodes[i]->size; j++) {
                result_hash += nodes[i]->buffer[j];
            }
            result_hash += nodes[i]->data;
        }
    }
    
    /* Cleanup */
    for (volatile int i = 0; i < 8; i++) {
        if (nodes[i]) {
            free(nodes[i]);
        }
    }
    
    printf("Result hash: %d\n", result_hash);
    printf("Builtin calls executed successfully\n");
    
    return 0;
}
