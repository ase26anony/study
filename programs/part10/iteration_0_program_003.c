/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char global_tokens[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    printf("Constructor: Initialized global tokens\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtin memcpy to copy to volatile buffer */
    volatile char cleanup_buf[128];
    __builtin_memcpy((void*)cleanup_buf, global_tokens, 128);
    printf("Destructor: Cleaned up %d bytes\n", 128);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for node initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = depth;
    
    /* Copy data with builtin memcpy */
    size_t copy_len = (size_t)(volatile_len % 256);
    if (copy_len > 255) copy_len = 255;
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data[copy_len] = '\0';
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        char child_data[256];
        __builtin_memset(child_data, 'B' + depth, sizeof(child_data));
        
        /* Jump label for goto */
        create_left:
        node->left = create_ast(depth - 1, child_data);
        
        /* Use goto to create unusual control flow */
        if (volatile_flag && depth > 2) {
            volatile_flag = 0;
            goto create_right;
        }
        
        create_right:
        __builtin_memmove(child_data, node->data, copy_len);
        node->right = create_ast(depth - 2, child_data);
        
        /* Another goto jumping into memmove block */
        if (node->id % 3 == 0) {
            goto memmove_block;
        }
    }
    
    return node;
    
memmove_block:
    /* This block is entered via goto */
    char temp[256];
    __builtin_memmove(temp, node->data, 128);
    __builtin_memcpy(node->data + 128, temp, 128);
    return node;
}

/* Calculate hash of AST */
static unsigned long hash_ast(const ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    const char* p = node->data;
    
    while (*p) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    return hash + hash_ast(node->left) + hash_ast(node->right);
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_buffers = 16;
    char* buffers[num_buffers];
    
    /* Allocate and initialize buffers */
    for (int i = 0; i < num_buffers; i++) {
        buffers[i] = (char*)malloc(1024);
        if (buffers[i]) {
            __builtin_memset(buffers[i], i + '0', 1024);
        }
    }
    
    /* OpenMP parallel region */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread performs memory operations */
        #pragma omp for
        for (int i = 0; i < num_buffers; i++) {
            if (buffers[i]) {
                /* Mix of memory builtins */
                if (i % 3 == 0) {
                    __builtin_memcpy(buffers[i] + 512, buffers[(i + 1) % num_buffers], 512);
                } else if (i % 3 == 1) {
                    __builtin_memset(buffers[i] + 256, 'X', 256);
                } else {
                    __builtin_memmove(buffers[i], buffers[i] + 128, 896);
                }
                
                /* Volatile length control */
                volatile int local_len = volatile_len;
                if (local_len > 0) {
                    __builtin_memcpy(buffers[i] + 768, global_tokens, 
                                   (size_t)(local_len % 256));
                }
            }
        }
        
        /* Thread-specific memory operation */
        char thread_buf[256];
        __builtin_memset(thread_buf, 'T' + thread_id, sizeof(thread_buf));
        
        #pragma omp barrier
        
        /* Copy between thread buffers */
        if (thread_id > 0) {
            int src_thread = (thread_id - 1) % omp_get_num_threads();
            /* Simulated cross-thread copy */
            __builtin_memcpy(thread_buf + 128, thread_buf, 128);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_buffers; i++) {
        if (buffers[i]) {
            free(buffers[i]);
        }
    }
}

/* Complex initialization with goto */
static void initialize_with_goto(void) {
    char buffer1[512];
    char buffer2[512];
    
    /* Initial memset */
    __builtin_memset(buffer1, '1', sizeof(buffer1));
    
    /* Goto jumping into memcpy block */
    if (volatile_flag) {
        goto memcpy_block;
    }
    
    normal_path:
    __builtin_memset(buffer2, '2', sizeof(buffer2));
    return;
    
memcpy_block:
    /* Entered via goto */
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    
    /* Another goto to exit */
    if (volatile_len > 32) {
        goto normal_path;
    }
    
    /* Fall through with memmove */
    __builtin_memmove(buffer1, buffer2, 256);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Initialize with goto patterns */
    initialize_with_goto();
    
    /* Phase 2: Create recursive AST */
    ASTNode* root = create_ast(5, "BaseDataForAST");
    
    /* Phase 3: Perform parallel memory operations */
    parallel_memory_ops();
    
    /* Phase 4: Complex memory operations between AST nodes */
    if (root && root->left && root->right) {
        /* Copy between tree nodes */
        __builtin_memcpy(root->left->data + 64, root->right->data, 128);
        __builtin_memmove(root->right->data, root->left->data, 64);
        
        /* Self-overlapping copy */
        __builtin_memmove(root->data + 32, root->data, 96);
    }
    
    /* Phase 5: Global token manipulation */
    for (int i = 0; i < 8; i++) {
        size_t offset = (i * 512) % sizeof(global_tokens);
        size_t len = (size_t)(volatile_len % 128) + 64;
        
        if (i % 2 == 0) {
            __builtin_memset(global_tokens + offset, 'G' + i, len);
        } else {
            __builtin_memcpy(global_tokens + offset, 
                           global_tokens + (offset - 256 + sizeof(global_tokens)) % sizeof(global_tokens),
                           len);
        }
    }
    
    /* Calculate and print verification result */
    unsigned long total_hash = 0;
    if (root) {
        total_hash = hash_ast(root);
        
        /* Additional memory operation in verification */
        char verify_buf[1024];
        __builtin_memset(verify_buf, 0, sizeof(verify_buf));
        __builtin_memcpy(verify_buf, root->data, 256);
        
        if (root->left) {
            __builtin_memcpy(verify_buf + 256, root->left->data, 256);
        }
        
        /* Hash the verification buffer */
        for (int i = 0; i < 512; i++) {
            total_hash += verify_buf[i] * (i + 1);
        }
    }
    
    printf("Verification hash: %lu\n", total_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real code, you'd want to free the AST recursively */
    
    return 0;
}
