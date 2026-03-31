/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
    unsigned char padding[16]; /* Ensure size for redzone testing */
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_hooks(void) {
    /* Force early initialization of ASAN runtime */
    volatile char buffer[64];
    __builtin_memset(buffer, 0, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char dummy[32];
    __builtin_memset(dummy, 0xFF, sizeof(dummy));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Volatile-controlled size */
    size_t data_len = g_mem_size % 128 + 64;
    node->len = data_len;
    node->data = malloc(data_len);
    
    if (node->data) {
        /* Use all three builtins with volatile control */
        volatile size_t copy_len = data_len / 2;
        
        /* memset pattern */
        __builtin_memset(node->data, depth, data_len);
        
        /* Conditional goto to test flow sensitivity */
        if (depth % 3 == 0) {
            goto copy_block;
        }
        
        /* memcpy between buffers */
        char *temp = malloc(data_len);
        if (temp) {
            __builtin_memcpy(temp, node->data, copy_len);
            free(temp);
        }
        
        copy_block:
        /* memmove with overlapping regions */
        if (data_len > 32) {
            __builtin_memmove(node->data + 16, node->data, data_len - 16);
        }
    }
    
    /* Recursive creation */
    node->left = create_ast(depth - 1, base_data);
    node->right = create_ast(depth - 1, base_data);
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void goto_mem_operations(char *buf1, char *buf2, size_t len) {
    volatile int use_memmove = 1;
    
    if (use_memmove) {
        goto do_memmove;
    }
    
    __builtin_memset(buf1, 0xAA, len);
    return;
    
do_memmove:
    /* Jump target containing memmove */
    __builtin_memmove(buf1, buf2, len);
    
    /* Jump back out */
    if (len > 100) {
        goto finish;
    }
    
    __builtin_memcpy(buf2, buf1, len/2);
    
finish:
    return;
}

/* OpenMP parallel section with memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[256];
        char shared_buf[256];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Critical section with memcpy */
        #pragma omp critical
        {
            __builtin_memcpy(shared_buf, local_buf, sizeof(local_buf)/4);
        }
        
        /* Another barrier before memmove */
        #pragma omp barrier
        
        if (tid % 2 == 0) {
            __builtin_memmove(local_buf + 128, local_buf, 128);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Initialize and test basic builtins */
    char buffer1[512];
    char buffer2[512];
    
    /* Force all three builtins in main context */
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1)/2);
    __builtin_memmove(buffer1 + 256, buffer1, 256);
    
    /* Phase 2: Test goto flow sensitivity */
    goto_mem_operations(buffer1, buffer2, g_mem_size % 384 + 128);
    
    /* Phase 3: Create recursive AST structure */
    ASTNode *root = create_ast(4, "test_data");
    
    /* Phase 4: Parallel memory operations */
    parallel_mem_ops();
    
    /* Phase 5: Complex memory pattern between AST nodes */
    if (root && root->left && root->right) {
        size_t copy_size = root->len;
        if (root->left->len < copy_size) copy_size = root->left->len;
        if (root->right->len < copy_size) copy_size = root->right->len;
        
        /* Copy data between tree nodes */
        __builtin_memcpy(root->left->data, root->data, copy_size);
        __builtin_memmove(root->right->data, root->left->data, copy_size);
        
        /* Final memset to verify */
        __builtin_memset(root->data, 0xFF, root->len);
    }
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash = (hash * 31) + buffer1[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed\n");
    
    /* Cleanup */
    /* Note: In real usage, would need proper AST cleanup */
    
    return 0;
}
