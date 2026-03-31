/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int g_volatile_size = 64;
static volatile char g_volatile_char = 'A';

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_token_array[4096];
static int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i % 26) + 'A');
    }
    printf("Constructor: Token array initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memset to initialize data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy pattern into node data using __builtin_memcpy */
    int copy_size = g_volatile_size % 128;
    __builtin_memcpy(node->data, g_token_array + (id * 32), copy_size);
    
    /* Create children with goto-controlled flow */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_left;
        }
        
        node->left = create_ast_node(depth - 1, id * 2);
        
    create_left:
        if (use_goto) {
            node->left = create_ast_node(depth - 1, id * 2);
        }
        
        node->right = create_ast_node(depth - 1, id * 2 + 1);
        
        /* Copy between child nodes using __builtin_memmove */
        if (node->left && node->right) {
            int move_size = (g_volatile_size % 64) + 16;
            __builtin_memmove(node->left->data + 32, 
                            node->right->data, 
                            move_size);
        }
    }
    
    return node;
}

/* Process AST with memory operations */
static int process_ast(ASTNode* node, int* sum) {
    if (!node) return 0;
    
    int local_sum = 0;
    
    /* Compute hash of node data */
    for (int i = 0; i < 256; i++) {
        local_sum += node->data[i];
    }
    
    /* Use volatile to control flow */
    volatile int should_recurse = 1;
    
    if (should_recurse) {
        process_ast(node->left, sum);
        process_ast(node->right, sum);
    }
    
    *sum += local_sum + node->id;
    return local_sum;
}

/* Free AST memory */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char buffer1[512];
        char buffer2[512];
        
        /* Initialize with __builtin_memset */
        __builtin_memset(buffer1, thread_id + '0', sizeof(buffer1));
        __builtin_memset(buffer2, thread_id + 'A', sizeof(buffer2));
        
        /* Copy between buffers with goto */
        int use_goto = (thread_id % 2 == 0);
        size_t copy_len = (g_volatile_size % 256) + 128;
        
        if (use_goto) {
            goto do_copy;
        }
        
        /* Normal path */
        __builtin_memcpy(buffer1 + 128, buffer2, copy_len);
        goto after_copy;
        
    do_copy:
        /* Goto path with memmove */
        __builtin_memmove(buffer1 + 64, buffer2 + 64, copy_len);
        
    after_copy:
        /* Verify copy */
        volatile char check_char = buffer1[128];
        (void)check_char; /* Prevent unused warning */
        
        #pragma omp barrier
        
        /* Collective operation */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            char temp[100];
            __builtin_memset(temp, i, sizeof(temp));
            
            if (i % 3 == 0) {
                __builtin_memcpy(g_token_array + (i * 10), temp, 50);
            } else if (i % 3 == 1) {
                __builtin_memmove(g_token_array + (i * 10), temp + 25, 50);
            }
        }
    }
}

/* Multi-stage initialization */
static void stage1_init(void) {
    /* Initialize first half with pattern */
    for (int i = 0; i < 2048; i++) {
        g_token_array[i] = (char)((i % 10) + '0');
    }
}

static void stage2_init(void) {
    /* Initialize second half with different pattern */
    for (int i = 2048; i < 4096; i++) {
        g_token_array[i] = (char)((i % 26) + 'a');
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Multi-stage initialization */
    stage1_init();
    stage2_init();
    
    /* Create recursive AST */
    ASTNode* root = create_ast_node(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process AST */
    int ast_sum = 0;
    process_ast(root, &ast_sum);
    printf("AST processing sum: %d\n", ast_sum);
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Additional memory operations with goto edge cases */
    char buffer_a[1024];
    char buffer_b[1024];
    
    /* Complex goto pattern */
    int mode = g_volatile_size % 4;
    
    switch (mode) {
        case 0:
            __builtin_memset(buffer_a, 0xAA, sizeof(buffer_a));
            goto copy_block;
            
        case 1:
            __builtin_memset(buffer_a, 0xBB, sizeof(buffer_a));
            goto move_block;
            
        case 2:
            __builtin_memset(buffer_a, 0xCC, sizeof(buffer_a));
            goto mixed_ops;
            
        default:
            __builtin_memset(buffer_a, 0xDD, sizeof(buffer_a));
            break;
    }
    
copy_block:
    __builtin_memcpy(buffer_b, buffer_a, sizeof(buffer_a));
    goto verify_result;
    
move_block:
    __builtin_memmove(buffer_b, buffer_a + 256, 512);
    goto verify_result;
    
mixed_ops:
    __builtin_memcpy(buffer_b, buffer_a, 256);
    __builtin_memmove(buffer_b + 256, buffer_a, 256);
    /* Fall through */
    
verify_result:
    /* Compute verification hash */
    int verify_sum = 0;
    for (int i = 0; i < 1024; i++) {
        verify_sum += buffer_b[i];
    }
    printf("Buffer verification sum: %d\n", verify_sum);
    
    /* Cleanup */
    free_ast(root);
    
    /* Final token array hash */
    int final_hash = 0;
    for (int i = 0; i < 4096; i += 64) {
        final_hash += g_token_array[i];
    }
    printf("Final token array hash: %d\n", final_hash);
    
    printf("Test completed successfully\n");
    return 0;
}
