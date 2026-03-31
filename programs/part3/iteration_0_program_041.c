/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

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
    g_mem_size = 128; /* Force non-constant size */
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Function with goto jumps around memory operations */
static void test_goto_memmove(void) {
    char buffer1[256];
    char buffer2[256];
    volatile int flag = 1;
    
    /* Initialize buffers with pattern */
    __builtin_memset(buffer1, 'A', sizeof(buffer1));
    __builtin_memset(buffer2, 'B', sizeof(buffer2));
    
    goto jump_point;
    
    /* This block should be jumped over */
    __builtin_memcpy(buffer1, buffer2, 64);
    
jump_point:
    if (flag) {
        /* Perform memmove with goto control flow */
        __builtin_memmove(buffer1 + 32, buffer1, 96);
        goto after_move;
    }
    
after_move:
    /* Cross-block memmove */
    __builtin_memmove(buffer2, buffer1, 128);
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_ast_node(const char* data, size_t len) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with builtin memcpy */
    size_t copy_len = len < sizeof(node->data) ? len : sizeof(node->data) - 1;
    __builtin_memcpy(node->data, data, copy_len);
    node->data[copy_len] = '\0';
    node->size = copy_len;
    
    return node;
}

static void copy_ast_node(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Copy entire structure using memcpy */
    __builtin_memcpy(dest->data, src->data, sizeof(src->data));
    
    /* Recursive copy of children */
    if (src->left) {
        dest->left = create_ast_node(src->left->data, src->left->size);
        copy_ast_node(dest->left, src->left);
    }
    if (src->right) {
        dest->right = create_ast_node(src->right->data, src->right->size);
        copy_ast_node(dest->right, src->right);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    size_t sizes[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        sizes[tid] = g_mem_size + tid * 16; /* Varying sizes */
        buffers[tid] = malloc(sizes[tid]);
        
        if (buffers[tid]) {
            /* Each thread uses different builtins */
            switch (tid % 3) {
                case 0:
                    __builtin_memset(buffers[tid], tid, sizes[tid]);
                    break;
                case 1:
                    if (tid > 0) {
                        __builtin_memcpy(buffers[tid], buffers[tid-1], 
                                       sizes[tid] < sizes[tid-1] ? sizes[tid] : sizes[tid-1]);
                    }
                    break;
                case 2:
                    __builtin_memmove(buffers[tid] + 8, buffers[tid], sizes[tid] - 8);
                    break;
            }
        }
        
        #pragma omp barrier
        
        /* Cross-thread memory operation */
        if (tid == 0 && buffers[1]) {
            __builtin_memcpy(buffers[0], buffers[1], 
                           sizes[0] < sizes[1] ? sizes[0] : sizes[1]);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        if (buffers[i]) free(buffers[i]);
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Basic builtin calls */
    char src[256], dst[256];
    
    __builtin_memset(src, 0x42, sizeof(src));
    __builtin_memcpy(dst, src, sizeof(src));
    __builtin_memmove(src + 32, src, 128);
    
    /* Phase 2: Goto control flow */
    test_goto_memmove();
    
    /* Phase 3: Recursive structure operations */
    ASTNode* root = create_ast_node("Root node data", 14);
    ASTNode* copy = create_ast_node("", 0);
    
    if (root && copy) {
        root->left = create_ast_node("Left child data", 16);
        root->right = create_ast_node("Right child data", 17);
        
        copy_ast_node(copy, root);
        
        /* Verify copy with memcmp */
        if (__builtin_memcmp(root->data, copy->data, root->size) == 0) {
            printf("AST copy verified successfully\n");
        }
        
        free(root->left);
        free(root->right);
        free(root);
        free(copy->left);
        free(copy->right);
        free(copy);
    }
    
    /* Phase 4: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Phase 5: Variable-sized operations */
    volatile size_t dynamic_size = g_mem_size;
    char* dyn_buf1 = malloc(dynamic_size);
    char* dyn_buf2 = malloc(dynamic_size * 2);
    
    if (dyn_buf1 && dyn_buf2) {
        __builtin_memset(dyn_buf1, 0xAA, dynamic_size);
        __builtin_memcpy(dyn_buf2, dyn_buf1, dynamic_size);
        __builtin_memmove(dyn_buf1, dyn_buf2 + 16, dynamic_size - 16);
        
        /* Compute verification hash */
        unsigned long hash = 0;
        for (size_t i = 0; i < dynamic_size; i++) {
            hash = hash * 31 + dyn_buf1[i];
        }
        printf("Verification hash: %lu\n", hash);
    }
    
    free(dyn_buf1);
    free(dyn_buf2);
    
    printf("=== Test completed ===\n");
    return 0;
}
