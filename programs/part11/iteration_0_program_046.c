/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[128];
static volatile char volatile_src[128];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "asan", "hwasan"};
static const int token_count = 5;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < sizeof(volatile_src); i++) {
        volatile_src[i] = (char)(i % 256);
    }
    
    /* Force early built-in usage in constructor */
    __builtin_memset(volatile_dest, 0xA5, sizeof(volatile_dest));
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operation in destructor */
    __builtin_memset(volatile_dest, 0, sizeof(volatile_dest));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use all three built-ins in recursive context */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy token into node data with goto for flow control */
    int token_idx = node->id % token_count;
    
    /* Goto-based control flow around memmove */
    if (token_idx == 2) { /* "memmove" token */
        goto memmove_block;
    }
    
    __builtin_memcpy(node->data, tokens[token_idx], 
                    strlen(tokens[token_idx]));
    goto skip_memmove;
    
memmove_block:
    /* This block tests goto into memory operation */
    char temp[32];
    __builtin_memcpy(temp, tokens[token_idx], strlen(tokens[token_idx]));
    __builtin_memmove(node->data, temp, strlen(tokens[token_idx]));
    
skip_memmove:
    /* Recursive creation with varying lengths */
    int left_len = volatile_len / 2;
    int right_len = volatile_len - left_len;
    
    node->left = create_ast(depth - 1, counter);
    node->right = create_ast(depth - 1, counter);
    
    /* Copy between nodes if both exist */
    if (node->left && node->right) {
        __builtin_memcpy(node->right->data, node->left->data, 
                        sizeof(node->left->data));
    }
    
    return node;
}

/* Calculate hash of AST */
static unsigned long hash_ast(const ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    
    /* Hash node data */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Recursive hash */
    hash += hash_ast(node->left);
    hash += hash_ast(node->right);
    
    return hash;
}

/* Free AST */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[256];
        char local_src[256];
        
        /* Initialize with thread-specific pattern */
        __builtin_memset(local_src, thread_id, sizeof(local_src));
        
        /* All three built-ins in parallel region */
        __builtin_memcpy(local_buf, local_src, sizeof(local_buf));
        
        /* Conditional memmove with goto */
        if (thread_id % 3 == 0) {
            goto do_memmove;
        }
        
        __builtin_memset(local_buf + 128, 0xFF, 64);
        goto skip_parallel_memmove;
        
    do_memmove:
        __builtin_memmove(local_buf + 64, local_buf, 128);
        
    skip_parallel_memmove:
        /* Use volatile length */
        int len = volatile_len;
        if (len > 256) len = 256;
        
        __builtin_memset(local_buf + 192, 0xAA, len % 64);
        
        /* Memory operation with computed length */
        #pragma omp barrier
        
        #pragma omp single
        {
            /* Master thread consolidates */
            __builtin_memcpy((void*)volatile_dest, local_buf, 64);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Basic built-in calls */
    char buffer1[256];
    char buffer2[256];
    
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 128, buffer1, 128);
    
    /* Phase 2: Recursive AST with memory operations */
    int counter = 0;
    ASTNode* root = create_ast(4, &counter);
    
    if (root) {
        unsigned long hash = hash_ast(root);
        printf("AST hash: %lu\n", hash);
        
        /* Copy entire AST to new location */
        ASTNode* root_copy = (ASTNode*)malloc(sizeof(ASTNode));
        if (root_copy) {
            __builtin_memcpy(root_copy, root, sizeof(ASTNode));
            
            /* Recursive copy of children */
            if (root->left) {
                root_copy->left = (ASTNode*)malloc(sizeof(ASTNode));
                if (root_copy->left) {
                    __builtin_memcpy(root_copy->left, root->left, 
                                    sizeof(ASTNode));
                }
            }
            
            free(root_copy);
        }
        
        free_ast(root);
    }
    
    /* Phase 3: Parallel operations */
    parallel_memory_operations();
    
    /* Phase 4: Volatile-based operations */
    int dynamic_len = volatile_len;
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                __builtin_memset((void*)volatile_dest, i, dynamic_len);
                break;
            case 1:
                __builtin_memcpy((void*)volatile_dest, 
                               (void*)volatile_src, 
                               dynamic_len);
                break;
            case 2:
                __builtin_memmove((void*)volatile_dest + 32,
                                (void*)volatile_dest,
                                dynamic_len / 2);
                break;
        }
        
        /* Goto out of switch for flow control */
        if (i == 1) {
            goto early_loop;
        }
    }
    
early_loop:
    /* Phase 5: Final verification */
    int sum = 0;
    for (size_t i = 0; i < sizeof(volatile_dest); i++) {
        sum += volatile_dest[i];
    }
    
    printf("Final checksum: %d\n", sum);
    printf("Test completed.\n");
    
    return 0;
}
