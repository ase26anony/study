/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {"memcpy", "memset", "memmove", "asan", "hwasan"};
static const int g_token_count = 5;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_env(void) {
    volatile char buf[32];
    /* Force early builtin usage in constructor */
    __builtin_memset(buf, 0, sizeof(buf));
    printf("Constructor: Initialized sanitizer environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_sanitizer_env(void) {
    volatile char buf[16];
    __builtin_memset(buf, 0xFF, sizeof(buf));
    printf("Destructor: Cleaning up\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = (*counter)++;
    
    /* Copy token data using builtin memcpy */
    int token_idx = node->id % g_token_count;
    __builtin_memcpy(node->data, g_tokens[token_idx], 
                     strlen(g_tokens[token_idx]) + 1);
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = (depth % 2 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, counter);
        node->right = create_ast(depth - 1, counter);
        return node;
        
    create_children:
        /* Jump into block with memmove operation */
        ASTNode* temp = create_ast(depth - 2, counter);
        if (temp) {
            /* Use builtin memmove within goto block */
            __builtin_memmove(&node->left, &temp, sizeof(ASTNode*));
            __builtin_memset(&temp, 0, sizeof(ASTNode*));
        }
        node->right = create_ast(depth - 1, counter);
    }
    
    return node;
}

/* Process AST with memory operations */
static int process_ast(ASTNode* node, int* sum) {
    if (!node) return 0;
    
    volatile char buffer[128];
    volatile size_t local_size = g_mem_size % 128;
    
    /* Copy node data using builtins */
    __builtin_memcpy(buffer, node->data, sizeof(node->data));
    
    /* Conditional memmove based on node ID */
    if (node->id % 3 == 0) {
        char temp[64];
        __builtin_memmove(temp, buffer, local_size);
        __builtin_memcpy(buffer, temp, local_size);
    }
    
    /* Update sum */
    for (size_t i = 0; i < sizeof(node->data) && node->data[i]; i++) {
        *sum += node->data[i];
    }
    
    /* Recursive processing */
    int left_sum = 0, right_sum = 0;
    process_ast(node->left, &left_sum);
    process_ast(node->right, &right_sum);
    
    return *sum + left_sum + right_sum;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Final clear of node structure */
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(void) {
    const int array_size = 1024;
    char* src = (char*)malloc(array_size);
    char* dst = (char*)malloc(array_size);
    
    if (!src || !dst) {
        free(src);
        free(dst);
        return;
    }
    
    /* Initialize source with pattern */
    for (int i = 0; i < array_size; i++) {
        src[i] = (char)(i % 256);
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        volatile size_t chunk_size = (array_size / omp_get_num_threads());
        
        /* Each thread performs memory operations */
        #pragma omp for
        for (int i = 0; i < array_size; i += chunk_size) {
            size_t actual_size = (i + chunk_size > array_size) ? 
                                 (array_size - i) : chunk_size;
            
            /* Mix of builtin memory operations */
            if (thread_id % 3 == 0) {
                __builtin_memcpy(dst + i, src + i, actual_size);
            } else if (thread_id % 3 == 1) {
                __builtin_memset(dst + i, thread_id, actual_size);
            } else {
                __builtin_memmove(dst + i, src + i, actual_size);
            }
        }
        
        /* Barrier with memory operation */
        #pragma omp barrier
        
        #pragma omp single
        {
            /* Final consolidation with memmove */
            __builtin_memmove(src, dst, array_size / 2);
        }
    }
    
    /* Verify by computing checksum */
    int checksum = 0;
    for (int i = 0; i < array_size; i++) {
        checksum += dst[i];
    }
    printf("Parallel ops checksum: %d\n", checksum);
    
    free(src);
    free(dst);
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    int counter = 0;
    ASTNode* root = create_ast(4, &counter);
    
    int ast_sum = 0;
    process_ast(root, &ast_sum);
    printf("AST processing sum: %d\n", ast_sum);
    
    /* Phase 2: Direct builtin calls with volatile control */
    volatile char buffer1[256], buffer2[256];
    volatile size_t op_size = g_mem_size % 128 + 64;
    
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, op_size);
    __builtin_memmove(buffer1, buffer2, op_size);
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Phase 4: Complex control flow with goto */
    {
        volatile int use_memmove = 1;
        char temp_buf[64];
        
        if (use_memmove) {
            goto perform_memmove;
        }
        
        __builtin_memset(temp_buf, 0, sizeof(temp_buf));
        goto cleanup;
        
    perform_memmove:
        __builtin_memmove(temp_buf, buffer1, sizeof(temp_buf));
        
    cleanup:
        __builtin_memset(temp_buf, 0, sizeof(temp_buf));
    }
    
    /* Cleanup */
    free_ast(root);
    
    /* Final verification */
    volatile int final_check = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        final_check += buffer1[i];
    }
    printf("Final buffer checksum: %d\n", final_check);
    printf("Test completed successfully\n");
    
    return 0;
}
