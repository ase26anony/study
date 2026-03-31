/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 1024;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i * 31) & 0xFF);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Clear sensitive data */
    __builtin_memset(g_token_pool, 0, sizeof(g_token_pool));
    printf("Destructor: Token pool cleared\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_recursive(int depth, volatile int* counter) {
    if (depth <= 0 || *counter >= 100) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    (*counter)++;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with tokens using __builtin_memcpy */
    size_t copy_size = (depth * 16) % 256;
    if (copy_size > 0) {
        __builtin_memcpy(node->data, 
                        &g_token_pool[g_token_idx], 
                        copy_size);
        g_token_idx = (g_token_idx + copy_size) % sizeof(g_token_pool);
    }
    node->size = copy_size;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 3) {
        goto skip_left;
    }
    
    create_left = 0;
    
skip_left:
    if (create_left) {
        node->left = create_ast_recursive(depth - 1, counter);
        
        /* Use __builtin_memmove when jumping back */
        if (node->left && node->right) {
            char temp[256];
            __builtin_memcpy(temp, node->left->data, 128);
            __builtin_memmove(node->left->data, node->right->data, 128);
            __builtin_memmove(node->right->data, temp, 128);
        }
    } else {
        node->left = NULL;
    }
    
    node->right = create_ast_recursive(depth - 2, counter);
    
    return node;
}

/* Complex memory operation with goto jumps */
static void process_with_goto(ASTNode* node, volatile size_t size) {
    if (!node) return;
    
    char buffer[512];
    volatile int do_copy = 1;
    
    /* Jump into memory operation block */
    if (node->size > 128) {
        goto perform_memmove;
    }
    
    do_copy = 0;
    
perform_memmove:
    if (do_copy) {
        /* Force __builtin_memmove redirection */
        __builtin_memmove(buffer, node->data, 
                         node->size > sizeof(buffer) ? sizeof(buffer) : node->size);
        
        /* Jump out and back in */
        if (node->left) {
            goto process_left;
        }
    }
    
    /* Normal path */
    if (node->right) {
        __builtin_memcpy(node->right->data, buffer, 
                        node->size > 256 ? 256 : node->size);
    }
    
    goto finish;
    
process_left:
    /* Another memory operation after goto */
    __builtin_memset(node->left->data, 0xA5, 
                    node->left->size > 128 ? 128 : node->left->size);
    
finish:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            char local_buf[256];
            volatile size_t op_size = g_mem_size % 256;
            
            /* Mix of memory built-ins in parallel region */
            __builtin_memset(local_buf, i, op_size);
            __builtin_memcpy(nodes[i]->data, local_buf, op_size);
            
            if (i > 0 && nodes[i-1]) {
                __builtin_memmove(nodes[i-1]->data + 128, 
                                 nodes[i]->data, 
                                 op_size > 128 ? 128 : op_size);
            }
        }
    }
}

/* Calculate hash from AST */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    for (size_t i = 0; i < node->size && i < 256; i++) {
        hash = ((hash << 5) + hash) + ptr[i];
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before freeing */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    volatile int node_counter = 0;
    ASTNode* root = NULL;
    
    /* Create complex AST */
    root = create_ast_recursive(5, &node_counter);
    printf("Created AST with %d nodes\n", node_counter);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto flow control */
    process_with_goto(root, g_mem_size);
    
    /* Create node array for parallel processing */
    ASTNode* node_array[8];
    node_array[0] = root;
    
    for (int i = 1; i < 8; i++) {
        volatile int counter = 0;
        node_array[i] = create_ast_recursive(3 + (i % 3), &counter);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Compute verification hash */
    unsigned long total_hash = 0;
    for (int i = 0; i < 8; i++) {
        if (node_array[i]) {
            total_hash ^= compute_ast_hash(node_array[i]);
        }
    }
    
    printf("Result hash: 0x%08lx\n", total_hash);
    
    /* Cleanup */
    for (int i = 1; i < 8; i++) {
        if (node_array[i]) {
            free_ast(node_array[i]);
        }
    }
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
