/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char final_check[8];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile lengths */
    __builtin_memset(node->data, depth, g_memset_len % 256);
    
    node->type = depth;
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    /* Copy data between nodes if both children exist */
    if (node->left && node->right) {
        __builtin_memcpy(node->right->data, node->left->data, 
                        g_memcpy_len % sizeof(node->data));
    }
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(ASTNode* nodes[], int count) {
    int i = 0;
    
    /* Jump into memory operation block */
    if (count > 0) goto process_block;
    
    return;
    
process_block:
    for (; i < count; i++) {
        if (nodes[i] && nodes[i]->left) {
            /* Use memmove with goto control flow */
            __builtin_memmove(nodes[i]->data + 32, nodes[i]->data, 
                            g_memmove_len % 128);
            
            /* Jump out and back in */
            if (i % 3 == 0) goto skip_operation;
            continue;
            
skip_operation:
            /* Alternative path */
            __builtin_memset(nodes[i]->data, i, 64);
            goto continue_loop;
        }
continue_loop:
        ;
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** pool, int pool_size) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < pool_size; i++) {
            if (pool[i]) {
                /* Thread-specific memory operations */
                volatile char local_buf[256];
                
                /* Mix of all three builtins */
                __builtin_memset(local_buf, tid, sizeof(local_buf));
                __builtin_memcpy(pool[i]->data, local_buf, 
                               g_memcpy_len % sizeof(local_buf));
                
                if (i > 0 && pool[i-1]) {
                    __builtin_memmove(pool[i]->data + 128, pool[i-1]->data,
                                    g_memmove_len % 128);
                }
            }
        }
        
        /* Barrier with memory operation */
        #pragma omp barrier
        
        #pragma omp single
        {
            volatile char sync_buf[64];
            __builtin_memset(sync_buf, 0xCC, sizeof(sync_buf));
        }
    }
}

/* Complex initialization with varied patterns */
static void initialize_token_array(char tokens[][256], int rows) {
    for (int i = 0; i < rows; i++) {
        /* Pattern-based initialization */
        __builtin_memset(tokens[i], i, 256);
        
        if (i > 0) {
            /* Overlap some regions */
            __builtin_memcpy(tokens[i] + 128, tokens[i-1], 128);
            __builtin_memmove(tokens[i] + 64, tokens[i] + 128, 64);
        }
    }
}

int main(void) {
    const int AST_DEPTH = 4;
    const int POOL_SIZE = 8;
    const int TOKEN_ROWS = 16;
    
    /* Create recursive structures */
    ASTNode* root = create_ast(AST_DEPTH);
    
    /* Create node pool */
    ASTNode* node_pool[POOL_SIZE];
    for (int i = 0; i < POOL_SIZE; i++) {
        node_pool[i] = create_ast(AST_DEPTH - (i % 3));
    }
    
    /* Initialize token array */
    char token_array[TOKEN_ROWS][256];
    initialize_token_array(token_array, TOKEN_ROWS);
    
    /* Process with goto edge cases */
    process_with_goto(node_pool, POOL_SIZE);
    
    /* Execute parallel operations */
    parallel_memory_ops(node_pool, POOL_SIZE);
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < TOKEN_ROWS; i++) {
        for (int j = 0; j < 256; j++) {
            hash = (hash * 31 + token_array[i][j]) % 1000000007;
        }
    }
    
    /* Additional builtin calls in main */
    volatile char final_buffer[512];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 256, token_array[0], 256);
    __builtin_memmove(final_buffer, final_buffer + 128, 128);
    
    printf("Verification hash: %lu\n", hash);
    
    /* Cleanup */
    /* Note: In real ASAN, memory leaks would be reported */
    
    return 0;
}
