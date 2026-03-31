/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
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

/* Global token array for parser simulation */
static const char* TOKENS[] = {"memcpy", "memset", "memmove", "data", "node"};
static const size_t NUM_TOKENS = sizeof(TOKENS) / sizeof(TOKENS[0]);

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 128; /* Force non-constant size */
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, const char** tokens, size_t token_idx) {
    if (depth <= 0 || token_idx >= NUM_TOKENS) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data with memcpy */
    size_t token_len = strlen(tokens[token_idx]);
    size_t copy_len = token_len < sizeof(node->data) ? token_len : sizeof(node->data) - 1;
    __builtin_memcpy(node->data, tokens[token_idx], copy_len);
    node->data[copy_len] = '\0';
    node->size = copy_len;
    
    /* Recursive parsing with goto for flow control */
    int use_left = (depth % 2 == 0);
    
    if (use_left) {
        goto parse_left;
    } else {
        goto parse_right;
    }
    
parse_left:
    node->left = parse_expression(depth - 1, tokens, (token_idx + 1) % NUM_TOKENS);
    goto after_left;
    
parse_right:
    node->right = parse_expression(depth - 1, tokens, (token_idx + 2) % NUM_TOKENS);
    goto after_right;
    
after_left:
    /* Fall through */
after_right:
    return node;
}

/* Complex memory operation with goto jumping */
static void complex_memory_operations(void* dest, const void* src, size_t size) {
    volatile char* vdest = (volatile char*)dest;
    volatile const char* vsrc = (volatile const char*)src;
    
    /* Jump into memory operation block */
    goto start_memmove;
    
    /* This label is jumped into */
start_memmove:
    /* Use __builtin_memmove with volatile destination */
    __builtin_memmove((void*)vdest, (const void*)vsrc, size);
    
    /* Jump out to different context */
    goto after_memmove;
    
    /* Dead code that won't be executed normally */
    __builtin_memset((void*)vdest, 0xFF, size);
    
after_memmove:
    /* Additional memory operations */
    char temp[256];
    __builtin_memcpy(temp, dest, size < 256 ? size : 256);
    
    /* Conditional memset */
    if (size > 32) {
        __builtin_memset(((char*)dest) + 32, 0xAA, size - 32);
    }
}

/* OpenMP parallel memory dispatcher */
static void parallel_memory_dispatch(ASTNode** nodes, size_t num_nodes) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (size_t i = 0; i < num_nodes; i++) {
            if (nodes[i] && nodes[i]->left && nodes[i]->right) {
                /* Inter-node memory copy */
                size_t copy_size = nodes[i]->left->size < nodes[i]->right->size ? 
                                 nodes[i]->left->size : nodes[i]->right->size;
                
                /* Force ASAN to handle built-in in parallel region */
                __builtin_memcpy(nodes[i]->left->data, 
                               nodes[i]->right->data, 
                               copy_size);
                
                /* Thread-specific memset pattern */
                char pattern = (char)(thread_id + 'A');
                __builtin_memset(nodes[i]->data + copy_size, 
                               pattern, 
                               sizeof(nodes[i]->data) - copy_size);
            }
        }
        
        /* Barrier with memory operation */
        #pragma omp barrier
        
        #pragma omp single
        {
            /* Master thread does additional memmove */
            if (num_nodes >= 2 && nodes[0] && nodes[1]) {
                size_t move_size = nodes[0]->size < nodes[1]->size ? 
                                 nodes[0]->size : nodes[1]->size;
                __builtin_memmove(nodes[0]->data, nodes[1]->data, move_size);
            }
        }
    }
}

/* Compute hash of AST structure */
static size_t compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 5381;
    for (size_t i = 0; i < node->size && i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    hash ^= compute_ast_hash(node->left);
    hash ^= compute_ast_hash(node->right);
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Initialize test data */
    const size_t num_nodes = 8;
    ASTNode* nodes[num_nodes];
    
    /* Create AST structure */
    for (size_t i = 0; i < num_nodes; i++) {
        nodes[i] = parse_expression(3, TOKENS, i % NUM_TOKENS);
    }
    
    /* Phase 2: Complex memory operations */
    for (size_t i = 0; i < num_nodes - 1; i++) {
        if (nodes[i] && nodes[i + 1]) {
            complex_memory_operations(nodes[i]->data, 
                                    nodes[i + 1]->data, 
                                    g_mem_size);
        }
    }
    
    /* Phase 3: Parallel memory dispatch */
    parallel_memory_dispatch(nodes, num_nodes);
    
    /* Phase 4: Verify and compute results */
    size_t total_hash = 0;
    for (size_t i = 0; i < num_nodes; i++) {
        if (nodes[i]) {
            size_t node_hash = compute_ast_hash(nodes[i]);
            total_hash ^= node_hash;
            
            /* Additional built-in usage in verification */
            char verify_buf[256];
            __builtin_memcpy(verify_buf, nodes[i]->data, 
                           nodes[i]->size < 256 ? nodes[i]->size : 256);
            
            /* Use all three built-ins in final phase */
            __builtin_memset(verify_buf + nodes[i]->size, 0, 
                           sizeof(verify_buf) - nodes[i]->size);
            
            char temp_buf[256];
            __builtin_memmove(temp_buf, verify_buf, sizeof(temp_buf));
        }
    }
    
    /* Phase 5: Cleanup with memory operations */
    for (size_t i = 0; i < num_nodes; i++) {
        if (nodes[i]) {
            /* Clear sensitive data before free */
            __builtin_memset(nodes[i]->data, 0, sizeof(nodes[i]->data));
            free(nodes[i]);
        }
    }
    
    printf("Test completed. Total hash: 0x%zx\n", total_hash);
    printf("Built-in functions tested: __builtin_memcpy, __builtin_memset, __builtin_memmove\n");
    
    return 0;
}
