/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_array[1024];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(token_array); i++) {
        token_array[i] = (char)((i * 7) & 0xFF);
    }
    
    /* Use __builtin_memset in constructor */
    __builtin_memset(volatile_dest, 0xAA, sizeof(volatile_dest));
    
    printf("Constructor initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use __builtin_memcpy in destructor */
    char temp[256];
    __builtin_memcpy(temp, volatile_dest, sizeof(volatile_dest));
    printf("Destructor cleaning up\n");
}

/* Recursive parser function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memcpy between token array and node data */
    int offset = id * 64 % sizeof(token_array);
    __builtin_memcpy(node->data, &token_array[offset], 64);
    
    /* Recursive creation with goto for control flow testing */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        
        if (use_goto) {
            create_children:
            /* Jump into block with __builtin_memmove */
            char buffer[128];
            __builtin_memmove(buffer, node->data, 64);
            __builtin_memmove(node->data, buffer, 64);
            
            node->left = create_ast(depth - 1, id * 2);
            node->right = create_ast(depth - 1, id * 2 + 1);
        }
    }
    
    return node;
}

/* Function with complex memory operations */
static void process_ast(ASTNode* node) {
    if (!node) return;
    
    /* Use volatile length for memory operations */
    int len = volatile_len;
    if (len > 64) len = 64;
    
    /* Test all three builtins with volatile control */
    char temp[128];
    
    /* memset */
    __builtin_memset(temp, node->id, len);
    
    /* memcpy with goto jumping out */
    int do_copy = 1;
    if (node->id % 2 == 0) {
        goto skip_copy;
    }
    
    __builtin_memcpy(node->data, temp, len);
    do_copy = 0;
    
    skip_copy:
    if (do_copy) {
        __builtin_memcpy(&node->data[32], &temp[32], len - 32);
    }
    
    /* memmove with overlapping regions */
    __builtin_memmove(&node->data[16], node->data, 32);
    
    /* Recursive processing */
    process_ast(node->left);
    process_ast(node->right);
}

/* Calculate hash of AST */
static unsigned long long ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long long hash = 0;
    for (int i = 0; i < 64; i++) {
        hash = hash * 31 + (unsigned char)node->data[i];
    }
    
    return hash + ast_hash(node->left) + ast_hash(node->right);
}

/* Free AST memory */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

/* Main execution with OpenMP parallel section */
int main(void) {
    printf("Starting ASAN coverage test\n");
    
    /* Create complex AST */
    ASTNode* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process AST in main thread */
    process_ast(root);
    
    /* OpenMP parallel section with memory operations */
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs memory operations */
        char local_buf[256];
        char src_buf[256];
        
        /* Initialize source buffer */
        for (int i = 0; i < sizeof(src_buf); i++) {
            src_buf[i] = (char)((i + thread_id * 17) & 0xFF);
        }
        
        /* Use all three builtins in parallel */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        __builtin_memcpy(&local_buf[64], src_buf, 128);
        __builtin_memmove(&local_buf[32], &local_buf[96], 64);
        
        /* Copy to volatile destination */
        int copy_len = volatile_len;
        if (copy_len > 128) copy_len = 128;
        __builtin_memcpy((void*)volatile_dest, local_buf, copy_len);
        
        #pragma omp barrier
        
        /* Verify copy with another memcpy */
        char verify_buf[256];
        __builtin_memcpy(verify_buf, volatile_dest, copy_len);
    }
    
    /* Calculate and print result */
    unsigned long long hash = ast_hash(root);
    printf("AST hash: %llu\n", hash);
    
    /* Additional memory operations in main */
    char final_buffer[512];
    
    /* Chain of memory operations */
    __builtin_memset(final_buffer, 0xCC, sizeof(final_buffer));
    __builtin_memcpy(&final_buffer[128], token_array, 256);
    __builtin_memmove(&final_buffer[64], &final_buffer[192], 128);
    
    /* Use volatile source */
    __builtin_memcpy((void*)volatile_src, final_buffer, 128);
    
    /* Cleanup */
    free_ast(root);
    
    /* Final verification sum */
    unsigned int sum = 0;
    for (int i = 0; i < sizeof(final_buffer); i++) {
        sum += (unsigned char)final_buffer[i];
    }
    printf("Final buffer sum: %u\n", sum);
    
    return 0;
}
