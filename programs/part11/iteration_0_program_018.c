/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* AST-like recursive structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char global_tokens[1024];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize with memset builtin */
    __builtin_memset(global_tokens, 0, sizeof(global_tokens));
    
    /* Fill with pattern using memcpy builtin */
    char pattern[] = "ASAN_TEST_PATTERN_0123456789_ABCDEF";
    for (int i = 0; i < sizeof(global_tokens); i += sizeof(pattern)) {
        size_t copy_len = (sizeof(global_tokens) - i < sizeof(pattern)) ? 
                         (sizeof(global_tokens) - i) : sizeof(pattern);
        __builtin_memcpy(&global_tokens[i], pattern, copy_len);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    /* Clear sensitive data with memset */
    __builtin_memset(global_tokens, 0xAA, sizeof(global_tokens));
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use memset builtin for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = (*counter)++;
    
    /* Fill data with memcpy using volatile length */
    char buffer[256];
    size_t len = (volatile_len % 128) + 64; /* Non-constant length */
    __builtin_memset(buffer, 'A' + (node->id % 26), len);
    __builtin_memcpy(node->data, buffer, len);
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = volatile_flag;
        
        if (use_goto) {
            /* Jump into block with memmove */
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, counter);
        node->right = create_ast(depth - 1, counter);
        return node;
        
    create_left:
        /* Block entered via goto with memmove operation */
        node->left = create_ast(depth - 1, counter);
        
        /* Use memmove builtin to shift data */
        char temp[128];
        __builtin_memcpy(temp, node->data, 64);
        __builtin_memmove(node->data, node->data + 32, 192);
        __builtin_memcpy(node->data + 192, temp, 64);
        
        node->right = create_ast(depth - 1, counter);
    }
    
    return node;
}

/* Process AST with memory operations */
static int process_ast(ASTNode* node, int* sum) {
    if (!node) return 0;
    
    int local_sum = 0;
    
    /* Complex memory operations between nodes */
    if (node->left && node->right) {
        /* Copy data between child nodes using memcpy */
        size_t copy_len = (volatile_len % 128) + 32;
        __builtin_memcpy(node->left->data + 64, node->right->data, copy_len);
        
        /* Overlap copy with memmove */
        __builtin_memmove(node->right->data + 32, node->right->data, 96);
    }
    
    /* Calculate hash of node data */
    for (int i = 0; i < 256; i++) {
        local_sum += node->data[i];
    }
    
    local_sum += process_ast(node->left, sum);
    local_sum += process_ast(node->right, sum);
    
    *sum += local_sum;
    return local_sum;
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

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf1[512];
        char local_buf2[512];
        
        /* Initialize with memset */
        __builtin_memset(local_buf1, thread_id, sizeof(local_buf1));
        __builtin_memset(local_buf2, 0xFF - thread_id, sizeof(local_buf2));
        
        /* Complex memory pattern with all three builtins */
        for (int i = 0; i < 10; i++) {
            size_t len = (volatile_len + thread_id * 16) % 256 + 64;
            
            /* Mix of memcpy and memmove */
            if (i % 3 == 0) {
                __builtin_memcpy(local_buf1 + i * 32, local_buf2, len);
            } else if (i % 3 == 1) {
                __builtin_memmove(local_buf1 + i * 32, local_buf1, len);
            } else {
                __builtin_memset(local_buf1 + i * 32, i, len);
            }
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Final memory operation */
        #pragma omp master
        {
            __builtin_memcpy(global_tokens + 512, local_buf1, 256);
        }
    }
}

/* Main test driver */
int main(void) {
    int ast_counter = 0;
    int total_sum = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Create and process AST */
    ASTNode* root = create_ast(4, &ast_counter);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    process_ast(root, &total_sum);
    
    /* Phase 2: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 3: Additional built-in usage with goto */
    char buffer1[1024];
    char buffer2[1024];
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memset(buffer2, 0xDD, sizeof(buffer2));
    
    /* Goto-based control flow with memmove */
    int use_complex_path = volatile_flag;
    
    if (use_complex_path) {
        goto complex_memory_path;
    }
    
    /* Simple path */
    __builtin_memcpy(buffer1, buffer2, 256);
    goto cleanup;
    
complex_memory_path:
    /* Complex path with overlapping operations */
    for (int i = 0; i < 8; i++) {
        size_t offset = i * 64;
        size_t len = (volatile_len + i * 8) % 128 + 64;
        
        if (i % 2 == 0) {
            __builtin_memmove(buffer1 + offset, buffer2, len);
        } else {
            __builtin_memcpy(buffer1 + offset, global_tokens + i * 128, len);
        }
    }
    
    /* Final memset */
    __builtin_memset(buffer1 + 512, 0xEE, 256);

cleanup:
    /* Calculate final verification hash */
    int final_hash = 0;
    for (int i = 0; i < sizeof(buffer1); i++) {
        final_hash += buffer1[i];
    }
    final_hash += total_sum;
    
    /* Cleanup */
    free_ast(root);
    
    /* Verify by printing result (prevents dead code elimination) */
    printf("Test completed. Final hash: %d\n", final_hash);
    printf("AST nodes created: %d\n", ast_counter);
    
    return 0;
}
