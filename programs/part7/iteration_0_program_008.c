#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* volatile to prevent optimization */
    struct ASTNode *left;
    struct ASTNode *right;
    char padding[32];      /* Ensure size for memcpy operations */
} ASTNode;

/* Global volatile variables to control memory operations */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    /* Force initialization of ASAN runtime */
    volatile char buffer[128];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    /* Final memory operation to ensure coverage */
    volatile int final_check[16];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int value) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile size to prevent constant folding */
    volatile size_t node_size = sizeof(ASTNode) - 8;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = value;
    node->size = node_size;
    
    /* Create children with different memory patterns */
    node->left = create_ast(depth - 1, value * 2);
    node->right = create_ast(depth - 1, value * 2 + 1);
    
    /* Copy data between nodes if children exist */
    if (node->left && node->right) {
        /* Use goto to create control flow edge cases */
        if (g_use_memmove) {
            goto use_memmove_block;
        } else {
            goto use_memcpy_block;
        }
        
    use_memmove_block:
        /* Jump into block with memmove */
        __builtin_memmove(&node->value, &node->left->value, sizeof(int));
        goto after_copy;
        
    use_memcpy_block:
        /* Alternative path with memcpy */
        __builtin_memcpy(&node->value, &node->right->value, sizeof(int));
        goto after_copy;
        
    after_copy:
        /* Additional operation after goto */
        node->type += 1;
    }
    
    return node;
}

/* Function with complex memory operations */
static void process_ast(ASTNode* root, int* result) {
    if (!root) return;
    
    /* Local volatile to prevent optimization */
    volatile char temp_buffer[256];
    
    /* Initialize temp buffer */
    __builtin_memset(temp_buffer, root->value, g_mem_size);
    
    /* Copy node data to buffer */
    __builtin_memcpy(temp_buffer, root, 
                    root->size < sizeof(temp_buffer) ? root->size : sizeof(temp_buffer));
    
    /* Process children recursively */
    process_ast(root->left, result);
    process_ast(root->right, result);
    
    /* Update result */
    *result += root->value;
    
    /* Conditional memmove based on value */
    if (root->value % 3 == 0) {
        char buffer2[256];
        __builtin_memmove(buffer2, temp_buffer, g_mem_size);
        *result += buffer2[0];
    }
}

/* Function to free AST */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    volatile size_t clear_size = sizeof(ASTNode);
    __builtin_memset(node, 0, clear_size);
    free(node);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char buffer1[512];
        char buffer2[512];
        
        /* Initialize with memset */
        __builtin_memset(buffer1, thread_id, sizeof(buffer1));
        
        /* Copy between buffers */
        __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
        
        /* Conditional memmove */
        if (thread_id % 2 == 0) {
            __builtin_memmove(buffer1 + 128, buffer1, 256);
        }
        
        /* Use volatile to prevent dead code elimination */
        volatile int check = buffer1[0] + buffer2[255];
        (void)check;  /* Suppress unused warning */
    }
}

/* Main test driver */
int main(void) {
    int result = 0;
    
    printf("Starting ASAN memory operation tests...\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast(4, 1);
    if (root) {
        process_ast(root, &result);
        
        /* Additional memory operation on entire tree */
        ASTNode* root_copy = (ASTNode*)malloc(sizeof(ASTNode));
        if (root_copy) {
            __builtin_memcpy(root_copy, root, sizeof(ASTNode));
            result += root_copy->value;
            free(root_copy);
        }
        
        free_ast(root);
    }
    
    /* Phase 2: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 3: Direct builtin calls with volatile control */
    {
        volatile char src[1024];
        volatile char dst[1024];
        volatile size_t op_size = g_mem_size;
        
        /* Pattern of memory operations */
        for (int i = 0; i < 3; i++) {
            __builtin_memset(src, i, op_size);
            __builtin_memcpy(dst, src, op_size);
            
            if (i == 1) {
                __builtin_memmove(dst + 64, dst, 128);
            }
            
            op_size += 32;  /* Change size each iteration */
        }
        
        /* Verify by computing checksum */
        for (size_t i = 0; i < sizeof(dst); i++) {
            result += dst[i];
        }
    }
    
    /* Phase 4: Array operations with gotos */
    {
        int array1[100];
        int array2[100];
        volatile int use_goto = 1;
        
        __builtin_memset(array1, 0xAA, sizeof(array1));
        
        if (use_goto) {
            goto copy_block;
        }
        
        __builtin_memset(array2, 0xBB, sizeof(array2));
        goto end_block;
        
    copy_block:
        __builtin_memcpy(array2, array1, sizeof(array1));
        
        /* Nested goto */
        if (array2[0] == 0xAA) {
            goto move_block;
        }
        goto end_block;
        
    move_block:
        __builtin_memmove(array1 + 50, array1, 50 * sizeof(int));
        
    end_block:
        /* Add to result */
        result += array1[0] + array2[99];
    }
    
    printf("Test completed. Result checksum: %d\n", result);
    
    /* Final memory operation in main */
    volatile int final_array[10];
    __builtin_memset(final_array, result & 0xFF, sizeof(final_array));
    
    return 0;
}
