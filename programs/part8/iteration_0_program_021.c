/* ISO C99-compliant test program for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Global token array */
static char token_pool[1024];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)((i * 13) & 0xFF);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    
    /* Fill data with tokens using __builtin_memcpy */
    size_t copy_size = (depth < 32) ? (size_t)depth : 32;
    __builtin_memcpy(node->data, &token_pool[token_index], copy_size);
    token_index = (token_index + 16) % 1024;
    
    /* Recursive calls */
    node->left = parse_expression(depth - 1);
    
    /* Jump label for goto testing */
    if (node->left) {
        volatile int flag = 1;
        if (flag) goto copy_right;
    }
    
    node->right = NULL;
    goto skip_copy;
    
copy_right:
    /* This goto tests flow-sensitivity of ASAN logic */
    if (node->left) {
        /* Copy between nodes using __builtin_memmove */
        __builtin_memmove(node->right, node->left, sizeof(ASTNode));
    }
    
skip_copy:
    node->right = parse_expression(depth - 2);
    
    return node;
}

/* Parallel memory operations */
static void parallel_mem_operations(void) {
    volatile char buffer1[256];
    volatile char buffer2[256];
    volatile char buffer3[256];
    
    /* Initialize with __builtin_memset */
    __builtin_memset((void*)buffer1, 0xAA, sizeof(buffer1));
    __builtin_memset((void*)buffer2, 0xBB, sizeof(buffer2));
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                /* Use volatile size to prevent folding */
                __builtin_memcpy((void*)buffer3, (void*)buffer1, g_mem_size);
                break;
            case 1:
                __builtin_memmove((void*)buffer2, (void*)buffer3, g_mem_size / 2);
                break;
            case 2:
                __builtin_memset((void*)&buffer1[128], thread_id, g_mem_size / 4);
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Additional memory operation after barrier */
        if (thread_id == 0) {
            __builtin_memcpy((void*)buffer3, (void*)buffer2, g_mem_size);
        }
    }
}

/* Complex memory stress test */
static void memory_stress_test(void) {
    /* Multi-dimensional array for complex access patterns */
    char matrix[8][64];
    volatile int indices[8] = {0, 8, 16, 24, 32, 40, 48, 56};
    
    /* Nested loops with memory operations */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Conditional goto for edge case testing */
            if (i == 3 && j == 3) {
                goto special_case;
            }
            
            /* Regular memory copy */
            __builtin_memcpy(&matrix[i][indices[j]], 
                           &token_pool[(i * j) % 1024], 
                           8);
            continue;
            
        special_case:
            /* Special case with memmove */
            __builtin_memmove(&matrix[i][0], &matrix[0][0], 32);
            
            /* Jump back */
            goto continue_loop;
        }
    continue_loop:
        /* Additional memset */
        __builtin_memset(&matrix[i][32], i, 16);
    }
}

/* Calculate hash of AST tree */
static unsigned long calculate_tree_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    
    /* Hash node data */
    for (int i = 0; i < 32; i++) {
        hash = ((hash << 5) + hash) + (unsigned long)node->data[i];
    }
    
    /* Recursive hash calculation */
    hash += calculate_tree_hash(node->left);
    hash += calculate_tree_hash(node->right);
    
    return hash;
}

/* Main function with execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive parsing with memory operations */
    ASTNode* root = parse_expression(5);
    
    /* Phase 2: Parallel memory operations */
    parallel_mem_operations();
    
    /* Phase 3: Memory stress test */
    memory_stress_test();
    
    /* Phase 4: Verify results */
    unsigned long final_hash = 0;
    if (root) {
        final_hash = calculate_tree_hash(root);
        
        /* Additional memory operation on tree */
        ASTNode temp_node;
        __builtin_memcpy(&temp_node, root, sizeof(ASTNode));
        __builtin_memset(root->data, 0xFF, 16);
        __builtin_memmove(root, &temp_node, sizeof(ASTNode));
        
        /* Free tree */
        /* Note: In real ASAN, this would trigger use-after-free detection */
    }
    
    /* Final verification */
    volatile int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += token_pool[i];
    }
    
    printf("Test completed: Hash = %lu, Checksum = %d\n", 
           final_hash, checksum);
    
    return 0;
}
