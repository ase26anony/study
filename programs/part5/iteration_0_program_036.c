/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force memcpy redirection early */
    __builtin_memcpy(buffer, "constructor_init", 16);
    __builtin_memset(buffer + 16, 0, 32);
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char final_check[64];
    __builtin_memset(final_check, 0xFF, 64);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile control */
    size_t copy_len = g_mem_size % 128;
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->size = copy_len;
    
    /* Create children with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, "left_branch");
        
    create_left:
        if (use_goto) {
            node->left = create_ast(depth - 1, "goto_left");
        }
        
        /* Memmove between node data sections */
        if (node->left) {
            volatile char temp[256];
            __builtin_memcpy(temp, node->data, node->size);
            __builtin_memmove(node->left->data, temp, node->size);
        }
    } else {
        node->left = NULL;
    }
    
    node->right = create_ast(depth - 1, "right_branch");
    
    return node;
}

/* Function with complex control flow and memmove */
static void process_ast_with_goto(ASTNode* root) {
    if (!root) return;
    
    volatile int jump_flag = 0;
    
    /* Goto into memory operation block */
    if (root->left) {
        jump_flag = 1;
        goto memory_ops;
    }
    
    /* Normal path */
    __builtin_memset(root->data + root->size, 0, 32);
    
memory_ops:
    if (jump_flag) {
        /* This should trigger memmove redirection */
        char temp[256];
        __builtin_memcpy(temp, root->left->data, root->left->size);
        __builtin_memmove(root->data, temp, root->left->size);
        jump_flag = 0;
        goto after_ops;
    }
    
    /* Alternative path with memcpy */
    __builtin_memcpy(root->data, "default_data", 12);
    
after_ops:
    /* Process children */
    process_ast_with_goto(root->left);
    process_ast_with_goto(root->right);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_operations(void) {
    volatile size_t local_size = g_mem_size;
    char buffers[4][256];
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        /* Each thread uses different builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(buffers[tid], tid, local_size);
                break;
            case 1:
                __builtin_memcpy(buffers[tid], buffers[(tid + 1) % 4], local_size);
                break;
            case 2:
                __builtin_memmove(buffers[tid], buffers[(tid + 2) % 4], local_size);
                break;
        }
        
        #pragma omp barrier
        
        /* Cross-thread memory operations */
        if (tid == 0) {
            __builtin_memcpy(buffers[3], buffers[1], local_size);
        }
    }
}

/* Multi-stage initialization function */
static void initialize_complex_buffer(char* buffer, size_t size) {
    volatile size_t half = size / 2;
    
    /* Stage 1: memset */
    __builtin_memset(buffer, 0xAA, half);
    
    /* Stage 2: memcpy with overlap */
    __builtin_memcpy(buffer + half, buffer, half);
    
    /* Stage 3: memmove to handle overlap properly */
    __builtin_memmove(buffer, buffer + half/2, half);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Basic builtin calls */
    volatile char test_buf[512];
    volatile size_t op_size = g_mem_size;
    
    __builtin_memset(test_buf, 0, sizeof(test_buf));
    __builtin_memcpy(test_buf, "initial_data", 12);
    __builtin_memmove(test_buf + 64, test_buf, 32);
    
    /* Phase 2: Recursive AST operations */
    ASTNode* root = create_ast(4, "root_node_data");
    if (root) {
        process_ast_with_goto(root);
        
        /* Calculate verification hash */
        unsigned long hash = 0;
        for (size_t i = 0; i < root->size && i < sizeof(root->data); i++) {
            hash = hash * 31 + root->data[i];
        }
        printf("AST hash: %lu\n", hash);
    }
    
    /* Phase 3: OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_operations();
    printf("OpenMP parallel operations completed\n");
    #endif
    
    /* Phase 4: Multi-stage buffer processing */
    char complex_buf[1024];
    initialize_complex_buffer(complex_buf, sizeof(complex_buf));
    
    /* Final verification */
    volatile int final_check = 0;
    for (size_t i = 0; i < sizeof(complex_buf); i++) {
        final_check ^= complex_buf[i];
    }
    printf("Final XOR check: %d\n", final_check);
    
    /* Cleanup */
    /* Note: Proper AST cleanup omitted for brevity - would need recursive free */
    
    printf("Test completed successfully\n");
    return 0;
}
