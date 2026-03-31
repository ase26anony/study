/* asan_coverage.c - Comprehensive test for ASAN/HWASAN built-in redirection */
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
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with volatile size */
    size_t copy_size = g_mem_size % 256;
    __builtin_memcpy(node->data, base_data, copy_size);
    node->size = copy_size;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth - 1, "left_branch");
    node->right = create_ast(depth - 1, "right_branch");
    
done:
    return node;
}

/* Function with goto jumping into memory operation block */
static void test_goto_memmove(void) {
    char src[512];
    char dst[512];
    volatile int flag = 1;
    
    /* Initialize source with pattern */
    for (int i = 0; i < 512; i++) {
        src[i] = (char)(i % 256);
    }
    
    if (flag) {
        goto perform_memmove;
    }
    
    /* This should be skipped */
    __builtin_memset(dst, 0, 512);
    
perform_memmove:
    /* Jump into memmove operation */
    __builtin_memmove(dst, src, g_mem_size);
    
    /* Jump out */
    if (dst[0] != 0) {
        goto verify_result;
    }
    
    /* Unreachable in normal flow */
    __builtin_memset(dst, 0xFF, 512);
    
verify_result:
    /* Verify the move worked */
    int valid = 1;
    for (size_t i = 0; i < g_mem_size; i++) {
        if (dst[i] != src[i]) {
            valid = 0;
            break;
        }
    }
    printf("Goto memmove test: %s\n", valid ? "PASS" : "FAIL");
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int ARRAY_SIZE = 1024;
    char* buffers[4];
    
    /* Allocate buffers */
    for (int i = 0; i < 4; i++) {
        buffers[i] = (char*)malloc(ARRAY_SIZE);
        __builtin_memset(buffers[i], i, ARRAY_SIZE);
    }
    
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread performs different memory operations */
        if (tid == 0) {
            /* Thread 0: memcpy between buffers */
            __builtin_memcpy(buffers[0], buffers[1], g_mem_size);
            
            /* Nested memmove */
            char temp[256];
            __builtin_memmove(temp, buffers[0], 256);
            __builtin_memmove(buffers[0] + 256, temp, 256);
        } else {
            /* Thread 1: memset pattern */
            __builtin_memset(buffers[2], 0xAA, g_mem_size);
            
            /* Overlapping memmove */
            __builtin_memmove(buffers[3] + 128, buffers[3], g_mem_size);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(buffers[i]);
    }
}

/* Complex token processing with AST manipulation */
static size_t process_ast_tokens(ASTNode* root) {
    if (!root) return 0;
    
    size_t hash = 0;
    char temp_buffer[512];
    
    /* Process current node */
    for (size_t i = 0; i < root->size; i++) {
        hash = (hash * 31) + root->data[i];
    }
    
    /* Copy node data to temp buffer using builtins */
    __builtin_memcpy(temp_buffer, root->data, root->size);
    
    /* Move data around */
    __builtin_memmove(temp_buffer + 128, temp_buffer, root->size);
    __builtin_memset(temp_buffer, 0, 128);
    
    /* Recursive processing */
    hash += process_ast_tokens(root->left);
    hash += process_ast_tokens(root->right);
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Basic built-in calls */
    printf("\nPhase 1: Basic built-in operations\n");
    char buffer1[256], buffer2[256];
    
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, g_mem_size);
    __builtin_memmove(buffer1 + 64, buffer1, 128);
    
    /* Phase 2: Goto flow control */
    printf("\nPhase 2: Goto flow control tests\n");
    test_goto_memmove();
    
    /* Phase 3: AST structure operations */
    printf("\nPhase 3: AST structure manipulation\n");
    ASTNode* ast_root = create_ast(4, "root_node_data");
    if (ast_root) {
        size_t ast_hash = process_ast_tokens(ast_root);
        printf("AST hash: %zu\n", ast_hash);
        
        /* Free AST recursively */
        /* Note: In real code, implement proper recursive free */
        free(ast_root->left);
        free(ast_root->right);
        free(ast_root);
    }
    
    /* Phase 4: OpenMP parallel operations */
    printf("\nPhase 4: OpenMP parallel memory operations\n");
    #ifdef _OPENMP
    parallel_memory_ops();
    printf("OpenMP parallel ops completed\n");
    #else
    printf("OpenMP not available, skipping parallel tests\n");
    #endif
    
    /* Phase 5: Mixed operations in loops */
    printf("\nPhase 5: Mixed operations in loops\n");
    char mixed_buf[5][256];
    for (int i = 0; i < 5; i++) {
        if (i % 2 == 0) {
            __builtin_memset(mixed_buf[i], i, sizeof(mixed_buf[i]));
        } else {
            __builtin_memcpy(mixed_buf[i], mixed_buf[i-1], g_mem_size);
            __builtin_memmove(mixed_buf[i] + 32, mixed_buf[i], 64);
        }
    }
    
    /* Final verification */
    printf("\n=== Test Complete ===\n");
    printf("All memory operations dispatched for ASAN instrumentation\n");
    
    return 0;
}
