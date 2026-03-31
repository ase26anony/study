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
        token_pool[i] = (i % 26) + 'A';
    }
    /* Use builtin memset in constructor */
    __builtin_memset(token_pool + 2048, 'X', 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Verify memory was properly handled */
    __builtin_memset(token_pool, 0, 256);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = depth;
    
    /* Use builtin memcpy with volatile length */
    int copy_len = volatile_len % 128;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Fill remainder with builtin memset */
    __builtin_memset(node->data + copy_len, depth + '0', 
                     sizeof(node->data) - copy_len);
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        goto create_children;
        
        /* This label tests jump into memory operation block */
        create_children:
        node->left = create_ast(depth - 1, node->data);
        
        /* Use builtin memmove with goto */
        if (node->left) {
            char temp[256];
            __builtin_memcpy(temp, node->left->data, sizeof(temp));
            goto move_data;
            
            move_data:
            __builtin_memmove(node->data + 128, temp, 128);
        }
        
        node->right = create_ast(depth - 2, node->data + 128);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_operations(ASTNode* root) {
    if (!root) return;
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread works on different memory regions */
        char local_buffer[512];
        
        /* Use all three builtins with volatile control */
        if (volatile_flag) {
            __builtin_memset(local_buffer, thread_id + 'A', 256);
            
            /* Conditional goto around memcpy */
            if (thread_id % 2 == 0) {
                goto do_copy;
            } else {
                goto do_move;
            }
            
            do_copy:
            __builtin_memcpy(local_buffer + 256, root->data, 128);
            goto after_ops;
            
            do_move:
            __builtin_memmove(local_buffer + 256, root->data + 128, 128);
            after_ops:;
        }
        
        /* Update shared data with synchronization */
        #pragma omp critical
        {
            int offset = (thread_id * 64) % 2048;
            __builtin_memcpy(token_pool + offset, local_buffer, 64);
        }
    }
}

/* Recursive tree traversal with memory operations */
static unsigned long compute_tree_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char temp[256];
    
    /* Use goto for complex control flow around memmove */
    if (node->id % 3 == 0) {
        goto hash_with_move;
    } else {
        goto hash_direct;
    }
    
    hash_with_move:
    /* Move data before hashing */
    __builtin_memmove(temp, node->data, sizeof(temp));
    for (size_t i = 0; i < sizeof(temp); i++) {
        hash = ((hash << 5) + hash) + temp[i];
    }
    goto hash_children;
    
    hash_direct:
    /* Hash directly */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    hash_children:
    return hash + compute_tree_hash(node->left) + compute_tree_hash(node->right);
}

/* Free tree with memory clearing */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before freeing */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Use goto for cleanup control flow */
    if (node->id > 10) {
        goto deep_clean;
    }
    
    free(node);
    return;
    
    deep_clean:
    /* Additional cleanup for deep nodes */
    char cleanup_buf[256];
    __builtin_memcpy(cleanup_buf, "CLEAN", 6);
    free(node);
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Create and populate AST */
    ASTNode* root = create_ast(5, "ROOT_NODE_DATA");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Parallel memory operations */
    parallel_memory_operations(root);
    
    /* Phase 3: Verify operations with hash */
    unsigned long hash = compute_tree_hash(root);
    printf("Tree hash: %lu\n", hash);
    
    /* Phase 4: Additional builtin tests with goto */
    char test_buf1[1024], test_buf2[1024];
    
    /* Test memcpy with goto */
    __builtin_memset(test_buf1, 'A', sizeof(test_buf1));
    goto copy_test;
    
    copy_test:
    __builtin_memcpy(test_buf2, test_buf1, sizeof(test_buf1));
    
    /* Test memmove with overlapping regions */
    if (volatile_flag) {
        goto move_test;
        
        move_test:
        __builtin_memmove(test_buf1 + 512, test_buf1, 512);
    }
    
    /* Phase 5: Cleanup */
    free_ast(root);
    
    /* Final verification */
    int sum = 0;
    for (int i = 0; i < 1024; i++) {
        sum += test_buf1[i] + test_buf2[i];
    }
    printf("Verification sum: %d\n", sum);
    
    printf("Test completed successfully\n");
    return 0;
}
