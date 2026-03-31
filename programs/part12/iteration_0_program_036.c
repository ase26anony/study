/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* AST-like recursive structure */
struct TreeNode {
    char data[256];
    struct TreeNode *left;
    struct TreeNode *right;
    int depth;
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    /* Force early initialization of memory functions */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive tree manipulation with memory operations */
static struct TreeNode* create_tree(int depth) {
    if (depth <= 0) return NULL;
    
    struct TreeNode* node = malloc(sizeof(struct TreeNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(struct TreeNode));
    node->depth = depth;
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[32];
    __builtin_memset(pattern, 'A' + (depth % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(pattern));
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 3) {
        goto skip_left;
    }
    
    create_left = 0;
    
skip_left:
    if (create_left) {
        node->left = create_tree(depth - 1);
    }
    
    node->right = create_tree(depth - 1);
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(struct TreeNode* dest, struct TreeNode* src) {
    int use_memmove = 0;
    
    /* Jump into block containing __builtin_memmove */
    if (src && dest) {
        goto mem_operation;
    }
    
    return;
    
mem_operation:
    {
        /* This block tests flow-sensitivity of ASAN logic */
        char temp[256];
        
        /* Copy using __builtin_memcpy */
        __builtin_memcpy(temp, src->data, g_mem_size);
        
        /* Conditional jump out and back in */
        if (use_memmove) {
            goto use_memmove_op;
        }
        
        /* Use __builtin_memmove for overlapping regions */
        __builtin_memmove(dest->data + 32, dest->data, 128);
        return;
        
    use_memmove_op:
        /* Overlapping copy with __builtin_memmove */
        __builtin_memmove(dest->data, dest->data + 64, 192);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    const int num_arrays = 8;
    char* arrays[num_arrays];
    
    #pragma omp parallel for
    for (int i = 0; i < num_arrays; i++) {
        arrays[i] = malloc(g_mem_size * 4);
        if (arrays[i]) {
            /* Force ASAN instrumentation with all three builtins */
            __builtin_memset(arrays[i], i, g_mem_size * 4);
            
            /* Copy between arrays */
            if (i > 0) {
                __builtin_memcpy(arrays[i], arrays[i-1], g_mem_size);
            }
            
            /* Move within same array */
            __builtin_memmove(arrays[i] + g_mem_size, arrays[i], g_mem_size * 2);
        }
    }
    
    /* Cleanup */
    #pragma omp parallel for
    for (int i = 0; i < num_arrays; i++) {
        free(arrays[i]);
    }
}

/* Complex token processing with memory operations */
static unsigned long process_tokens(const char** tokens, int count) {
    unsigned long hash = 5381;
    char buffer[512];
    
    for (int i = 0; i < count; i++) {
        /* Clear buffer with __builtin_memset */
        __builtin_memset(buffer, 0, sizeof(buffer));
        
        /* Copy token with __builtin_memcpy */
        size_t len = strlen(tokens[i]);
        if (len > sizeof(buffer) - 1) len = sizeof(buffer) - 1;
        __builtin_memcpy(buffer, tokens[i], len);
        
        /* Hash computation */
        for (size_t j = 0; j < len; j++) {
            hash = ((hash << 5) + hash) + buffer[j];
        }
        
        /* Move data around with __builtin_memmove */
        if (i % 3 == 0) {
            __builtin_memmove(buffer + 128, buffer, 256);
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Tree creation and manipulation */
    struct TreeNode* tree = create_tree(4);
    if (tree) {
        /* Process tree with goto jumps */
        process_with_goto(tree, tree->right);
        
        /* Free tree */
        free(tree);
    }
    
    /* Phase 2: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 3: Token processing */
    const char* tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "instrumentation", "redirection", "builtin"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    unsigned long result = process_tokens(tokens, token_count);
    printf("Token processing result: %lu\n", result);
    
    /* Phase 4: Direct built-in calls in varied contexts */
    {
        char src[1024], dst[1024];
        
        /* Pattern initialization */
        for (size_t i = 0; i < sizeof(src); i++) {
            src[i] = (char)(i % 256);
        }
        
        /* Test all three builtins */
        __builtin_memset(dst, 0xCC, sizeof(dst));
        __builtin_memcpy(dst, src, g_mem_size);
        __builtin_memmove(dst + 256, dst, 512);
        
        /* Verify by computing checksum */
        unsigned long checksum = 0;
        for (size_t i = 0; i < sizeof(dst); i++) {
            checksum += (unsigned char)dst[i];
        }
        printf("Memory checksum: %lu\n", checksum);
    }
    
    printf("ASAN test completed successfully\n");
    return 0;
}
