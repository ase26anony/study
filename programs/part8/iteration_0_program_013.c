/* asan_coverage.c - Comprehensive test for ASAN/HWASAN built-in redirection */
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
    g_mem_size = 128; /* Force non-constant size */
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
    char buffer[256];
    __builtin_snprintf(buffer, sizeof(buffer), "%s-depth-%d", prefix, depth);
    __builtin_memcpy(node->data, buffer, __builtin_strlen(buffer) + 1);
    
    node->size = sizeof(ASTNode);
    node->left = create_ast(depth - 1, "left");
    node->right = create_ast(depth - 1, "right");
    
    return node;
}

/* Function with goto jumps around memory operations */
static void test_goto_memmove(void* dest, const void* src, size_t n) {
    int use_memmove = 1;
    
    if (n == 0) goto skip_op;
    
    /* Jump into memory operation block */
    goto do_memmove;
    
memmove_block:
    {
        volatile char temp[256];
        /* Force __builtin_memmove usage */
        __builtin_memmove(temp, src, n > 256 ? 256 : n);
        __builtin_memmove(dest, temp, n > 256 ? 256 : n);
    }
    goto after_op;
    
do_memmove:
    if (use_memmove) {
        goto memmove_block;
    }
    
skip_op:
    /* Minimal operation when n == 0 */
    *(char*)dest = '\0';
    
after_op:
    return;
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(void) {
    const int num_arrays = 8;
    char* arrays[num_arrays];
    size_t sizes[num_arrays];
    
    /* Initialize arrays with varying sizes */
    for (int i = 0; i < num_arrays; i++) {
        sizes[i] = (g_mem_size * (i + 1)) % 512 + 64;
        arrays[i] = (char*)malloc(sizes[i]);
        if (arrays[i]) {
            __builtin_memset(arrays[i], i, sizes[i]);
        }
    }
    
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < num_arrays - 1; i++) {
            if (arrays[i] && arrays[i + 1]) {
                size_t copy_size = sizes[i] < sizes[i + 1] ? sizes[i] : sizes[i + 1];
                
                /* Use all three builtins in parallel context */
                __builtin_memcpy(arrays[i + 1], arrays[i], copy_size);
                
                /* Add some overlap for memmove */
                if (copy_size > 16) {
                    __builtin_memmove(arrays[i] + 8, arrays[i], copy_size - 8);
                }
                
                /* Clear part of array */
                __builtin_memset(arrays[i] + copy_size/2, 0, copy_size/4);
            }
        }
    }
    
    /* Cleanup */
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
        size_t len = __builtin_strlen(tokens[i]);
        if (len < sizeof(buffer)) {
            __builtin_memcpy(buffer, tokens[i], len + 1);
            
            /* Process with overlapping memmove */
            if (len > 32) {
                __builtin_memmove(buffer + 16, buffer, len - 16);
            }
        }
        
        /* Update hash */
        for (int j = 0; buffer[j] && j < sizeof(buffer); j++) {
            hash = ((hash << 5) + hash) + buffer[j];
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast(4, "root");
    if (root) {
        /* Copy between AST nodes */
        char temp[256];
        __builtin_memcpy(temp, root->data, sizeof(temp));
        if (root->left) {
            __builtin_memcpy(root->left->data, temp, sizeof(root->left->data));
        }
        
        /* Recursive cleanup */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    /* Phase 2: Goto-based memory operations */
    char src_data[128];
    char dest_data[128];
    
    __builtin_memset(src_data, 'A', sizeof(src_data));
    test_goto_memmove(dest_data, src_data, sizeof(src_data));
    
    /* Verify with memcpy */
    char verify[128];
    __builtin_memcpy(verify, dest_data, sizeof(verify));
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Phase 4: Token processing */
    const char* tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "builtin", "redirection", "coverage", "test"
    };
    
    unsigned long final_hash = process_tokens(tokens, 
        sizeof(tokens)/sizeof(tokens[0]));
    
    printf("Final hash: %lu\n", final_hash);
    printf("Test completed successfully\n");
    
    return 0;
}
