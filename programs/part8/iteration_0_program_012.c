/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Initializing ASAN environment...\n");
    /* Force initialization of memory function caches */
    char buffer1[128];
    char buffer2[128];
    
    /* Use builtins in constructor */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1, buffer2, sizeof(buffer1));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    printf("Cleaning up ASAN environment...\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->size = g_mem_size + depth;
    
    /* Fill data with pattern */
    for (size_t i = 0; i < sizeof(node->data) - 1; i++) {
        node->data[i] = 'A' + (i % 26);
    }
    node->data[sizeof(node->data) - 1] = '\0';
    
    /* Recursive creation */
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int use_memmove = 1;
    
    /* Jump into memory operation block */
    if (use_memmove) goto mem_op_block;
    
    normal_path:
        __builtin_memcpy(dst->data, src->data, src->size);
        return;
    
    mem_op_block:
        /* This tests flow sensitivity */
        __builtin_memmove(dst->data, src->data, 
                         src->size < sizeof(dst->data) ? src->size : sizeof(dst->data));
        
        /* Jump out */
        if (dst->size > 100) goto normal_path;
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        char local_buf1[512];
        char local_buf2[512];
        volatile size_t local_size = g_mem_size + omp_get_thread_num() * 16;
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf1, omp_get_thread_num(), local_size);
        __builtin_memcpy(local_buf2, local_buf1, local_size);
        
        /* Conditional memmove */
        if (omp_get_thread_num() % 2 == 0) {
            __builtin_memmove(local_buf1 + 32, local_buf1, local_size - 32);
        }
        
        #pragma omp barrier
        
        /* Verify pattern */
        for (size_t i = 0; i < local_size; i++) {
            if (local_buf2[i] != (char)omp_get_thread_num()) {
                printf("Thread %d: verification failed at %zu\n", 
                       omp_get_thread_num(), i);
            }
        }
    }
}

/* Multi-stage memory dispatch */
static size_t dispatch_memory_operations(ASTNode* nodes[], size_t count) {
    size_t total_hash = 0;
    
    for (size_t i = 0; i < count; i++) {
        if (!nodes[i]) continue;
        
        /* Create temporary buffer */
        char temp[512];
        volatile size_t op_size = nodes[i]->size % 256;
        
        /* Use all three builtins in sequence */
        __builtin_memset(temp, i, op_size);
        __builtin_memcpy(nodes[i]->data, temp, op_size);
        
        /* Overlapping memmove */
        if (op_size > 64) {
            __builtin_memmove(nodes[i]->data + 32, nodes[i]->data, op_size - 32);
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < op_size; j++) {
            total_hash += (size_t)nodes[i]->data[j];
        }
    }
    
    return total_hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create AST structures */
    ASTNode* ast1 = create_ast(3);
    ASTNode* ast2 = create_ast(3);
    ASTNode* ast3 = create_ast(2);
    
    if (!ast1 || !ast2 || !ast3) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Test goto flow with memory operations */
    process_with_goto(ast1, ast2);
    
    /* Array of nodes for batch processing */
    ASTNode* nodes[] = {ast1, ast2, ast3, NULL, ast1};
    size_t node_count = sizeof(nodes) / sizeof(nodes[0]);
    
    /* Execute parallel section */
    #ifdef _OPENMP
    printf("Running parallel memory operations...\n");
    parallel_memory_ops();
    #endif
    
    /* Dispatch memory operations */
    printf("Dispatching memory operations...\n");
    size_t final_hash = dispatch_memory_operations(nodes, node_count);
    
    /* Additional stress: vary sizes dynamically */
    for (int i = 0; i < 10; i++) {
        char buf1[1024], buf2[1024];
        volatile size_t size = (g_mem_size * (i + 1)) % 1024;
        
        __builtin_memset(buf1, i, size);
        __builtin_memcpy(buf2, buf1, size);
        __builtin_memmove(buf1 + 128, buf1, size > 128 ? size - 128 : size);
        
        /* Update hash */
        for (size_t j = 0; j < size; j++) {
            final_hash += (size_t)buf1[j];
        }
    }
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    free(ast3);
    
    printf("Test completed. Final hash: %zu\n", final_hash);
    printf("Expected ASAN coverage:\n");
    printf("1. Built-in redirection (memcpy/memset/memmove)\n");
    printf("2. Flow-sensitive goto logic\n");
    printf("3. Volatile size control\n");
    printf("4. Recursive AST structures\n");
    printf("5. OpenMP parallel sections\n");
    printf("6. Constructor/destructor hooks\n");
    
    return 0;
}
