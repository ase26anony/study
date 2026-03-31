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
static void init_tokens(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (i % 26) + 'a';
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup(void) {
    /* Verify memory was properly handled */
    volatile char check = token_pool[0];
    (void)check; /* Prevent unused warning */
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
    
    /* Copy pattern into node using __builtin_memcpy */
    int copy_len = volatile_len % 128;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    
    __builtin_memcpy(node->data, token_pool + token_index, copy_len);
    token_index = (token_index + copy_len) % sizeof(token_pool);
    
    /* Create children recursively */
    if (depth < max_depth - 1) {
        node->left = create_ast(depth + 1, max_depth);
        node->right = create_ast(depth + 1, max_depth);
        
        /* Copy between nodes using __builtin_memmove with goto */
        if (node->left && node->right) {
            int use_goto = volatile_flag;
            
            if (use_goto) {
                goto copy_block;
            } else {
                /* Direct copy */
                __builtin_memmove(node->right->data, node->left->data, 
                                 sizeof(node->left->data));
            }
            
copy_block:
            if (use_goto) {
                /* Jump into memmove block */
                __builtin_memmove(node->left->data, node->right->data,
                                 sizeof(node->right->data));
            }
        }
    }
    
    return node;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

/* Calculate hash of AST */
static unsigned long ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Process each character */
    for (size_t i = 0; i < sizeof(node->data) && *ptr; i++) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    return hash + ast_hash(node->left) + ast_hash(node->right) + node->id;
}

/* Parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char src_buf[512];
        char dst_buf[512];
        
        /* Initialize with pattern */
        for (int i = 0; i < sizeof(src_buf); i++) {
            src_buf[i] = (i + thread_id) % 256;
        }
        
        /* Clear destination */
        __builtin_memset(dst_buf, 0, sizeof(dst_buf));
        
        /* Copy with varying lengths */
        int len = (thread_id * 64 + 128) % sizeof(src_buf);
        
        /* Use all three builtins */
        __builtin_memcpy(dst_buf, src_buf, len);
        
        /* Move data around */
        __builtin_memmove(src_buf + 128, src_buf, len / 2);
        
        /* Set part of buffer */
        __builtin_memset(dst_buf + 256, thread_id, 64);
        
        /* Verify by computing checksum */
        unsigned long checksum = 0;
        for (int i = 0; i < sizeof(dst_buf); i++) {
            checksum += dst_buf[i];
        }
        
        #pragma omp critical
        {
            printf("Thread %d checksum: %lu\n", thread_id, checksum);
        }
    }
}

/* Complex memory dispatch with control flow */
static void memory_dispatch(void) {
    char buffer1[1024];
    char buffer2[1024];
    char buffer3[1024];
    
    /* Initialize buffers */
    for (int i = 0; i < sizeof(buffer1); i++) {
        buffer1[i] = i % 256;
        buffer2[i] = 255 - (i % 256);
    }
    
    int stage = 0;
    
    /* Multi-stage memory operations with gotos */
    while (stage < 4) {
        switch (stage) {
            case 0:
                /* Simple copy */
                __builtin_memcpy(buffer3, buffer1, volatile_len % 512);
                stage++;
                break;
                
            case 1:
                /* Copy with overlap using memmove */
                if (volatile_flag) {
                    goto overlap_copy;
                }
                __builtin_memcpy(buffer3 + 256, buffer3, 256);
                stage++;
                break;
                
            case 2:
overlap_copy:
                /* This label is jumped to from case 1 */
                __builtin_memmove(buffer3 + 128, buffer3, 384);
                stage++;
                break;
                
            case 3:
                /* Final memset */
                __builtin_memset(buffer2, 0xAA, sizeof(buffer2));
                stage++;
                break;
        }
    }
    
    /* Verify by comparing */
    int diff_count = 0;
    for (int i = 0; i < sizeof(buffer1); i++) {
        if (buffer1[i] != buffer3[i]) diff_count++;
    }
    printf("Buffer differences: %d\n", diff_count);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Create and process AST */
    ASTNode* root = create_ast(0, 4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    unsigned long hash = ast_hash(root);
    printf("AST hash: %lu\n", hash);
    
    /* Phase 2: Parallel memory operations */
    printf("\nParallel memory operations:\n");
    parallel_mem_ops();
    
    /* Phase 3: Complex dispatch */
    printf("\nComplex memory dispatch:\n");
    memory_dispatch();
    
    /* Phase 4: Additional built-in usage in cleanup */
    char final_buffer[256];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, root->data, 
                    sizeof(final_buffer) < sizeof(root->data) ? 
                    sizeof(final_buffer) : sizeof(root->data));
    
    /* Compute final verification */
    unsigned long final_sum = 0;
    for (int i = 0; i < sizeof(final_buffer); i++) {
        final_sum += final_buffer[i];
    }
    printf("\nFinal verification sum: %lu\n", final_sum);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
