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
struct ast_node {
    int type;
    char data[256];
    struct ast_node *left;
    struct ast_node *right;
};

/* Global token array */
static char token_pool[4096];

/* Constructor function to force early initialization */
__attribute__((constructor))
static void init_constructor(void) {
    volatile char *p = token_pool;
    /* Use __builtin_memset in constructor */
    __builtin_memset(p, 0xAA, sizeof(token_pool));
    printf("Constructor initialized token pool\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    volatile char *p = token_pool;
    __builtin_memset(p, 0xFF, 256);
    printf("Destructor cleaned up token pool\n");
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    struct ast_node *node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, 0, sizeof(struct ast_node));
    
    node->type = depth;
    
    /* Copy data using __builtin_memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data))
        copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Build children with goto for flow control */
    char child_data[256];
    __builtin_memset(child_data, 0, sizeof(child_data));
    
    /* Goto block for memmove testing */
    if (depth > 1) {
        goto build_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    return node;
    
build_children:
    /* Use volatile to control memmove */
    volatile char temp_buf[512];
    __builtin_memset((void*)temp_buf, 'X', sizeof(temp_buf));
    
    /* __builtin_memmove with goto context */
    __builtin_memmove(child_data, (void*)temp_buf, 
                     g_memmove_len < sizeof(child_data) ? 
                     g_memmove_len : sizeof(child_data));
    
    /* Jump out of goto block */
    if (depth % 2 == 0) {
        goto build_left;
    } else {
        goto build_right;
    }
    
build_left:
    node->left = build_ast(depth - 1, child_data);
    goto build_right;
    
build_right:
    node->right = build_ast(depth - 1, child_data);
    return node;
}

/* OpenMP parallel memory operations */
static void parallel_mem_operations(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[1024];
        char src_buf[1024];
        
        /* Initialize source buffer */
        __builtin_memset(src_buf, thread_id + 'A', sizeof(src_buf));
        
        /* Parallel memcpy */
        #pragma omp for
        for (int i = 0; i < 16; i++) {
            size_t len = g_memcpy_len + i * 8;
            if (len > sizeof(local_buf)) len = sizeof(local_buf);
            __builtin_memcpy(local_buf, src_buf, len);
            
            /* Verify with volatile read */
            volatile char v = local_buf[0];
            (void)v;
        }
        
        /* Parallel memset */
        #pragma omp for
        for (int i = 0; i < 16; i++) {
            size_t len = g_memset_len + i * 4;
            if (len > sizeof(local_buf)) len = sizeof(local_buf);
            __builtin_memset(local_buf, i + '0', len);
        }
        
        /* Barrier before memmove */
        #pragma omp barrier
        
        /* Parallel memmove with overlap */
        #pragma omp for
        for (int i = 0; i < 8; i++) {
            char overlap_buf[512];
            __builtin_memset(overlap_buf, 'Z', sizeof(overlap_buf));
            
            size_t len = g_memmove_len;
            if (len > sizeof(overlap_buf) / 2) 
                len = sizeof(overlap_buf) / 2;
            
            __builtin_memmove(overlap_buf + len, overlap_buf, len);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Initialize token pool with memset */
    volatile size_t init_len = 512;
    __builtin_memset(token_pool, 0xCC, init_len);
    
    /* Build recursive AST */
    struct ast_node *root = build_ast(4, "RootNodeData");
    if (!root) {
        fprintf(stderr, "Failed to build AST\n");
        return 1;
    }
    
    /* Perform AST node copying */
    struct ast_node node_copy;
    __builtin_memcpy(&node_copy, root, sizeof(struct ast_node));
    
    /* Complex memmove within AST */
    if (root->left && root->right) {
        char temp[256];
        __builtin_memcpy(temp, root->left->data, sizeof(temp));
        __builtin_memmove(root->right->data, temp, 
                         g_memmove_len < sizeof(temp) ? 
                         g_memmove_len : sizeof(temp));
    }
    
    /* Execute parallel memory operations */
    parallel_mem_operations();
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(token_pool); i++) {
        hash = (hash * 31) + token_pool[i];
    }
    
    /* Use builtins in verification */
    char verify_buf[64];
    __builtin_memset(verify_buf, 0, sizeof(verify_buf));
    __builtin_memcpy(verify_buf, &hash, sizeof(hash));
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(root);
    return 0;
}
