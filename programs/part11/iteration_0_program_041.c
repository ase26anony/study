/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char* data;
    size_t size;
    int id;
} ASTNode;

/* Global token array */
static const char* TOKENS[] = {"memcpy", "memset", "memmove", "asan", "hwasan"};
static const int NUM_TOKENS = sizeof(TOKENS) / sizeof(TOKENS[0]);

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_env(void) {
    /* Force initialization of sanitizer runtime */
    volatile char dummy[64];
    __builtin_memset(dummy, 0, sizeof(dummy));
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_sanitizer_env(void) {
    printf("Destructor: ASAN environment cleanup\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->size = (size_t)(depth * 16 + id);
    node->data = malloc(node->size);
    
    /* Use __builtin_memset with volatile size */
    volatile size_t fill_size = node->size;
    __builtin_memset(node->data, id, fill_size);
    
    /* Create children with goto-controlled flow */
    int use_goto = (id % 3 == 0);
    
    if (use_goto) {
        goto create_left;
    }
    
    node->left = create_ast(depth - 1, id * 2);
    
create_left:
    if (!use_goto) {
        node->right = create_ast(depth - 1, id * 2 + 1);
    } else {
        /* Jump back to normal flow */
        node->right = create_ast(depth - 1, id * 3);
    }
    
    return node;
}

/* Function with __builtin_memmove and goto */
static void process_ast(ASTNode* src, ASTNode* dest) {
    if (!src || !dest) return;
    
    volatile int do_copy = 1;
    
copy_block:
    if (do_copy && src->data && dest->data) {
        size_t copy_size = src->size < dest->size ? src->size : dest->size;
        
        /* Force __builtin_memmove usage */
        __builtin_memmove(dest->data, src->data, copy_size);
        
        /* Modify source after move */
        volatile size_t clear_size = src->size / 2;
        __builtin_memset(src->data + copy_size / 2, 0, clear_size);
    }
    
    if (src->left && dest->left) {
        do_copy = (src->id % 2);
        if (do_copy) goto copy_block;
        process_ast(src->left, dest->left);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_ops = 100;
    char* buffers[3];
    
    /* Allocate buffers */
    for (int i = 0; i < 3; i++) {
        buffers[i] = malloc(g_mem_size);
        __builtin_memset(buffers[i], i + 1, g_mem_size);
    }
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < num_ops; i++) {
            /* Mix different builtins based on thread and iteration */
            switch ((tid + i) % 3) {
                case 0:
                    __builtin_memcpy(buffers[(tid + 1) % 3], 
                                    buffers[tid % 3], 
                                    g_mem_size / 4);
                    break;
                case 1:
                    __builtin_memset(buffers[tid % 3], 
                                    tid, 
                                    g_mem_size / 8);
                    break;
                case 2:
                    __builtin_memmove(buffers[(tid + 2) % 3], 
                                     buffers[tid % 3], 
                                     g_mem_size / 16);
                    break;
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 3; i++) {
        free(buffers[i]);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Initialize AST structures */
    ASTNode* ast1 = create_ast(4, 1);
    ASTNode* ast2 = create_ast(4, 2);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST structures\n");
        return 1;
    }
    
    /* Phase 2: Process AST with memory operations */
    process_ast(ast1, ast2);
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_ops();
    
    /* Phase 4: Direct built-in calls with volatile control */
    volatile char buffer1[256];
    volatile char buffer2[256];
    volatile size_t op_size = 128;
    
    /* Test all three builtins in sequence */
    __builtin_memset(buffer1, 0xAA, op_size);
    __builtin_memcpy(buffer2, buffer1, op_size);
    __builtin_memmove(buffer1 + 64, buffer2, op_size / 2);
    
    /* Phase 5: Complex flow with goto and builtins */
    int counter = 0;
    volatile int max_iters = 5;
    
start_loop:
    if (counter < max_iters) {
        char temp[64];
        size_t iter_size = (counter + 1) * 16;
        
        __builtin_memset(temp, counter, iter_size);
        
        if (counter % 2 == 0) {
            goto skip_memmove;
        }
        
        __builtin_memmove(temp + 16, temp, iter_size - 16);
        
    skip_memmove:
        counter++;
        goto start_loop;
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < NUM_TOKENS; i++) {
        for (const char* p = TOKENS[i]; *p; p++) {
            hash = hash * 31 + *p;
        }
    }
    
    /* Add AST data to hash */
    if (ast1->data && ast2->data) {
        for (size_t i = 0; i < 16 && i < ast1->size && i < ast2->size; i++) {
            hash = hash * 17 + ast1->data[i];
            hash = hash * 19 + ast2->data[i];
        }
    }
    
    printf("Test completed. Verification hash: %lu\n", hash);
    
    /* Cleanup */
    /* Helper function to free AST recursively */
    void free_ast(ASTNode* node) {
        if (!node) return;
        free_ast(node->left);
        free_ast(node->right);
        free(node->data);
        free(node);
    }
    
    free_ast(ast1);
    free_ast(ast2);
    
    return 0;
}
