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

/* Constructor function to force early initialization */
__attribute__((constructor))
static void init_memory_pools(void) {
    printf("Constructor: Initializing memory pools\n");
    /* Force builtin usage in constructor */
    char buf1[100], buf2[100];
    __builtin_memset(buf1, 0xA5, sizeof(buf1));
    __builtin_memcpy(buf2, buf1, sizeof(buf1));
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_memory(void) {
    printf("Destructor: Cleaning up\n");
}

/* Function with goto jumps around memmove */
static void test_goto_memmove(void *dst, const void *src, size_t n) {
    int use_memmove = 1;
    
    if (n > 0) {
        goto do_copy;
    } else {
        goto skip_copy;
    }
    
do_copy:
    /* This should trigger memmove redirection */
    __builtin_memmove(dst, src, n);
    goto after_copy;
    
skip_copy:
    /* Alternative path */
    *(char*)dst = 0;
    
after_copy:
    /* Control flow continues */
    return;
}

/* Recursive tree manipulation with memory operations */
static struct ast_node* create_tree(int depth) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(*node));
    node->type = depth;
    
    /* Fill data with pattern */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        node->data[i] = (char)(depth + i);
    }
    
    /* Recursive creation */
    node->left = create_tree(depth - 1);
    node->right = create_tree(depth - 1);
    
    return node;
}

/* Copy tree data between nodes */
static void copy_tree_data(struct ast_node* dst, const struct ast_node* src) {
    if (!dst || !src) return;
    
    /* Use builtin memcpy for data transfer */
    __builtin_memcpy(dst->data, src->data, sizeof(dst->data));
    
    /* Recursive copying */
    if (dst->left && src->left) {
        copy_tree_data(dst->left, src->left);
    }
    if (dst->right && src->right) {
        copy_tree_data(dst->right, src->right);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[256];
        char shared_buf[256];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp single
        {
            /* Master thread initializes shared buffer */
            __builtin_memset(shared_buf, 0xFF, sizeof(shared_buf));
        }
        
        #pragma omp barrier
        
        /* Copy from shared to local */
        __builtin_memcpy(local_buf + 128, shared_buf, 128);
        
        /* Move data within local buffer */
        __builtin_memmove(local_buf, local_buf + 64, 64);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Direct builtin calls with volatile lengths */
    char buffer1[512], buffer2[512];
    
    __builtin_memset(buffer1, 0xAA, g_memset_len);
    __builtin_memcpy(buffer2, buffer1, g_memcpy_len);
    __builtin_memmove(buffer1 + 32, buffer1, g_memmove_len);
    
    /* Phase 2: Goto-controlled memmove */
    test_goto_memmove(buffer1 + 64, buffer2, 32);
    
    /* Phase 3: Tree operations */
    struct ast_node* tree1 = create_tree(3);
    struct ast_node* tree2 = create_tree(3);
    
    if (tree1 && tree2) {
        copy_tree_data(tree2, tree1);
        
        /* Additional builtin usage on tree data */
        __builtin_memcpy(tree1->data + 128, tree2->data, 64);
        __builtin_memmove(tree2->data, tree2->data + 32, 32);
    }
    
    /* Phase 4: OpenMP parallel section */
    parallel_memory_ops();
    
    /* Phase 5: Complex nested operations */
    {
        char* dynamic_buf = malloc(1024);
        if (dynamic_buf) {
            __builtin_memset(dynamic_buf, 0xCC, 512);
            
            /* Overlapping copy with memmove */
            __builtin_memmove(dynamic_buf + 256, dynamic_buf, 256);
            
            /* Copy to stack */
            __builtin_memcpy(buffer1, dynamic_buf, 128);
            
            free(dynamic_buf);
        }
    }
    
    /* Verification: Compute simple hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash = (hash * 31) + (unsigned char)buffer1[i];
    }
    printf("Result hash: 0x%08lx\n", hash);
    
    /* Cleanup */
    free(tree1);
    free(tree2);
    
    printf("Test completed\n");
    return 0;
}
