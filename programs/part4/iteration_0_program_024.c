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

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)(i % 256);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill with pattern using volatile length */
    int len = volatile_len % 128;
    for (int i = 0; i < len; i++) {
        node->data[i] = (char)((id + i) % 256);
    }
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = volatile_flag;
        
        if (use_goto) {
            goto create_left;
        } else {
            node->left = create_ast(depth - 1, id * 2);
            node->right = create_ast(depth - 1, id * 2 + 1);
            return node;
        }
        
    create_left:
        node->left = create_ast(depth - 1, id * 2);
        
        /* Jump back to normal flow */
        if (depth > 2) {
            goto create_right;
        }
        
        node->right = NULL;
        return node;
        
    create_right:
        node->right = create_ast(depth - 1, id * 2 + 1);
    }
    
    return node;
}

/* Function with __builtin_memcpy between AST nodes */
static void copy_ast_data(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use volatile to control copy size */
    volatile size_t copy_size = sizeof(dest->data);
    if (volatile_flag) {
        copy_size = volatile_len % sizeof(dest->data);
    }
    
    /* Force __builtin_memcpy call */
    __builtin_memcpy(dest->data, src->data, copy_size);
    
    /* Also test __builtin_memmove with overlapping regions */
    if (dest == src) {
        __builtin_memmove(dest->data + 10, dest->data, 50);
    }
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_ops(void) {
    int buffer_size = 1024;
    char* buffers[8];
    
    /* Allocate buffers */
    for (int i = 0; i < 8; i++) {
        buffers[i] = (char*)malloc(buffer_size);
        if (buffers[i]) {
            __builtin_memset(buffers[i], i, buffer_size);
        }
    }
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs memory operations */
        if (buffers[thread_id % 8]) {
            /* Use __builtin_memcpy with volatile length */
            int len = (volatile_len + thread_id) % buffer_size;
            __builtin_memcpy(buffers[thread_id % 8] + 100, 
                           buffers[(thread_id + 1) % 8], 
                           len > 0 ? len : 64);
            
            /* Use __builtin_memset */
            __builtin_memset(buffers[thread_id % 8] + 200, 
                           thread_id, 
                           (volatile_len * 2) % 128);
            
            /* Use __builtin_memmove for overlapping */
            __builtin_memmove(buffers[thread_id % 8] + 300,
                            buffers[thread_id % 8] + 250,
                            75);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free(buffers[i]);
    }
}

/* Complex token processing with memory builtins */
static unsigned long process_tokens(void) {
    unsigned long hash = 0;
    char local_buffer[512];
    char temp_buffer[512];
    
    /* Initialize with __builtin_memset */
    __builtin_memset(local_buffer, 0, sizeof(local_buffer));
    __builtin_memset(temp_buffer, 0, sizeof(temp_buffer));
    
    /* Process tokens with jumps */
    int i = 0;
    volatile int jump_point = 256;
    
process_loop:
    if (i >= sizeof(token_pool) / 2) goto finish;
    
    /* Copy from token pool using __builtin_memcpy */
    __builtin_memcpy(local_buffer, 
                    token_pool + i, 
                    volatile_len % sizeof(local_buffer));
    
    /* Move data around with __builtin_memmove */
    if (i > jump_point) {
        __builtin_memmove(temp_buffer, 
                         local_buffer + 100, 
                         200);
        goto special_case;
    }
    
    i += 128;
    goto process_loop;
    
special_case:
    /* Additional memory operation */
    __builtin_memset(local_buffer + 300, 0xFF, 100);
    i += 64;
    goto process_loop;
    
finish:
    /* Compute hash from buffers */
    for (int j = 0; j < sizeof(local_buffer); j++) {
        hash = (hash * 31) + (unsigned long)local_buffer[j];
    }
    for (int j = 0; j < sizeof(temp_buffer); j++) {
        hash = (hash * 31) + (unsigned long)temp_buffer[j];
    }
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Create and manipulate AST */
    ASTNode* root = create_ast(4, 1);
    if (root && root->left) {
        copy_ast_data(root, root->left);
        
        /* Additional memory operation */
        if (root->right) {
            __builtin_memcpy(root->right->data, 
                           root->data, 
                           sizeof(root->data) / 2);
        }
    }
    
    /* Phase 2: Parallel memory operations */
    parallel_memory_ops();
    
    /* Phase 3: Token processing */
    unsigned long result_hash = process_tokens();
    
    /* Phase 4: Direct built-in calls in varied contexts */
    char final_buffer[1024];
    char src_buffer[1024];
    
    /* Initialize with pattern */
    for (int i = 0; i < sizeof(src_buffer); i++) {
        src_buffer[i] = (char)(i % 128);
    }
    
    /* Test all three builtins */
    __builtin_memcpy(final_buffer, src_buffer, sizeof(final_buffer));
    __builtin_memset(final_buffer + 512, 0xAA, 256);
    __builtin_memmove(final_buffer + 256, final_buffer, 384);
    
    /* Verify with simple checksum */
    unsigned long checksum = 0;
    for (int i = 0; i < sizeof(final_buffer); i++) {
        checksum += (unsigned long)final_buffer[i];
    }
    
    result_hash ^= checksum;
    
    printf("Test completed. Result hash: 0x%016lx\n", result_hash);
    
    /* Cleanup AST */
    /* Note: In real ASAN, this would detect leaks if present */
    
    return 0;
}
