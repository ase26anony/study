/* asan_coverage.c - Comprehensive test for ASAN memory builtin redirection */
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
static void init_asan_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (i % 26) + 'A';
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with volatile length */
    int copy_len = volatile_len % 128;
    if (copy_len > 255) copy_len = 255;
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->id = depth;
    
    /* Create children with goto-based control flow */
    if (depth > 1) {
        char child_data[256];
        __builtin_memset(child_data, 'C', sizeof(child_data));
        
        /* Jump label for goto */
        create_left_child:
        node->left = create_ast(depth - 1, child_data);
        
        /* Conditional goto */
        if (volatile_flag) {
            __builtin_memmove(child_data, node->data, copy_len);
            goto create_right_child;
        }
        
        create_right_child:
        node->right = create_ast(depth - 1, child_data);
    }
    
    return node;
}

/* Complex memory operation with OpenMP */
static void parallel_memory_operations(void) {
    char buffer1[1024];
    char buffer2[1024];
    char buffer3[1024];
    
    /* Initialize buffers with different patterns */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memset(buffer2, 0xBB, sizeof(buffer2));
    __builtin_memset(buffer3, 0xCC, sizeof(buffer3));
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                /* Use volatile to control length */
                __builtin_memcpy(buffer1 + thread_id * 16, 
                               buffer2, 
                               volatile_len % 256);
                break;
            case 1:
                /* Overlapping memory move */
                __builtin_memmove(buffer2 + 128, 
                                buffer2, 
                                volatile_len % 512);
                break;
            case 2:
                /* Set memory with pattern */
                __builtin_memset(buffer3 + thread_id * 32, 
                               thread_id, 
                               volatile_len % 128);
                break;
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* Cross-thread memory operations */
        #pragma omp for
        for (int i = 0; i < 16; i++) {
            int src_idx = (i * 31) % 1024;
            int dst_idx = (i * 17) % 1024;
            int len = (i * 7) % 128;
            
            /* Mix of memory operations */
            if (i % 3 == 0) {
                __builtin_memcpy(buffer1 + dst_idx, 
                               buffer2 + src_idx, 
                               len);
            } else if (i % 3 == 1) {
                __builtin_memmove(buffer3 + dst_idx, 
                                buffer1 + src_idx, 
                                len);
            } else {
                __builtin_memset(buffer2 + dst_idx, 
                               i, 
                               len);
            }
        }
    }
    
    /* Verify operations by computing hash */
    unsigned long hash = 0;
    for (int i = 0; i < sizeof(buffer1); i++) {
        hash = hash * 31 + buffer1[i];
        hash = hash * 31 + buffer2[i];
        hash = hash * 31 + buffer3[i];
    }
    printf("Parallel operations hash: %lu\n", hash);
}

/* Function with goto jumping into memory operation block */
static void goto_memory_operations(void) {
    char src[256], dst[256];
    
    /* Initialize source */
    for (int i = 0; i < sizeof(src); i++) {
        src[i] = i % 10;
    }
    
    /* Jump into the middle of operations */
    goto jump_point;
    
    normal_path:
    __builtin_memcpy(dst, src, sizeof(src));
    return;
    
    jump_point:
    /* This goto jumps into a block with memmove */
    if (volatile_flag) {
        __builtin_memmove(dst, src, volatile_len % 256);
        goto normal_path;
    }
    
    /* Another goto jumping out */
    goto exit_block;
    
    {
        char temp[128];
        __builtin_memset(temp, 0xFF, sizeof(temp));
    }
    
    exit_block:
    return;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN builtin redirection test\n");
    
    /* Phase 1: Basic builtin calls */
    printf("\nPhase 1: Basic builtin initialization\n");
    char test_buf1[512], test_buf2[512];
    
    __builtin_memset(test_buf1, 'X', sizeof(test_buf1));
    __builtin_memcpy(test_buf2, test_buf1, sizeof(test_buf1));
    __builtin_memmove(test_buf1 + 100, test_buf1, 200);
    
    /* Phase 2: Recursive AST operations */
    printf("\nPhase 2: Recursive AST operations\n");
    ASTNode* root = create_ast(4, "ROOT_NODE_DATA");
    
    if (root) {
        /* Copy between AST nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->right->data, 
                           root->left->data, 
                           volatile_len % 256);
        }
        
        /* Compute AST checksum */
        unsigned long ast_sum = 0;
        ASTNode* stack[32];
        int top = 0;
        stack[top++] = root;
        
        while (top > 0) {
            ASTNode* node = stack[--top];
            for (int i = 0; i < 256; i++) {
                ast_sum += node->data[i];
            }
            if (node->right) stack[top++] = node->right;
            if (node->left) stack[top++] = node->left;
            free(node);
        }
        printf("AST checksum: %lu\n", ast_sum);
    }
    
    /* Phase 3: Goto-based control flow */
    printf("\nPhase 3: Goto-based memory operations\n");
    goto_memory_operations();
    
    /* Phase 4: OpenMP parallel operations */
    printf("\nPhase 4: OpenMP parallel operations\n");
    parallel_memory_operations();
    
    /* Phase 5: Token pool operations */
    printf("\nPhase 5: Token pool finalization\n");
    char final_buffer[2048];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    
    for (int i = 0; i < 8; i++) {
        int offset = (i * 257) % 4096;
        int len = (i * 63) % 512;
        __builtin_memcpy(final_buffer + i * 256, 
                       token_pool + offset, 
                       len);
    }
    
    /* Final verification hash */
    unsigned long final_hash = 0;
    for (int i = 0; i < sizeof(final_buffer); i++) {
        final_hash = final_hash * 31 + final_buffer[i];
    }
    printf("Final verification hash: %lu\n", final_hash);
    
    printf("\nTest completed successfully\n");
    return 0;
}
