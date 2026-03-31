/* ISO C99-compliant test program for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_token_array[1024];
static volatile size_t g_token_pos = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token array with pattern */
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 13) % 256);
    }
    g_init_flag = 1;
    printf("Constructor: Initialized ASAN test environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    /* Clear sensitive data */
    __builtin_memset(g_token_array, 0, sizeof(g_token_array));
    printf("Destructor: Cleaned up ASAN test environment\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_recursive(int depth, volatile size_t* counter) {
    if (depth <= 0 || *counter >= 100) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->size = sizeof(ASTNode);
    
    /* Copy pattern data using builtin memcpy */
    size_t copy_size = (depth * 16) % 64;
    if (copy_size > 0) {
        __builtin_memcpy(node->data, &g_token_array[*counter], copy_size);
        (*counter) += copy_size;
    }
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    if (depth % 3 == 0) {
        goto skip_left;
    }
    
    node->left = create_ast_recursive(depth - 1, counter);
    
skip_left:
    if (depth % 2 == 0) {
        goto skip_right;
    }
    
    node->right = create_ast_recursive(depth - 1, counter);
    
skip_right:
    return node;
}

/* Function with goto jumping into memmove block */
static void process_with_goto(ASTNode* src, ASTNode* dst, volatile int mode) {
    if (!src || !dst) return;
    
    int use_memmove = 0;
    
    if (mode & 1) {
        goto use_builtin_memmove;
    }
    
    /* Regular path with memcpy */
    __builtin_memcpy(dst->data, src->data, sizeof(src->data));
    return;
    
use_builtin_memmove:
    /* Jumped-into block with memmove */
    __builtin_memmove(dst->data, src->data, sizeof(src->data));
    
    /* Jump back out */
    if (mode & 2) {
        goto finalize;
    }
    
    /* Additional processing */
    __builtin_memset(src->data + 32, 0xFF, 16);
    
finalize:
    return;
}

/* Parallel memory dispatch function */
static void parallel_memory_ops(volatile size_t iter_count) {
    char buffer1[512];
    char buffer2[512];
    volatile size_t local_size = g_mem_size;
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        #pragma omp for
        for (size_t i = 0; i < iter_count; i++) {
            /* Each thread performs memory operations */
            size_t offset = (i * thread_id * 17) % 256;
            size_t op_size = (local_size + offset) % 256 + 1;
            
            /* Mix of memory builtins */
            if (i % 3 == 0) {
                __builtin_memset(&buffer1[offset], thread_id, op_size);
            } else if (i % 3 == 1) {
                __builtin_memcpy(&buffer2[offset], &buffer1[offset], op_size);
            } else {
                /* Overlapping regions force memmove */
                size_t overlap = op_size / 2;
                __builtin_memmove(&buffer1[offset], &buffer1[offset + overlap], op_size);
            }
        }
        
        /* Thread-private memory operation */
        char thread_buf[128];
        __builtin_memset(thread_buf, thread_id, sizeof(thread_buf));
        __builtin_memcpy(&buffer1[thread_id * 64], thread_buf, 64);
    }
}

/* Main test execution */
int main(void) {
    volatile size_t ast_counter = 0;
    size_t hash_result = 0;
    
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Create recursive AST structure */
    ASTNode* root = create_ast_recursive(5, &ast_counter);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create destination node */
    ASTNode* dest = (ASTNode*)malloc(sizeof(ASTNode));
    if (!dest) {
        free(root);
        return 1;
    }
    
    __builtin_memset(dest, 0, sizeof(ASTNode));
    
    /* Test goto flow with memmove */
    printf("Testing goto flow control with memmove...\n");
    for (int i = 0; i < 4; i++) {
        process_with_goto(root, dest, i);
    }
    
    /* Execute parallel memory operations */
    printf("Executing parallel memory operations...\n");
    parallel_memory_ops(100);
    
    /* Process AST and compute verification hash */
    printf("Computing verification hash...\n");
    
    /* Use all three builtins in verification */
    char verify_buf[256];
    __builtin_memset(verify_buf, 0, sizeof(verify_buf));
    
    /* Copy AST data */
    __builtin_memcpy(verify_buf, root->data, sizeof(root->data));
    __builtin_memcpy(verify_buf + 64, dest->data, sizeof(dest->data));
    
    /* Move data around */
    __builtin_memmove(verify_buf + 128, verify_buf, 64);
    
    /* Compute simple hash */
    for (size_t i = 0; i < sizeof(verify_buf); i++) {
        hash_result = (hash_result * 31 + verify_buf[i]) % 1000000007;
    }
    
    printf("Verification hash: %zu\n", hash_result);
    printf("AST nodes processed: %zu\n", ast_counter);
    
    /* Cleanup */
    free(root);
    free(dest);
    
    /* Final memory operation to ensure all builtins are used */
    char final_buf[100];
    volatile size_t final_size = 50;
    __builtin_memset(final_buf, 0xAA, final_size);
    __builtin_memcpy(final_buf + 25, final_buf, 25);
    __builtin_memmove(final_buf, final_buf + 10, 40);
    
    printf("=== Test completed successfully ===\n");
    return 0;
}
