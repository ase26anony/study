/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static volatile char g_token_pool[4096];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force memset initialization in constructor context */
    __builtin_memset((void*)g_token_pool, 0xAA, sizeof(g_token_pool));
    printf("Constructor: Initialized token pool\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Force memset in destructor context */
    __builtin_memset((void*)g_token_pool, 0xFF, 128);
    printf("Destructor: Cleaned token pool\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Use memcpy to copy pattern into data */
    const char pattern[] = "AST_Node_Data_Pattern_For_Sanitizer_Testing";
    __builtin_memcpy(node->data, pattern, 
                    sizeof(pattern) < sizeof(node->data) ? 
                    sizeof(pattern) : sizeof(node->data));
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        
        if (use_goto) {
            create_children:
            /* Additional memcpy between nodes if they exist */
            if (node->left && node->right) {
                __builtin_memcpy(node->right->data, 
                               node->left->data, 
                               sizeof(node->data));
            }
        }
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* node1, ASTNode* node2) {
    volatile int flag = 1;
    
    if (node1 && node2) {
        if (flag) {
            goto perform_memmove;
        }
        
        /* Normal path */
        __builtin_memcpy(node2->data, node1->data, sizeof(node1->data));
        return;
        
        perform_memmove:
        /* Jump target with memmove */
        __builtin_memmove(node2->data, node1->data, sizeof(node1->data));
        
        /* Jump out of block */
        goto after_operation;
    }
    
    after_operation:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[256];
        volatile size_t local_size = g_mem_size;
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, local_size);
        
        #pragma omp barrier
        
        /* Some threads use memcpy, others memmove */
        if (tid % 2 == 0) {
            char dest[256];
            __builtin_memcpy(dest, local_buf, local_size);
            
            /* Verify copy */
            if (__builtin_memcmp(dest, local_buf, local_size) != 0) {
                #pragma omp critical
                printf("Thread %d: memcpy verification failed\n", tid);
            }
        } else {
            char dest[256];
            __builtin_memmove(dest, local_buf, local_size);
            
            /* Verify move */
            if (__builtin_memcmp(dest, local_buf, local_size) != 0) {
                #pragma omp critical
                printf("Thread %d: memmove verification failed\n", tid);
            }
        }
    }
}

/* Multi-stage processing function */
static unsigned long process_ast(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 0;
    
    /* Compute hash from node data */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        hash = hash * 31 + (unsigned char)node->data[i];
    }
    
    /* Recursive processing */
    hash += process_ast(node->left);
    hash += process_ast(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Initialize and create AST */
    ASTNode* root = create_ast(4, 1);
    ASTNode* copy = create_ast(3, 100);
    
    if (!root || !copy) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Phase 2: Control flow with goto */
    process_with_goto(root, copy);
    
    /* Phase 3: Direct builtin calls with volatile sizes */
    volatile size_t copy_size = 128;
    __builtin_memcpy(copy->data, root->data, copy_size);
    
    /* Conditional memmove */
    if (g_use_memmove) {
        char temp[256];
        __builtin_memmove(temp, root->data, sizeof(root->data));
        __builtin_memmove(root->data, temp, sizeof(root->data));
    }
    
    /* Phase 4: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Phase 5: Process and verify */
    unsigned long root_hash = process_ast(root);
    unsigned long copy_hash = process_ast(copy);
    
    printf("Root AST hash: %lu\n", root_hash);
    printf("Copy AST hash: %lu\n", copy_hash);
    printf("Hash difference: %ld\n", (long)(root_hash - copy_hash));
    
    /* Additional stress: array operations */
    volatile int array_size = 1024;
    int* arr1 = (int*)malloc(array_size * sizeof(int));
    int* arr2 = (int*)malloc(array_size * sizeof(int));
    
    if (arr1 && arr2) {
        __builtin_memset(arr1, 0xCC, array_size * sizeof(int));
        __builtin_memcpy(arr2, arr1, array_size * sizeof(int));
        
        /* Overlapping memmove */
        __builtin_memmove(arr1 + 100, arr1, 200 * sizeof(int));
        
        free(arr1);
        free(arr2);
    }
    
    /* Cleanup */
    /* Note: Proper AST cleanup omitted for brevity */
    
    printf("Test completed successfully\n");
    return 0;
}
