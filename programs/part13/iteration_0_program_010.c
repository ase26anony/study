/* ISO C99-compliant test program for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode* parent;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Force initialization of ASAN runtime */
    volatile char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char buffer[16];
    __builtin_memset(buffer, 0xFF, sizeof(buffer));
    printf("Destructor: ASAN environment cleaned up\n");
}

/* Function with goto control flow */
static void memory_operations_with_goto(int mode) {
    char src[128];
    char dst[128];
    volatile int condition = 1;
    
    /* Initialize source with pattern */
    for (int i = 0; i < 128; i++) {
        src[i] = (char)(i % 256);
    }
    
    if (mode == 0) {
        goto memcpy_block;
    } else if (mode == 1) {
        goto memset_block;
    } else {
        goto memmove_block;
    }
    
memcpy_block:
    /* Force __builtin_memcpy redirection */
    __builtin_memcpy(dst, src, (size_t)g_mem_size % 128);
    if (condition) {
        goto after_ops;
    }
    
memset_block:
    /* Force __builtin_memset redirection */
    __builtin_memset(dst, 0x42, (size_t)g_mem_size % 128);
    condition = 0;
    goto memmove_block;
    
memmove_block:
    /* Force __builtin_memmove redirection with overlap */
    __builtin_memmove(dst + 32, dst, 64);
    /* Jump back to test flow sensitivity */
    if (!condition) {
        goto memset_block;
    }
    
after_ops:
    /* Verify operations */
    volatile char verify = dst[0];
    (void)verify;
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast_node(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    
    /* Fill data with pattern */
    for (int i = 0; i < 63; i++) {
        node->data[i] = (char)('A' + (depth + i) % 26);
    }
    node->data[63] = '\0';
    
    /* Recursive creation */
    node->left = create_ast_node(depth - 1);
    node->right = create_ast_node(depth - 1);
    
    if (node->left && node->right) {
        /* Copy data between nodes using __builtin_memcpy */
        __builtin_memcpy(node->left->data, node->right->data, 32);
    }
    
    return node;
}

/* Function to copy AST nodes */
static void copy_ast_node(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Direct structure copy using __builtin_memcpy */
    __builtin_memcpy(dest, src, sizeof(ASTNode));
    
    /* Handle pointer fields separately */
    dest->left = NULL;
    dest->right = NULL;
    dest->parent = NULL;
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_operations(void) {
    const int num_arrays = 8;
    char* arrays[num_arrays];
    volatile size_t sizes[num_arrays];
    
    /* Initialize sizes with volatile values */
    for (int i = 0; i < num_arrays; i++) {
        sizes[i] = (g_mem_size * (i + 1)) % 512 + 64;
    }
    
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < num_arrays; i++) {
            arrays[i] = (char*)malloc(sizes[i]);
            if (arrays[i]) {
                /* Use different builtins based on thread ID */
                int thread_specific = omp_get_thread_num() % 3;
                
                switch (thread_specific) {
                    case 0:
                        __builtin_memset(arrays[i], i, sizes[i]);
                        break;
                    case 1:
                        if (i > 0) {
                            __builtin_memcpy(arrays[i], arrays[i-1], 
                                           sizes[i] < sizes[i-1] ? sizes[i] : sizes[i-1]);
                        }
                        break;
                    case 2:
                        /* Create overlapping regions for memmove */
                        if (sizes[i] > 128) {
                            __builtin_memmove(arrays[i] + 64, arrays[i], 64);
                        }
                        break;
                }
            }
        }
        
        /* Barrier to ensure all allocations are done */
        #pragma omp barrier
        
        /* Second phase: mixed operations */
        #pragma omp for
        for (int i = 0; i < num_arrays; i++) {
            if (arrays[i]) {
                /* Alternate between operations */
                if (i % 2 == 0) {
                    __builtin_memset(arrays[i] + 32, 0xFF, 32);
                } else {
                    char temp[64];
                    __builtin_memcpy(temp, arrays[i], 64);
                    __builtin_memcpy(arrays[i], temp, 64);
                }
            }
        }
        
        /* Cleanup in parallel */
        #pragma omp for
        for (int i = 0; i < num_arrays; i++) {
            if (arrays[i]) {
                free(arrays[i]);
            }
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Basic built-in calls */
    char buffer1[256];
    char buffer2[256];
    
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 128, buffer1, 128);
    
    /* Phase 2: Control flow with goto */
    for (int i = 0; i < 3; i++) {
        memory_operations_with_goto(i);
    }
    
    /* Phase 3: Recursive AST operations */
    ASTNode* root = create_ast_node(4);
    if (root) {
        ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
        if (copy) {
            copy_ast_node(copy, root);
            
            /* Verify copy with memcmp */
            int cmp = __builtin_memcmp(root, copy, sizeof(ASTNode) - 3*sizeof(void*));
            printf("AST copy verification: %s\n", cmp == 0 ? "PASS" : "FAIL");
            
            free(copy);
        }
        
        /* Cleanup AST */
        free(root);
    }
    
    /* Phase 4: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 5: Mixed operations in loops */
    volatile char dynamic_buffer[1024];
    for (int i = 0; i < 10; i++) {
        size_t offset = (g_mem_size * i) % 768;
        size_t length = 128 + (i * 32) % 256;
        
        switch (i % 3) {
            case 0:
                __builtin_memset(dynamic_buffer + offset, i, length);
                break;
            case 1:
                __builtin_memcpy(dynamic_buffer + offset + 64, 
                               dynamic_buffer + offset, length - 64);
                break;
            case 2:
                __builtin_memmove(dynamic_buffer + offset, 
                                dynamic_buffer + offset + 32, length - 32);
                break;
        }
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash = (hash * 31) + (unsigned char)buffer1[i];
    }
    
    printf("Test completed. Verification hash: %lu\n", hash);
    printf("All built-in memory functions should have been redirected\n");
    
    return 0;
}
