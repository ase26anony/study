/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_init_value = 0x42;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char *data;
    size_t data_len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    printf("ASAN Test Constructor: Initializing test environment\n");
    g_mem_size = 256 + (rand() % 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("ASAN Test Destructor: Cleaning up\n");
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast(int depth, const char *label) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = depth * 100;
    node->data_len = g_mem_size / (depth + 1);
    node->data = malloc(node->data_len);
    
    if (node->data) {
        /* Fill data with pattern using builtin memset */
        __builtin_memset(node->data, g_init_value + depth, node->data_len);
        
        /* Copy label into data section */
        size_t label_len = strlen(label);
        if (label_len > 0) {
            size_t copy_len = label_len < node->data_len ? label_len : node->data_len;
            __builtin_memcpy(node->data, label, copy_len);
        }
    }
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 2 == 0);
        
        if (use_goto) {
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, "left");
        goto skip_left;
        
    create_left:
        node->left = create_ast(depth - 1, "left_goto");
        
    skip_left:
        node->right = create_ast(depth - 1, "right");
    } else {
        node->left = NULL;
        node->right = NULL;
    }
    
    return node;
}

/* Process AST with memory operations */
static long process_ast(ASTNode *node, int *counter) {
    if (!node) return 0;
    
    long sum = node->value;
    (*counter)++;
    
    /* Memory operations between nodes */
    if (node->left && node->right) {
        /* Use builtin memmove for overlapping regions */
        char temp_buf[128];
        size_t move_size = node->left->data_len < sizeof(temp_buf) ? 
                          node->left->data_len : sizeof(temp_buf);
        
        if (move_size > 0) {
            /* Copy left data to temp */
            __builtin_memcpy(temp_buf, node->left->data, move_size);
            
            /* Move right data to left (overlapping) */
            if (node->right->data_len >= move_size) {
                __builtin_memmove(node->left->data, node->right->data, move_size);
            }
            
            /* Restore from temp */
            __builtin_memcpy(node->right->data, temp_buf, move_size);
        }
    }
    
    /* Recursive processing */
    sum += process_ast(node->left, counter);
    
    /* Jump point for goto testing */
    if (node->type == 3) {
        goto process_right;
    }
    
    sum += process_ast(node->right, counter);
    return sum;
    
process_right:
    sum += process_ast(node->right, counter);
    return sum;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode *node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    if (node->data) {
        /* Clear sensitive data before free */
        __builtin_memset(node->data, 0, node->data_len);
        free(node->data);
    }
    
    /* Clear node structure */
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_arrays = 8;
    const size_t array_size = 1024;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char *src = malloc(array_size);
        char *dst = malloc(array_size);
        
        if (src && dst) {
            /* Initialize with builtin memset */
            __builtin_memset(src, thread_id + 0x30, array_size);
            __builtin_memset(dst, 0, array_size);
            
            /* Copy with builtin memcpy */
            __builtin_memcpy(dst, src, array_size / 2);
            
            /* Move with builtin memmove (overlapping) */
            size_t overlap_size = array_size / 4;
            __builtin_memmove(dst + overlap_size, dst, overlap_size);
            
            /* Verify copy */
            int errors = 0;
            for (size_t i = 0; i < array_size / 2; i++) {
                if (dst[i] != src[i]) errors++;
            }
            
            #pragma omp critical
            {
                printf("Thread %d: Memory ops completed, errors: %d\n", 
                       thread_id, errors);
            }
        }
        
        free(src);
        free(dst);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: AST operations */
    printf("\nPhase 1: AST Memory Operations\n");
    int node_count = 0;
    ASTNode *root = create_ast(4, "root");
    
    if (root) {
        long ast_sum = process_ast(root, &node_count);
        printf("AST processed: %d nodes, sum: %ld\n", node_count, ast_sum);
        free_ast(root);
    }
    
    /* Phase 2: Parallel operations */
    printf("\nPhase 2: OpenMP Parallel Operations\n");
    #ifdef _OPENMP
    parallel_memory_ops();
    #else
    printf("OpenMP not available, skipping parallel phase\n");
    #endif
    
    /* Phase 3: Direct built-in stress test */
    printf("\nPhase 3: Built-in Function Stress Test\n");
    {
        char buffer1[512];
        char buffer2[512];
        volatile size_t op_size = g_mem_size;
        
        /* Test all three builtins in sequence */
        __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
        __builtin_memcpy(buffer2, buffer1, op_size);
        
        /* Create overlapping region for memmove */
        char *mid = buffer1 + 128;
        __builtin_memmove(mid, buffer1, 256);
        
        /* Verify results */
        int match_count = 0;
        for (size_t i = 0; i < op_size && i < sizeof(buffer2); i++) {
            if (buffer2[i] == 0xAA) match_count++;
        }
        printf("Built-in verification: %d bytes match pattern\n", match_count);
    }
    
    /* Phase 4: Variable-sized operations */
    printf("\nPhase 4: Variable-sized Operations\n");
    {
        size_t sizes[] = {16, 64, 128, 256, 512};
        char *blocks[5];
        
        for (int i = 0; i < 5; i++) {
            blocks[i] = malloc(sizes[i]);
            if (blocks[i]) {
                __builtin_memset(blocks[i], i + 0x10, sizes[i]);
                
                if (i > 0) {
                    size_t copy_size = sizes[i] < sizes[i-1] ? sizes[i] : sizes[i-1];
                    __builtin_memcpy(blocks[i], blocks[i-1], copy_size);
                }
            }
        }
        
        /* Cleanup */
        for (int i = 0; i < 5; i++) {
            if (blocks[i]) {
                free(blocks[i]);
            }
        }
    }
    
    printf("\nASAN test completed successfully\n");
    return 0;
}
