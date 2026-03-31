/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int value;
} ASTNode;

/* Global token array */
static char g_tokens[][32] = {
    "memcpy_test", "memset_test", "memmove_test",
    "recursive", "parallel", "volatile"
};

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_test(void) {
    printf("ASAN/HWASAN Test Constructor: Initializing...\n");
    g_mem_size = 256 + (rand() % 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("ASAN/HWASAN Test Destructor: Cleaning up...\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with goto for flow control */
    int copy_success = 0;
    
    copy_start:
    if (base_data) {
        size_t len = strlen(base_data);
        if (len > sizeof(node->data) - 1)
            len = sizeof(node->data) - 1;
        
        __builtin_memcpy(node->data, base_data, len);
        node->data[len] = '\0';
        copy_success = 1;
    }
    
    if (!copy_success) {
        __builtin_memset(node->data, 'X', sizeof(node->data) - 1);
        node->data[sizeof(node->data) - 1] = '\0';
    }
    
    node->value = depth;
    
    /* Recursive creation with goto jumping around */
    if (depth > 1) {
        goto create_children;
    } else {
        node->left = node->right = NULL;
        goto node_done;
    }
    
create_children:
    node->left = create_ast(depth - 1, "left_child");
    node->right = create_ast(depth - 1, "right_child");
    
node_done:
    return node;
}

/* Function with complex memory operations and goto */
static void process_ast(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int operation = 0;
    
operation_loop:
    switch (operation) {
        case 0:
            /* Use __builtin_memcpy between nodes */
            __builtin_memcpy(dst->data, src->data, sizeof(src->data));
            operation++;
            goto operation_loop;
            
        case 1:
            /* Use __builtin_memmove with overlapping regions */
            char buffer[128];
            __builtin_memset(buffer, 0, sizeof(buffer));
            __builtin_memcpy(buffer, src->data, 32);
            
            /* Overlapping memmove */
            __builtin_memmove(buffer + 16, buffer, 64);
            operation++;
            goto operation_loop;
            
        case 2:
            /* Final memset */
            __builtin_memset(dst->data + 32, 0xFF, 16);
            break;
    }
    
    /* Process children recursively */
    process_ast(src->left, dst->left);
    process_ast(src->right, dst->right);
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_ops(void) {
    const int num_arrays = 8;
    char* arrays[num_arrays];
    
    /* Allocate arrays */
    for (int i = 0; i < num_arrays; i++) {
        arrays[i] = (char*)malloc(g_mem_size);
        if (!arrays[i]) continue;
        
        /* Initialize with memset */
        __builtin_memset(arrays[i], i, g_mem_size);
    }
    
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < num_arrays - 1; i++) {
            if (arrays[i] && arrays[i + 1]) {
                /* Parallel memcpy operations */
                __builtin_memcpy(arrays[i + 1], arrays[i], g_mem_size / 2);
                
                /* Parallel memmove with overlap */
                __builtin_memmove(arrays[i] + g_mem_size / 4,
                                 arrays[i],
                                 g_mem_size / 2);
            }
        }
        
        #pragma omp single
        {
            /* Single thread memset */
            if (arrays[0]) {
                __builtin_memset(arrays[0], 0xAA, g_mem_size);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_arrays; i++) {
        free(arrays[i]);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test...\n");
    
    /* Initialize token processing */
    char token_buffer[1024];
    __builtin_memset(token_buffer, 0, sizeof(token_buffer));
    
    /* Copy tokens using builtins */
    size_t offset = 0;
    for (int i = 0; i < sizeof(g_tokens)/sizeof(g_tokens[0]); i++) {
        size_t len = strlen(g_tokens[i]);
        __builtin_memcpy(token_buffer + offset, g_tokens[i], len);
        offset += len;
        token_buffer[offset++] = ' ';
    }
    
    /* Create and process AST */
    ASTNode* src_tree = create_ast(4, "root_source");
    ASTNode* dst_tree = create_ast(4, "root_dest");
    
    if (src_tree && dst_tree) {
        process_ast(src_tree, dst_tree);
        
        /* Calculate hash/sum from processed trees */
        unsigned long long hash = 0;
        ASTNode* stack[64];
        int top = 0;
        stack[top++] = dst_tree;
        
        while (top > 0) {
            ASTNode* node = stack[--top];
            if (!node) continue;
            
            /* Add node data to hash */
            for (size_t i = 0; i < sizeof(node->data); i++) {
                hash = (hash * 31 + node->data[i]) % 1000000007;
            }
            hash = (hash + node->value) % 1000000007;
            
            if (node->right) stack[top++] = node->right;
            if (node->left) stack[top++] = node->left;
        }
        
        printf("AST Hash Result: %llu\n", hash);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Final builtin operations with volatile control */
    volatile char final_buf[512];
    volatile char src_buf[512];
    
    for (volatile int i = 0; i < sizeof(src_buf); i++) {
        src_buf[i] = (char)(i % 256);
    }
    
    /* Force all three builtins in sequence */
    __builtin_memcpy((void*)final_buf, (void*)src_buf, g_mem_size);
    __builtin_memmove((void*)(final_buf + 128), (void*)final_buf, 256);
    __builtin_memset((void*)(final_buf + 384), 0xCC, 128);
    
    /* Verify final buffer */
    volatile int verify_sum = 0;
    for (volatile int i = 0; i < 512; i++) {
        verify_sum += final_buf[i];
    }
    printf("Final buffer checksum: %d\n", verify_sum);
    
    /* Cleanup */
    /* Helper function to free AST */
    void free_ast(ASTNode* node) {
        if (!node) return;
        free_ast(node->left);
        free_ast(node->right);
        free(node);
    }
    
    free_ast(src_tree);
    free_ast(dst_tree);
    
    printf("Test completed successfully.\n");
    return 0;
}
