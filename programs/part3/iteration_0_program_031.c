/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char *volatile_dest = NULL;
static volatile const char *volatile_src = NULL;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode *left;
    struct ASTNode *right;
    int id;
} ASTNode;

/* Global token array */
static char global_tokens[256];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize global tokens with pattern */
    for (int i = 0; i < sizeof(global_tokens); i++) {
        global_tokens[i] = (i % 26) + 'a';
    }
    
    /* Force early built-in calls in constructor */
    char local_buf[32];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    __builtin_memcpy(local_buf + 16, "constructor", 11);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Final memory operations in destructor */
    char final_buf[64];
    __builtin_memset(final_buf, 0xFF, 64);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with built-ins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data */
    char pattern[16];
    __builtin_memset(pattern, 'A' + (id % 26), 15);
    pattern[15] = '\0';
    __builtin_memcpy(node->data, pattern, 16);
    
    node->id = id;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast_node(depth - 1, id * 2);
        node->right = create_ast_node(depth - 1, id * 2 + 1);
        
        if (use_goto) {
            create_children:
            /* Jump into block with memmove */
            char temp[32];
            __builtin_memmove(temp, node->data, 16);
            __builtin_memmove(node->data + 16, temp, 16);
            
            node->left = create_ast_node(depth - 1, id * 2);
            node->right = create_ast_node(depth - 1, id * 2 + 1);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Copy between AST nodes with goto jumps */
static void copy_ast_data(ASTNode *dest, const ASTNode *src) {
    if (!dest || !src) return;
    
    int use_complex_copy = (dest->id % 4 == 0);
    
    if (use_complex_copy) {
        goto complex_copy_path;
    }
    
    /* Simple copy path */
    __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    return;
    
complex_copy_path:
    /* Complex path with goto and memmove */
    char buffer[64];
    __builtin_memset(buffer, 0, sizeof(buffer));
    
    /* Jump around different memory operations */
    if (dest->id % 2 == 0) {
        goto do_memmove;
    }
    
    __builtin_memcpy(buffer, src->data, 32);
    goto finish_copy;
    
do_memmove:
    __builtin_memmove(buffer, src->data, 32);
    
finish_copy:
    __builtin_memcpy(dest->data, buffer, 32);
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char thread_buf[128];
        char thread_src[128];
        
        /* Initialize with built-ins */
        __builtin_memset(thread_buf, thread_id, sizeof(thread_buf));
        __builtin_memset(thread_src, 'X' + thread_id, sizeof(thread_src));
        
        /* Mix of memory operations */
        for (int i = 0; i < 10; i++) {
            switch (i % 3) {
                case 0:
                    __builtin_memcpy(thread_buf + i * 8, thread_src, 8);
                    break;
                case 1:
                    __builtin_memset(thread_buf + i * 8, i, 8);
                    break;
                case 2:
                    __builtin_memmove(thread_buf + i * 8, thread_buf, 8);
                    break;
            }
        }
        
        /* Use volatile variables */
        int len = volatile_len / (thread_id + 1);
        if (len > 0) {
            __builtin_memcpy(thread_buf + 64, thread_src, len);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create AST structures */
    ASTNode *root1 = create_ast_node(4, 1);
    ASTNode *root2 = create_ast_node(3, 100);
    
    if (!root1 || !root2) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Copy between structures */
    copy_ast_data(root2, root1);
    
    /* Test with volatile-controlled operations */
    char *dest = malloc(volatile_len * 2);
    const char *src = global_tokens;
    
    if (dest && src) {
        volatile_dest = dest;
        volatile_src = src;
        
        /* Force multiple built-in calls */
        __builtin_memset(dest, 0, volatile_len * 2);
        __builtin_memcpy(dest, src, volatile_len);
        __builtin_memmove(dest + volatile_len, dest, volatile_len / 2);
        
        /* Verify with another memset */
        __builtin_memset(dest + volatile_len * 3 / 2, 0xCC, volatile_len / 2);
    }
    
    /* Execute parallel operations */
    parallel_memory_ops();
    
    /* Compute verification hash */
    unsigned long hash = 0;
    if (dest) {
        for (int i = 0; i < volatile_len * 2; i++) {
            hash = hash * 31 + dest[i];
        }
        free(dest);
    }
    
    /* Cleanup AST */
    free(root1);
    free(root2);
    
    printf("Test completed. Hash: %lu\n", hash);
    printf("Check ASAN instrumentation in compiler output\n");
    
    return 0;
}
