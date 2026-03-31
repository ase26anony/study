/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    int type;
    char data[64];
    struct ast_node* left;
    struct ast_node* right;
    struct ast_node* next;
} ast_node_t;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    
    /* Force initialization with volatile memory operations */
    volatile char buffer[128];
    __builtin_memset((void*)buffer, 0xAA, sizeof(buffer));
    
    /* Use __builtin_memcpy in constructor */
    volatile char src[64];
    for (int i = 0; i < 64; i++) src[i] = i;
    __builtin_memcpy((void*)buffer, (void*)src, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up ASAN test\n");
    
    /* Final memory operation in destructor */
    volatile int final_check[16];
    __builtin_memset((void*)final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ast_node_t* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    node->type = depth;
    
    /* Fill data with pattern */
    for (int i = 0; i < 63; i++) {
        node->data[i] = 'A' + (depth + i) % 26;
    }
    node->data[63] = '\0';
    
    /* Recursive creation */
    node->left = create_ast(depth + 1, max_depth);
    node->right = create_ast(depth + 2, max_depth);
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(ast_node_t* node, int mode) {
    if (!node) return;
    
    volatile char temp[128];
    int state = 0;
    
    /* Jump into memory operation block */
    goto start_block;
    
memory_ops:
    /* This block contains the critical builtins */
    if (mode & 1) {
        __builtin_memcpy(node->data, temp, 64);
    }
    
    if (mode & 2) {
        __builtin_memset(temp, node->type, 64);
    }
    
    if (state == 0) {
        state = 1;
        goto conditional_jump;
    }
    
    return;
    
start_block:
    /* Initialize before jumping to memory ops */
    __builtin_memset(temp, 0, sizeof(temp));
    goto memory_ops;
    
conditional_jump:
    /* Jump back with memmove */
    __builtin_memmove(node->data + 32, node->data, 32);
    goto memory_ops;
}

/* Parallel processing function */
static void parallel_memory_operations(void) {
    volatile size_t local_size = g_mem_size;
    volatile char* buffers[4];
    
    #pragma omp parallel
    {
        int tid = 0;
        #ifdef _OPENMP
        tid = omp_get_thread_num();
        #endif
        
        /* Each thread allocates and operates */
        buffers[tid] = (volatile char*)malloc(local_size);
        if (buffers[tid]) {
            /* Use all three builtins */
            __builtin_memset((void*)buffers[tid], tid, local_size);
            
            if (tid > 0) {
                __builtin_memcpy((void*)buffers[tid], 
                                (void*)buffers[tid-1], 
                                local_size / 2);
            }
            
            /* Circular shift with memmove */
            __builtin_memmove((void*)(buffers[tid] + local_size/2),
                             (void*)buffers[tid],
                             local_size/2);
        }
        
        #pragma omp barrier
        
        /* Cleanup */
        if (buffers[tid]) {
            free((void*)buffers[tid]);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Basic builtin calls */
    volatile char buffer1[256];
    volatile char buffer2[256];
    
    __builtin_memset((void*)buffer1, 0xCC, sizeof(buffer1));
    __builtin_memcpy((void*)buffer2, (void*)buffer1, sizeof(buffer1));
    __builtin_memmove((void*)buffer1, (void*)buffer2, sizeof(buffer1)/2);
    
    /* Phase 2: Recursive AST operations */
    ast_node_t* root = create_ast(0, 4);
    if (root) {
        /* Copy between nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->right->data, 
                           root->left->data, 
                           sizeof(root->left->data));
            
            /* Move within node */
            __builtin_memmove(root->data + 16, 
                            root->data, 
                            32);
        }
        
        /* Goto-based processing */
        process_with_goto(root, 3);
        process_with_goto(root->left, 1);
        process_with_goto(root->right, 2);
    }
    
    /* Phase 3: OpenMP parallel section */
    #ifdef _OPENMP
    printf("Running OpenMP parallel section\n");
    parallel_memory_operations();
    #endif
    
    /* Phase 4: Variable-sized operations */
    for (volatile int i = 1; i <= 8; i++) {
        size_t size = g_mem_size / i;
        volatile char* dyn_buf = (volatile char*)malloc(size);
        if (dyn_buf) {
            __builtin_memset((void*)dyn_buf, i, size);
            
            volatile char* dyn_buf2 = (volatile char*)malloc(size);
            if (dyn_buf2) {
                __builtin_memcpy((void*)dyn_buf2, (void*)dyn_buf, size);
                __builtin_memmove((void*)dyn_buf, (void*)dyn_buf2, size/2);
                free((void*)dyn_buf2);
            }
            free((void*)dyn_buf);
        }
    }
    
    /* Verification phase */
    unsigned long hash = 0;
    if (root) {
        /* Simple hash calculation */
        for (int i = 0; i < 64 && root->data[i]; i++) {
            hash = hash * 31 + root->data[i];
        }
        
        /* Cleanup AST */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    printf("Test completed. Hash: %lu\n", hash);
    printf("=== End of ASAN Test ===\n");
    
    return 0;
}
