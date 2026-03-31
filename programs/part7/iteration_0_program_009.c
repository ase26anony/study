/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer1[128];
    volatile char buffer2[128];
    
    /* Force builtin calls in constructor */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1, buffer2, sizeof(buffer1));
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    printf("Destructor: ASAN cleanup verification\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Create pattern in data */
    char pattern[32];
    __builtin_memset(pattern, 'A' + (id % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(pattern));
    
    /* Recursive creation with goto for flow control */
    int use_goto = (id % 3 == 0);
    
    if (use_goto) {
        goto create_children;
    }
    
    node->left = create_ast(depth - 1, id * 2);
    
create_children:
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    if (!use_goto && node->left) {
        /* Copy data between nodes */
        __builtin_memcpy(node->left->data, node->data, 
                        volatile_len < sizeof(node->data) ? volatile_len : sizeof(node->data));
    }
    
    return node;
}

/* Function with goto jumping into memory block */
static void goto_memory_operations(void) {
    volatile char src[256];
    volatile char dst[256];
    
    /* Initialize source */
    for (int i = 0; i < 256; i++) {
        src[i] = (char)(i % 256);
    }
    
    int mode = volatile_flag % 3;
    
    if (mode == 0) {
        goto do_memcpy;
    } else if (mode == 1) {
        goto do_memset;
    } else {
        goto do_memmove;
    }

do_memcpy:
    __builtin_memcpy(dst, src, volatile_len);
    goto after_ops;
    
do_memset:
    __builtin_memset(dst, 0xCC, volatile_len);
    goto after_ops;
    
do_memmove:
    /* Overlapping regions */
    __builtin_memmove(dst + 32, dst, volatile_len - 32);
    goto after_ops;
    
after_ops:
    /* Verify with another operation */
    __builtin_memcpy(src, dst, volatile_len / 2);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        volatile char thread_buf[512];
        size_t len = volatile_len + tid * 16;
        
        /* Each thread uses different builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(thread_buf, tid, len);
                break;
            case 1:
                __builtin_memcpy(thread_buf + 64, thread_buf, len);
                break;
            case 2:
                __builtin_memmove(thread_buf + 128, thread_buf + 32, len);
                break;
        }
        
        #pragma omp barrier
        
        /* All threads do a final memcpy */
        volatile char shared_buf[1024];
        __builtin_memcpy(shared_buf + tid * 64, thread_buf, 64);
    }
}

/* Complex function mixing all patterns */
static unsigned long process_ast(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 0;
    volatile char temp[256];
    
    /* Copy node data to volatile buffer */
    __builtin_memcpy(temp, node->data, sizeof(node->data));
    
    /* Process with goto */
    if (node->id % 2 == 0) {
        goto process_left;
    }
    
    /* Hash current data */
    for (int i = 0; i < 256; i++) {
        hash = hash * 31 + temp[i];
    }
    
process_left:
    hash += process_ast(node->left);
    
    /* Move data around */
    __builtin_memmove(temp + 128, temp, 128);
    
    hash += process_ast(node->right);
    
    /* Final memset */
    __builtin_memset(temp, 0, sizeof(temp));
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Basic builtin calls */
    volatile char buffer1[1024];
    volatile char buffer2[1024];
    
    __builtin_memset(buffer1, 0x55, volatile_len);
    __builtin_memcpy(buffer2, buffer1, volatile_len);
    __builtin_memmove(buffer1 + 256, buffer1, volatile_len);
    
    /* Phase 2: Goto flow control */
    goto_memory_operations();
    
    /* Phase 3: Recursive AST */
    ASTNode* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 4: OpenMP parallel */
    parallel_memory_ops();
    
    /* Phase 5: Process AST */
    unsigned long result = process_ast(root);
    printf("AST processing result: %lu\n", result);
    
    /* Phase 6: Final builtin calls with varying sizes */
    for (int i = 0; i < 10; i++) {
        size_t len = volatile_len + i * 8;
        __builtin_memset(buffer1, i, len);
        __builtin_memcpy(buffer2, buffer1, len);
        __builtin_memmove(buffer1 + i * 16, buffer2, len / 2);
    }
    
    /* Cleanup */
    /* Note: In real code, would need to free AST recursively */
    
    printf("Test completed successfully\n");
    return 0;
}
