/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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
    int depth;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 256; /* Force non-zero size */
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Recursive function with memory operations */
static ASTNode* build_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with goto for flow control */
    int copy_success = 0;
    
    copy_start:
    if (base_data) {
        size_t len = strlen(base_data);
        if (len > sizeof(node->data) - 1)
            len = sizeof(node->data) - 1;
        
        __builtin_memcpy(node->data, base_data, len);
        node->data[len] = '\0';
        copy_success = 1;
    }
    
    if (!copy_success) {
        const char* default_data = "default";
        __builtin_memcpy(node->data, default_data, 7);
        node->data[7] = '\0';
    }
    
    node->depth = depth;
    
    /* Build children with goto for unusual flow */
    if (depth > 1) {
        goto build_children;
    } else {
        node->left = node->right = NULL;
        goto node_done;
    }
    
build_children:
    node->left = build_ast(depth - 1, "left");
    node->right = build_ast(depth - 1, "right");
    
node_done:
    return node;
}

/* Function with __builtin_memmove and complex flow */
static void manipulate_ast(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int use_memmove = 1;
    
    /* Jump into memory operation block */
    if (use_memmove) {
        goto do_memmove;
    } else {
        goto do_memcpy;
    }
    
do_memmove:
    /* __builtin_memmove with overlapping regions */
    __builtin_memmove(dst->data + 10, dst->data, 20);
    goto after_move;
    
do_memcpy:
    __builtin_memcpy(dst->data, src->data, sizeof(src->data));
    goto after_move;
    
after_move:
    /* Additional memset */
    __builtin_memset(dst->data + 30, 0xAA, 16);
    
    /* Recursive manipulation */
    if (src->left && dst->left) {
        manipulate_ast(src->left, dst->left);
    }
    if (src->right && dst->right) {
        manipulate_ast(src->right, dst->right);
    }
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_ops(void) {
    const size_t buf_size = g_mem_size;
    char* buffer1 = (char*)malloc(buf_size);
    char* buffer2 = (char*)malloc(buf_size);
    
    if (!buffer1 || !buffer2) {
        free(buffer1);
        free(buffer2);
        return;
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread uses different builtins */
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            switch (i % 3) {
                case 0:
                    __builtin_memset(buffer1 + thread_id * 64, thread_id, 64);
                    break;
                case 1:
                    __builtin_memcpy(buffer2 + thread_id * 64, 
                                   buffer1 + thread_id * 64, 64);
                    break;
                case 2:
                    __builtin_memmove(buffer1 + thread_id * 32,
                                     buffer1 + thread_id * 64, 32);
                    break;
            }
        }
        
        /* Barrier to ensure all memory ops complete */
        #pragma omp barrier
        
        /* Additional memory operation after barrier */
        if (thread_id == 0) {
            __builtin_memset(buffer1, 0, 32);
        }
    }
    
    /* Verify operations with final memcpy */
    __builtin_memcpy(buffer2 + buf_size - 64, buffer1, 64);
    
    free(buffer1);
    free(buffer2);
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* ast1 = build_ast(3, "root_node");
    ASTNode* ast2 = build_ast(3, "copy_target");
    
    if (ast1 && ast2) {
        manipulate_ast(ast1, ast2);
        
        /* Calculate hash/sum of AST data */
        unsigned long hash = 0;
        ASTNode* nodes[2] = {ast1, ast2};
        
        for (int i = 0; i < 2; i++) {
            for (size_t j = 0; j < sizeof(nodes[i]->data); j++) {
                hash += (unsigned long)nodes[i]->data[j];
            }
        }
        
        printf("AST hash sum: %lu\n", hash);
    }
    
    /* Phase 2: Parallel memory operations */
    printf("Running parallel memory operations\n");
    parallel_memory_ops();
    
    /* Phase 3: Direct built-in calls with volatile control */
    volatile char small_buf[128];
    volatile char small_buf2[128];
    
    /* Force all three builtins to be called */
    __builtin_memset((void*)small_buf, 0xCC, sizeof(small_buf));
    __builtin_memcpy((void*)small_buf2, (void*)small_buf, sizeof(small_buf));
    __builtin_memmove((void*)small_buf + 32, (void*)small_buf, 64);
    
    /* Phase 4: Variable-sized operations */
    size_t dynamic_size = g_mem_size % 256;
    if (dynamic_size > 0) {
        char* dyn_buf = (char*)malloc(dynamic_size);
        if (dyn_buf) {
            __builtin_memset(dyn_buf, 0xAA, dynamic_size);
            __builtin_memcpy(dyn_buf + dynamic_size/2, dyn_buf, dynamic_size/2);
            free(dyn_buf);
        }
    }
    
    /* Cleanup */
    /* Helper function to free AST recursively */
    void free_ast(ASTNode* node) {
        if (!node) return;
        free_ast(node->left);
        free_ast(node->right);
        free(node);
    }
    
    free_ast(ast1);
    free_ast(ast2);
    
    printf("ASAN test completed successfully\n");
    return 0;
}
