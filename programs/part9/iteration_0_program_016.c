/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i * 7) & 0xFF);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill data with pattern using volatile size */
    volatile size_t fill_size = g_mem_size % 64;
    for (size_t i = 0; i < fill_size; i++) {
        node->data[i] = (char)((node->id + i) & 0xFF);
    }
    
    /* Recursive creation with goto for control flow */
    int create_left = 1;
    
create_children:
    if (create_left) {
        node->left = create_ast(depth - 1, counter);
        create_left = 0;
        goto create_children; /* Jump back to create right child */
    } else {
        node->right = create_ast(depth - 1, counter);
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int stage = 0;
    
    if (!src || !dst) return;
    
    /* Jump into memory operation block */
    if (src->id % 2 == 0) {
        goto mem_operation;
    }
    
    /* Normal path */
    stage = 1;
    
mem_operation:
    {
        /* This block contains __builtin_memmove with goto entry */
        volatile size_t copy_len = g_mem_size % 64;
        
        /* Force ASAN to handle memmove redirection */
        __builtin_memmove(dst->data, src->data, copy_len);
        
        if (stage == 0) {
            /* Jumped directly here */
            goto after_copy;
        }
    }
    
    stage = 2;
    
after_copy:
    /* Additional processing */
    if (dst->left) {
        goto mem_operation; /* Jump back for another copy */
    }
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i] && nodes[i]->left && nodes[i]->right) {
            volatile size_t op_size = (g_mem_size + i) % 64;
            
            /* Use all three builtins in parallel regions */
            __builtin_memset(nodes[i]->data, i, op_size);
            
            if (i > 0) {
                __builtin_memcpy(nodes[i]->left->data, 
                               nodes[i-1]->data, 
                               op_size);
            }
            
            if (i < count - 1) {
                __builtin_memmove(nodes[i]->right->data,
                                nodes[i]->data,
                                op_size);
            }
        }
    }
}

/* Complex initialization with multiple memory builtins */
static void initialize_complex_buffer(char* buffer, size_t size) {
    volatile size_t segment = size / 4;
    
    /* Pattern initialization using memset */
    __builtin_memset(buffer, 0xAA, segment);
    
    /* Copy pattern to other segments */
    __builtin_memcpy(buffer + segment, buffer, segment);
    
    /* Overlap copy with memmove */
    __builtin_memmove(buffer + segment * 2, buffer + segment, segment);
    
    /* Final fill */
    __builtin_memset(buffer + segment * 3, 0x55, size - segment * 3);
}

/* Main test driver */
int main(void) {
    const int AST_DEPTH = 4;
    const int NODE_COUNT = 8;
    int node_counter = 0;
    ASTNode* nodes[NODE_COUNT];
    ASTNode* copy_nodes[NODE_COUNT];
    
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Create AST structures */
    for (int i = 0; i < NODE_COUNT; i++) {
        nodes[i] = create_ast(AST_DEPTH, &node_counter);
        copy_nodes[i] = create_ast(AST_DEPTH, &node_counter);
        
        if (!nodes[i] || !copy_nodes[i]) {
            printf("Error: Memory allocation failed\n");
            return 1;
        }
    }
    
    /* Phase 2: Test goto control flow with memmove */
    for (int i = 0; i < NODE_COUNT; i++) {
        process_with_goto(nodes[i], copy_nodes[i]);
    }
    
    /* Phase 3: Parallel memory operations */
    printf("Starting parallel memory operations...\n");
    parallel_memory_ops(nodes, NODE_COUNT);
    
    /* Phase 4: Complex buffer operations */
    char complex_buf[1024];
    initialize_complex_buffer(complex_buf, sizeof(complex_buf));
    
    /* Phase 5: Token pool operations with all builtins */
    volatile size_t token_op_size = g_mem_size % 256;
    
    /* Use all three builtins in sequence */
    __builtin_memset(g_token_pool + 1024, 0, token_op_size);
    __builtin_memcpy(g_token_pool + 1536, g_token_pool + 512, token_op_size);
    __builtin_memmove(g_token_pool + 2048, g_token_pool + 1024, token_op_size);
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < NODE_COUNT; i++) {
        if (nodes[i]) {
            for (int j = 0; j < 64; j++) {
                hash += (unsigned long)nodes[i]->data[j];
            }
        }
    }
    
    for (int i = 0; i < sizeof(complex_buf); i++) {
        hash += (unsigned long)complex_buf[i];
    }
    
    /* Add token pool contribution */
    for (int i = 0; i < 256; i++) {
        hash += (unsigned long)g_token_pool[i * 16];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    for (int i = 0; i < NODE_COUNT; i++) {
        free(nodes[i]);
        free(copy_nodes[i]);
    }
    
    return 0;
}
