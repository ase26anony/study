/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force early initialization of memory functions */
    char buf1[32], buf2[32];
    __builtin_memset(buf1, 0xAA, sizeof(buf1));
    __builtin_memcpy(buf2, buf1, sizeof(buf1));
    printf("Constructor: Initialized memory buffers\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern */
    for (int i = 0; i < 31; i++) {
        node->data[i] = (char)((id + i) % 256);
    }
    node->data[31] = '\0';
    
    /* Recursive creation with goto for control flow */
    int create_left = 1;
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
done:
    return node;
}

/* Function with goto jumping into memory operation block */
static void goto_memmove_test(char* dest, const char* src, size_t n) {
    int use_memmove = 1;
    
    if (n > 100) {
        goto large_copy;
    }
    
    /* Small copy path */
    __builtin_memcpy(dest, src, n);
    return;
    
large_copy:
    /* Jump into memmove block */
    if (use_memmove) {
        goto perform_memmove;
    }
    
perform_memmove:
    /* This tests flow-sensitivity */
    __builtin_memmove(dest, src, n);
    
    /* Jump out */
    goto cleanup;
    
cleanup:
    return;
}

/* Function that copies between AST nodes */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Copy data between nodes */
    __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    
    /* Recursive copy */
    if (src->left && dest->left) {
        copy_ast_data(dest->left, src->left);
    }
    if (src->right && dest->right) {
        copy_ast_data(dest->right, src->right);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buf[128];
        char src_buf[128];
        
        /* Initialize source with thread-specific pattern */
        for (int i = 0; i < 128; i++) {
            src_buf[i] = (char)((thread_id * 31 + i) % 256);
        }
        
        /* Force builtin calls in parallel region */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        __builtin_memcpy(local_buf + 32, src_buf + 32, 64);
        __builtin_memmove(local_buf + 16, local_buf + 48, 32);
        
        /* Use volatile variables */
        int len = volatile_len;
        if (len > 0 && len < 128) {
            __builtin_memcpy((char*)volatile_dest + thread_id * 16, 
                           src_buf, len);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Basic built-in calls */
    char buffer1[256], buffer2[256];
    
    __builtin_memset(buffer1, 0x55, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 64, buffer1 + 32, 128);
    
    /* Phase 2: Volatile variable usage */
    for (int i = 0; i < sizeof(volatile_src); i++) {
        ((char*)volatile_src)[i] = (char)(i % 256);
    }
    
    int copy_len = volatile_len;
    if (copy_len > 0 && copy_len <= 256) {
        __builtin_memcpy((char*)volatile_dest, (char*)volatile_src, copy_len);
        __builtin_memset((char*)volatile_dest + 128, 0xCC, 64);
    }
    
    /* Phase 3: AST operations */
    ASTNode* ast1 = create_ast(4, 1);
    ASTNode* ast2 = create_ast(4, 100);
    
    if (ast1 && ast2) {
        copy_ast_data(ast2, ast1);
        
        /* Test goto with memmove */
        goto_memmove_test(ast2->data, ast1->data, 32);
    }
    
    /* Phase 4: OpenMP parallel section */
    #ifdef _OPENMP
    printf("Running OpenMP parallel memory operations\n");
    parallel_memory_ops();
    #else
    printf("OpenMP not available, skipping parallel section\n");
    #endif
    
    /* Phase 5: Complex token array */
    const char* tokens[] = {"memcpy", "memset", "memmove", "asan", "hwasan"};
    char token_buffer[512];
    size_t offset = 0;
    
    for (size_t i = 0; i < sizeof(tokens)/sizeof(tokens[0]); i++) {
        size_t len = strlen(tokens[i]);
        __builtin_memcpy(token_buffer + offset, tokens[i], len);
        offset += len;
        token_buffer[offset++] = ' ';
    }
    token_buffer[offset] = '\0';
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < offset; i++) {
        hash = hash * 31 + (unsigned char)token_buffer[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real ASAN, memory leaks would be reported */
    
    return 0;
}
