/* asan_coverage.c - Comprehensive test for ASAN memory built-in redirection */
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
        token_pool[i] = (char)(i % 256);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Verify memory was properly handled */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += token_pool[i];
    }
    printf("Destructor: Token pool checksum = %d\n", sum);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = depth;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy base data using __builtin_memcpy */
    size_t copy_len = (size_t)(volatile_len % 128);
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Create children with goto-based control flow */
    if (depth > 1) {
        int use_left = volatile_flag & 1;
        
        if (use_left) {
            /* Jump into block with memmove */
            goto create_left;
        } else {
            /* Jump around memmove */
            goto skip_memmove;
        }
        
    create_left:
        /* Use __builtin_memmove within goto block */
        char temp[128];
        __builtin_memcpy(temp, node->data, 64);
        __builtin_memmove(node->data + 64, temp, 64);
        
        node->left = create_ast(depth - 1, node->data);
        
        /* Jump out of block */
        goto after_children;
        
    skip_memmove:
        node->left = create_ast(depth - 1, "left");
        
    after_children:
        node->right = create_ast(depth - 1, "right");
    }
    
    return node;
}

/* Process AST with memory operations */
static int process_ast(ASTNode* node, int* result) {
    if (!node) return 0;
    
    int left_sum = process_ast(node->left, result);
    int right_sum = process_ast(node->right, result);
    
    /* Perform memory operation on node data */
    char buffer[512];
    
    /* Use all three builtins in sequence */
    __builtin_memset(buffer, node->id, sizeof(buffer));
    __builtin_memcpy(buffer + 128, node->data, 128);
    __builtin_memmove(node->data, buffer + 64, 128);
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 128; i++) {
        sum += (int)node->data[i];
    }
    
    *result += sum + left_sum + right_sum;
    return sum;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Use goto for non-linear control flow */
    if (node->left) {
        goto free_left;
    } else {
        goto free_right;
    }
    
free_left:
    free_ast(node->left);
    
free_right:
    free_ast(node->right);
    
    /* Clear data before free */
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
        char src_buf[1024];
        char dst_buf[1024];
        
        /* Initialize with pattern */
        for (int i = 0; i < sizeof(src_buf); i++) {
            src_buf[i] = (char)((i + thread_id) % 256);
        }
        
        /* Use all memory builtins in parallel region */
        __builtin_memset(dst_buf, 0, sizeof(dst_buf));
        __builtin_memcpy(dst_buf, src_buf, (size_t)(volatile_len % 512));
        
        /* Conditional memmove with goto */
        if (thread_id % 2 == 0) {
            goto do_memmove;
        } else {
            goto skip_memmove_parallel;
        }
        
    do_memmove:
        __builtin_memmove(dst_buf + 256, dst_buf, 256);
        
    skip_memmove_parallel:
        /* Verify copy */
        int errors = 0;
        for (int i = 0; i < 256; i++) {
            if (dst_buf[i] != src_buf[i]) errors++;
        }
        
        #pragma omp critical
        {
            printf("Thread %d: Memory ops completed, errors = %d\n", 
                   thread_id, errors);
        }
    }
}

/* Multi-stage initialization */
static void stage_initialization(void) {
    /* Stage 1: Direct builtin calls */
    char stage1_buf[256];
    __builtin_memset(stage1_buf, 0xAA, sizeof(stage1_buf));
    __builtin_memcpy(stage1_buf + 128, token_pool, 128);
    
    /* Stage 2: Indirect through function pointer */
    void* (*mem_funcs[3])(void*, const void*, size_t) = {
        (void* (*)(void*, const void*, size_t))__builtin_memcpy,
        (void* (*)(void*, const void*, size_t))__builtin_memset,
        (void* (*)(void*, const void*, size_t))__builtin_memmove
    };
    
    char stage2_buf[256];
    for (int i = 0; i < 3; i++) {
        mem_funcs[i](stage2_buf, stage1_buf, 64);
    }
    
    /* Stage 3: Nested calls */
    char temp[128];
    __builtin_memcpy(temp, stage2_buf, 64);
    __builtin_memset(stage2_buf, 0, 64);
    __builtin_memmove(stage2_buf, temp, 64);
}

int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Force initialization of asan_memfn_rtls cache */
    stage_initialization();
    
    /* Create and process recursive AST */
    ASTNode* root = create_ast(4, token_pool);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    int ast_result = 0;
    process_ast(root, &ast_result);
    printf("AST processing result: %d\n", ast_result);
    
    /* Execute parallel memory operations */
    printf("\nParallel memory operations:\n");
    parallel_memory_ops();
    
    /* Complex control flow with goto and memory ops */
    printf("\nComplex control flow test:\n");
    {
        char flow_buf[512];
        int counter = 0;
        
    start_loop:
        if (counter >= 3) goto end_loop;
        
        /* Alternate between memcpy and memmove */
        if (counter % 2 == 0) {
            __builtin_memcpy(flow_buf + counter * 64, token_pool, 64);
        } else {
            __builtin_memmove(flow_buf + counter * 64, flow_buf, 64);
        }
        
        counter++;
        goto start_loop;
        
    end_loop:
        /* Final memset */
        __builtin_memset(flow_buf + 384, 0xFF, 128);
        printf("Control flow test completed\n");
    }
    
    /* Cleanup */
    free_ast(root);
    
    /* Final verification */
    int final_sum = 0;
    for (int i = 0; i < 1024; i++) {
        final_sum += token_pool[i];
    }
    printf("\nFinal token pool checksum: %d\n", final_sum);
    printf("Test completed successfully\n");
    
    return 0;
}
