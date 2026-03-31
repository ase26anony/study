/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)((i * 7) & 0xFF);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Clear sensitive data */
    __builtin_memset(token_pool, 0, sizeof(token_pool));
    printf("Destructor: Token pool cleared\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = depth;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy pattern from token pool using __builtin_memcpy */
    int copy_len = volatile_len % 256;
    if (copy_len > 0) {
        __builtin_memcpy(node->data, 
                        &token_pool[(depth * 32) % sizeof(token_pool)],
                        copy_len);
    }
    
    /* Recursive creation with goto for flow control */
    if (volatile_flag) {
        goto create_left;
    }
    
create_left:
    node->left = create_ast(depth + 1, max_depth);
    goto skip_right;
    
skip_right:
    if (depth % 2 == 0) {
        node->right = create_ast(depth + 2, max_depth);
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    char buffer[512];
    
    if (!src || !dst) return;
    
    /* Jump into memory operation block */
    goto start_copy;
    
start_copy:
    /* Use __builtin_memmove with overlapping regions */
    __builtin_memmove(buffer, src->data, sizeof(src->data));
    
    /* Conditional jump out */
    if (src->id % 3 == 0) {
        goto skip_processing;
    }
    
    /* Modify buffer */
    for (int i = 0; i < 128; i++) {
        buffer[i] ^= 0x55;
    }
    
    /* Jump back in */
    goto continue_processing;
    
skip_processing:
    /* Simple copy without processing */
    __builtin_memcpy(buffer, src->data, 128);
    
continue_processing:
    /* Copy to destination */
    __builtin_memcpy(dst->data, buffer, sizeof(dst->data));
    
    /* Handle potential overlap with memmove */
    if (src == dst) {
        __builtin_memmove(dst->data + 64, dst->data, 128);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes)
    for (i = 0; i < count - 1; i++) {
        if (nodes[i] && nodes[i + 1]) {
            /* Force built-in usage in parallel context */
            __builtin_memcpy(nodes[i]->data + 128, 
                           nodes[i + 1]->data, 
                           128);
            
            /* Use memset in parallel */
            __builtin_memset(nodes[i]->data + 192, 
                           i % 256, 
                           64);
            
            /* Conditional memmove */
            if (i % 2 == 0) {
                __builtin_memmove(nodes[i]->data + 64,
                                nodes[i]->data,
                                64);
            }
        }
    }
}

/* Calculate hash of AST structure */
static unsigned long calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Process data with volatile control */
    int len = volatile_len % 256;
    for (int i = 0; i < len && i < 256; i++) {
        hash = ((hash << 5) + hash) + ptr[i];
    }
    
    /* Recursive hash calculation */
    hash += calculate_ast_hash(node->left);
    hash += calculate_ast_hash(node->right);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear node data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create AST structures */
    ASTNode* ast1 = create_ast(0, 4);
    ASTNode* ast2 = create_ast(0, 3);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST structures\n");
        return 1;
    }
    
    /* Process with goto flow control */
    process_with_goto(ast1, ast2);
    
    /* Create array for parallel processing */
    ASTNode* node_array[8];
    node_array[0] = ast1;
    node_array[1] = ast2;
    
    for (int i = 2; i < 8; i++) {
        node_array[i] = create_ast(i, 2);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Calculate and print verification hash */
    unsigned long total_hash = 0;
    for (int i = 0; i < 8; i++) {
        if (node_array[i]) {
            total_hash ^= calculate_ast_hash(node_array[i]);
        }
    }
    
    printf("Verification hash: 0x%08lx\n", total_hash);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        if (node_array[i]) {
            free_ast(node_array[i]);
        }
    }
    
    /* Final built-in calls to ensure coverage */
    char final_buffer[256];
    __builtin_memset(final_buffer, 0xAA, sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 64, final_buffer, 128);
    __builtin_memmove(final_buffer, final_buffer + 32, 192);
    
    printf("Test completed successfully\n");
    return 0;
}
