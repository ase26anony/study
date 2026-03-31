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
static void init_asan_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)(i % 256);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = depth;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy base data using __builtin_memcpy */
    size_t copy_len = strlen(base_data);
    if (copy_len > sizeof(node->data) - 1)
        copy_len = sizeof(node->data) - 1;
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data[copy_len] = '\0';
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_left = volatile_flag;
        
        if (use_left) {
            goto create_left;
        } else {
            goto create_right;
        }
        
    create_left:
        node->left = create_ast(depth - 1, base_data);
        if (depth > 2) goto skip_right;
        
    create_right:
        node->right = create_ast(depth - 2, base_data);
        
    skip_right:
        ; /* Empty statement for label */
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int mode = volatile_flag;
    
    if (mode == 0) {
        goto direct_copy;
    } else if (mode == 1) {
        goto reverse_copy;
    } else {
        goto overlap_copy;
    }
    
direct_copy:
    /* Jump into memmove block */
    if (src && dst) {
        __builtin_memmove(dst->data, src->data, 
                         volatile_len % sizeof(src->data));
    }
    goto finish;
    
reverse_copy:
    /* Alternative path */
    if (src && dst) {
        char temp[256];
        __builtin_memcpy(temp, src->data, sizeof(temp));
        __builtin_memcpy(dst->data, temp, sizeof(temp));
    }
    goto finish;
    
overlap_copy:
    /* Overlapping memory operation */
    if (src) {
        __builtin_memmove(src->data + 10, src->data, 100);
    }
    
finish:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[512];
        char result_buf[512];
        
        /* Initialize with __builtin_memset */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        /* Copy between buffers */
        __builtin_memcpy(result_buf, local_buf, sizeof(local_buf));
        
        /* Move data around */
        __builtin_memmove(local_buf + 128, local_buf, 256);
        
        /* Verify with volatile access */
        volatile char check = result_buf[volatile_len % sizeof(result_buf)];
        (void)check; /* Suppress unused warning */
    }
}

/* Complex memory dispatch with multiple stages */
static unsigned long complex_memory_dispatch(void) {
    ASTNode* nodes[4];
    unsigned long hash = 0;
    
    /* Create AST nodes */
    for (int i = 0; i < 4; i++) {
        char base[32];
        __builtin_memset(base, 'A' + i, sizeof(base));
        base[sizeof(base) - 1] = '\0';
        nodes[i] = create_ast(3 + i, base);
    }
    
    /* Process nodes with goto flow */
    for (int i = 0; i < 3; i++) {
        process_with_goto(nodes[i], nodes[i + 1]);
    }
    
    /* Compute hash from node data */
    for (int i = 0; i < 4; i++) {
        if (nodes[i]) {
            for (int j = 0; j < 64; j++) {
                hash = hash * 31 + (unsigned char)nodes[i]->data[j];
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(nodes[i]);
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Stage 1: Initialize and verify token pool */
    volatile int len = volatile_len;
    __builtin_memset(token_pool + 1024, 0x42, len);
    
    /* Stage 2: Run parallel memory operations */
    parallel_memory_ops();
    
    /* Stage 3: Complex memory dispatch */
    unsigned long result = complex_memory_dispatch();
    
    /* Stage 4: Final memory operations */
    char final_buf[1024];
    __builtin_memcpy(final_buf, token_pool, sizeof(final_buf));
    __builtin_memmove(final_buf + 512, final_buf, 512);
    
    /* Use result to prevent optimization */
    printf("Result hash: %lu\n", result);
    
    /* Verify with volatile */
    volatile char verify = final_buf[volatile_len % sizeof(final_buf)];
    (void)verify;
    
    printf("Test completed successfully\n");
    return 0;
}
