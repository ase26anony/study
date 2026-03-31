/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) static void init_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor)) static void cleanup_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(const char *src, int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->len = g_mem_size % 128 + 128;
    node->data = malloc(node->len);
    
    /* Force builtin memcpy with non-foldable size */
    __builtin_memset(node->data, 0, node->len);
    
    /* Copy data using builtin with volatile size */
    size_t copy_len = node->len < strlen(src) ? node->len : strlen(src);
    __builtin_memcpy(node->data, src, copy_len);
    
    /* Create children with goto control flow */
    int use_left = 1;
    
    if (depth > 2) {
        use_left = 0;
        goto skip_left;
    }
    
    node->left = create_ast("LeftChild", depth - 1);
    
skip_left:
    if (use_left) {
        node->right = create_ast("RightChild", depth - 1);
    } else {
        node->right = NULL;
        goto finalize;
    }
    
finalize:
    return node;
}

/* Function with goto jumping into memmove block */
static void test_memmove_goto(char *dest, char *src, size_t n) {
    int do_copy = 1;
    
    if (n == 0) {
        do_copy = 0;
        goto no_copy;
    }
    
copy_block:
    /* This goto target contains the builtin memmove */
    if (g_use_memmove) {
        __builtin_memmove(dest, src, n);
    } else {
        __builtin_memcpy(dest, src, n);
    }
    goto done;
    
no_copy:
    __builtin_memset(dest, 0, n);
    
done:
    return;
    
    /* Jump back into the copy block */
    if (do_copy) {
        do_copy = 0;
        goto copy_block;
    }
}

/* Parallel memory operations */
static void parallel_mem_ops(void) {
    const int num_threads = 4;
    char *buffers[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        size_t size = (tid + 1) * 64;
        
        buffers[tid] = malloc(size);
        
        /* Each thread uses different builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(buffers[tid], tid, size);
                break;
            case 1:
                if (tid > 0) {
                    __builtin_memcpy(buffers[tid], buffers[tid-1], 
                                   size < 64 ? size : 64);
                }
                break;
            case 2:
                if (tid > 0 && tid < num_threads-1) {
                    __builtin_memmove(buffers[tid], buffers[tid+1], 
                                    size < 32 ? size : 32);
                }
                break;
        }
        
        /* Cross-thread memory operation */
        #pragma omp barrier
        
        if (tid == 0) {
            for (int i = 1; i < num_threads; i++) {
                __builtin_memcpy(buffers[0] + i*16, buffers[i], 16);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        free(buffers[i]);
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Basic builtin calls */
    char buffer1[512];
    char buffer2[512];
    
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    
    volatile size_t overlap_size = 128;
    __builtin_memmove(buffer1 + 100, buffer1, overlap_size);
    
    /* Phase 2: AST operations */
    ASTNode *root = create_ast("RootNode", 3);
    
    if (root && root->left && root->right) {
        /* Copy between AST nodes */
        size_t copy_len = root->left->len < root->right->len ? 
                         root->left->len : root->right->len;
        __builtin_memcpy(root->right->data, root->left->data, copy_len);
        
        /* Self-overlapping move */
        __builtin_memmove(root->data + 10, root->data, 50);
    }
    
    /* Phase 3: Goto flow control */
    test_memmove_goto(buffer1, buffer2, 256);
    
    /* Phase 4: OpenMP parallel section */
    parallel_mem_ops();
    
    /* Phase 5: Mixed operations in loop */
    unsigned long hash = 0;
    for (int i = 0; i < 100; i++) {
        char temp[100];
        __builtin_memset(temp, i, sizeof(temp));
        
        if (i % 3 == 0) {
            __builtin_memcpy(buffer1, temp, 50);
        } else if (i % 3 == 1) {
            __builtin_memmove(temp + 10, temp, 40);
        }
        
        for (size_t j = 0; j < sizeof(temp); j++) {
            hash += (unsigned long)temp[j];
        }
    }
    
    printf("Final hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Recursive free implementation omitted for brevity */
    
    return 0;
}
