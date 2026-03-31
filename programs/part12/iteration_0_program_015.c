/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char *data;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[256];
    /* Force builtin initialization in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 128, buffer, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    volatile char final_buf[32];
    __builtin_memset(final_buf, 0xFF, 16);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = depth * 7;
    
    /* Allocate and fill data with volatile length */
    size_t data_len = (size_t)(g_memset_len % 64) + 16;
    node->data = malloc(data_len);
    if (node->data) {
        __builtin_memset(node->data, 'A' + depth, data_len - 1);
        node->data[data_len - 1] = '\0';
    }
    
    /* Recursive creation with goto for flow control */
    int use_left = 1;
    
    if (depth % 3 == 0) {
        goto skip_left;
    }
    
    node->left = create_ast(depth + 1, max_depth);
    
skip_left:
    if (depth % 2 == 0) {
        goto skip_right;
    }
    
    node->right = create_ast(depth + 1, max_depth);
    
skip_right:
    /* Copy node data using builtin */
    if (node->left && node->right) {
        volatile size_t copy_len = sizeof(int) * 2;
        __builtin_memcpy(&node->right->type, &node->left->type, copy_len);
    }
    
    return node;
}

/* Function with goto jumping into memmove block */
static void goto_memmove_test(char *dest, char *src, size_t len) {
    int condition = (len > 32);
    
    if (condition) {
        goto do_memmove;
    }
    
    /* Some intermediate code */
    __builtin_memset(dest, 0xCC, len/2);
    
do_memmove:
    /* Target of goto - contains builtin */
    __builtin_memmove(dest, src, len);
    
    /* Jump out */
    if (len > 64) {
        goto after_memmove;
    }
    
    __builtin_memset(src, 0xDD, len);
    
after_memmove:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[256];
        char shared_buf[256];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp master
        {
            __builtin_memset(shared_buf, 0xEE, sizeof(shared_buf));
        }
        
        #pragma omp barrier
        
        /* Memcpy with volatile length */
        volatile size_t copy_len = g_memcpy_len % 128;
        __builtin_memcpy(local_buf + 64, shared_buf, copy_len);
        
        /* Memmove with overlap */
        __builtin_memmove(local_buf + 32, local_buf, 64);
    }
}

/* Main execution flow */
int main(void) {
    /* Initialize token array */
    char tokens[512];
    for (int i = 0; i < sizeof(tokens); i++) {
        tokens[i] = (char)(i % 256);
    }
    
    /* Create recursive AST */
    ASTNode *root = create_ast(0, 4);
    
    /* Perform AST memory operations */
    if (root && root->left && root->right) {
        /* Copy between nodes */
        size_t copy_size = sizeof(int) * 3;
        __builtin_memcpy(root->right, root->left, copy_size);
        
        /* Move data with overlap */
        if (root->data && root->left->data) {
            volatile size_t move_len = g_memmove_len % 48;
            __builtin_memmove(root->data + 8, root->left->data, move_len);
        }
    }
    
    /* Test goto flow with memmove */
    char src_buf[128], dst_buf[128];
    __builtin_memset(src_buf, 0x11, sizeof(src_buf));
    goto_memmove_test(dst_buf, src_buf, 80);
    
    /* Execute parallel operations */
    parallel_mem_ops();
    
    /* Complex memory pattern to stress redirection */
    char pattern_buf[1024];
    volatile size_t pattern_len = 256;
    
    for (int i = 0; i < 4; i++) {
        __builtin_memset(pattern_buf + i * 64, i * 0x40, 64);
        __builtin_memcpy(pattern_buf + i * 64 + 32, pattern_buf + i * 64, 32);
        __builtin_memmove(pattern_buf + i * 64 + 16, pattern_buf + i * 64, 48);
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < sizeof(pattern_buf); i++) {
        hash = (hash * 31) + (unsigned char)pattern_buf[i];
    }
    
    /* Also hash AST contents */
    if (root && root->data) {
        for (int i = 0; i < 32 && root->data[i]; i++) {
            hash = (hash * 17) + root->data[i];
        }
    }
    
    printf("Verification hash: %lu\n", hash);
    
    /* Cleanup */
    /* ... cleanup code for AST ... */
    
    return (int)(hash % 255);
}
