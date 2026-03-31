/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 16;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (i % 26) + 'A';
    }
    
    /* Use __builtin_memset in constructor */
    char local_buf[32];
    __builtin_memset(local_buf, 0xCC, sizeof(local_buf));
    
    /* Force RTL generation for memset */
    if (volatile_flag) {
        __builtin_memcpy(&token_pool[100], local_buf, 16);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Use __builtin_memmove in destructor */
    char cleanup_buf[48];
    __builtin_memmove(cleanup_buf, token_pool, sizeof(cleanup_buf));
    
    /* Verify pattern */
    int sum = 0;
    for (int i = 0; i < 48; i++) {
        sum += cleanup_buf[i];
    }
    printf("Destructor checksum: %d\n", sum);
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with __builtin_memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill with pattern using __builtin_memcpy */
    char pattern[32];
    for (int i = 0; i < 32; i++) {
        pattern[i] = 'A' + (id + i) % 26;
    }
    __builtin_memcpy(node->data, pattern, 32);
    
    node->id = id;
    
    /* Create children with goto-based control flow */
    int child_id = id * 2;
    
    /* Jump into block with memory operation */
    if (depth > 2) {
        goto create_left;
    }
    
    node->left = NULL;
    node->right = NULL;
    return node;
    
create_left:
    /* Use volatile to control flow */
    if (volatile_flag) {
        node->left = create_ast(depth - 1, child_id);
        
        /* Jump out of block */
        if (node->left) {
            goto create_right;
        }
    }
    
    node->left = NULL;
    
create_right:
    node->right = create_ast(depth - 1, child_id + 1);
    
    /* Memory copy between nodes if both exist */
    if (node->left && node->right) {
        __builtin_memcpy(node->right->data + 16, 
                        node->left->data, 
                        volatile_len);
    }
    
    return node;
}

/* Process AST with memory operations */
static int process_ast(ASTNode* node, int* sum) {
    if (!node) return 0;
    
    int local_sum = 0;
    
    /* Calculate checksum of node data */
    for (int i = 0; i < volatile_len; i++) {
        local_sum += node->data[i];
    }
    
    *sum += local_sum;
    
    /* Recursive processing */
    process_ast(node->left, sum);
    process_ast(node->right, sum);
    
    return local_sum;
}

/* Free AST with memory verification */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Verify data integrity before freeing */
    char verify_buf[64];
    __builtin_memcpy(verify_buf, node->data, sizeof(verify_buf));
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

/* Parallel memory dispatch function */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char src_buf[128];
        char dst_buf[128];
        
        /* Initialize with pattern */
        for (int i = 0; i < 128; i++) {
            src_buf[i] = (i + thread_id) % 256;
        }
        
        /* Use all three builtins in parallel region */
        __builtin_memset(dst_buf, 0, sizeof(dst_buf));
        __builtin_memcpy(dst_buf, src_buf, volatile_len);
        __builtin_memmove(dst_buf + 32, dst_buf, volatile_len);
        
        /* Verify copy */
        int match = 1;
        for (int i = 0; i < volatile_len; i++) {
            if (dst_buf[i] != src_buf[i]) {
                match = 0;
                break;
            }
        }
        
        #pragma omp critical
        {
            printf("Thread %d: %s\n", thread_id, match ? "PASS" : "FAIL");
        }
    }
}

/* Complex token parser with goto jumps */
static int parse_tokens(void) {
    char local_copy[256];
    int result = 0;
    
    /* Copy tokens with __builtin_memcpy */
    __builtin_memcpy(local_copy, token_pool + token_index, 256);
    
    /* Jump-based parsing */
    int i = 0;
parse_loop:
    if (i >= 256) goto parse_done;
    
    /* Use volatile to control jumps */
    if (volatile_flag && local_copy[i] == 'A') {
        goto handle_a;
    }
    
    result += local_copy[i];
    i++;
    goto parse_loop;
    
handle_a:
    /* Memory move within buffer */
    __builtin_memmove(&local_copy[i], &local_copy[i + 1], 10);
    result += 1000;
    i += 2;
    goto parse_loop;
    
parse_done:
    return result;
}

/* Main function with comprehensive test sequence */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: AST operations */
    ASTNode* root = create_ast(4, 1);
    
    int ast_sum = 0;
    process_ast(root, &ast_sum);
    printf("AST checksum: %d\n", ast_sum);
    
    /* Phase 2: Token parsing */
    token_index = 512;
    int parse_result = parse_tokens();
    printf("Parse result: %d\n", parse_result);
    
    /* Phase 3: Parallel memory operations */
    printf("\nParallel memory operations:\n");
    parallel_memory_ops();
    
    /* Phase 4: Direct built-in calls with varying sizes */
    char test_buf1[1024];
    char test_buf2[1024];
    
    /* Varying sizes to test different code paths */
    for (int size = 8; size <= 128; size *= 2) {
        volatile_len = size;
        
        __builtin_memset(test_buf1, size % 256, sizeof(test_buf1));
        __builtin_memcpy(test_buf2, test_buf1, size);
        __builtin_memmove(test_buf1 + size/2, test_buf1, size/2);
        
        /* Verify */
        int valid = 1;
        for (int i = 0; i < size/2; i++) {
            if (test_buf1[size/2 + i] != test_buf2[i]) {
                valid = 0;
                break;
            }
        }
        
        printf("Size %d: %s\n", size, valid ? "VALID" : "INVALID");
    }
    
    /* Cleanup */
    free_ast(root);
    
    /* Final verification */
    char final_buf[64];
    __builtin_memset(final_buf, 0xAA, sizeof(final_buf));
    __builtin_memcpy(final_buf + 16, final_buf, 16);
    
    int final_sum = 0;
    for (int i = 0; i < 64; i++) {
        final_sum += final_buf[i];
    }
    printf("\nFinal verification sum: %d\n", final_sum);
    printf("Test completed.\n");
    
    return 0;
}
