/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
static const int token_count = sizeof(tokens) / sizeof(tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < 256; i++) {
        volatile_src[i] = (char)(i % 256);
    }
    
    /* Force early built-in usage in constructor */
    __builtin_memset(volatile_dest, 0xAA, sizeof(volatile_dest));
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operations in destructor */
    __builtin_memset(volatile_dest, 0, sizeof(volatile_dest));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use all three built-ins with volatile control */
    int len = volatile_len % 32;
    
    /* memset the node data */
    __builtin_memset(node->data, node->id, sizeof(node->data));
    
    /* memcpy from volatile source */
    __builtin_memcpy(node->data, (void*)volatile_src, len);
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 2 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, counter);
        node->right = NULL;
        return node;
        
    create_children:
        node->left = create_ast(depth - 1, counter);
        node->right = create_ast(depth - 1, counter);
        
        /* memmove between child nodes */
        if (node->left && node->right) {
            __builtin_memmove(node->left->data, node->right->data, 
                            sizeof(node->left->data));
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void test_goto_memmove(void) {
    char buffer1[128], buffer2[128];
    int condition = 1;
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 0x11, sizeof(buffer1));
    __builtin_memset(buffer2, 0x22, sizeof(buffer2));
    
    if (condition) {
        goto jump_into_memmove;
    }
    
    /* Normal path */
    __builtin_memcpy(buffer1, buffer2, 64);
    return;
    
jump_into_memmove:
    /* Jump into memmove operation */
    __builtin_memmove(buffer1, buffer2, volatile_len % 128);
    
    /* Jump out */
    goto exit_function;
    
    /* Unreachable code */
    __builtin_memset(buffer1, 0, sizeof(buffer1));
    
exit_function:
    return;
}

/* Parallel memory operations */
static void parallel_memory_operations(void) {
    const int array_size = 1024;
    char* arrays[4];
    
    /* Allocate arrays */
    for (int i = 0; i < 4; i++) {
        arrays[i] = (char*)malloc(array_size);
        if (!arrays[i]) return;
    }
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread uses different built-ins */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(arrays[thread_id], thread_id, array_size);
                break;
            case 1:
                __builtin_memcpy(arrays[thread_id], arrays[(thread_id + 1) % 4], 
                               volatile_len % array_size);
                break;
            case 2:
                __builtin_memmove(arrays[thread_id], arrays[(thread_id + 2) % 4],
                                volatile_len % array_size);
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Mixed operations after barrier */
        if (thread_id == 0) {
            __builtin_memcpy(arrays[0], arrays[1], 256);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(arrays[i]);
    }
}

/* Calculate hash from AST */
static unsigned long calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    
    /* Hash node data */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Recursive hash calculation */
    hash += calculate_ast_hash(node->left);
    hash += calculate_ast_hash(node->right);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize counter for AST creation */
    int counter = 0;
    
    /* Create complex AST */
    ASTNode* root = create_ast(4, &counter);
    printf("Created AST with %d nodes\n", counter);
    
    /* Test goto with memmove */
    test_goto_memmove();
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Process tokens with memory operations */
    char token_buffer[256] = {0};
    for (int i = 0; i < token_count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Use different built-ins based on token */
        if (i % 3 == 0) {
            __builtin_memcpy(token_buffer + i * 10, tokens[i], len);
        } else if (i % 3 == 1) {
            __builtin_memset(token_buffer + i * 10, tokens[i][0], len);
        } else {
            __builtin_memmove(token_buffer + i * 10, tokens[i], len);
        }
    }
    
    /* Calculate and print verification hash */
    unsigned long ast_hash = calculate_ast_hash(root);
    unsigned long buffer_hash = 0;
    
    for (int i = 0; i < 256; i++) {
        buffer_hash = ((buffer_hash << 5) + buffer_hash) + token_buffer[i];
    }
    
    printf("AST Hash: %lu\n", ast_hash);
    printf("Buffer Hash: %lu\n", buffer_hash);
    printf("Verification: %lu\n", ast_hash ^ buffer_hash);
    
    /* Cleanup */
    free_ast(root);
    
    /* Final memory operation in main */
    __builtin_memset(token_buffer, 0, sizeof(token_buffer));
    
    printf("Test completed successfully.\n");
    return 0;
}
