/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char *volatile_ptr;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode *left;
    struct ASTNode *right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    volatile_ptr = token_pool;
    /* Force memset built-in in constructor */
    __builtin_memset(token_pool, 0xAA, sizeof(token_pool));
    printf("Constructor: Initialized token pool with memset\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    /* Force memmove built-in in destructor */
    char temp[256];
    __builtin_memmove(temp, token_pool, 256);
    printf("Destructor: Cleaned up with memmove\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to control length */
    int copy_len = volatile_len % 128 + 64;
    
    /* Force memcpy built-in */
    __builtin_memcpy(node->data, base_data, copy_len);
    node->id = depth;
    
    /* Create children with goto-controlled flow */
    if (depth > 1) {
        char child_data[256];
        __builtin_memset(child_data, depth, sizeof(child_data));
        
        /* Jump into memory operation block */
        goto create_left;
        
        /* This label is jumped into */
        create_left:
        node->left = create_ast(depth - 1, child_data);
        
        /* Jump out and back in */
        if (depth > 2) {
            goto skip_right;
        }
        
        skip_right_back:
        node->right = create_ast(depth - 2, child_data);
        goto end_create;
        
        skip_right:
        /* Force memmove with goto */
        char buffer[128];
        __builtin_memmove(buffer, node->data, 64);
        goto skip_right_back;
    } else {
        node->left = node->right = NULL;
    }
    
    end_create:
    return node;
}

/* Complex memory operation with OpenMP */
static void parallel_memory_operations(ASTNode *root) {
    if (!root) return;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buffer[512];
        
        /* Each thread performs different memory operations */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            switch (i % 3) {
                case 0:
                    /* Force memcpy in parallel region */
                    __builtin_memcpy(local_buffer, root->data, 
                                    volatile_len % 256);
                    break;
                case 1:
                    /* Force memset in parallel region */
                    __builtin_memset(local_buffer + thread_id * 16, 
                                    i, 32);
                    break;
                case 2:
                    /* Force memmove in parallel region */
                    __builtin_memmove(local_buffer + 128, 
                                     local_buffer, 64);
                    break;
            }
            
            /* Update token pool with results */
            #pragma omp critical
            {
                int offset = (token_index + i) % sizeof(token_pool);
                __builtin_memcpy(&token_pool[offset], 
                                local_buffer, 32);
                token_index = (token_index + 32) % sizeof(token_pool);
            }
        }
    }
}

/* Multi-stage processing with different memory built-ins */
static unsigned long process_ast(ASTNode *node) {
    if (!node) return 0;
    
    unsigned long hash = 0;
    char temp_buffer[512];
    
    /* Stage 1: Copy node data */
    __builtin_memcpy(temp_buffer, node->data, 256);
    
    /* Stage 2: Clear part of buffer */
    __builtin_memset(temp_buffer + 128, 0, 128);
    
    /* Stage 3: Move data around */
    __builtin_memmove(temp_buffer + 64, temp_buffer, 192);
    
    /* Calculate hash from processed data */
    for (int i = 0; i < 256; i++) {
        hash = hash * 31 + temp_buffer[i];
    }
    
    /* Process children recursively */
    hash += process_ast(node->left);
    hash += process_ast(node->right);
    
    return hash;
}

/* Free AST with memory operations */
static void free_ast(ASTNode *node) {
    if (!node) return;
    
    /* Clear data before freeing */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Final memory operation before free */
    char verify[256];
    __builtin_memcpy(verify, node, sizeof(ASTNode));
    
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize with volatile-controlled values */
    volatile_len = 128;
    volatile_ptr = token_pool + 1024;
    
    /* Create complex AST */
    ASTNode *root = create_ast(4, "Base data for AST construction");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    printf("Created AST with depth 4\n");
    
    /* Perform parallel memory operations */
    parallel_memory_operations(root);
    printf("Completed parallel memory operations\n");
    
    /* Process AST with multiple memory built-ins */
    unsigned long result_hash = process_ast(root);
    printf("AST processing hash: %lu\n", result_hash);
    
    /* Additional explicit built-in calls in main */
    char final_buffer[1024];
    
    /* Chain of memory operations */
    __builtin_memset(final_buffer, 0xCC, sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 512, root->data, 256);
    __builtin_memmove(final_buffer, final_buffer + 256, 512);
    
    /* Use volatile to force non-optimization */
    if (volatile_len > 0) {
        __builtin_memset(volatile_ptr, volatile_len, 256);
    }
    
    /* Clean up */
    free_ast(root);
    
    /* Final verification */
    int sum = 0;
    for (size_t i = 0; i < sizeof(token_pool); i++) {
        sum += token_pool[i];
    }
    printf("Token pool checksum: %d\n", sum % 1000);
    
    printf("Test completed successfully\n");
    return 0;
}
