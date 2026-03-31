/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Constructor: Initializing ASAN environment\n");
    /* Force initialization of memory function redirection */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Destructor: Cleaning up ASAN environment\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) {
        pattern[i] = (char)((id + i) % 256);
    }
    __builtin_memcpy(node->data, pattern, 64);
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = (id % 2 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        return node;
        
    create_children:
        /* This block tests goto into memory operation */
        ASTNode* temp_left = NULL;
        ASTNode* temp_right = NULL;
        
        /* Force __builtin_memmove with goto */
        temp_left = create_ast(depth - 1, id * 2);
        if (temp_left) {
            __builtin_memmove(&node->left, &temp_left, sizeof(ASTNode*));
        }
        
        temp_right = create_ast(depth - 1, id * 2 + 1);
        if (temp_right) {
            __builtin_memmove(&node->right, &temp_right, sizeof(ASTNode*));
        }
    }
    
    return node;
}

/* Function with complex memory operations in OpenMP parallel region */
static void parallel_memory_operations(void) {
    const int num_threads = 4;
    const size_t block_size = (size_t)g_mem_size;
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        char* buffer1 = (char*)malloc(block_size);
        char* buffer2 = (char*)malloc(block_size);
        
        if (buffer1 && buffer2) {
            /* Each thread uses different builtins */
            switch (tid % 3) {
                case 0:
                    __builtin_memset(buffer1, tid, block_size);
                    __builtin_memcpy(buffer2, buffer1, block_size);
                    break;
                case 1:
                    __builtin_memset(buffer2, tid + 1, block_size / 2);
                    __builtin_memmove(buffer1 + block_size/2, buffer2, block_size/2);
                    break;
                case 2:
                    __builtin_memcpy(buffer1, buffer2, block_size);
                    __builtin_memset(buffer2, 0xFF, block_size);
                    break;
            }
            
            /* Verify the operations */
            int sum = 0;
            for (size_t i = 0; i < block_size; i++) {
                sum += buffer1[i];
            }
            
            #pragma omp critical
            {
                printf("Thread %d: buffer sum = %d\n", tid, sum);
            }
        }
        
        free(buffer1);
        free(buffer2);
    }
}

/* Function with goto jumping around memory operations */
static void goto_memory_test(void) {
    char src[128], dst[128];
    volatile int condition = 1;
    
    /* Initialize source */
    for (int i = 0; i < 128; i++) {
        src[i] = (char)i;
    }
    
    /* Complex goto pattern */
    if (condition) {
        goto copy_block;
    }
    
    skip_copy:
    __builtin_memset(dst, 0, 64);
    goto finish;
    
    copy_block:
    __builtin_memcpy(dst, src, 64);
    
    if (condition) {
        goto move_block;
    }
    
    goto skip_copy;
    
    move_block:
    __builtin_memmove(dst + 32, dst, 32);
    goto skip_copy;
    
    finish:
    /* Verify */
    int check = 0;
    for (int i = 0; i < 64; i++) {
        check += dst[i];
    }
    printf("Goto test checksum: %d\n", check);
}

/* Main execution flow */
int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Initialize AST with recursive memory operations */
    printf("\nPhase 1: Creating AST structure\n");
    ASTNode* root = create_ast(3, 1);
    
    if (root) {
        /* Copy between AST nodes */
        ASTNode temp_node;
        __builtin_memcpy(&temp_node, root, sizeof(ASTNode));
        __builtin_memmove(root->data, temp_node.data, sizeof(temp_node.data));
        
        /* Calculate hash of AST data */
        unsigned long hash = 0;
        for (int i = 0; i < 64; i++) {
            hash = hash * 31 + root->data[i];
        }
        printf("AST root hash: %lu\n", hash);
    }
    
    /* Phase 2: Parallel memory operations */
    printf("\nPhase 2: Parallel memory operations\n");
    parallel_memory_operations();
    
    /* Phase 3: Goto-based control flow */
    printf("\nPhase 3: Goto-based memory operations\n");
    goto_memory_test();
    
    /* Phase 4: Direct built-in calls with volatile sizes */
    printf("\nPhase 4: Direct built-in calls\n");
    volatile size_t dynamic_size = g_mem_size;
    char* final_buffer = (char*)malloc(dynamic_size * 2);
    
    if (final_buffer) {
        /* Use all three builtins in sequence */
        __builtin_memset(final_buffer, 0xAA, dynamic_size);
        __builtin_memcpy(final_buffer + dynamic_size, final_buffer, dynamic_size);
        __builtin_memmove(final_buffer, final_buffer + dynamic_size/2, dynamic_size/2);
        
        /* Final verification */
        unsigned long final_sum = 0;
        for (size_t i = 0; i < dynamic_size; i++) {
            final_sum += final_buffer[i];
        }
        printf("Final buffer sum: %lu\n", final_sum);
        
        free(final_buffer);
    }
    
    /* Cleanup AST */
    /* ... cleanup code would go here ... */
    
    printf("\n=== Test completed ===\n");
    return 0;
}
