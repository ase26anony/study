/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_globals(void) {
    /* Force initialization of ASAN runtime */
    volatile char init_buf[32];
    __builtin_memset(init_buf, 0xA5, sizeof(init_buf));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    /* Final memory operation to ensure cleanup path */
    volatile char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy with volatile size */
    size_t copy_size = g_mem_size % 64;
    __builtin_memcpy(node->data, base_data, copy_size);
    node->size = copy_size;
    
    /* Recursive creation with different memory patterns */
    char child_data[64];
    __builtin_memset(child_data, depth, sizeof(child_data));
    
    node->left = create_ast(depth - 1, child_data);
    node->right = create_ast(depth - 1, child_data);
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    volatile int use_copy = 1;
    
copy_block:
    if (use_copy) {
        /* This block should trigger memcpy redirection */
        __builtin_memcpy(dst->data, src->data, src->size);
        use_copy = 0;
        goto move_block;
    }
    
move_block:
    if (g_use_memmove) {
        /* Force memmove redirection with overlapping regions */
        char temp[128];
        __builtin_memcpy(temp, src->data, src->size);
        __builtin_memmove(dst->data + 10, dst->data, src->size - 10);
        __builtin_memcpy(dst->data, temp, src->size);
    }
    
    /* Jump back for edge case */
    if (dst->size > 50) {
        goto copy_block;
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        volatile char local_buf[256];
        
        /* Each thread uses builtins independently */
        __builtin_memset(local_buf, i, sizeof(local_buf));
        
        if (nodes[i]) {
            /* Mix of memcpy and memmove in parallel region */
            if (i % 2 == 0) {
                __builtin_memcpy(nodes[i]->data, local_buf, 
                               nodes[i]->size % sizeof(local_buf));
            } else {
                /* Create overlapping scenario for memmove */
                __builtin_memmove(nodes[i]->data + 32, nodes[i]->data, 
                                nodes[i]->size - 32);
                __builtin_memcpy(nodes[i]->data, local_buf + 32, 32);
            }
        }
        
        /* Additional memset in same region */
        __builtin_memset(local_buf + 128, 0xFF, 64);
    }
}

/* Complex initialization with multiple builtins */
static void initialize_token_array(char tokens[][64], int rows) {
    volatile int pattern = 0xCC;
    
    for (int i = 0; i < rows; i++) {
        /* Alternate between memset and memcpy patterns */
        if (i % 3 == 0) {
            __builtin_memset(tokens[i], pattern++, 64);
        } else if (i % 3 == 1) {
            char source[64];
            __builtin_memset(source, i, 64);
            __builtin_memcpy(tokens[i], source, 64);
        } else {
            /* Use memmove for overlapping copy */
            if (i > 0) {
                __builtin_memmove(tokens[i], tokens[i-1], 64);
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    const int NUM_NODES = 8;
    const int TOKEN_ROWS = 16;
    
    /* Initialize complex token array */
    char tokens[TOKEN_ROWS][64];
    initialize_token_array(tokens, TOKEN_ROWS);
    
    /* Create recursive AST structures */
    ASTNode* root = create_ast(4, "BaseASTNodeData");
    ASTNode* nodes[NUM_NODES];
    
    for (int i = 0; i < NUM_NODES; i++) {
        nodes[i] = create_ast(3, tokens[i % TOKEN_ROWS]);
    }
    
    /* Process with goto edge cases */
    if (root && nodes[0]) {
        process_with_goto(root, nodes[0]);
    }
    
    /* Execute parallelized memory operations */
    parallel_memory_ops(nodes, NUM_NODES);
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < NUM_NODES; i++) {
        if (nodes[i]) {
            for (size_t j = 0; j < nodes[i]->size && j < 64; j++) {
                hash = (hash * 31) + nodes[i]->data[j];
            }
            
            /* Final memory operation in main */
            __builtin_memset(nodes[i]->data + 48, 0, 16);
        }
    }
    
    /* Additional builtin calls in cleanup */
    char final_buf[256];
    __builtin_memcpy(final_buf, tokens[TOKEN_ROWS-1], 64);
    __builtin_memset(final_buf + 64, 0xAA, 128);
    __builtin_memmove(final_buf + 128, final_buf, 64);
    
    printf("Verification hash: %lu\n", hash);
    printf("Memory operations completed successfully\n");
    
    /* Cleanup */
    // Note: In real usage, free allocated nodes here
    // Omitted for coverage testing purposes
    
    return 0;
}
