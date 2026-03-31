/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 16;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Global token array */
static char tokens[][16] = {
    "memcpy_test", "memset_test", "memmove_test",
    "overlap_test", "recursive_test", "parallel_test"
};

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < 256; i++) {
        volatile_src[i] = (char)(i % 26 + 'A');
    }
    
    /* Force early built-in usage in constructor */
    __builtin_memset(volatile_dest, 0, sizeof(volatile_dest));
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final built-in usage in destructor */
    __builtin_memset(volatile_dest, 0xFF, 16);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use all three built-ins in recursive context */
    __builtin_memset(node, 0, sizeof(ASTNode));
    __builtin_memcpy(node->data, base_data, strlen(base_data) + 1);
    node->type = depth;
    
    /* Create children with overlapping memory operations */
    char child_data[32];
    __builtin_memcpy(child_data, base_data, 16);
    __builtin_memmove(child_data + 8, child_data, 16);  /* Overlap test */
    
    node->left = create_ast(depth - 1, child_data);
    node->right = create_ast(depth - 1, child_data + 8);
    
    return node;
}

/* Function with goto and memory operations */
static void goto_memory_operations(void) {
    char buffer1[64];
    char buffer2[64];
    int use_memmove = 0;
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 'X', sizeof(buffer1));
    __builtin_memset(buffer2, 'Y', sizeof(buffer2));
    
    /* Jump into block with memmove */
    if (volatile_len > 8) {
        goto memmove_block;
    }
    
    /* Normal path */
    __builtin_memcpy(buffer1, buffer2, 16);
    return;
    
memmove_block:
    /* This tests flow-sensitivity of ASAN logic */
    __builtin_memmove(buffer1 + 8, buffer1, 24);  /* Overlapping copy */
    
    /* Jump out and back in */
    if (volatile_len < 32) {
        goto finalize;
    }
    
    __builtin_memcpy(buffer2, buffer1, 16);
    
finalize:
    __builtin_memset(buffer1 + 40, 'Z', 8);
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(void) {
    const int num_workers = 4;
    char shared_buffers[num_workers][128];
    char private_buffers[num_workers][64];
    
    #pragma omp parallel num_threads(num_workers)
    {
        int tid = 0;
        #ifdef _OPENMP
        tid = omp_get_thread_num();
        #endif
        
        /* Each thread uses built-ins independently */
        __builtin_memset(private_buffers[tid], tid + 'A', 64);
        
        /* Copy to shared with overlap */
        __builtin_memcpy(shared_buffers[tid], private_buffers[tid], 32);
        __builtin_memmove(shared_buffers[tid] + 16, 
                         shared_buffers[tid], 32);
        
        /* Barrier to ensure all threads initialized */
        #pragma omp barrier
        
        /* Cross-thread memory operations */
        int src_tid = (tid + 1) % num_workers;
        __builtin_memcpy(shared_buffers[tid] + 48,
                        shared_buffers[src_tid], 32);
    }
}

/* Complex memory dispatch with varied contexts */
static unsigned long execute_memory_dispatch(void) {
    unsigned long hash = 0;
    char work_buffer[512];
    char* dynamic_buffer = NULL;
    
    /* Phase 1: Stack-based operations */
    for (int i = 0; i < 6; i++) {
        size_t len = (i * 8) + 8;
        
        /* Alternate between built-ins */
        switch (i % 3) {
            case 0:
                __builtin_memset(work_buffer + i * 16, i, len);
                break;
            case 1:
                __builtin_memcpy(work_buffer + i * 16, 
                               tokens[i % 6], len);
                break;
            case 2:
                __builtin_memmove(work_buffer + i * 16 + 4,
                                work_buffer + i * 16, len);
                break;
        }
    }
    
    /* Phase 2: Heap-based operations */
    dynamic_buffer = (char*)malloc(256);
    if (dynamic_buffer) {
        /* Initialize with pattern */
        __builtin_memset(dynamic_buffer, 0xAA, 256);
        
        /* Copy from stack to heap with overlap */
        __builtin_memcpy(dynamic_buffer, work_buffer, 128);
        __builtin_memmove(dynamic_buffer + 64, dynamic_buffer, 128);
        
        /* Compute hash */
        for (int i = 0; i < 256; i++) {
            hash = (hash * 31) + dynamic_buffer[i];
        }
        
        free(dynamic_buffer);
    }
    
    /* Phase 3: Volatile memory operations */
    int len = volatile_len;
    if (len > 0 && len < 256) {
        __builtin_memcpy((void*)volatile_dest, 
                        (void*)volatile_src, len);
        __builtin_memset((void*)(volatile_dest + len), 0, 16);
    }
    
    return hash;
}

int main(void) {
    unsigned long final_hash = 0;
    ASTNode* ast_root = NULL;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Step 1: Initialize and create recursive structure */
    ast_root = create_ast(3, "AST_Base_Data");
    
    /* Step 2: Execute goto-based memory operations */
    goto_memory_operations();
    
    /* Step 3: Run parallel memory operations */
    #ifdef _OPENMP
    parallel_memory_operations();
    #endif
    
    /* Step 4: Execute main memory dispatch */
    final_hash = execute_memory_dispatch();
    
    /* Step 5: Process AST with memory operations */
    if (ast_root) {
        char ast_buffer[256];
        ASTNode* nodes[3] = {ast_root, ast_root->left, ast_root->right};
        
        for (int i = 0; i < 3; i++) {
            if (nodes[i]) {
                /* Copy between AST nodes */
                __builtin_memcpy(ast_buffer + i * 32,
                               nodes[i]->data, 32);
                
                /* Move within buffer */
                if (i > 0) {
                    __builtin_memmove(ast_buffer + i * 16,
                                    ast_buffer + (i-1) * 16, 32);
                }
            }
        }
        
        /* Cleanup */
        free(ast_root->left);
        free(ast_root->right);
        free(ast_root);
    }
    
    /* Final verification */
    __builtin_memset(volatile_dest + 240, 0x42, 16);
    
    printf("Test completed. Final hash: %lu\n", final_hash);
    printf("Volatile dest[0]: %c\n", volatile_dest[0]);
    
    return 0;
}
