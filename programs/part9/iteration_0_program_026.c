/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char *volatile_ptr;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode *left;
    struct ASTNode *right;
    int id;
} ASTNode;

/* Global token array */
static char token_array[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(token_array); i++) {
        token_array[i] = (char)((i * 7) & 0xFF);
    }
    
    /* Force early built-in usage in constructor */
    char local_buf[128];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    __builtin_memcpy(&token_array[0], local_buf, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Verify memory operations in destructor */
    char check_buf[64];
    __builtin_memset(check_buf, 0, sizeof(check_buf));
    __builtin_memcpy(check_buf, &token_array[3840], 64);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with built-ins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in node data */
    char pattern[32];
    __builtin_memset(pattern, (char)id, sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(pattern));
    
    node->id = id;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        
        if (use_goto) {
            create_children:
            /* Memory move with goto context */
            char temp[128];
            __builtin_memmove(temp, node->data, 128);
            __builtin_memmove(node->data + 128, temp, 128);
            
            node->left = create_ast(depth - 1, id * 2);
            node->right = create_ast(depth - 1, id * 2 + 1);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Copy between AST nodes with built-ins */
static void copy_ast_data(ASTNode *dest, ASTNode *src) {
    if (!dest || !src) return;
    
    /* Direct built-in usage */
    __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    
    /* Handle overlapping regions with memmove */
    if (dest == src) {
        __builtin_memmove(dest->data + 128, dest->data, 128);
    }
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
        char local_buf[256];
        char result_buf[256];
        
        /* Initialize with built-ins */
        __builtin_memset(local_buf, (char)thread_id, sizeof(local_buf));
        
        /* Copy from global token array with volatile length */
        int len = volatile_len;
        if (len > sizeof(local_buf)) len = sizeof(local_buf);
        
        __builtin_memcpy(result_buf, &token_array[thread_id * 64], len);
        
        /* Move data around */
        __builtin_memmove(local_buf + 128, local_buf, 128);
        
        /* Store back to global array */
        __builtin_memcpy(&token_array[thread_id * 64], local_buf, 64);
    }
}

/* Complex memory dispatch with goto flow */
static void memory_dispatch_logic(void) {
    char buffer_a[512];
    char buffer_b[512];
    char buffer_c[512];
    
    /* Initialize buffers */
    __builtin_memset(buffer_a, 0x11, sizeof(buffer_a));
    __builtin_memset(buffer_b, 0x22, sizeof(buffer_b));
    __builtin_memset(buffer_c, 0x33, sizeof(buffer_c));
    
    int mode = 0;
    
    /* Goto-based state machine */
    start_dispatch:
    
    switch (mode) {
        case 0:
            /* Chain of memory operations */
            __builtin_memcpy(buffer_b, buffer_a, 256);
            mode = 1;
            goto start_dispatch;
            
        case 1:
            __builtin_memmove(buffer_c, buffer_b, 256);
            mode = 2;
            goto start_dispatch;
            
        case 2:
            /* Overlapping move */
            __builtin_memmove(buffer_a + 128, buffer_a, 128);
            mode = 3;
            goto start_dispatch;
            
        case 3:
            /* Final copy with volatile pointer */
            if (volatile_ptr) {
                __builtin_memcpy(volatile_ptr, buffer_c, 128);
            }
            break;
    }
}

/* Main test execution */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Initialize volatile pointer */
    volatile_ptr = (char*)&token_array[2048];
    
    /* Create recursive AST structure */
    ASTNode *root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Copy between nodes */
    if (root->left && root->right) {
        copy_ast_data(root->left, root->right);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Complex dispatch logic */
    memory_dispatch_logic();
    
    /* Additional built-in calls with varying sizes */
    for (int i = 0; i < 16; i++) {
        int size = 16 * (i + 1);
        if (size > 256) size = 256;
        
        char src[256], dst[256];
        __builtin_memset(src, (char)i, size);
        __builtin_memcpy(dst, src, size);
        
        /* Move with partial overlap */
        if (i % 3 == 0) {
            __builtin_memmove(dst + size/2, dst, size/2);
        }
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < sizeof(token_array); i++) {
        hash = (hash * 31) + (unsigned char)token_array[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    
    /* Cleanup */
    /* Note: In real code, you'd need to free the AST recursively */
    
    return 0;
}
