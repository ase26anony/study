/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_init_value = 0x42;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int depth;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 64 + (rand() % 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->depth = depth;
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, g_init_value, sizeof(node->data));
    
    return node;
}

/* Complex memory operation with goto flow control */
static void complex_memory_operation(char* dest, const char* src, size_t n) {
    int use_memmove = 0;
    
    /* Jump into memory operation block */
    if (n > 32) {
        goto use_memmove_block;
    }
    
    /* Normal memcpy path */
    __builtin_memcpy(dest, src, n);
    goto cleanup;
    
use_memmove_block:
    /* Jump target with memmove */
    use_memmove = 1;
    __builtin_memmove(dest, src, n);
    
cleanup:
    /* Verify operation */
    if (use_memmove) {
        printf("Used memmove for size %zu\n", n);
    }
}

/* Parallel memory dispatch with OpenMP */
static void parallel_memory_dispatch(void) {
    const int num_blocks = 8;
    char* blocks[num_blocks];
    size_t i;
    
    /* Allocate memory blocks */
    for (i = 0; i < num_blocks; i++) {
        blocks[i] = (char*)malloc(g_mem_size);
        if (!blocks[i]) abort();
    }
    
    #pragma omp parallel for
    for (i = 0; i < num_blocks; i++) {
        /* Initialize each block with memset */
        __builtin_memset(blocks[i], i, g_mem_size);
        
        /* Copy between blocks with memcpy */
        size_t next = (i + 1) % num_blocks;
        __builtin_memcpy(blocks[next], blocks[i], g_mem_size / 2);
        
        /* Use memmove for overlapping regions */
        if (i % 2 == 0) {
            __builtin_memmove(blocks[i] + 10, blocks[i], g_mem_size - 20);
        }
    }
    
    /* Cleanup */
    for (i = 0; i < num_blocks; i++) {
        free(blocks[i]);
    }
}

/* Recursive parser with memory operations */
static int recursive_parser(ASTNode* node, char* buffer, size_t buf_size) {
    if (!node || buf_size < 64) return 0;
    
    int result = 0;
    
    /* Copy node data to buffer */
    __builtin_memcpy(buffer, node->data, 64);
    
    /* Process left subtree */
    if (node->left) {
        char temp[64];
        __builtin_memset(temp, 0, sizeof(temp));
        result += recursive_parser(node->left, temp, sizeof(temp));
        
        /* Move result back */
        __builtin_memmove(buffer + 32, temp, 32);
    }
    
    /* Process right subtree */
    if (node->right) {
        char temp[64];
        __builtin_memset(temp, 0, sizeof(temp));
        result += recursive_parser(node->right, temp, sizeof(temp));
        
        /* Overlapping move */
        __builtin_memmove(buffer, temp, 32);
    }
    
    return result + node->depth;
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Basic built-in usage */
    char src[128], dest[128];
    __builtin_memset(src, 0xAA, sizeof(src));
    
    /* Force all three built-ins */
    __builtin_memcpy(dest, src, sizeof(src));
    __builtin_memset(dest + 64, 0xBB, 32);
    __builtin_memmove(dest + 32, dest + 16, 48);
    
    /* Phase 2: Recursive AST operations */
    ASTNode* root = create_ast(3);
    if (root) {
        char buffer[256];
        __builtin_memset(buffer, 0, sizeof(buffer));
        
        int ast_result = recursive_parser(root, buffer, sizeof(buffer));
        printf("AST traversal result: %d\n", ast_result);
        
        /* Free AST */
        /* Note: In real code, implement proper recursive free */
        free(root);
    }
    
    /* Phase 3: Parallel execution */
    printf("Starting parallel memory operations...\n");
    parallel_memory_dispatch();
    
    /* Phase 4: Volatile-controlled operations */
    volatile char* volatile_ptr = (volatile char*)malloc(256);
    if (volatile_ptr) {
        /* These should not be optimized away */
        __builtin_memset((char*)volatile_ptr, g_init_value, 256);
        __builtin_memcpy((char*)volatile_ptr + 128, (char*)volatile_ptr, 128);
        free((void*)volatile_ptr);
    }
    
    /* Phase 5: Edge case with goto */
    char edge_buf1[100], edge_buf2[100];
    __builtin_memset(edge_buf1, 0xCC, sizeof(edge_buf1));
    
    int large_copy = 0;
    if (g_mem_size > 100) {
        large_copy = 1;
        goto large_operation;
    }
    
    /* Small copy */
    __builtin_memcpy(edge_buf2, edge_buf1, 50);
    goto operation_complete;
    
large_operation:
    __builtin_memmove(edge_buf2, edge_buf1, 99);
    
operation_complete:
    printf("Edge case operation %s\n", 
           large_copy ? "used memmove" : "used memcpy");
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(dest); i++) {
        hash = hash * 31 + dest[i];
    }
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    return 0;
}
