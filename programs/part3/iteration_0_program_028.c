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
static char global_tokens[1024];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize global tokens with pattern */
    for (int i = 0; i < sizeof(global_tokens); i++) {
        global_tokens[i] = (i % 26) + 'a';
    }
    
    /* Use builtins in constructor to force early initialization */
    __builtin_memset(global_tokens + 512, 0, 128);
    __builtin_memcpy(global_tokens + 256, global_tokens, 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    /* Final memory operations in destructor */
    volatile char cleanup_buf[256];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy with volatile control */
    int copy_len = volatile_len % 128;
    if (copy_len > 255) copy_len = 255;
    
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->id = depth;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        char new_data[256];
        __builtin_memcpy(new_data, node->data, copy_len);
        
        /* Goto block for testing flow sensitivity */
        if (volatile_flag) {
            goto create_left;
        }
        
        node->left = NULL;
        node->right = NULL;
        return node;
        
    create_left:
        node->left = create_ast(depth - 1, new_data);
        
        /* Another goto for right branch */
        if (depth % 2 == 0) {
            goto create_right;
        } else {
            node->right = create_ast(depth - 2, new_data);
            return node;
        }
        
    create_right:
        __builtin_memmove(new_data, node->data + 64, 64);
        node->right = create_ast(depth - 1, new_data);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Memory operation between AST nodes */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Complex memory copy with builtin */
    __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    
    /* Partial overlap copy with memmove */
    if (dest->data + 128 < dest->data + sizeof(dest->data)) {
        __builtin_memmove(dest->data + 64, dest->data, 128);
    }
}

/* Parallel memory dispatch logic */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[512];
        char shared_buf[1024];
        
        /* Initialize with builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Collective memory operations */
        #pragma omp for
        for (int i = 0; i < 16; i++) {
            int offset = i * 64;
            if (offset + 128 <= sizeof(shared_buf)) {
                __builtin_memcpy(shared_buf + offset, local_buf, 128);
                
                /* Conditional memmove with goto */
                if (i % 3 == 0) {
                    goto do_memmove;
                }
                
                continue;
                
            do_memmove:
                __builtin_memmove(shared_buf + offset + 32, 
                                 shared_buf + offset, 96);
            }
        }
        
        /* Final memset in parallel region */
        #pragma omp single
        {
            __builtin_memset(shared_buf + 768, 0xFF, 256);
        }
    }
}

/* Compute hash of AST structure */
static unsigned long compute_ast_hash(const ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    const char* data = node->data;
    
    /* Hash computation with memory access */
    for (int i = 0; i < 256 && data[i]; i++) {
        hash = ((hash << 5) + hash) + data[i];
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    hash += node->id;
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize and create AST */
    ASTNode* root = create_ast(5, global_tokens);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Memory operations between nodes */
    ASTNode* copy = create_ast(3, "test data for copy");
    if (copy) {
        copy_ast_data(copy, root);
        
        /* Additional builtin usage */
        __builtin_memset(copy->data + 200, 0xAA, 32);
        __builtin_memmove(copy->data, copy->data + 100, 100);
    }
    
    /* Phase 3: Parallel operations */
    printf("Running parallel memory operations...\n");
    parallel_memory_operations();
    
    /* Phase 4: Complex goto patterns with memory ops */
    char buffer_a[1024];
    char buffer_b[1024];
    
    __builtin_memset(buffer_a, 'A', sizeof(buffer_a));
    
    int use_memcpy = volatile_flag;
    
    if (use_memcpy) {
        goto use_builtin_memcpy;
    } else {
        goto use_builtin_memmove;
    }
    
use_builtin_memcpy:
    __builtin_memcpy(buffer_b, buffer_a, 512);
    goto after_copy;
    
use_builtin_memmove:
    __builtin_memmove(buffer_b, buffer_a, 512);
    /* Overlapping region copy */
    __builtin_memmove(buffer_b + 256, buffer_b, 256);
    
after_copy:
    /* Final builtin memset */
    __builtin_memset(buffer_b + 768, 0, 256);
    
    /* Phase 5: Compute and verify results */
    unsigned long hash = compute_ast_hash(root);
    if (copy) {
        hash ^= compute_ast_hash(copy);
    }
    
    /* Add hash from buffers */
    for (int i = 0; i < 256; i++) {
        hash += buffer_b[i * 4];
    }
    
    printf("Final hash: %lu\n", hash);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    /* Note: In real code, you'd want to free the AST nodes */
    
    return 0;
}
