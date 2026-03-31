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
    int depth;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    
    /* Force initialization with volatile memory operations */
    volatile char buffer[128];
    __builtin_memset((void*)buffer, 0xAA, sizeof(buffer));
    
    /* This should trigger ASAN built-in redirection */
    volatile char dest[64];
    __builtin_memcpy((void*)dest, (void*)buffer, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
    
    /* Final memory operation to ensure coverage */
    volatile int final_check[16];
    __builtin_memset((void*)final_check, 0xFF, sizeof(final_check));
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->depth = depth;
    
    /* Fill data with pattern */
    for (int i = 0; i < 255; i++) {
        node->data[i] = (char)((depth * 31 + i * 17) % 256);
    }
    node->data[255] = '\0';
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int create_left = 1;
        
        /* Use goto to create complex control flow */
        if (depth % 2 == 0) goto create_children;
        
        /* Alternative path */
        node->left = create_ast(depth - 1);
        create_left = 0;
        
    create_children:
        if (create_left) {
            node->left = create_ast(depth - 1);
        }
        node->right = create_ast(depth - 2);
    }
    
    return node;
}

/* Copy AST nodes with memcpy between structures */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Direct structure copy using memcpy */
    __builtin_memcpy(dest->data, src->data, sizeof(src->data));
    
    /* Copy depth field */
    dest->depth = src->depth;
}

/* Complex function with goto jumps around memory operations */
static void process_with_goto(ASTNode* nodes[], int count) {
    int i = 0;
    
start_loop:
    if (i >= count) goto end_processing;
    
    /* Jump into memory operation block */
    if (nodes[i] && nodes[i]->left) {
        goto copy_operation;
    }
    
    /* Skip copy for NULL nodes */
    i++;
    goto start_loop;
    
copy_operation:
    {
        /* This block contains the critical memmove operation */
        char temp[256];
        
        /* Use memmove for overlapping regions */
        __builtin_memmove(temp, nodes[i]->data, 128);
        __builtin_memmove(nodes[i]->data + 128, temp, 128);
        
        /* Jump out of the block */
        i++;
        goto start_loop;
    }
    
end_processing:
    /* Final memory operation before return */
    volatile char cleanup[32];
    __builtin_memset(cleanup, 0, sizeof(cleanup));
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    const int num_threads = 4;
    char thread_buffers[4][256];
    int results[4] = {0};
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread performs different memory operations */
        switch (tid % 3) {
            case 0:
                __builtin_memset(thread_buffers[tid], tid, g_mem_size);
                break;
            case 1:
                if (tid > 0) {
                    __builtin_memcpy(thread_buffers[tid], 
                                   thread_buffers[tid-1], 
                                   g_mem_size);
                }
                break;
            case 2:
                /* Overlapping memmove */
                __builtin_memmove(thread_buffers[tid] + 32,
                                thread_buffers[tid],
                                g_mem_size - 32);
                break;
        }
        
        /* Compute checksum */
        for (size_t j = 0; j < g_mem_size; j++) {
            results[tid] += thread_buffers[tid][j];
        }
    }
    
    /* Verify results */
    int total = 0;
    for (int i = 0; i < num_threads; i++) {
        total += results[i];
    }
    printf("OpenMP checksum: %d\n", total);
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Basic built-in calls */
    printf("\nPhase 1: Basic built-in operations\n");
    {
        char src[256], dest[256];
        
        /* All three built-ins in sequence */
        __builtin_memset(src, 0x42, sizeof(src));
        __builtin_memcpy(dest, src, sizeof(src));
        __builtin_memmove(src + 64, src, 128);
        
        /* Verify with volatile check */
        volatile int check = 0;
        for (size_t i = 0; i < sizeof(dest); i++) {
            check += dest[i];
        }
        printf("Basic ops checksum: %d\n", check);
    }
    
    /* Phase 2: Recursive AST operations */
    printf("\nPhase 2: Recursive AST manipulation\n");
    ASTNode* root = create_ast(4);
    ASTNode* copy = create_ast(3);
    
    if (root && copy) {
        /* Copy data between AST nodes */
        copy_ast_data(copy, root);
        
        /* Process with goto jumps */
        ASTNode* nodes[] = {root, root->left, root->right, copy};
        process_with_goto(nodes, 4);
    }
    
    /* Phase 3: OpenMP parallel operations */
    printf("\nPhase 3: OpenMP parallel memory operations\n");
    parallel_memory_operations();
    
    /* Phase 4: Mixed operations in loops */
    printf("\nPhase 4: Mixed operations with volatile control\n");
    {
        volatile size_t sizes[] = {16, 32, 64, 128};
        char buffers[4][256];
        
        for (int i = 0; i < 4; i++) {
            volatile size_t current_size = sizes[i];
            
            /* Alternate between memset and memcpy */
            if (i % 2 == 0) {
                __builtin_memset(buffers[i], i, current_size);
            } else {
                __builtin_memcpy(buffers[i], buffers[i-1], current_size);
            }
            
            /* Always use memmove for the last quarter */
            if (i == 3) {
                __builtin_memmove(buffers[i] + 32, buffers[i], 96);
            }
        }
    }
    
    /* Cleanup */
    if (root) free(root);
    if (copy) free(copy);
    
    printf("\n=== Test completed ===\n");
    return 0;
}
