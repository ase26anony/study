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
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Constructor attribute to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force builtin usage before main */
    char buf1[256], buf2[256];
    __builtin_memset(buf1, 0xAA, sizeof(buf1));
    __builtin_memcpy(buf2, buf1, sizeof(buf1));
    __builtin_memmove(buf1, buf2, sizeof(buf1));
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    /* Final builtin calls */
    char final_buf[64];
    __builtin_memset(final_buf, 0xFF, 64);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile lengths */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    
    /* Fill data with pattern */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        node->data[i] = (char)(depth + i);
    }
    
    /* Create children with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        /* Goto into memory operation block */
        goto create_children;
    }
    
create_children:
    node->left = create_ast(depth - 1);
    
    /* Jump out and back in */
    if (node->left) {
        char temp[256];
        __builtin_memcpy(temp, node->left->data, g_memcpy_len);
        goto skip_right;
    }
    
skip_right:
    node->right = create_ast(depth - 2);
    
    /* Copy between nodes if both exist */
    if (node->left && node->right) {
        __builtin_memmove(node->right->data, 
                         node->left->data, 
                         g_memmove_len);
    }
    
    return node;
}

/* OpenMP parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[512];
        char shared_buf[512];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp single
        {
            __builtin_memset(shared_buf, 0xCC, sizeof(shared_buf));
        }
        
        #pragma omp barrier
        
        /* Copy with volatile length */
        __builtin_memcpy(local_buf + tid * 64, 
                        shared_buf, 
                        g_memcpy_len);
        
        /* Move within buffer */
        __builtin_memmove(local_buf + 128, 
                         local_buf, 
                         g_memset_len);
    }
}

/* Complex control flow with gotos */
static void goto_mem_operations(void) {
    char buffer1[1024];
    char buffer2[1024];
    int state = 0;
    
    /* Initial memset */
    __builtin_memset(buffer1, 0x11, sizeof(buffer1));
    
    goto label2;
    
label1:
    /* This should be reached via goto */
    __builtin_memcpy(buffer2, buffer1, g_memcpy_len);
    goto label3;
    
label2:
    /* Jump into memmove */
    __builtin_memmove(buffer1 + 256, buffer1, g_memmove_len);
    if (state == 0) {
        state = 1;
        goto label1;
    }
    
label3:
    /* Final operation */
    __builtin_memset(buffer2 + 512, 0x22, g_memset_len);
}

/* Main execution flow */
int main(void) {
    ASTNode* root = NULL;
    unsigned long hash = 0;
    
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Recursive AST creation */
    root = create_ast(5);
    
    /* Phase 2: Parallel operations */
    parallel_mem_ops();
    
    /* Phase 3: Goto-based control flow */
    goto_mem_operations();
    
    /* Phase 4: Direct builtin calls with volatile */
    {
        char direct_buf1[256];
        char direct_buf2[256];
        
        __builtin_memset(direct_buf1, 0x33, g_memset_len);
        __builtin_memcpy(direct_buf2, direct_buf1, g_memcpy_len);
        __builtin_memmove(direct_buf1 + 64, direct_buf2, g_memmove_len);
        
        /* Compute verification hash */
        for (size_t i = 0; i < sizeof(direct_buf1); i++) {
            hash += (unsigned long)direct_buf1[i];
        }
    }
    
    /* Phase 5: AST traversal and cleanup */
    if (root) {
        /* Copy between tree nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->right->data, 
                           root->left->data, 
                           g_memcpy_len);
        }
        
        /* Free recursively */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    printf("Test completed. Hash: %lu\n", hash);
    return (hash != 0) ? 0 : 1;
}
