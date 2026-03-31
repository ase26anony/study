/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 128;  /* Force non-constant size */
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive function using builtins with goto */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to prevent constant folding */
    volatile size_t local_size = g_mem_size / (depth + 1);
    
    /* Initialize with memset (builtin) */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Label for goto testing */
    init_left:
    node->left = create_ast(depth - 1, counter);
    
    /* Copy counter value using memcpy (builtin) */
    char temp[16];
    __builtin_memcpy(temp, counter, sizeof(int));
    
    /* Jump to avoid optimization */
    if (depth > 2) goto skip_right;
    
    node->right = create_ast(depth - 1, counter);
    goto done;
    
    skip_right:
    node->right = NULL;
    
    done:
    /* Use memmove (builtin) to shift data */
    __builtin_memmove(node->data + 16, node->data, local_size);
    
    (*counter)++;
    return node;
}

/* Function with complex control flow */
static void process_ast(ASTNode* node, int depth) {
    if (!node) return;
    
    volatile int use_goto = depth % 3;
    
    if (use_goto == 0) {
        goto process_left;
    } else if (use_goto == 1) {
        goto process_right;
    } else {
        goto process_both;
    }
    
    process_left:
    __builtin_memcpy(node->data, "LEFT", 5);
    if (node->left) {
        process_ast(node->left, depth + 1);
    }
    if (use_goto == 0) goto end;
    
    process_right:
    __builtin_memcpy(node->data + 64, "RIGHT", 6);
    if (node->right) {
        process_ast(node->right, depth + 1);
    }
    if (use_goto == 1) goto end;
    
    process_both:
    /* Move data around with memmove */
    __builtin_memmove(node->data + 32, node->data, 32);
    process_ast(node->left, depth + 1);
    process_ast(node->right, depth + 1);
    
    end:
    /* Final memset */
    __builtin_memset(node->data + 128, 0xFF, 32);
}

/* Parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char buffer1[256];
        char buffer2[256];
        
        /* Initialize with builtins */
        __builtin_memset(buffer1, thread_id, sizeof(buffer1));
        __builtin_memset(buffer2, 0, sizeof(buffer2));
        
        /* Copy between buffers */
        __builtin_memcpy(buffer2, buffer1, g_mem_size);
        
        /* Move data around */
        __builtin_memmove(buffer1 + 128, buffer1, 128);
        
        /* Verify with volatile access */
        volatile char check = buffer1[64];
        (void)check;  /* Prevent unused warning */
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    int counter = 0;
    
    /* Create recursive structure */
    ASTNode* root = create_ast(4, &counter);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    printf("Created AST with %d nodes\n", counter);
    
    /* Process with control flow variations */
    process_ast(root, 0);
    
    /* Execute parallel memory operations */
    #ifdef _OPENMP
    printf("Running parallel memory operations\n");
    #endif
    parallel_mem_ops();
    
    /* Additional builtin usage in main */
    char final_buffer[512];
    volatile size_t final_size = g_mem_size * 2;
    
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, root->data, 256);
    __builtin_memmove(final_buffer + 256, final_buffer, 256);
    
    /* Compute simple hash for verification */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        hash = hash * 31 + final_buffer[i];
    }
    
    printf("Final hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real code, would need recursive free function */
    
    return 0;
}
