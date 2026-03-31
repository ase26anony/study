/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (i % 26) + 'a';
    }
    
    /* Use __builtin_memset in constructor */
    __builtin_memset(token_pool + 1024, 'X', 512);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Verify memory was properly handled */
    __builtin_memset(token_pool, 0, 256);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = depth;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memcpy to copy data */
    int copy_len = volatile_len % 128;
    if (copy_len > 255) copy_len = 255;
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data[copy_len] = '\0';
    
    /* Create children with goto for flow control */
    if (depth > 1) {
        goto create_left;
        
        skipped_block:
        /* This label is jumped over */
        return node;
        
        create_left:
        node->left = create_ast(depth - 1, base_data + 16);
        
        /* Jump around memory operation */
        if (volatile_flag) {
            goto right_side;
        }
        
        middle_block:
        /* Use __builtin_memmove with goto */
        char temp[128];
        __builtin_memmove(temp, node->data, 64);
        __builtin_memmove(node->data + 64, temp, 64);
        goto skipped_block;
        
        right_side:
        node->right = create_ast(depth - 1, base_data + 32);
        goto middle_block;
    }
    
    return node;
}

/* Process AST with memory operations */
static int process_ast(ASTNode* node, int* sum) {
    if (!node) return 0;
    
    int local_sum = 0;
    
    /* Process data with __builtin_memcpy */
    char buffer[256];
    int len = strlen(node->data);
    if (len > 0) {
        __builtin_memcpy(buffer, node->data, len);
        
        /* Calculate simple hash */
        for (int i = 0; i < len; i++) {
            local_sum += buffer[i];
        }
    }
    
    /* Recursive processing */
    local_sum += process_ast(node->left, sum);
    local_sum += process_ast(node->right, sum);
    
    *sum += local_sum;
    return local_sum;
}

/* Free AST memory */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear node data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
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
        char src_buf[512];
        char dst_buf[512];
        
        /* Initialize with pattern */
        for (int i = 0; i < sizeof(src_buf); i++) {
            src_buf[i] = (i + thread_id) % 256;
        }
        
        /* Use all three builtins in parallel region */
        __builtin_memset(dst_buf, thread_id, sizeof(dst_buf));
        __builtin_memcpy(dst_buf + 128, src_buf + 128, 256);
        __builtin_memmove(dst_buf + 64, dst_buf + 128, 128);
        
        /* Verify with volatile access */
        volatile char check = dst_buf[200];
        (void)check; /* Suppress unused warning */
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Create and process AST */
    ASTNode* root = create_ast(4, token_pool);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    int ast_sum = 0;
    process_ast(root, &ast_sum);
    printf("AST processing sum: %d\n", ast_sum);
    
    /* Phase 2: Parallel memory operations */
    printf("Starting parallel memory operations...\n");
    parallel_memory_ops();
    
    /* Phase 3: Direct built-in calls with volatile control */
    char direct_src[1024];
    char direct_dst[1024];
    
    /* Initialize with volatile length */
    int op_len = volatile_len;
    if (op_len > sizeof(direct_src)) op_len = sizeof(direct_src);
    
    /* Chain of memory operations */
    __builtin_memset(direct_src, 0xAA, op_len);
    __builtin_memcpy(direct_dst, direct_src, op_len / 2);
    __builtin_memmove(direct_dst + op_len / 4, direct_dst, op_len / 4);
    
    /* Calculate verification hash */
    int verify_sum = 0;
    for (int i = 0; i < op_len; i++) {
        verify_sum += direct_dst[i];
    }
    printf("Direct operations verification sum: %d\n", verify_sum);
    
    /* Phase 4: Edge case with goto around memmove */
    {
        char edge_buf[256];
        int use_memmove = volatile_flag;
        
        __builtin_memset(edge_buf, 'E', sizeof(edge_buf));
        
        if (use_memmove) {
            goto do_memmove;
        } else {
            goto skip_memmove;
        }
        
        do_memmove:
        __builtin_memmove(edge_buf + 128, edge_buf, 128);
        goto continue_exec;
        
        skip_memmove:
        __builtin_memcpy(edge_buf + 128, edge_buf, 128);
        
        continue_exec:
        /* Verify */
        volatile char check_edge = edge_buf[150];
        (void)check_edge;
    }
    
    /* Cleanup */
    free_ast(root);
    
    /* Final memory operation in main */
    __builtin_memset(token_pool + 2048, 0, 512);
    
    printf("Test completed successfully.\n");
    return 0;
}
