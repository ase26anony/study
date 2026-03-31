/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char* g_tokens[] = {"memcpy", "memset", "memmove", "test", "data"};
static const int g_token_count = 5;

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_asan_hooks(void) {
    printf("Constructor: Initializing ASAN hooks\n");
}

__attribute__((destructor)) static void cleanup_asan_hooks(void) {
    printf("Destructor: Cleaning up ASAN hooks\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to control memset size */
    volatile size_t clear_size = sizeof(node->data);
    __builtin_memset(node->data, 0, clear_size);
    
    node->id = id;
    
    /* Create pattern in data using memcpy */
    char pattern[64];
    volatile int pattern_len = 32;
    __builtin_memset(pattern, 'A' + (id % 26), pattern_len);
    __builtin_memcpy(node->data, pattern, pattern_len);
    
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Function with goto statements around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_memmove = g_use_memmove;
    
    if (use_memmove) {
        goto use_memmove_block;
    } else {
        goto use_memcpy_block;
    }
    
use_memmove_block:
    {
        volatile size_t move_size = g_mem_size % 128;
        __builtin_memmove(dst->data, src->data, move_size);
        goto after_operation;
    }
    
use_memcpy_block:
    {
        volatile size_t copy_size = g_mem_size % 128;
        __builtin_memcpy(dst->data, src->data, copy_size);
        goto after_operation;
    }
    
after_operation:
    /* Modify data to ensure operation happened */
    dst->data[0] = 'Z';
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count - 1; i++) {
            volatile size_t op_size = (g_mem_size + tid) % 256;
            
            /* Alternate between memcpy and memmove */
            if (i % 3 == 0) {
                __builtin_memcpy(nodes[i]->data, nodes[i+1]->data, op_size);
            } else if (i % 3 == 1) {
                __builtin_memmove(nodes[i]->data, nodes[i+1]->data, op_size);
            } else {
                __builtin_memset(nodes[i]->data, '0' + tid, op_size);
            }
        }
    }
}

/* Compute hash of AST structure */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    
    /* Hash the data using builtins */
    volatile size_t hash_len = strlen(node->data);
    for (size_t i = 0; i < hash_len && i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create AST structures */
    ASTNode* ast1 = create_ast(3, 1);
    ASTNode* ast2 = create_ast(3, 100);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST structures\n");
        return 1;
    }
    
    /* Test goto with memmove */
    printf("Testing goto flow with memmove...\n");
    process_with_goto(ast1, ast2);
    
    /* Create array of nodes for parallel processing */
    ASTNode* node_array[8];
    node_array[0] = ast1;
    node_array[1] = ast2;
    
    for (int i = 2; i < 8; i++) {
        node_array[i] = create_ast(2, 200 + i);
    }
    
    /* Execute parallel memory operations */
    printf("Executing parallel memory operations...\n");
    parallel_memory_ops(node_array, 8);
    
    /* Process tokens with memory operations */
    char token_buffer[512] = {0};
    char* current = token_buffer;
    
    for (int i = 0; i < g_token_count; i++) {
        volatile size_t token_len = strlen(g_tokens[i]);
        __builtin_memcpy(current, g_tokens[i], token_len);
        current += token_len;
        
        if (i < g_token_count - 1) {
            __builtin_memset(current, '|', 1);
            current += 1;
        }
    }
    
    /* Final memory operation with conditional */
    volatile int final_op = 1;
    char final_buffer[256];
    
    if (final_op) {
        volatile size_t final_size = g_mem_size % 128;
        __builtin_memmove(final_buffer, token_buffer, final_size);
    } else {
        volatile size_t final_size = g_mem_size % 128;
        __builtin_memcpy(final_buffer, token_buffer, final_size);
    }
    
    /* Compute and print verification hash */
    unsigned long hash1 = compute_ast_hash(ast1);
    unsigned long hash2 = compute_ast_hash(ast2);
    unsigned long total_hash = hash1 + hash2;
    
    printf("Verification hash: %lu\n", total_hash);
    printf("Token buffer: %s\n", token_buffer);
    
    /* Cleanup */
    for (int i = 2; i < 8; i++) {
        free(node_array[i]);
    }
    free(ast1);
    free(ast2);
    
    printf("Test completed successfully\n");
    return 0;
}
