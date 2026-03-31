/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    uint32_t hash;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    /* Force initialization of ASAN globals */
    char buffer[32];
    volatile char* volatile_ptr = buffer;
    
    /* Use builtins in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 8);
    
    printf("Constructor: ASAN globals initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    printf("Destructor: ASAN cleanup completed\n");
}

/* Complex memory operation with goto flow control */
static void complex_mem_operations(char* dest, const char* src, size_t n) {
    volatile int use_memmove = 1;
    char temp[128];
    
    /* Goto into memory operation block */
    if (n > 64) goto large_copy;
    
    /* Normal path */
    __builtin_memcpy(dest, src, n);
    return;
    
large_copy:
    /* Jump target with memmove */
    __builtin_memmove(temp, src, n);
    
    /* Conditional goto out of block */
    if (use_memmove) {
        __builtin_memmove(dest, temp, n);
        goto cleanup;
    } else {
        __builtin_memcpy(dest, temp, n);
    }
    
cleanup:
    __builtin_memset(temp, 0, sizeof(temp));
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Create pattern in data */
    for (int i = 0; i < 63; i++) {
        node->data[i] = (char)((depth * 31 + i * 17) & 0xFF);
    }
    node->data[63] = '\0';
    
    /* Recursive creation */
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    /* Copy data between nodes if siblings exist */
    if (node->left && node->right) {
        __builtin_memcpy(node->right->data, node->left->data, 32);
        __builtin_memmove(node->left->data + 16, node->right->data, 16);
    }
    
    return node;
}

/* Calculate hash of AST */
static uint32_t hash_ast(const ASTNode* node) {
    if (!node) return 0;
    
    uint32_t hash = 5381;
    volatile size_t len = 64;  /* Prevent constant folding */
    
    /* Process data with builtin memory access */
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + (uint32_t)node->data[i];
    }
    
    /* Recursive hash combination */
    hash ^= hash_ast(node->left);
    hash ^= (hash_ast(node->right) << 1);
    
    return hash;
}

/* Parallel memory dispatch */
static void parallel_memory_dispatch(void) {
    const int num_buffers = 8;
    char* buffers[num_buffers];
    volatile size_t sizes[num_buffers];
    
    /* Initialize sizes with volatile to prevent optimization */
    for (int i = 0; i < num_buffers; i++) {
        sizes[i] = (g_mem_size * (i + 1)) % 128 + 64;
    }
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        /* Each thread allocates and manipulates memory */
        buffers[tid] = (char*)malloc(sizes[tid]);
        if (buffers[tid]) {
            /* Pattern initialization */
            __builtin_memset(buffers[tid], tid, sizes[tid]);
            
            /* Inter-thread memory operations */
            #pragma omp barrier
            
            if (tid > 0) {
                /* Copy from previous thread */
                __builtin_memcpy(buffers[tid], buffers[tid - 1], 
                               sizes[tid] < sizes[tid - 1] ? sizes[tid] : sizes[tid - 1]);
                
                /* Move within buffer */
                __builtin_memmove(buffers[tid] + 16, buffers[tid], sizes[tid] - 16);
            }
            
            /* Clear buffer */
            __builtin_memset(buffers[tid] + sizes[tid] - 16, 0, 16);
        }
        
        #pragma omp barrier
        
        /* Cleanup */
        if (buffers[tid]) {
            free(buffers[tid]);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Basic builtin usage */
    char src[256], dest[256];
    volatile size_t copy_size = g_mem_size;
    
    __builtin_memset(src, 0x42, sizeof(src));
    __builtin_memcpy(dest, src, copy_size);
    __builtin_memmove(dest + 128, dest, 128);
    
    /* Phase 2: Complex flow with gotos */
    complex_mem_operations(dest + 64, src, 96);
    
    /* Phase 3: AST operations */
    ASTNode* root = create_ast(4);
    if (root) {
        uint32_t hash = hash_ast(root);
        printf("AST hash: 0x%08X\n", hash);
        
        /* Additional memory operations on AST */
        if (root->left && root->right) {
            __builtin_memcpy(root->data, root->left->data, 32);
            __builtin_memmove(root->right->data, root->data, 32);
        }
    }
    
    /* Phase 4: OpenMP parallel operations */
    #ifdef _OPENMP
    printf("Running parallel memory dispatch\n");
    parallel_memory_dispatch();
    #endif
    
    /* Phase 5: Final verification */
    volatile int final_check = 0;
    for (int i = 0; i < 256; i++) {
        final_check += dest[i];
    }
    
    printf("Final checksum: %d\n", final_check);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* AST cleanup would go here */
    
    return 0;
}
