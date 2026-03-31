/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* AST-like recursive structure */
typedef struct ast_node {
    char data[256];
    struct ast_node* left;
    struct ast_node* right;
    size_t size;
} ast_node_t;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_globals(void) {
    /* Force initialization before main */
    g_mem_size = 128;
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_globals(void) {
    /* Ensure destructor logic is instrumented */
    volatile int dummy = 42;
    __builtin_memset(&dummy, 0, sizeof(dummy));
}

/* Recursive AST manipulation with memory operations */
static ast_node_t* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) pattern[i] = (char)(i + depth);
    __builtin_memcpy(node->data, pattern, 64);
    
    node->size = (size_t)(depth * 16);
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    return node;
}

/* Complex memory operation with goto flow control */
static void complex_mem_operation(char* dest, const char* src, size_t n) {
    volatile int use_memmove = 1;
    char temp[256];
    
    /* Goto into memory operation block */
    if (n > 100) goto large_copy;
    
    /* Small copy path */
    __builtin_memcpy(dest, src, n);
    return;
    
large_copy:
    /* Use __builtin_memmove with goto context */
    __builtin_memmove(temp, src, n);
    
    /* Conditional goto out of block */
    if (use_memmove) goto apply_memmove;
    
    __builtin_memcpy(dest, temp, n);
    return;
    
apply_memmove:
    __builtin_memmove(dest, temp, n);
}

/* OpenMP parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        char buffer1[1024];
        char buffer2[1024];
        volatile size_t local_size = g_mem_size;
        
        /* Initialize with __builtin_memset */
        __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
        
        /* Copy with __builtin_memcpy */
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            size_t offset = (size_t)(i * 256);
            __builtin_memcpy(&buffer2[offset], &buffer1[offset], 256);
        }
        
        /* Move with __builtin_memmove in parallel region */
        #pragma omp single
        {
            __builtin_memmove(buffer1, buffer2, 512);
        }
    }
}

/* Multi-stage initialization with memory builtins */
static uint64_t process_tokens(const char** tokens, int count) {
    uint64_t hash = 0;
    char combined[512];
    size_t offset = 0;
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Use volatile to prevent folding */
        volatile size_t copy_len = len;
        if (copy_len > 0) {
            /* Force __builtin_memcpy call */
            __builtin_memcpy(&combined[offset], tokens[i], copy_len);
            offset += copy_len;
            
            /* Pad with __builtin_memset */
            if (i < count - 1) {
                __builtin_memset(&combined[offset], ' ', 1);
                offset++;
            }
        }
    }
    
    /* Compute simple hash */
    for (size_t i = 0; i < offset; i++) {
        hash = hash * 31 + (uint64_t)combined[i];
    }
    
    return hash;
}

int main(void) {
    /* Initialize complex token array */
    const char* tokens[] = {
        "ASAN", "HWASAN", "MEMCPY", "MEMSET", "MEMMOVE",
        "BUILTIN", "REDIRECTION", "COVERAGE", "TEST"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Create recursive AST structure */
    ast_node_t* ast_root = create_ast(3);
    if (!ast_root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Perform AST memory operations */
    if (ast_root->left && ast_root->right) {
        /* Copy between AST nodes using __builtin_memcpy */
        __builtin_memcpy(ast_root->left->data, 
                        ast_root->right->data, 
                        sizeof(ast_root->left->data));
        
        /* Move data within node using __builtin_memmove */
        __builtin_memmove(&ast_root->data[64], 
                         ast_root->data, 
                         64);
    }
    
    /* Execute parallelized memory operations */
    parallel_mem_ops();
    
    /* Complex memory operation with goto */
    char src[200], dest[200];
    for (int i = 0; i < 200; i++) src[i] = (char)i;
    
    complex_mem_operation(dest, src, 150);
    
    /* Process tokens with memory builtins */
    uint64_t result_hash = process_tokens(tokens, token_count);
    
    /* Verify operations by printing hash */
    printf("Result hash: 0x%016llx\n", (unsigned long long)result_hash);
    
    /* Cleanup */
    /* Note: In real ASAN, this would detect leaks */
    
    return 0;
}
