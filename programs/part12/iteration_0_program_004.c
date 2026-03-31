/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

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
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan",
    "instrument", "redzone", "builtin", "coverage", "test"
};
static const int token_count = 10;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < sizeof(volatile_src); i++) {
        volatile_src[i] = (char)(i % 256);
    }
    
    /* Force early builtin calls in constructor */
    __builtin_memset(volatile_dest, 0xAA, sizeof(volatile_dest));
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final builtin calls in destructor */
    __builtin_memset(volatile_dest, 0xFF, 16);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int* node_id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*node_id)++;
    
    /* Use builtins with volatile lengths */
    int len = volatile_len % 32;
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy token data using builtin */
    const char* token = tokens[node->id % token_count];
    __builtin_memcpy(node->data, token, strlen(token));
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth - 1, node_id);
    
    /* Jump back into block with memmove */
    if (node->left) {
        char temp[32];
        __builtin_memcpy(temp, node->left->data, sizeof(temp));
        __builtin_memmove(node->data, temp, 16);
    }
    
    node->right = create_ast(depth - 1, node_id);
    
done:
    return node;
}

/* Complex memory operation with goto jumps */
static void complex_memory_operations(void) {
    char buffer1[256];
    char buffer2[256];
    char buffer3[256];
    
    /* Initialize buffers */
    for (int i = 0; i < 256; i++) {
        buffer1[i] = (char)(i);
        buffer2[i] = (char)(255 - i);
    }
    
    int use_memmove = 1;
    
    /* Goto jumping into block with builtin */
    if (use_memmove) {
        goto do_memmove;
    }
    
    __builtin_memset(buffer3, 0, sizeof(buffer3));
    goto after_memmove;
    
do_memmove:
    /* This block should trigger memmove redirection */
    __builtin_memmove(buffer3, buffer1, volatile_len % 128);
    
    /* Jump out and back in */
    if (buffer3[0] > 100) {
        goto after_memmove;
    }
    
    /* More builtin calls */
    __builtin_memcpy(buffer1, buffer2, 64);
    
after_memmove:
    /* Final builtin in normal flow */
    __builtin_memset(buffer2, 0xCC, 128);
}

/* Parallel memory dispatch */
static void parallel_memory_dispatch(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[64];
        char shared_buf[64];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Critical section with memory operations */
        #pragma omp critical
        {
            __builtin_memcpy(shared_buf, local_buf, 32);
            
            /* Nested builtin call */
            if (thread_id % 2 == 0) {
                __builtin_memmove(local_buf, shared_buf, 16);
            }
        }
        
        /* More builtin after critical section */
        __builtin_memset(local_buf + 32, 0xFF, 16);
    }
}

/* Calculate hash from AST */
static unsigned long calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    
    /* Hash node data */
    for (int i = 0; i < 32; i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Recursive hash calculation */
    hash += calculate_ast_hash(node->left);
    hash += calculate_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Initialize node counter */
    int node_id = 0;
    
    /* Create recursive AST structure */
    ASTNode* root = create_ast(4, &node_id);
    
    /* Perform complex memory operations with goto */
    complex_memory_operations();
    
    /* Execute parallel memory operations */
    parallel_memory_dispatch();
    
    /* Additional builtin calls in main */
    char final_buffer[512];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    
    /* Copy between AST nodes if they exist */
    if (root && root->left) {
        __builtin_memcpy(root->right->data, root->left->data, 32);
        __builtin_memmove(root->data, root->right->data, 16);
    }
    
    /* Calculate and print verification result */
    unsigned long hash = calculate_ast_hash(root);
    printf("AST Hash: %lu\n", hash);
    printf("Nodes created: %d\n", node_id);
    
    /* Cleanup */
    /* Note: In real code, you'd want to free the AST properly */
    
    printf("Test completed.\n");
    return 0;
}
