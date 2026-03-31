/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
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
__attribute__((constructor)) static void init_asan_test(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)(i % 256);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan_test(void) {
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
    
    if (depth > 1) {
        /* Create children with goto for control flow */
        int use_goto = volatile_flag;
        
        if (use_goto) {
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        
    create_left:
        if (use_goto) {
            node->left = create_ast(depth - 1, id * 2);
        }
        
        node->right = create_ast(depth - 1, id * 2 + 1);
        
        /* Copy data between nodes using __builtin_memcpy */
        if (node->left && node->right) {
            int copy_len = (volatile_len % 64) + 1;
            __builtin_memcpy(node->right->data, node->left->data, copy_len);
        }
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void goto_memmove_test(char* dest, char* src, size_t n) {
    int condition = volatile_flag;
    
    if (condition) {
        goto perform_memmove;
    }
    
    /* Some intermediate operations */
    for (int i = 0; i < 10; i++) {
        dest[i] = src[i];
    }
    
perform_memmove:
    /* This is where goto jumps to */
    __builtin_memmove(dest, src, n);
    
    /* Jump back out */
    if (condition) {
        goto after_memmove;
    }
    
after_memmove:
    return;
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buf[256];
        char src_buf[256];
        
        /* Initialize source buffer */
        for (int i = 0; i < 256; i++) {
            src_buf[i] = (char)((thread_id + i) % 256);
        }
        
        /* Use __builtin_memset in parallel region */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        /* Use __builtin_memcpy with volatile length */
        int copy_len = (volatile_len % 128) + 128;
        __builtin_memcpy(local_buf, src_buf, copy_len);
        
        /* Use __builtin_memmove for overlapping regions */
        if (thread_id % 2 == 0) {
            __builtin_memmove(local_buf + 64, local_buf + 32, 128);
        }
        
        /* Store result in token pool (with synchronization) */
        #pragma omp critical
        {
            int offset = (thread_id * 64) % sizeof(token_pool);
            __builtin_memcpy(token_pool + offset, local_buf, 64);
        }
    }
}

/* Complex initialization with multiple memory built-ins */
static void initialize_complex_buffer(char* buffer, size_t size) {
    /* Chain of memory operations */
    __builtin_memset(buffer, 0xAA, size);
    
    char temp[512];
    __builtin_memset(temp, 0x55, sizeof(temp));
    
    /* Overlapping copy with memmove */
    __builtin_memmove(buffer + 128, buffer, 256);
    
    /* Regular copy */
    __builtin_memcpy(buffer + 384, temp, 128);
    
    /* Another memset with different pattern */
    __builtin_memset(buffer + 512, 0xCC, 128);
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: AST operations */
    ASTNode* root = create_ast(4, 1);
    
    /* Phase 2: Goto-based memmove test */
    char buffer1[1024];
    char buffer2[1024];
    
    initialize_complex_buffer(buffer1, sizeof(buffer1));
    initialize_complex_buffer(buffer2, sizeof(buffer2));
    
    goto_memmove_test(buffer1 + 256, buffer2 + 128, 512);
    
    /* Phase 3: Parallel operations */
    parallel_memory_ops();
    
    /* Phase 4: Direct built-in calls with volatile parameters */
    volatile int dynamic_size = volatile_len * 2;
    char* dynamic_buf = malloc(dynamic_size);
    
    if (dynamic_buf) {
        __builtin_memset(dynamic_buf, 0xFF, dynamic_size);
        
        char pattern[128];
        __builtin_memset(pattern, 0x11, sizeof(pattern));
        __builtin_memcpy(dynamic_buf + 64, pattern, sizeof(pattern));
        
        /* Overlapping move */
        __builtin_memmove(dynamic_buf + 32, dynamic_buf + 16, 96);
        
        free(dynamic_buf);
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(token_pool); i++) {
        hash = (hash * 31) + (unsigned long)token_pool[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real code, you'd want to properly free the AST tree */
    
    return 0;
}
