/* asan_coverage.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_env(void) {
    printf("Constructor: Initializing ASAN environment\n");
    /* Force early initialization of memory functions */
    char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_env(void) {
    printf("Destructor: Cleaning up ASAN environment\n");
}

/* Function with goto statements for control flow testing */
static void test_goto_memmove(void* dest, const void* src, size_t n) {
    int use_memmove = 1;
    
    if (n == 0) goto skip_copy;
    
    /* Jump into memory operation block */
    goto perform_copy;
    
perform_copy:
    /* Force memmove usage with goto */
    __builtin_memmove(dest, src, n);
    use_memmove = 0;
    
skip_copy:
    /* Jump out of block */
    if (use_memmove) {
        goto end;
    }
    
    /* Additional operation after goto */
    __builtin_memset(dest, 0xFF, n > 16 ? 16 : n);
    
end:
    return;
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_tree(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->id = (*counter)++;
    
    /* Fill data with pattern using memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (depth % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    /* Recursive creation */
    node->left = create_tree(depth - 1, counter);
    node->right = create_tree(depth - 1, counter);
    
    /* Copy between nodes if both children exist */
    if (node->left && node->right) {
        __builtin_memcpy(node->left->data, node->right->data, 32);
    }
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_ops(void) {
    const size_t buffer_size = (size_t)g_mem_size;
    char* buffers[4];
    
    /* Allocate buffers */
    for (int i = 0; i < 4; i++) {
        buffers[i] = (char*)malloc(buffer_size);
        if (!buffers[i]) return;
        
        /* Initialize with memset */
        __builtin_memset(buffers[i], i * 16, buffer_size);
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread performs different memory operations */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            char local_buf[256];
            
            /* Mix of memory operations */
            if (i % 3 == 0) {
                __builtin_memcpy(local_buf, buffers[thread_id % 4], 256);
            } else if (i % 3 == 1) {
                __builtin_memset(local_buf, thread_id, sizeof(local_buf));
            } else {
                /* Use memmove with overlapping regions */
                __builtin_memmove(local_buf + 128, local_buf, 128);
            }
            
            /* Copy back to shared buffer */
            __builtin_memcpy(buffers[(thread_id + i) % 4], 
                           local_buf, 
                           sizeof(local_buf) > buffer_size ? buffer_size : sizeof(local_buf));
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(buffers[i]);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Test 1: Direct built-in calls with volatile sizes */
    volatile size_t copy_size = 256;
    char src[512], dest[512];
    
    __builtin_memset(src, 0x42, sizeof(src));
    __builtin_memcpy(dest, src, copy_size);
    __builtin_memmove(dest + 128, dest, 128);
    
    /* Test 2: Goto-based control flow */
    test_goto_memmove(dest + 256, src + 256, 64);
    
    /* Test 3: Recursive AST operations */
    int counter = 0;
    ASTNode* root = create_tree(3, &counter);
    
    if (root) {
        /* Perform memory operations between tree nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->data, root->left->data, 32);
            __builtin_memmove(root->right->data, root->data, 32);
        }
        
        /* TODO: Add tree cleanup function */
        free(root);
    }
    
    /* Test 4: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Test 5: Variable-sized operations */
    for (volatile int i = 0; i < 10; i++) {
        size_t size = (i * 64) + 32;
        char buf1[size], buf2[size];
        
        __builtin_memset(buf1, i, size);
        __builtin_memcpy(buf2, buf1, size);
        
        /* Force memmove with partial overlap */
        if (i % 2 == 0) {
            __builtin_memmove(buf1 + size/2, buf1, size/2);
        }
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(dest); i++) {
        hash = (hash * 31) + (unsigned char)dest[i];
    }
    
    printf("Test completed. Verification hash: %lu\n", hash);
    printf("Expected to trigger ASAN built-in redirection for:\n");
    printf("  - __builtin_memcpy\n");
    printf("  - __builtin_memset\n");
    printf("  - __builtin_memmove\n");
    
    return 0;
}
