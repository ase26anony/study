/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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
static void init_asan_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (i % 256);
    }
    
    /* Use builtins in constructor to trigger early initialization */
    __builtin_memset(token_pool + 1024, 0xAA, 128);
    __builtin_memcpy(token_pool + 1152, token_pool, 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    /* Final builtin usage in destructor */
    __builtin_memset(token_pool, 0, 256);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = depth;
    
    /* Use volatile-controlled length */
    int copy_len = volatile_len % 128 + 64;
    
    /* Builtin memcpy with goto control flow */
    if (volatile_flag) {
        goto copy_block;
    } else {
        goto skip_copy;
    }
    
copy_block:
    __builtin_memcpy(node->data, base_data, copy_len);
    goto after_copy;
    
skip_copy:
    __builtin_memset(node->data, 0, sizeof(node->data));
    
after_copy:
    /* Recursive creation with different memory operations */
    node->left = create_ast(depth - 1, node->data);
    node->right = create_ast(depth - 2, node->data + 64);
    
    /* Builtin memmove between nodes */
    if (node->left && node->right) {
        __builtin_memmove(node->right->data, node->left->data, 32);
    }
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_operations(ASTNode* root) {
    if (!root) return;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buffer[512];
        
        /* Each thread uses builtins independently */
        __builtin_memset(local_buffer, thread_id, sizeof(local_buffer));
        
        #pragma omp for
        for (int i = 0; i < 8; i++) {
            char temp[256];
            int offset = (i * 32) % 256;
            
            /* Mix of builtin calls */
            if (i % 3 == 0) {
                __builtin_memcpy(temp, root->data + offset, 64);
                __builtin_memset(temp + 32, i, 32);
            } else if (i % 3 == 1) {
                __builtin_memmove(temp, local_buffer + offset, 128);
            } else {
                __builtin_memset(temp, 0xFF, 64);
                __builtin_memcpy(temp + 64, root->data, 64);
            }
            
            /* Conditional goto with builtin */
            if (thread_id % 2 == 0) {
                goto use_memmove;
            }
            
            __builtin_memcpy(local_buffer + (i * 64), temp, 64);
            goto continue_loop;
            
        use_memmove:
            __builtin_memmove(local_buffer + (i * 64), temp, 64);
            
        continue_loop:
            /* No-op label for goto target */
            ;
        }
        
        /* Final builtin in parallel region */
        __builtin_memcpy(root->data + (thread_id * 16), local_buffer, 16);
    }
}

/* Calculate hash of AST structure */
static unsigned long long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long long hash = 5381;
    char* ptr = node->data;
    
    /* Process data with builtin-assisted bounds */
    int len = volatile_len % 128;
    char temp[256];
    
    __builtin_memcpy(temp, ptr, len);
    
    for (int i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + temp[i];
    }
    
    /* Recursive hash computation */
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear sensitive data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize complex data */
    char init_data[512];
    for (int i = 0; i < sizeof(init_data); i++) {
        init_data[i] = (i * 7) % 256;
    }
    
    /* Create recursive structure */
    ASTNode* root = create_ast(5, init_data);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Builtin operations on global pool */
    __builtin_memcpy(token_pool + 2048, init_data, 256);
    __builtin_memset(token_pool + 2304, 0xCC, 128);
    __builtin_memmove(token_pool + 2432, token_pool + 2048, 64);
    
    /* Execute parallel memory operations */
    parallel_memory_operations(root);
    
    /* Compute verification hash */
    unsigned long long hash = compute_ast_hash(root);
    hash += (unsigned long long)token_pool[0];
    hash += (unsigned long long)token_pool[1024];
    
    printf("Computed hash: %llu\n", hash);
    printf("Token pool[0]=%d, [1024]=%d\n", 
           token_pool[0], token_pool[1024]);
    
    /* Cleanup */
    free_ast(root);
    
    /* Final builtin calls */
    __builtin_memset(init_data, 0, sizeof(init_data));
    
    printf("Test completed successfully.\n");
    return 0;
}
