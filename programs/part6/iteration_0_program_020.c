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
    g_mem_size = 128;  /* Force non-constant size */
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* prefix) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Build node data with __builtin_memcpy */
    char temp[256];
    __builtin_snprintf(temp, sizeof(temp), "%s-depth-%d", prefix, depth);
    __builtin_memcpy(node->data, temp, __builtin_strlen(temp) + 1);
    
    node->size = sizeof(ASTNode);
    node->left = create_ast(depth - 1, "left");
    node->right = create_ast(depth - 1, "right");
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int state = 0;
    
    /* Jump into memory operation block */
    if (src->size > 0) goto copy_block;
    
    skip_copy:
        state = 1;
        return;
    
    copy_block:
        /* Force __builtin_memmove with goto context */
        __builtin_memmove(dst->data, src->data, 
                         src->size < sizeof(dst->data) ? src->size : sizeof(dst->data));
        
        /* Jump out of block */
        if (state == 0) goto skip_copy;
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        volatile size_t local_size = g_mem_size;
        char* buffer1 = (char*)malloc(local_size);
        char* buffer2 = (char*)malloc(local_size);
        
        if (buffer1 && buffer2) {
            /* Each thread uses builtins independently */
            #pragma omp for
            for (int i = 0; i < 10; i++) {
                /* Pattern 1: memset then memcpy */
                __builtin_memset(buffer1, i, local_size);
                __builtin_memcpy(buffer2, buffer1, local_size);
                
                /* Pattern 2: memmove within same buffer */
                if (local_size > 16) {
                    __builtin_memmove(buffer1 + 8, buffer1, local_size - 8);
                }
            }
            
            /* Verify with volatile read */
            volatile char check = buffer1[0] + buffer2[local_size - 1];
            (void)check;  /* Suppress unused warning */
        }
        
        free(buffer1);
        free(buffer2);
    }
}

/* Multi-stage memory operation sequence */
static size_t execute_memory_sequence(void) {
    size_t hash = 0;
    volatile size_t sizes[] = {32, 64, 128, 256};
    
    for (int i = 0; i < 4; i++) {
        size_t current_size = sizes[i];
        char* arena = (char*)malloc(current_size * 3);
        
        if (arena) {
            char* src = arena;
            char* mid = arena + current_size;
            char* dst = arena + current_size * 2;
            
            /* Chain of memory operations */
            __builtin_memset(src, 0xAA, current_size);
            __builtin_memcpy(mid, src, current_size);
            __builtin_memmove(dst, mid, current_size);
            
            /* Mix with regular memcpy to test both paths */
            memcpy(src, dst, current_size / 2);
            
            /* Compute simple hash */
            for (size_t j = 0; j < current_size; j++) {
                hash += (size_t)src[j] + (size_t)dst[j];
            }
            
            free(arena);
        }
    }
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast(3, "root");
    ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
    
    if (root && copy) {
        /* Test goto-based memory operations */
        process_with_goto(root, copy);
        
        /* Direct memory operations on AST nodes */
        __builtin_memcpy(copy->data, root->data, sizeof(root->data));
        
        if (root->left && copy) {
            __builtin_memmove(&copy->left, &root->left, sizeof(ASTNode*));
        }
    }
    
    /* Phase 2: OpenMP parallel operations */
    printf("Launching parallel memory operations\n");
    parallel_memory_ops();
    
    /* Phase 3: Multi-stage sequence */
    printf("Executing memory operation sequence\n");
    size_t final_hash = execute_memory_sequence();
    printf("Final hash: %zu\n", final_hash);
    
    /* Cleanup */
    free(copy);
    /* Note: AST cleanup omitted for brevity - would need recursive free */
    
    printf("Test completed successfully\n");
    return 0;
}
