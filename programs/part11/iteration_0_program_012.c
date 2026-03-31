/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[32];
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_globals(void) {
    g_init_flag = 1;
    printf("Constructor: Global initialization complete\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_resources(void) {
    printf("Destructor: Cleaning up resources\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data using __builtin_memcpy */
    char pattern[32];
    for (int i = 0; i < 32; i++) {
        pattern[i] = (char)(node->id + i);
    }
    __builtin_memcpy(node->data, pattern, 32);
    
    /* Recursive creation with goto for control flow */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth - 1, counter);
    
    /* Use __builtin_memmove to shift data before creating right child */
    char temp[32];
    __builtin_memcpy(temp, node->data, 32);
    __builtin_memmove(node->data + 8, node->data, 24);
    __builtin_memcpy(node->data, temp + 8, 8);
    
    node->right = create_ast(depth - 2, counter);
    
done:
    return node;
}

/* Function with complex control flow using goto */
static void process_with_goto(ASTNode* node, char* buffer, size_t size) {
    if (!node) return;
    
    int stage = 0;
    
start:
    if (stage == 0) {
        /* First memory operation */
        __builtin_memset(buffer, 0xA5, size);
        stage = 1;
        goto middle;
    }
    
    if (stage == 2) {
        /* This should be reached via goto from middle */
        __builtin_memcpy(buffer + 16, node->data, 16);
        return;
    }
    
middle:
    /* Jump around memory operation */
    if (node->id % 2 == 0) {
        goto skip_memmove;
    }
    
    /* Use __builtin_memmove with overlapping regions */
    __builtin_memmove(buffer + 8, buffer, size - 8);
    
skip_memmove:
    stage = 2;
    goto start;
}

/* Parallel processing function */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[128];
        char src_buf[128];
        
        /* Initialize source buffer */
        for (int i = 0; i < 128; i++) {
            src_buf[i] = (char)(thread_id + i);
        }
        
        /* Use all three builtins in parallel region */
        __builtin_memset(local_buf, thread_id, 128);
        __builtin_memcpy(local_buf + 32, src_buf + 32, 64);
        
        /* Conditional memmove based on thread ID */
        if (thread_id % 3 == 0) {
            __builtin_memmove(local_buf + 16, local_buf, 48);
        }
        
        /* Use volatile to prevent optimization */
        volatile char* vptr = local_buf;
        (void)vptr; /* Suppress unused warning */
    }
}

/* Multi-stage processing with different memory operations */
static unsigned long process_ast(ASTNode* root) {
    if (!root) return 0;
    
    unsigned long hash = 0;
    char buffer[256];
    volatile size_t op_size = g_mem_size;
    
    /* Stage 1: Clear buffer */
    __builtin_memset(buffer, 0, sizeof(buffer));
    
    /* Stage 2: Copy AST data with goto control flow */
    process_with_goto(root, buffer, op_size);
    
    /* Stage 3: Process recursively */
    hash = root->id * 31;
    
    /* Copy between left and right children if both exist */
    if (root->left && root->right) {
        __builtin_memcpy(root->left->data + 8, root->right->data + 8, 16);
        __builtin_memmove(root->right->data, root->left->data, 16);
    }
    
    /* Recursive processing */
    hash += process_ast(root->left);
    hash += process_ast(root->right);
    
    /* Final memory operation on buffer */
    for (size_t i = 0; i < sizeof(buffer); i++) {
        hash += (unsigned char)buffer[i];
    }
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free (security practice) */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    if (!g_init_flag) {
        fprintf(stderr, "Error: Constructor not called\n");
        return 1;
    }
    
    /* Create recursive AST structure */
    int counter = 1;
    ASTNode* root = create_ast(5, &counter);
    
    if (!root) {
        fprintf(stderr, "Error: Failed to create AST\n");
        return 1;
    }
    
    printf("Created AST with %d nodes\n", counter - 1);
    
    /* Execute parallel memory operations */
    printf("Running parallel memory operations...\n");
    parallel_memory_ops();
    
    /* Process AST with complex memory operations */
    printf("Processing AST with memory operations...\n");
    unsigned long result_hash = process_ast(root);
    
    printf("Result hash: %lu\n", result_hash);
    
    /* Verify with additional memory operations */
    char verify_buf[128];
    volatile size_t verify_size = 64;
    
    __builtin_memset(verify_buf, 0xCC, verify_size);
    __builtin_memcpy(verify_buf + 32, root->data, 16);
    __builtin_memmove(verify_buf, verify_buf + 16, 32);
    
    /* Add verification hash */
    for (int i = 0; i < 64; i++) {
        result_hash += (unsigned char)verify_buf[i];
    }
    
    printf("Final verification hash: %lu\n", result_hash);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
