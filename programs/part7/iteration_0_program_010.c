/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_tokens[][32] = {
    "memcpy_test", "memset_test", "memmove_test",
    "recursive", "parallel", "asan_coverage"
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char buf[128];
    /* Force __builtin_memset in constructor */
    __builtin_memset(buf, 0xAA, sizeof(buf));
    printf("Constructor: Initialized ASAN environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char buf[64];
    /* Force __builtin_memcpy in destructor */
    __builtin_memcpy(buf, g_tokens[0], 32);
    printf("Destructor: Cleaning up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[32];
    __builtin_snprintf(pattern, sizeof(pattern), "AST_%d_%d", depth, id);
    __builtin_memcpy(node->data, pattern, strlen(pattern) + 1);
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = (id % 2 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        return node;
        
    create_children:
        /* Jump target with __builtin_memmove */
        ASTNode* temp = node;
        __builtin_memmove(&node->left, &temp, sizeof(ASTNode*));
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
    }
    
    return node;
}

/* Function with complex control flow and goto */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int do_copy = 1;
    
    if (do_copy) {
        goto perform_copy;
    }
    
    /* Dead code that might still be analyzed */
    __builtin_memset(dst->data, 0, sizeof(dst->data));
    
perform_copy:
    /* Target of goto with __builtin_memcpy */
    __builtin_memcpy(dst->data, src->data, sizeof(src->data));
    
    if (g_use_memmove) {
        goto perform_move;
    }
    
    return;
    
perform_move:
    /* Another goto target with __builtin_memmove */
    char buffer[256];
    __builtin_memmove(buffer, dst->data, sizeof(dst->data));
    __builtin_memmove(dst->data, buffer, sizeof(buffer));
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_operations(void) {
    const int num_threads = 4;
    volatile size_t local_size = g_mem_size;
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        char private_buf[128];
        char shared_buf[128];
        
        /* Each thread uses __builtin_memset */
        __builtin_memset(private_buf, tid, local_size);
        
        #pragma omp barrier
        
        /* Use __builtin_memcpy in parallel region */
        if (tid == 0) {
            __builtin_memcpy(shared_buf, private_buf, local_size);
        }
        
        #pragma omp barrier
        
        /* All threads use __builtin_memmove */
        __builtin_memmove(&private_buf[64], &private_buf[0], 64);
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Initialize AST structures */
    ASTNode* root = create_ast(3, 1);
    ASTNode* copy = create_ast(3, 100);
    
    if (!root || !copy) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Process with goto control flow */
    process_with_goto(root, copy);
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 4: Direct built-in calls with volatile control */
    volatile char buffer1[256], buffer2[256];
    volatile size_t size = g_mem_size;
    
    /* Force all three built-ins to be called */
    __builtin_memset(buffer1, 0xCC, size);
    __builtin_memcpy(buffer2, buffer1, size);
    
    if (g_use_memmove) {
        __builtin_memmove(&buffer1[128], &buffer1[0], 128);
    }
    
    /* Phase 5: Compute verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash = (hash * 31) + buffer1[i];
    }
    
    /* Add AST data to hash */
    if (root && copy) {
        for (size_t i = 0; i < sizeof(root->data); i++) {
            hash = (hash * 31) + root->data[i];
            hash = (hash * 31) + copy->data[i];
        }
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(root);
    free(copy);
    
    return 0;
}
