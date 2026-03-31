/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char *volatile_dest = NULL;
static volatile char *volatile_src = NULL;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode *left;
    struct ASTNode *right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    printf("Constructor: Initializing ASAN test environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Create pattern in data using __builtin_memcpy */
    char pattern[32];
    __builtin_memset(pattern, 'A' + (id % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 1) {
        goto create_children;
    }
    
    skip_children:
    node->left = NULL;
    node->right = NULL;
    return node;
    
    create_children:
    if (create_left) {
        node->left = create_ast(depth - 1, id * 2);
        create_left = 0;
        goto create_children;  /* Jump back to create right child */
    } else {
        node->right = create_ast(depth - 1, id * 2 + 1);
        goto skip_children;
    }
}

/* Function with complex memory operations and goto */
static void process_ast(ASTNode *node) {
    if (!node) return;
    
    ASTNode temp;
    
    /* Use volatile-controlled __builtin_memcpy */
    volatile_len = sizeof(temp.data);
    __builtin_memcpy(temp.data, node->data, volatile_len);
    
    /* Conditional goto around memmove */
    if (node->id % 3 == 0) {
        goto skip_memmove;
    }
    
    /* Use __builtin_memmove with overlapping regions */
    char buffer[64];
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, node->data, 16);
    __builtin_memmove(buffer + 8, buffer, 24);  /* Overlapping move */
    
    skip_memmove:
    /* Process children */
    process_ast(node->left);
    process_ast(node->right);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buf[128];
        char src_buf[128];
        
        /* Initialize source with thread-specific pattern */
        __builtin_memset(src_buf, '0' + thread_id, sizeof(src_buf));
        
        /* Use all three builtins in parallel region */
        __builtin_memset(local_buf, 0, sizeof(local_buf));
        __builtin_memcpy(local_buf, src_buf, volatile_len % 64);
        
        /* Conditional memmove with goto */
        if (thread_id % 2 == 0) {
            goto do_memmove;
        } else {
            goto skip_parallel_memmove;
        }
        
        do_memmove:
        __builtin_memmove(local_buf + 32, local_buf, 48);
        
        skip_parallel_memmove:
        /* Use result to prevent optimization */
        local_buf[0] = thread_id;
    }
}

/* Multi-stage initialization with memory builtins */
static char* initialize_buffer(size_t size) {
    char *buffer = (char*)malloc(size);
    if (!buffer) return NULL;
    
    /* Force initialization with all three builtins */
    __builtin_memset(buffer, 0, size);
    
    char init_pattern[256];
    __builtin_memset(init_pattern, 'X', sizeof(init_pattern));
    
    size_t copy_size = size < 256 ? size : 256;
    __builtin_memcpy(buffer, init_pattern, copy_size);
    
    /* Overlapping region adjustment */
    if (size > 128) {
        __builtin_memmove(buffer + 64, buffer, 64);
    }
    
    return buffer;
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Stage 1: Initialize volatile-controlled buffers */
    char *dest = (char*)malloc(256);
    char *src = (char*)malloc(256);
    
    if (!dest || !src) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    volatile_dest = dest;
    volatile_src = src;
    
    /* Force builtin calls with volatile lengths */
    __builtin_memset(volatile_dest, 0, volatile_len);
    __builtin_memset(volatile_src, 'S', volatile_len * 2);
    
    /* Stage 2: Create and process AST */
    ASTNode *root = create_ast(4, 1);
    if (root) {
        process_ast(root);
        
        /* Copy between AST nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->right->data, root->left->data, 32);
        }
    }
    
    /* Stage 3: Parallel operations */
    #ifdef _OPENMP
    printf("Running OpenMP parallel section\n");
    #endif
    parallel_memory_ops();
    
    /* Stage 4: Complex buffer operations */
    char *buffer = initialize_buffer(512);
    if (buffer) {
        /* Chain of memory operations */
        __builtin_memcpy(dest, buffer, 128);
        __builtin_memset(buffer + 128, 'Z', 128);
        __builtin_memmove(dest + 64, dest, 128);
        
        free(buffer);
    }
    
    /* Stage 5: Goto-based flow with memory ops */
    int counter = 0;
    
    start_loop:
    if (counter < 3) {
        char loop_buf[32];
        __builtin_memset(loop_buf, counter + 'a', sizeof(loop_buf));
        
        if (counter == 1) {
            goto skip_iteration;
        }
        
        __builtin_memcpy(dest + counter * 32, loop_buf, 32);
        
        skip_iteration:
        counter++;
        goto start_loop;
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < volatile_len; i++) {
        hash = hash * 31 + dest[i];
    }
    
    printf("Test completed. Verification hash: %lu\n", hash);
    
    /* Cleanup */
    free(dest);
    free(src);
    
    /* TODO: Add AST cleanup function */
    
    return 0;
}
