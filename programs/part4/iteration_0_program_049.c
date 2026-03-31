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

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    char buffer[128];
    volatile char* volatile_ptr = buffer;
    
    /* Force builtin initialization in constructor context */
    __builtin_memset(volatile_ptr, 0xAA, 64);
    __builtin_memcpy(buffer + 32, buffer, 32);
    __builtin_memmove(buffer + 16, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    char cleanup_buf[256];
    int len = volatile_len & 0xFF;
    
    /* More builtin calls in destructor context */
    __builtin_memset(cleanup_buf, 0xFF, len);
    __builtin_memcpy(cleanup_buf + 128, cleanup_buf, 64);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    /* Initialize node data with builtins */
    __builtin_memset(node->data, id & 0xFF, sizeof(node->data));
    
    /* Copy between nodes if children exist */
    if (node->left && node->right) {
        __builtin_memcpy(node->data + 128, node->left->data, 128);
        __builtin_memmove(node->right->data + 64, node->data, 64);
    }
    
    return node;
}

/* Function with goto edge cases */
static void test_goto_memmove(void) {
    char src[512], dst[512];
    int i = 0;
    
    /* Initialize source with pattern */
    for (i = 0; i < 512; i++) {
        src[i] = (i * 7) & 0xFF;
    }
    
    /* Jump into memory operation block */
    goto memmove_block;
    
    /* This label should be skipped */
    skip_label:
    i = 100;
    
memmove_block:
    /* Force memmove with goto context */
    __builtin_memmove(dst, src, volatile_len);
    
    /* Jump out and back in */
    if (volatile_flag) {
        goto after_memmove;
    }
    
    /* Unreachable but tests flow */
    __builtin_memset(dst + 256, 0, 64);
    
after_memmove:
    /* Verify the move worked */
    __builtin_memcpy(src + 256, dst, 64);
    
    /* Jump back for another operation */
    if (i < 200) {
        goto memmove_block;
    }
}

/* OpenMP parallel section */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    char thread_buffers[4][1024];
    int results[4] = {0};
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        char* buf = thread_buffers[tid];
        
        /* Each thread uses builtins independently */
        __builtin_memset(buf, tid, 1024);
        
        /* Barrier to ensure all memset complete */
        #pragma omp barrier
        
        /* Copy between thread buffers */
        int src_tid = (tid + 1) % num_threads;
        __builtin_memcpy(buf + 512, thread_buffers[src_tid], 512);
        
        /* Move within buffer */
        __builtin_memmove(buf + 256, buf, 256);
        
        /* Compute checksum */
        for (int i = 0; i < 1024; i++) {
            results[tid] += buf[i];
        }
    }
    
    /* Verify parallel execution */
    int total = 0;
    for (int i = 0; i < num_threads; i++) {
        total += results[i];
    }
    printf("Parallel checksum: %d\n", total);
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Test 1: Basic builtin calls with volatile */
    char buffer1[1024], buffer2[1024];
    int len = volatile_len;
    
    __builtin_memset(buffer1, 0x42, len);
    __builtin_memcpy(buffer2, buffer1, len);
    __builtin_memmove(buffer1 + 128, buffer2, len / 2);
    
    /* Test 2: Goto edge cases */
    test_goto_memmove();
    
    /* Test 3: Recursive AST operations */
    ASTNode* root = create_ast(4, 1);
    
    if (root) {
        /* Complex memory operations on AST */
        __builtin_memcpy(root->right->data, root->left->data, 128);
        __builtin_memmove(root->data, root->right->data + 64, 192);
        
        /* Free AST (triggers destructor-like cleanup) */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    /* Test 4: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Test 5: Variable length builtins */
    for (int i = 0; i < 10; i++) {
        char temp[256];
        int size = (i * 25) + 16;
        __builtin_memset(temp, i, size);
        __builtin_memcpy(buffer1 + (i * 25), temp, size);
        __builtin_memmove(temp + 8, temp, size - 8);
    }
    
    /* Final verification */
    unsigned long hash = 0;
    for (int i = 0; i < 1024; i++) {
        hash = (hash * 31) + buffer1[i];
    }
    printf("Final hash: %lu\n", hash);
    
    return 0;
}
