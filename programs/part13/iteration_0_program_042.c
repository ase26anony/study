/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 1024;
volatile size_t g_memset_len = 512;
volatile size_t g_memmove_len = 256;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char *data;
    size_t data_len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[64];
    /* Force early builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 32, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_late(void) {
    volatile char final_buf[32];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_node(const char *src, size_t len) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = len % 3;
    node->data_len = len;
    node->data = malloc(len + 1);
    node->left = node->right = NULL;
    
    if (node->data) {
        /* Use builtins for initialization */
        __builtin_memset(node->data, 0, len + 1);
        if (src) {
            __builtin_memcpy(node->data, src, len > 64 ? 64 : len);
        }
    }
    
    return node;
}

/* Complex function with goto and memory operations */
static void process_with_goto(ASTNode *a, ASTNode *b) {
    volatile int use_memmove = 1;
    char temp[256];
    
    if (!a || !b) goto cleanup;
    
    /* Jump into memory operation block */
    if (use_memmove) {
        goto do_memmove;
    }
    
    normal_path:
    __builtin_memcpy(temp, a->data, a->data_len > 256 ? 256 : a->data_len);
    return;
    
    do_memmove:
    /* This tests flow sensitivity for asan_memfn_rtls */
    if (a->data && b->data) {
        size_t len = a->data_len < b->data_len ? a->data_len : b->data_len;
        __builtin_memmove(b->data, a->data, len);
    }
    goto normal_path;
    
    cleanup:
    __builtin_memset(temp, 0, sizeof(temp));
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        volatile char local_buf[128];
        volatile char shared_buf[256];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp master
        {
            __builtin_memset(shared_buf, 0xCC, sizeof(shared_buf));
        }
        
        #pragma omp barrier
        
        /* Mixed memory operations in parallel region */
        __builtin_memcpy(local_buf + 64, shared_buf + tid * 8, 32);
        __builtin_memmove(local_buf, local_buf + 32, 64);
    }
}

/* Main test driver */
int main(void) {
    /* Initialize with volatile lengths */
    size_t cp_len = g_memcpy_len % 128 + 64;
    size_t set_len = g_memset_len % 96 + 32;
    size_t mv_len = g_memmove_len % 80 + 48;
    
    /* Create test buffers */
    char *src = malloc(cp_len + mv_len + 16);
    char *dst = malloc(cp_len + mv_len + 16);
    
    if (!src || !dst) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Force builtin usage with non-foldable sizes */
    __builtin_memset(src, 0x11, cp_len);
    __builtin_memset(dst, 0x22, cp_len);
    
    /* Basic builtin calls */
    __builtin_memcpy(dst, src, cp_len);
    __builtin_memmove(dst + 32, dst, mv_len);
    __builtin_memset(src + 16, 0x33, set_len);
    
    /* Create AST structures */
    ASTNode *root = create_node("ROOT NODE DATA", 32);
    ASTNode *child1 = create_node("CHILD ONE DATA", 28);
    ASTNode *child2 = create_node("CHILD TWO DATA", 24);
    
    if (root && child1 && child2) {
        root->left = child1;
        root->right = child2;
        
        /* Copy between AST nodes */
        if (root->data && child1->data) {
            size_t copy_len = root->data_len < child1->data_len ? 
                            root->data_len : child1->data_len;
            __builtin_memcpy(child1->data, root->data, copy_len);
        }
        
        /* Test goto flow */
        process_with_goto(child1, child2);
        
        /* Recursive tree processing */
        ASTNode *nodes[] = {root, child1, child2};
        for (int i = 0; i < 3; i++) {
            if (nodes[i] && nodes[(i+1)%3]) {
                size_t len = nodes[i]->data_len < 128 ? nodes[i]->data_len : 128;
                __builtin_memmove(nodes[(i+1)%3]->data + 8, 
                                nodes[i]->data, len);
            }
        }
    }
    
    /* Execute OpenMP section */
    parallel_memory_ops();
    
    /* Final verification hash */
    unsigned long hash = 0;
    if (dst) {
        for (size_t i = 0; i < (cp_len < 64 ? cp_len : 64); i++) {
            hash = (hash * 31) + (unsigned char)dst[i];
        }
    }
    
    printf("Result hash: %lu\n", hash);
    
    /* Cleanup */
    free(src);
    free(dst);
    
    if (root) {
        free(root->data);
        free(root);
    }
    if (child1) {
        free(child1->data);
        free(child1);
    }
    if (child2) {
        free(child2->data);
        free(child2);
    }
    
    return 0;
}
