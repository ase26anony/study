/* ISO C99-compliant test program for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_tokens[][32] = {
    "memcpy_test", "memset_test", "memmove_test",
    "recursive", "parallel", "asan_redirect"
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char init_buf[128];
    /* Force builtin usage in constructor */
    __builtin_memset(init_buf, 0xAA, sizeof(init_buf));
    __builtin_memcpy(init_buf + 64, init_buf, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    node->left = NULL;
    node->right = NULL;
    
    /* Use volatile to control memory operation sizes */
    volatile size_t copy_size = (depth % 3 + 1) * 16;
    
    /* Initialize node data with builtins */
    __builtin_memset(node->data, node->id, sizeof(node->data));
    
    /* Create left child with goto for flow control */
    int use_goto = (depth % 2 == 0);
    
    if (use_goto) {
        goto create_left;
    }
    
    /* Normal path */
    node->left = create_ast(depth - 1, counter);
    goto skip_goto;
    
create_left:
    /* Goto target with memmove */
    {
        char temp_buf[32];
        __builtin_memcpy(temp_buf, g_tokens[depth % 6], 32);
        __builtin_memmove(node->data, temp_buf, copy_size);
    }
    node->left = create_ast(depth - 1, counter);
    
skip_goto:
    /* Right child without goto */
    node->right = create_ast(depth - 1, counter);
    
    /* Copy between children if both exist */
    if (node->left && node->right) {
        volatile size_t child_copy = sizeof(node->left->data) / 2;
        __builtin_memcpy(node->right->data, node->left->data, child_copy);
    }
    
    return node;
}

/* Process AST with memory operations */
static unsigned long process_ast(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 0;
    volatile char temp[64];
    
    /* Complex memory pattern */
    __builtin_memcpy(temp, node->data, sizeof(node->data));
    
    /* Hash calculation with builtins */
    for (size_t i = 0; i < sizeof(node->data); i += 8) {
        char block[8];
        __builtin_memcpy(block, temp + i, 8);
        for (int j = 0; j < 8; j++) {
            hash = (hash * 31) + block[j];
        }
    }
    
    /* Recursive processing */
    hash += process_ast(node->left);
    hash += process_ast(node->right);
    
    return hash;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear sensitive data before free */
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
        char local_buf[256];
        char shared_buf[256];
        
        /* Initialize with builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* Memory operations in parallel */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            volatile size_t op_size = (i % 32) + 1;
            
            /* Mix of memory builtins */
            if (i % 3 == 0) {
                __builtin_memcpy(shared_buf + i, local_buf, op_size);
            } else if (i % 3 == 1) {
                __builtin_memset(shared_buf + i, i, op_size);
            } else {
                __builtin_memmove(shared_buf + i, shared_buf, op_size);
            }
        }
        
        /* Final memory operation with goto */
        if (thread_id % 2 == 0) {
            goto final_copy;
        }
        
        __builtin_memset(local_buf, 0xFF, 128);
        goto skip_final;
        
    final_copy:
        __builtin_memcpy(local_buf, shared_buf, 128);
        
    skip_final:
        /* Do nothing */
        ;
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive AST operations */
    int counter = 1;
    ASTNode* root = create_ast(4, &counter);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    unsigned long ast_hash = process_ast(root);
    printf("AST hash: %lu\n", ast_hash);
    
    /* Phase 2: Direct builtin calls with volatile control */
    volatile char buffer1[512];
    volatile char buffer2[512];
    volatile size_t sizes[] = {16, 32, 64, 128, 256};
    
    for (int i = 0; i < 5; i++) {
        volatile size_t current_size = sizes[i];
        
        /* Test all three builtins */
        __builtin_memset(buffer1, i + 1, current_size);
        __builtin_memcpy(buffer2, buffer1, current_size);
        
        /* memmove with overlapping regions */
        if (current_size > 32) {
            __builtin_memmove(buffer1 + 16, buffer1, current_size - 16);
        }
    }
    
    /* Phase 3: OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_operations();
    #else
    printf("OpenMP not available, skipping parallel phase\n");
    #endif
    
    /* Phase 4: Complex goto patterns with memmove */
    {
        char goto_buf[256];
        int use_memmove = 1;
        
        __builtin_memset(goto_buf, 0xCC, sizeof(goto_buf));
        
        if (use_memmove) {
            goto do_memmove;
        }
        
        __builtin_memcpy(goto_buf + 128, goto_buf, 128);
        goto skip_memmove;
        
    do_memmove:
        __builtin_memmove(goto_buf + 64, goto_buf, 192);
        
    skip_memmove:
        /* Verify the operation */
        volatile int sum = 0;
        for (size_t i = 0; i < sizeof(goto_buf); i++) {
            sum += goto_buf[i];
        }
        printf("Goto buffer sum: %d\n", sum);
    }
    
    /* Cleanup */
    free_ast(root);
    
    /* Final builtin calls */
    volatile char final_buf[1024];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf + 512, final_buf, 512);
    __builtin_memmove(final_buf, final_buf + 256, 256);
    
    printf("Test completed successfully\n");
    return 0;
}
