/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[32];
    int value;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char buffer[16];
    /* Force __builtin_memset in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    
    /* Initialize global with volatile memory operation */
    volatile int* init_ptr = (volatile int*)malloc(sizeof(int) * 4);
    if (init_ptr) {
        __builtin_memset((void*)init_ptr, 0xCC, sizeof(int) * 4);
        free((void*)init_ptr);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[8];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with __builtin_memcpy */
    __builtin_memcpy(node->data, base_data, 
                    (strlen(base_data) < 31) ? strlen(base_data) : 31);
    node->data[31] = '\0';
    
    /* Use volatile to control memset size */
    volatile size_t clear_size = sizeof(node->data) / 2;
    __builtin_memset(node->data + 16, 0, clear_size);
    
    node->value = depth;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        char child_data[32];
        __builtin_memcpy(child_data, base_data, 32);
        
        /* Goto block for testing flow sensitivity */
        if (depth % 3 == 0) {
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, child_data);
        
        create_left:
        /* Jump target with memmove operation */
        if (depth % 2 == 0) {
            char temp[32];
            __builtin_memmove(temp, child_data, 32);
            __builtin_memcpy(child_data, temp, 32);
        }
        
        node->right = create_ast(depth - 2, child_data);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with complex goto patterns around memmove */
static void process_with_goto(ASTNode* node, int mode) {
    if (!node) return;
    
    volatile char buffer[64];
    volatile size_t op_size = g_mem_size;
    
    switch (mode) {
        case 0:
            /* Direct path */
            __builtin_memset(buffer, 0x11, op_size);
            break;
            
        case 1:
            /* Goto into memmove block */
            goto memmove_block;
            
        case 2: {
            /* Goto around memmove */
            char temp[64];
            __builtin_memcpy(temp, node->data, 32);
            
            if (node->value > 10) {
                goto skip_memmove;
            }
            
            memmove_block:
            /* Target label with memmove */
            __builtin_memmove(buffer, temp, 32);
            /* Fall through */
            
            skip_memmove:
            __builtin_memcpy(buffer + 32, temp, 32);
            break;
        }
    }
    
    /* Copy processed data back to node */
    __builtin_memcpy(node->data, buffer, 32);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    const int num_workers = 4;
    char* buffers[num_workers];
    
    #pragma omp parallel for
    for (int i = 0; i < num_workers; i++) {
        buffers[i] = (char*)malloc(g_mem_size);
        if (buffers[i]) {
            /* Each thread uses different builtins */
            switch (i % 3) {
                case 0:
                    __builtin_memset(buffers[i], i, g_mem_size);
                    break;
                case 1:
                    if (i > 0) {
                        __builtin_memcpy(buffers[i], buffers[i-1], g_mem_size);
                    }
                    break;
                case 2:
                    __builtin_memmove(buffers[i], buffers[(i+1)%num_workers], 
                                     g_mem_size / 2);
                    break;
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_workers; i++) {
        if (buffers[i]) free(buffers[i]);
    }
}

/* Multi-stage processing with different memory operations */
static uint64_t process_ast_tree(ASTNode* root) {
    if (!root) return 0;
    
    uint64_t hash = 0;
    char temp_buffer[128];
    volatile size_t process_size = 48;
    
    /* Stage 1: Copy node data */
    __builtin_memcpy(temp_buffer, root->data, 32);
    
    /* Stage 2: Process with potential overlap (memmove) */
    if (root->left) {
        __builtin_memmove(temp_buffer + 16, root->left->data, 32);
    }
    
    /* Stage 3: Clear section (memset) */
    __builtin_memset(temp_buffer + 48, 0, process_size);
    
    /* Stage 4: Copy back processed data */
    __builtin_memcpy(root->data, temp_buffer, 32);
    
    /* Compute simple hash */
    for (int i = 0; i < 32; i++) {
        hash = (hash * 31) + (uint8_t)root->data[i];
    }
    
    /* Recursive processing */
    hash += process_ast_tree(root->left);
    hash += process_ast_tree(root->right);
    
    return hash;
}

/* Free AST tree */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear node before free */
    volatile char* data = node->data;
    __builtin_memset(data, 0, sizeof(node->data));
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Create and process AST */
    ASTNode* root = create_ast(5, "AST_ROOT_NODE_DATA_HERE");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Test goto flow control with memmove */
    for (int i = 0; i < 3; i++) {
        process_with_goto(root, i);
    }
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 4: Multi-stage tree processing */
    uint64_t final_hash = process_ast_tree(root);
    printf("AST processing hash: 0x%016llx\n", 
           (unsigned long long)final_hash);
    
    /* Phase 5: Additional built-in stress test */
    {
        char src[256], dst[256];
        volatile size_t test_sizes[] = {1, 16, 64, 128};
        
        __builtin_memset(src, 0xAA, sizeof(src));
        
        for (int i = 0; i < 4; i++) {
            volatile size_t sz = test_sizes[i];
            
            /* Test all three builtins in sequence */
            __builtin_memcpy(dst, src, sz);
            __builtin_memmove(src + 32, dst, sz);
            __builtin_memset(dst + 64, i, sz);
            
            /* Copy back with overlap */
            __builtin_memmove(dst, src, sz * 2);
        }
    }
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully.\n");
    return 0;
}
