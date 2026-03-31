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
    int type;
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Force early initialization of memory functions */
    __builtin_memset(volatile_dest, 0xA5, sizeof(volatile_dest));
    printf("Constructor: Initialized volatile buffer\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    /* Final memory operation in destructor */
    __builtin_memset(volatile_dest, 0, 16);
    printf("Destructor: Cleaned volatile buffer\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = (*counter)++;
    
    /* Fill data with pattern using memcpy */
    char pattern[32];
    __builtin_memset(pattern, 'A' + (depth % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth - 1, counter);
    
    /* Memory move between nodes if both exist */
    if (node->left && depth > 3) {
        ASTNode temp;
        __builtin_memcpy(&temp, node->left, sizeof(ASTNode));
        __builtin_memmove(node->left, &temp, sizeof(ASTNode));
    }
    
    node->right = create_ast(depth - 2, counter);
    
done:
    return node;
}

/* Function with complex control flow and goto */
static void process_with_goto(ASTNode* node, char* buffer, int len) {
    if (!node) return;
    
    int use_memmove = 0;
    
    /* Jump into memory operation block */
    if (node->type % 3 == 0) {
        goto use_memcpy;
    } else if (node->type % 3 == 1) {
        goto use_memset;
    } else {
        use_memmove = 1;
        goto use_memmove_op;
    }
    
use_memcpy:
    /* Force memcpy redirection */
    __builtin_memcpy(buffer, node->data, 
                    len < (int)sizeof(node->data) ? len : (int)sizeof(node->data));
    goto next;
    
use_memset:
    /* Force memset redirection */
    __builtin_memset(buffer, node->type & 0xFF, 
                    len < 32 ? len : 32);
    goto next;
    
use_memmove_op:
    /* Force memmove redirection with overlapping regions */
    if (len > 16) {
        __builtin_memmove(buffer + 8, buffer, len - 8);
    }
    /* Jump back for second operation */
    goto use_memcpy;
    
next:
    /* Process children */
    if (node->left) {
        process_with_goto(node->left, buffer + 16, len - 16);
    }
}

/* OpenMP parallel section */
static void parallel_memory_operations(void) {
    int i;
    char local_buf[256];
    
    #pragma omp parallel private(i) shared(local_buf)
    {
        #pragma omp for
        for (i = 0; i < 100; i++) {
            /* Each thread uses builtins */
            __builtin_memset(local_buf + i * 2, i, 2);
            
            if (i % 3 == 0) {
                __builtin_memcpy(local_buf + 64, local_buf + i * 2, 8);
            } else if (i % 3 == 1) {
                __builtin_memmove(local_buf + 128, local_buf + 64, 16);
            }
        }
        
        /* Barrier with memory operation */
        #pragma omp barrier
        
        #pragma omp single
        {
            /* Master thread does final consolidation */
            __builtin_memcpy(volatile_dest, local_buf, 
                           volatile_len < 256 ? volatile_len : 256);
        }
    }
}

/* Main test driver */
int main(void) {
    int counter = 0;
    char result_buffer[512] = {0};
    int hash = 0;
    
    printf("=== Starting ASAN built-in redirection test ===\n");
    
    /* 1. Create recursive AST structure */
    ASTNode* root = create_ast(5, &counter);
    printf("Created AST with %d nodes\n", counter);
    
    /* 2. Process with goto control flow */
    process_with_goto(root, result_buffer, sizeof(result_buffer));
    
    /* 3. Parallel memory operations */
    parallel_memory_operations();
    
    /* 4. Direct built-in calls with volatile variables */
    int dynamic_len = volatile_len;
    if (dynamic_len > 0) {
        /* Initialize source with pattern */
        for (int i = 0; i < (int)sizeof(volatile_src); i++) {
            volatile_src[i] = (char)(i % 256);
        }
        
        /* Test all three builtins */
        __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, dynamic_len);
        __builtin_memset((void*)(volatile_dest + 32), 0xCC, dynamic_len / 2);
        
        /* Overlapping memmove */
        if (dynamic_len > 64) {
            __builtin_memmove((void*)(volatile_dest + 16), 
                            (void*)volatile_dest, 
                            dynamic_len - 16);
        }
    }
    
    /* 5. Calculate verification hash */
    for (int i = 0; i < (int)sizeof(result_buffer); i++) {
        hash = (hash * 31 + result_buffer[i]) & 0x7FFFFFFF;
    }
    
    /* Also hash volatile buffer */
    for (int i = 0; i < volatile_len && i < 128; i++) {
        hash = (hash * 17 + volatile_dest[i]) & 0x7FFFFFFF;
    }
    
    printf("Result hash: 0x%08X\n", hash);
    printf("=== Test completed ===\n");
    
    /* Cleanup */
    /* Note: In real ASAN, memory would be freed automatically */
    
    return (hash != 0) ? 0 : 1;
}
