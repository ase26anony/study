/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_marker = 'X';

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
        token_pool[i] = (i % 26) + 'A';
    }
    
    /* Use __builtin_memset in constructor */
    __builtin_memset(token_pool + 1024, 'Z', 512);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Verify memory was properly handled */
    __builtin_memset(token_pool, 0, 128);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = depth;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memcpy with volatile length */
    int copy_len = volatile_len % 128;
    if (copy_len > 255) copy_len = 255;
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data[copy_len] = '\0';
    
    /* Add volatile marker */
    __builtin_memset(node->data + copy_len - 1, volatile_marker, 1);
    
    if (depth > 1) {
        char child_data[256];
        __builtin_memcpy(child_data, node->data, copy_len);
        child_data[copy_len - 1] = (depth % 26) + 'a';
        
        node->left = create_ast(depth - 1, child_data);
        
        /* Use goto for control flow testing */
        if (depth % 3 == 0) {
            goto skip_right;
        }
        
        __builtin_memmove(child_data, node->data + 32, copy_len - 32);
        node->right = create_ast(depth - 2, child_data);
        
    skip_right:
        /* Jump target */
        if (node->right) {
            /* Copy between nodes */
            __builtin_memcpy(node->right->data + 64, 
                           node->left->data, 
                           volatile_len % 64);
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

/* Calculate hash from AST */
static unsigned long ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    return hash + ast_hash(node->left) + ast_hash(node->right);
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buf[512];
        char src_buf[512];
        
        /* Initialize source buffer */
        for (int i = 0; i < sizeof(src_buf); i++) {
            src_buf[i] = (i + thread_id) % 256;
        }
        
        /* Use all three builtins in parallel region */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        __builtin_memcpy(local_buf + 128, src_buf, 256);
        
        /* Conditional memmove with goto */
        if (thread_id % 2 == 0) {
            goto do_memmove;
        }
        
        __builtin_memset(local_buf + 384, 'P', 64);
        goto skip_memmove;
        
    do_memmove:
        __builtin_memmove(local_buf, local_buf + 64, 128);
        
    skip_memmove:
        /* Verify the operations */
        __builtin_memset(local_buf + 448, 'V', 32);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast(5, token_pool + 512);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Parallel memory operations */
    parallel_memory_ops();
    
    /* Phase 3: Direct built-in calls with volatile control */
    char buffer1[1024];
    char buffer2[1024];
    
    /* Force initialization of all three builtins */
    __builtin_memset(buffer1, 'A', volatile_len);
    __builtin_memcpy(buffer2, buffer1, volatile_len);
    
    /* Use goto to jump into memmove block */
    int use_memmove = 1;
    if (use_memmove) {
        goto perform_memmove;
    }
    
    __builtin_memset(buffer1 + 512, 'B', 256);
    
perform_memmove:
    __builtin_memmove(buffer1 + 256, buffer1, 512);
    
    /* Phase 4: Complex nested operations */
    for (int i = 0; i < 10; i++) {
        char temp[256];
        int len = (volatile_len + i) % 128;
        
        __builtin_memset(temp, i, len);
        __builtin_memcpy(buffer2 + i * 64, temp, len);
        
        if (i % 3 == 0) {
            __builtin_memmove(temp + 32, temp, len - 32);
        }
    }
    
    /* Calculate and print result */
    unsigned long hash_result = ast_hash(root);
    printf("AST Hash: %lu\n", hash_result);
    
    /* Verify token pool */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += token_pool[i];
    }
    printf("Token pool sum: %d\n", sum);
    
    /* Cleanup */
    free_ast(root);
    
    /* Final built-in calls */
    __builtin_memset(buffer1, 0, sizeof(buffer1));
    __builtin_memcpy(buffer2, "TEST_COMPLETE", 13);
    
    printf("Test completed successfully\n");
    return 0;
}
