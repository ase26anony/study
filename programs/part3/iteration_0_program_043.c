/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
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
static int token_hash = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize global tokens with pattern */
    for (int i = 0; i < sizeof(global_tokens); i++) {
        global_tokens[i] = (i % 26) + 'a';
    }
    
    /* Use __builtin_memset in constructor */
    __builtin_memset(global_tokens + 512, 0, 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    /* Verify operations by checking sentinel values */
    if (global_tokens[0] != 'a') {
        fprintf(stderr, "ASAN test: Data corruption detected\n");
    }
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for node initialization */
    size_t copy_len = (size_t)(volatile_len % 128) + 1;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->id = depth;
    node->left = node->right = NULL;
    
    if (depth > 1) {
        char child_data[256];
        
        /* Label for goto testing */
        create_left_child:
        __builtin_memcpy(child_data, node->data, sizeof(child_data));
        child_data[0] = 'L';
        
        node->left = create_ast(depth - 1, child_data);
        
        /* Conditional goto to test flow sensitivity */
        if (volatile_flag) {
            goto create_right_child;
        }
        
        /* Unreachable in normal flow but tests edge cases */
        __builtin_memset(child_data, 0xFF, sizeof(child_data));
        
        create_right_child:
        child_data[0] = 'R';
        __builtin_memmove(child_data, node->data, sizeof(child_data));
        node->right = create_ast(depth - 1, child_data);
    }
    
    return node;
}

/* Process AST with memory operations */
static int process_ast(ASTNode* node, int* hash) {
    if (!node) return 0;
    
    int local_hash = node->id;
    char buffer[512];
    
    /* Complex memory operation pattern */
    if (node->left && node->right) {
        /* Copy between child nodes using builtins */
        size_t copy_size = (size_t)(volatile_len % 256);
        
        /* Force multiple builtin calls */
        __builtin_memcpy(buffer, node->left->data, copy_size);
        __builtin_memmove(node->right->data, buffer, copy_size);
        __builtin_memset(buffer, node->id, sizeof(buffer));
    }
    
    /* Recursive processing */
    local_hash += process_ast(node->left, hash);
    local_hash += process_ast(node->right, hash);
    
    *hash ^= local_hash;
    return local_hash;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

/* Parallel memory dispatch logic */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buffer[1024];
        char shared_buffer[1024];
        
        /* Initialize with pattern */
        for (int i = 0; i < sizeof(local_buffer); i++) {
            local_buffer[i] = (char)((i + thread_id) % 256);
        }
        
        /* OpenMP synchronization point */
        #pragma omp barrier
        
        /* Memory operations in parallel region */
        #pragma omp critical
        {
            /* Use all three builtins with volatile lengths */
            size_t len = (size_t)((volatile_len + thread_id) % 512);
            
            __builtin_memcpy(shared_buffer, local_buffer, len);
            __builtin_memmove(local_buffer, shared_buffer, len);
            __builtin_memset(shared_buffer + len, thread_id, 64);
        }
        
        /* Verify the operations */
        #pragma omp barrier
        #pragma omp atomic
        token_hash += local_buffer[0];
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast(4, global_tokens);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    int ast_hash = 0;
    process_ast(root, &ast_hash);
    printf("AST processing complete. Hash: %d\n", ast_hash);
    
    /* Phase 2: Parallel memory operations */
    parallel_memory_operations();
    printf("Parallel operations complete. Token hash: %d\n", token_hash);
    
    /* Phase 3: Direct builtin stress test */
    char src[256], dst[256];
    volatile int dynamic_len = volatile_len % 128;
    
    /* Test all three builtins in sequence */
    __builtin_memset(src, 0x42, sizeof(src));
    __builtin_memcpy(dst, src, dynamic_len);
    __builtin_memmove(src, dst, dynamic_len);
    
    /* Verify with standard memcpy for comparison */
    memcpy(dst + 128, src, 64);
    
    /* Phase 4: Edge case with goto around memmove */
    int use_goto = volatile_flag;
    char goto_buffer[100];
    
    if (use_goto) {
        goto skip_memmove;
    }
    
    /* This should be skipped via goto */
    __builtin_memmove(goto_buffer, global_tokens, 50);
    
    skip_memmove:
    __builtin_memset(goto_buffer, 0, sizeof(goto_buffer));
    
    /* Cleanup */
    free_ast(root);
    
    /* Final verification */
    int final_result = ast_hash ^ token_hash ^ dst[0] ^ goto_buffer[0];
    printf("Test complete. Final result: %d\n", final_result);
    
    return (final_result != 0) ? 0 : 1;
}
