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
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 128; /* Force non-constant size */
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Function with goto jumps around memmove */
static void test_goto_memmove(char* dest, char* src, size_t n) {
    int use_builtin = 1;
    
    if (n > 100) {
        goto skip_builtin;
    }
    
    /* This block should be jumped into */
    builtin_block:
    __builtin_memmove(dest, src, n);
    goto after_builtin;
    
    skip_builtin:
    use_builtin = 0;
    memmove(dest, src, n);
    if (use_builtin) {
        goto builtin_block; /* Jump back into builtin block */
    }
    
    after_builtin:
    return;
}

/* Recursive function copying between AST nodes */
static void copy_ast_nodes(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use builtin memcpy for node data */
    __builtin_memcpy(dest->data, src->data, 
                     g_mem_size < sizeof(dest->data) ? g_mem_size : sizeof(dest->data));
    
    /* Recursive copies */
    if (dest->left && src->left) {
        copy_ast_nodes(dest->left, src->left);
    }
    if (dest->right && src->right) {
        copy_ast_nodes(dest->right, src->right);
    }
}

/* Function using all three builtins in different contexts */
static void test_all_builtins(char* buffer1, char* buffer2, size_t size) {
    volatile size_t local_size = size;
    
    /* memset with volatile size */
    __builtin_memset(buffer1, 0xAA, local_size);
    
    /* memcpy with conditional size */
    size_t copy_size = (local_size > 32) ? 32 : local_size;
    __builtin_memcpy(buffer2, buffer1, copy_size);
    
    /* memmove with overlapping regions */
    __builtin_memmove(buffer1 + 16, buffer1, local_size - 16);
    
    /* Test goto pattern */
    test_goto_memmove(buffer2 + 8, buffer2, local_size - 8);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    size_t sizes[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        sizes[tid] = (tid + 1) * g_mem_size;
        buffers[tid] = malloc(sizes[tid]);
        
        if (buffers[tid]) {
            /* Each thread uses different builtins */
            switch (tid % 3) {
                case 0:
                    __builtin_memset(buffers[tid], tid, sizes[tid]);
                    break;
                case 1:
                    if (tid > 0) {
                        __builtin_memcpy(buffers[tid], buffers[tid-1], 
                                        sizes[tid] < sizes[tid-1] ? sizes[tid] : sizes[tid-1]);
                    }
                    break;
                case 2:
                    __builtin_memmove(buffers[tid] + sizes[tid]/2, 
                                     buffers[tid], sizes[tid]/2);
                    break;
            }
            
            #pragma omp barrier
            
            /* Cross-thread memory operation */
            if (tid == 0) {
                for (int i = 1; i < num_threads; i++) {
                    __builtin_memcpy(buffers[0] + i*16, buffers[i], 16);
                }
            }
        }
        
        #pragma omp barrier
        
        if (buffers[tid]) {
            free(buffers[tid]);
        }
    }
}

/* Complex initialization with recursive structures */
static ASTNode* create_ast_tree(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with pattern */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        node->data[i] = (char)((depth * 17 + i) % 256);
    }
    
    node->size = sizeof(node->data);
    node->left = create_ast_tree(depth - 1);
    node->right = create_ast_tree(depth - 1);
    
    return node;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Direct builtin calls */
    char buffer1[256];
    char buffer2[256];
    test_all_builtins(buffer1, buffer2, sizeof(buffer1));
    
    /* Phase 2: Recursive structure operations */
    ASTNode* ast1 = create_ast_tree(3);
    ASTNode* ast2 = create_ast_tree(3);
    
    if (ast1 && ast2) {
        copy_ast_nodes(ast2, ast1);
        
        /* Verify copy with checksum */
        unsigned long sum1 = 0, sum2 = 0;
        for (size_t i = 0; i < sizeof(ast1->data); i++) {
            sum1 += (unsigned char)ast1->data[i];
            sum2 += (unsigned char)ast2->data[i];
        }
        printf("AST checksums: %lu vs %lu (match: %s)\n", 
               sum1, sum2, sum1 == sum2 ? "YES" : "NO");
    }
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Phase 4: Mixed memory operations in loops */
    for (int i = 0; i < 10; i++) {
        volatile int iter = i;
        char temp[128];
        
        if (iter % 2 == 0) {
            __builtin_memset(temp, iter, sizeof(temp));
        } else {
            __builtin_memcpy(temp, buffer1, 
                           sizeof(temp) < sizeof(buffer1) ? sizeof(temp) : sizeof(buffer1));
        }
        
        /* Force memmove every 3rd iteration */
        if (iter % 3 == 0) {
            __builtin_memmove(temp + 32, temp, 64);
        }
    }
    
    /* Cleanup */
    /* Note: In real code, would need recursive free function for AST */
    
    printf("ASAN test completed successfully\n");
    return 0;
}
