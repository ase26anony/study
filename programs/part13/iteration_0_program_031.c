/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    printf("ASAN Test Constructor: Initializing test environment\n");
    /* Force initialization of ASAN runtime */
    volatile char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("ASAN Test Destructor: Cleaning up\n");
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_node(const char* data, size_t len) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy with volatile length */
    volatile size_t copy_len = len < 255 ? len : 255;
    __builtin_memcpy(node->data, data, copy_len);
    node->data[copy_len] = '\0';
    node->size = copy_len;
    node->left = node->right = NULL;
    
    return node;
}

/* Complex memory operation with goto flow control */
static void process_with_goto(ASTNode* dest, ASTNode* src) {
    int state = 0;
    
    /* Jump into memory operation block */
    if (dest && src) {
        goto copy_block;
    }
    
    return;
    
copy_block:
    /* This tests flow-sensitivity of ASAN redirection */
    state = 1;
    volatile size_t move_size = dest->size < src->size ? dest->size : src->size;
    
    if (move_size > 0) {
        /* Force __builtin_memmove redirection */
        __builtin_memmove(dest->data, src->data, move_size);
    }
    
    if (state == 1) {
        goto cleanup;
    }
    
cleanup:
    /* Additional memory operation after goto */
    __builtin_memset(src->data, 0, src->size);
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char buffer1[128];
        char buffer2[128];
        
        /* Initialize with __builtin_memset */
        __builtin_memset(buffer1, thread_id, sizeof(buffer1));
        
        /* Copy between buffers */
        __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
        
        /* Move data around */
        __builtin_memmove(buffer1 + 64, buffer2, 64);
        
        #pragma omp barrier
        
        /* Verify the copy */
        volatile int check = 0;
        for (size_t i = 0; i < 64; i++) {
            if (buffer1[i + 64] != (char)thread_id) {
                check = 1;
            }
        }
        
        #pragma omp critical
        {
            printf("Thread %d: Memory ops completed%s\n", 
                   thread_id, check ? " with errors" : " successfully");
        }
    }
}

/* Multi-stage memory operation sequence */
static size_t complex_memory_sequence(void) {
    size_t hash = 0;
    char* buffers[4];
    
    /* Allocate and initialize buffers */
    for (int i = 0; i < 4; i++) {
        buffers[i] = malloc(g_mem_size);
        if (!buffers[i]) continue;
        
        /* Use volatile to prevent folding */
        volatile char pattern = 'A' + i;
        __builtin_memset(buffers[i], pattern, g_mem_size);
    }
    
    /* Chain memory operations */
    if (buffers[0] && buffers[1]) {
        __builtin_memcpy(buffers[1], buffers[0], g_mem_size / 2);
    }
    
    if (buffers[2] && buffers[3]) {
        __builtin_memmove(buffers[3], buffers[2], g_mem_size / 4);
    }
    
    /* Overlapping memory operation */
    if (buffers[0]) {
        __builtin_memmove(buffers[0] + 16, buffers[0], 32);
    }
    
    /* Compute hash from buffers */
    for (int i = 0; i < 4; i++) {
        if (buffers[i]) {
            for (size_t j = 0; j < g_mem_size; j++) {
                hash = (hash * 31) + buffers[i][j];
            }
            free(buffers[i]);
        }
    }
    
    return hash;
}

/* Recursive tree copy with memory operations */
static void copy_tree(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Copy node data */
    volatile size_t copy_size = dest->size < src->size ? dest->size : src->size;
    __builtin_memcpy(dest->data, src->data, copy_size);
    
    /* Recursive copy of children */
    if (src->left) {
        dest->left = create_node(src->left->data, src->left->size);
        copy_tree(dest->left, src->left);
    }
    
    if (src->right) {
        dest->right = create_node(src->right->data, src->right->size);
        copy_tree(dest->right, src->right);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create AST structure */
    ASTNode* tree1 = create_node("Root node data for testing", 28);
    ASTNode* tree2 = create_node("Another tree for copying", 24);
    
    if (tree1 && tree2) {
        /* Add children */
        tree1->left = create_node("Left child data", 16);
        tree1->right = create_node("Right child with longer text", 28);
        
        /* Test goto flow with memory operations */
        process_with_goto(tree2, tree1);
        
        /* Recursive tree copy */
        ASTNode* tree3 = create_node("", 0);
        copy_tree(tree3, tree1);
        
        /* Cleanup */
        free(tree1->left);
        free(tree1->right);
        free(tree1);
        free(tree2);
        free(tree3);
    }
    
    /* Execute parallel memory operations */
    printf("\nExecuting parallel memory operations:\n");
    parallel_memory_ops();
    
    /* Complex memory sequence */
    printf("\nExecuting complex memory sequence:\n");
    size_t final_hash = complex_memory_sequence();
    printf("Memory operations hash: 0x%08zx\n", final_hash);
    
    /* Final built-in calls to ensure all paths are hit */
    char final_buffer[256];
    volatile size_t final_size = 128;
    
    __builtin_memset(final_buffer, 0xAA, final_size);
    __builtin_memcpy(final_buffer + 64, final_buffer, 64);
    __builtin_memmove(final_buffer, final_buffer + 32, 96);
    
    printf("\nASAN test completed successfully\n");
    return 0;
}
