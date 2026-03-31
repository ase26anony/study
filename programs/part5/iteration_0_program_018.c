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
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    printf("ASAN Test Constructor: Initializing...\n");
    /* Force early initialization of ASAN runtime */
    volatile char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("ASAN Test Destructor: Cleaning up...\n");
}

/* Function with goto control flow around memmove */
static void goto_memmove_test(char* dest, char* src, size_t n) {
    int use_memmove = 1;
    
    if (n > 100) {
        goto skip_memmove;
    }
    
    /* Jump into block with builtin */
    if (use_memmove) {
        goto do_memmove;
    }
    
skip_memmove:
    __builtin_memset(dest, 0, n);
    return;
    
do_memmove:
    /* This should trigger ASAN memmove redirection */
    __builtin_memmove(dest, src, n);
    
    /* Jump out of block */
    if (n < 50) {
        goto skip_memmove;
    }
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast_node(int id, const char* data) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = node->right = NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Use builtin memcpy for data copying */
    size_t len = strlen(data);
    if (len > sizeof(node->data) - 1)
        len = sizeof(node->data) - 1;
    __builtin_memcpy(node->data, data, len);
    node->data[len] = '\0';
    
    return node;
}

static void copy_ast_data(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Copy data between nodes using builtins */
    __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    
    /* Recursive copy */
    if (src->left) {
        if (!dest->left) dest->left = create_ast_node(src->left->id * 2, "");
        copy_ast_data(dest->left, src->left);
    }
    if (src->right) {
        if (!dest->right) dest->right = create_ast_node(src->right->id * 2, "");
        copy_ast_data(dest->right, src->right);
    }
}

/* Function with varied memory operation contexts */
static void memory_operations_test(void) {
    volatile size_t local_size = g_mem_size;
    char buffer1[256];
    char buffer2[256];
    char buffer3[256];
    
    /* Test 1: Basic builtin usage with volatile size */
    __builtin_memset(buffer1, 0xAA, local_size);
    __builtin_memcpy(buffer2, buffer1, local_size);
    
    /* Test 2: Overlapping regions with memmove */
    __builtin_memmove(buffer1 + 32, buffer1, local_size - 32);
    
    /* Test 3: Goto-based control flow */
    goto_memmove_test(buffer3, buffer2, local_size);
    
    /* Test 4: Nested memory operations */
    for (int i = 0; i < 4; i++) {
        size_t chunk = local_size / 4;
        __builtin_memset(buffer1 + i * chunk, i, chunk);
        __builtin_memcpy(buffer2 + i * chunk, buffer1 + i * chunk, chunk);
    }
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_test(void) {
    const int num_threads = 4;
    char shared_buffer[1024];
    char thread_buffers[4][256];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread uses builtins independently */
        __builtin_memset(thread_buffers[tid], tid, sizeof(thread_buffers[tid]));
        
        #pragma omp barrier
        
        /* Copy to shared buffer with synchronization */
        #pragma omp critical
        {
            __builtin_memcpy(shared_buffer + tid * 256, 
                           thread_buffers[tid], 
                           sizeof(thread_buffers[tid]));
        }
        
        /* Move data between thread buffers */
        if (tid > 0) {
            __builtin_memmove(thread_buffers[tid], 
                            thread_buffers[tid - 1], 
                            sizeof(thread_buffers[tid]) / 2);
        }
    }
    
    /* Verify with final builtin */
    char verify_buffer[1024];
    __builtin_memcpy(verify_buffer, shared_buffer, sizeof(shared_buffer));
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Basic memory operations */
    printf("Phase 1: Basic builtin operations\n");
    memory_operations_test();
    
    /* Phase 2: AST structure testing */
    printf("Phase 2: AST structure operations\n");
    ASTNode* root = create_ast_node(1, "Root node data");
    ASTNode* copy = create_ast_node(2, "Copy target");
    
    if (root && copy) {
        copy_ast_data(copy, root);
        
        /* Additional memory operations on AST */
        __builtin_memcpy(root->data + 128, copy->data + 64, 64);
        __builtin_memset(copy->data + 192, 0xFF, 32);
    }
    
    /* Phase 3: OpenMP parallel testing */
    printf("Phase 3: OpenMP parallel operations\n");
    #ifdef _OPENMP
    parallel_memory_test();
    #else
    printf("OpenMP not available, skipping parallel test\n");
    #endif
    
    /* Phase 4: Edge case testing */
    printf("Phase 4: Edge cases\n");
    {
        char small[8];
        char large[512];
        
        /* Zero-length operations */
        __builtin_memcpy(small, large, 0);
        __builtin_memset(small, 0, 0);
        
        /* Full buffer operations */
        __builtin_memset(large, 0xCC, sizeof(large));
        __builtin_memcpy(small, large, sizeof(small));
        __builtin_memmove(large + 256, large, 256);
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    if (root) {
        for (size_t i = 0; i < sizeof(root->data); i++) {
            hash = hash * 31 + (unsigned char)root->data[i];
        }
        free(root);
    }
    if (copy) free(copy);
    
    printf("Test completed. Verification hash: %lu\n", hash);
    printf("Expected: ASAN should have redirected all builtin memory functions\n");
    
    return 0;
}
