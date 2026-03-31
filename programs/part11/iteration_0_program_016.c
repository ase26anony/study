/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "test", "data"};
static const int num_tokens = 5;

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
}

__attribute__((destructor)) static void cleanup(void) {
    printf("Destructor: Cleaning up\n");
}

/* Function with goto jumps around memmove */
static void test_goto_memmove(char* dest, const char* src, size_t n) {
    int use_builtin = 1;
    
    if (g_use_memmove) {
        goto use_memmove;
    } else {
        goto skip_memmove;
    }
    
use_memmove:
    /* Force builtin memmove with goto context */
    __builtin_memmove(dest, src, n);
    goto after_memmove;
    
skip_memmove:
    dest[0] = 'X';
    
after_memmove:
    /* Another goto back into memory operation */
    if (use_builtin) {
        goto do_memset;
    }
    return;
    
do_memset:
    __builtin_memset(dest + n/2, 0, n/4);
}

/* Recursive function copying between AST nodes */
static void copy_ast_data(ASTNode* dest, const ASTNode* src, int depth) {
    if (depth <= 0 || !dest || !src) return;
    
    /* Use builtin memcpy between node data */
    __builtin_memcpy(dest->data, src->data, 
                     sizeof(dest->data) < sizeof(src->data) ? 
                     sizeof(dest->data) : sizeof(src->data));
    
    /* Recursive calls */
    if (dest->left && src->left) {
        copy_ast_data(dest->left, src->left, depth - 1);
    }
    if (dest->right && src->right) {
        copy_ast_data(dest->right, src->right, depth - 1);
    }
}

/* Function using all three builtins in sequence */
static void memory_operations_sequence(char* buffer1, char* buffer2, size_t size) {
    volatile size_t local_size = size; /* Prevent constant folding */
    
    /* 1. memset */
    __builtin_memset(buffer1, 0xAA, local_size);
    
    /* 2. memcpy */
    __builtin_memcpy(buffer2, buffer1, local_size);
    
    /* 3. memmove with overlap */
    size_t half = local_size / 2;
    __builtin_memmove(buffer1 + half, buffer1, half);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char private_buf[128];
        char shared_buf[256];
        
        /* Each thread uses builtins */
        __builtin_memset(private_buf, tid, sizeof(private_buf));
        
        #pragma omp critical
        {
            /* memcpy to shared buffer */
            __builtin_memcpy(shared_buf + tid * 32, private_buf, 32);
            
            /* memmove within shared buffer */
            if (tid % 2 == 0) {
                __builtin_memmove(shared_buf, shared_buf + 64, 64);
            }
        }
    }
}

/* Create and manipulate AST */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with token data */
    const char* token = tokens[id % num_tokens];
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, token, strlen(token));
    
    node->id = id;
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Main test execution */
int main(void) {
    printf("Starting ASAN builtin redirection test\n");
    
    /* Phase 1: Basic builtin calls */
    char buffer1[256];
    char buffer2[256];
    volatile size_t op_size = g_mem_size;
    
    memory_operations_sequence(buffer1, buffer2, op_size);
    
    /* Phase 2: Goto with memmove */
    test_goto_memmove(buffer1, buffer2, op_size);
    
    /* Phase 3: AST operations */
    ASTNode* ast1 = create_ast(3, 1);
    ASTNode* ast2 = create_ast(3, 100);
    
    if (ast1 && ast2) {
        copy_ast_data(ast2, ast1, 3);
        
        /* Additional builtin in AST context */
        __builtin_memmove(ast1->data, ast2->data, 128);
    }
    
    /* Phase 4: OpenMP parallel section */
    parallel_memory_ops();
    
    /* Phase 5: Complex sequence mixing all builtins */
    char* dynamic_buf = (char*)malloc(512);
    if (dynamic_buf) {
        /* memset */
        __builtin_memset(dynamic_buf, 0xCC, 512);
        
        /* memcpy from stack to heap */
        __builtin_memcpy(dynamic_buf + 128, buffer1, 128);
        
        /* memmove with overlap in dynamic buffer */
        __builtin_memmove(dynamic_buf, dynamic_buf + 256, 128);
        
        free(dynamic_buf);
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash = hash * 31 + buffer1[i];
    }
    
    printf("Test completed. Hash: %lu\n", hash);
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    return 0;
}
