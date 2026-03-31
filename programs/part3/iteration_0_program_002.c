/* ISO C99-compliant test program for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_huge = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Global token array */
static char token_pool[4096];
volatile int token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)(i % 256);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    node->value = depth * 100;
    
    /* Copy data using __builtin_memcpy */
    __builtin_memcpy(node->data, &token_pool[token_idx], 64);
    token_idx = (token_idx + 64) % sizeof(token_pool);
    
    /* Recursive calls with goto for control flow */
    int use_goto = (depth % 2 == 0);
    
    if (use_goto) {
        goto parse_left;
    } else {
        node->left = parse_expression(depth - 1);
        goto parse_right;
    }
    
parse_left:
    node->left = parse_expression(depth - 2);
    
parse_right:
    node->right = parse_expression(depth - 1);
    
    return node;
}

/* Function with complex memory operations and goto */
static void process_ast(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int copy_mode = 0;
    
copy_start:
    /* Use __builtin_memmove with goto jumps */
    if (copy_mode == 0) {
        __builtin_memmove(dst->data, src->data, 32);
        copy_mode = 1;
        goto copy_middle;
    }
    
copy_middle:
    __builtin_memmove(dst->data + 32, src->data + 32, 32);
    copy_mode = 2;
    goto copy_end;
    
copy_end:
    /* Copy structure using memcpy */
    __builtin_memcpy(&dst->value, &src->value, sizeof(int));
    
    /* Jump back for recursion */
    if (src->left && dst->left) {
        copy_mode = 0;
        goto copy_start;
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf1[128];
        char local_buf2[128];
        
        /* Initialize with __builtin_memset */
        __builtin_memset(local_buf1, thread_id, sizeof(local_buf1));
        __builtin_memset(local_buf2, 0xFF, sizeof(local_buf2));
        
        /* Copy between buffers */
        __builtin_memcpy(local_buf2, local_buf1, 64);
        
        /* Move data around */
        __builtin_memmove(local_buf1 + 32, local_buf2, 32);
        
        #pragma omp barrier
        
        /* Verify copy */
        int sum = 0;
        for (int i = 0; i < 64; i++) {
            sum += local_buf2[i];
        }
        
        #pragma omp critical
        {
            printf("Thread %d: memcpy sum = %d\n", thread_id, sum);
        }
    }
}

/* Function with variable-sized operations */
static void variable_mem_ops(size_t size) {
    volatile char* buf1 = (char*)malloc(size);
    volatile char* buf2 = (char*)malloc(size);
    
    if (!buf1 || !buf2) {
        free((void*)buf1);
        free((void*)buf2);
        return;
    }
    
    /* Force non-constant size operations */
    size_t op_size = g_mem_size;
    if (g_use_huge) {
        op_size = size / 2;
    }
    
    /* Chain of memory operations */
    __builtin_memset((void*)buf1, 0xAA, op_size);
    __builtin_memcpy((void*)buf2, (void*)buf1, op_size);
    __builtin_memmove((void*)buf1, (void*)buf2, op_size / 2);
    
    /* Verify by computing checksum */
    unsigned long checksum = 0;
    for (size_t i = 0; i < op_size; i++) {
        checksum += buf1[i];
    }
    
    printf("Variable ops: checksum = %lu (size=%zu)\n", checksum, op_size);
    
    free((void*)buf1);
    free((void*)buf2);
}

int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Recursive AST parsing */
    printf("\nPhase 1: Building AST\n");
    ASTNode* ast1 = parse_expression(4);
    ASTNode* ast2 = parse_expression(3);
    
    if (ast1 && ast2) {
        process_ast(ast1, ast2);
        
        /* Compute hash from AST data */
        unsigned long hash = 0;
        for (int i = 0; i < 64; i++) {
            hash = hash * 31 + ast1->data[i];
            hash = hash * 31 + ast2->data[i];
        }
        printf("AST hash: %lu\n", hash);
    }
    
    /* Phase 2: OpenMP parallel operations */
    printf("\nPhase 2: Parallel memory operations\n");
    parallel_memory_ops();
    
    /* Phase 3: Variable-sized operations */
    printf("\nPhase 3: Variable-sized memory operations\n");
    for (int i = 0; i < 3; i++) {
        g_use_huge = (i == 2);
        variable_mem_ops(1024 * (i + 1));
    }
    
    /* Phase 4: Direct built-in calls in loops */
    printf("\nPhase 4: Direct built-in calls\n");
    char final_buf1[512];
    char final_buf2[512];
    
    for (int i = 0; i < 10; i++) {
        __builtin_memset(final_buf1, i, sizeof(final_buf1));
        __builtin_memcpy(final_buf2, final_buf1, 256 + i * 25);
        __builtin_memmove(final_buf1, final_buf2, 128);
    }
    
    /* Final verification */
    int final_sum = 0;
    for (int i = 0; i < 512; i++) {
        final_sum += final_buf1[i] + final_buf2[i];
    }
    printf("Final sum: %d\n", final_sum);
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
