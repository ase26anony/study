/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    uint64_t hash;
} ASTNode;

/* Global token array */
static char global_tokens[4096];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize global tokens with pattern */
    for (int i = 0; i < sizeof(global_tokens); i++) {
        global_tokens[i] = (char)((i * 13) & 0xFF);
    }
    token_index = 512;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Clear sensitive data using builtin memset */
    __builtin_memset(global_tokens, 0, sizeof(global_tokens));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy to copy data */
    size_t copy_len = (size_t)(depth * 16) % 256;
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data[copy_len] = '\0';
    
    /* Initialize children with goto for flow control */
    node->left = NULL;
    node->right = NULL;
    
    if (depth > 1) {
        /* Jump label for goto */
        create_left_child:
        node->left = create_ast(depth - 1, node->data);
        
        /* Conditional goto to test flow sensitivity */
        if (volatile_flag) {
            goto create_right_child;
        }
        
        /* This block should be jumped over */
        __builtin_memset(node->data + 128, 0xAA, 32);
        
        create_right_child:
        node->right = create_ast(depth - 1, node->data + 64);
        
        /* Copy between nodes using builtin memmove */
        if (node->left && node->right) {
            size_t move_len = volatile_len % 128;
            __builtin_memmove(node->right->data, node->left->data, move_len);
        }
    }
    
    /* Compute hash using volatile length */
    node->hash = 0;
    for (size_t i = 0; i < (volatile_len % 64); i++) {
        node->hash = (node->hash * 31) + (uint64_t)node->data[i];
    }
    
    return node;
}

/* Parallel memory dispatch logic */
static uint64_t parallel_memory_ops(ASTNode* root) {
    uint64_t total_hash = 0;
    
    #pragma omp parallel reduction(+:total_hash)
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread works on different memory regions */
        char local_buffer[1024];
        size_t offset = (thread_id * 256) % sizeof(global_tokens);
        
        /* Copy from global to local using builtin memcpy */
        __builtin_memcpy(local_buffer, global_tokens + offset, 
                        volatile_len % 256);
        
        /* Process buffer with builtin memset */
        __builtin_memset(local_buffer + 128, thread_id, 64);
        
        /* Conditional memmove based on thread ID */
        if (thread_id % 2 == 0) {
            __builtin_memmove(local_buffer, local_buffer + 64, 128);
        }
        
        /* Update hash */
        for (int i = 0; i < 256; i++) {
            total_hash += (uint64_t)local_buffer[i];
        }
        
        /* Copy back using another builtin memcpy */
        __builtin_memcpy(global_tokens + offset, local_buffer, 128);
    }
    
    return total_hash;
}

/* Tree traversal with memory operations */
static uint64_t traverse_and_process(ASTNode* node, int level) {
    if (!node) return 0;
    
    uint64_t hash = node->hash;
    
    /* Process current node data */
    char temp[256];
    
    /* Jump label for goto into memmove block */
    if (level % 3 == 0) {
        goto do_memmove;
    }
    
    /* Regular memcpy path */
    __builtin_memcpy(temp, node->data, volatile_len % 128);
    
    /* Jump target */
    do_memmove:
    if (node->left && node->right) {
        /* Move data between children */
        size_t move_size = (volatile_len * level) % 128;
        __builtin_memmove(node->right->data + 32, 
                         node->left->data + 16, 
                         move_size);
    }
    
    /* Recursive traversal */
    hash += traverse_and_process(node->left, level + 1);
    hash += traverse_and_process(node->right, level + 1);
    
    return hash;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear node data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test...\n");
    
    /* Phase 1: Create and populate AST */
    ASTNode* root = create_ast(5, "BaseASTDataForMemoryOperations");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Parallel memory operations */
    uint64_t parallel_hash = parallel_memory_ops(root);
    printf("Parallel operations hash: %llu\n", 
           (unsigned long long)parallel_hash);
    
    /* Phase 3: Tree traversal with memory ops */
    uint64_t traversal_hash = traverse_and_process(root, 0);
    printf("Traversal hash: %llu\n", 
           (unsigned long long)traversal_hash);
    
    /* Phase 4: Additional builtin calls in complex flow */
    char final_buffer[2048];
    
    /* Chain of memory operations */
    __builtin_memset(final_buffer, 0xCC, sizeof(final_buffer));
    
    /* Goto jumping over memset */
    if (volatile_flag > 0) {
        goto skip_memset;
    }
    
    __builtin_memset(final_buffer + 512, 0xDD, 256);
    
    skip_memset:
    __builtin_memcpy(final_buffer + 1024, global_tokens, 512);
    
    /* Conditional memmove with goto */
    if (token_index > 256) {
        goto do_final_memmove;
    }
    
    __builtin_memcpy(final_buffer, final_buffer + 768, 256);
    
    do_final_memmove:
    __builtin_memmove(final_buffer + 256, final_buffer + 1536, 256);
    
    /* Compute final checksum */
    uint64_t final_sum = 0;
    for (int i = 0; i < 2048; i++) {
        final_sum += (uint64_t)final_buffer[i];
    }
    printf("Final buffer checksum: %llu\n", 
           (unsigned long long)final_sum);
    
    /* Cleanup */
    free_ast(root);
    
    /* Final builtin calls */
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    
    printf("Test completed successfully.\n");
    return 0;
}
