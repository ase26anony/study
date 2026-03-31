#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* volatile to prevent optimization */
    struct ASTNode* left;
    struct ASTNode* right;
    char padding[32];      /* Ensure size for memcpy operations */
} ASTNode;

/* Global token array */
volatile char global_tokens[256];
volatile int token_index = 0;

/* Constructor function - forces early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset((void*)global_tokens, 'A', sizeof(global_tokens));
    
    /* Force memcpy in constructor */
    char local_buf[64];
    __builtin_memcpy(local_buf, global_tokens, 64);
    
    /* Use volatile to prevent dead code elimination */
    volatile int dummy = local_buf[0];
    (void)dummy;
}

/* Destructor function */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtin memmove in destructor */
    char temp[128];
    __builtin_memcpy(temp, (void*)global_tokens, 128);
    __builtin_memmove((void*)global_tokens + 64, (void*)global_tokens, 64);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, volatile size_t* counter) {
    if (depth <= 0 || *counter >= 100) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = (int)(*counter);
    node->size = sizeof(ASTNode) - 8; /* volatile size */
    
    (*counter)++;
    
    /* Create children with goto control flow */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, counter);
        
        create_left:
        if (use_goto) {
            node->left = create_ast(depth - 1, counter);
        }
        
        node->right = create_ast(depth - 1, counter);
        
        /* Copy data between nodes using builtin memcpy */
        if (node->left && node->right) {
            __builtin_memcpy(&node->left->padding, 
                           &node->right->padding, 
                           sizeof(node->padding));
        }
    }
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_ops(ASTNode* root, int iterations) {
    volatile char local_buffer[512];
    volatile int buffer_size = sizeof(local_buffer);
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < iterations; i++) {
            /* Use all three builtins in parallel region */
            volatile int offset = (thread_id * 64 + i * 8) % buffer_size;
            
            /* memset with volatile size */
            __builtin_memset((void*)(local_buffer + offset), 
                           thread_id + '0', 
                           32);
            
            /* memcpy between global and local */
            __builtin_memcpy((void*)(local_buffer + offset + 32),
                           (void*)global_tokens,
                           32);
            
            /* memmove with overlapping regions */
            if (offset + 64 < buffer_size) {
                __builtin_memmove((void*)(local_buffer + offset + 16),
                                (void*)(local_buffer + offset),
                                48);
            }
            
            /* Jump into block with goto */
            if (i % 7 == 0) {
                goto memmove_block;
            }
            
            continue;
            
            memmove_block:
            /* Additional memmove in goto block */
            if (root && offset < 256) {
                __builtin_memmove((void*)((char*)root + offset),
                                (void*)local_buffer,
                                16);
            }
        }
    }
    
    /* Verify operations weren't optimized away */
    volatile int sum = 0;
    for (int i = 0; i < buffer_size; i++) {
        sum += local_buffer[i];
    }
    (void)sum;
}

/* Calculate hash from AST */
static int calculate_ast_hash(ASTNode* node, int depth) {
    if (!node || depth <= 0) return 0;
    
    int hash = node->type * 31 + node->value;
    
    /* Use builtin memcpy in hash calculation */
    char temp[16];
    __builtin_memcpy(temp, node->padding, 16);
    
    for (int i = 0; i < 16; i++) {
        hash = hash * 17 + temp[i];
    }
    
    /* Recursive calls */
    hash += calculate_ast_hash(node->left, depth - 1);
    hash += calculate_ast_hash(node->right, depth - 1);
    
    return hash;
}

/* Free AST with memory operations */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free using builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

int main(void) {
    volatile size_t counter = 0;
    int final_hash = 0;
    
    printf("Starting ASAN coverage test...\n");
    
    /* Phase 1: Create recursive AST */
    ASTNode* root = create_ast(5, &counter);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    printf("Created AST with %zu nodes\n", counter);
    
    /* Phase 2: Initialize global tokens with builtins */
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            __builtin_memset((void*)global_tokens, 'B', 128);
        }
        
        #pragma omp section
        {
            __builtin_memcpy((void*)(global_tokens + 128),
                           (void*)global_tokens,
                           128);
        }
        
        #pragma omp section
        {
            __builtin_memmove((void*)(global_tokens + 64),
                            (void*)global_tokens,
                            64);
        }
    }
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_ops(root, 100);
    
    /* Phase 4: Calculate and verify result */
    final_hash = calculate_ast_hash(root, 5);
    
    /* Additional builtin usage in main */
    char verification[64];
    __builtin_memset(verification, 0, sizeof(verification));
    __builtin_memcpy(verification, (void*)&final_hash, sizeof(final_hash));
    
    /* Use goto for control flow edge case */
    int use_alternative = (final_hash % 2 == 0);
    
    if (use_alternative) {
        goto alternative_path;
    }
    
    /* Normal path with memmove */
    __builtin_memmove(verification + 32, verification, 32);
    goto cleanup;
    
    alternative_path:
    /* Alternative path with different memcpy */
    __builtin_memcpy(verification + 16, global_tokens, 16);
    
    cleanup:
    /* Cleanup */
    free_ast(root);
    
    printf("Final hash: %d\n", final_hash);
    printf("Test completed successfully\n");
    
    return 0;
}
