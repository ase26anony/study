/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 1024;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 256;  /* Force non-constant size */
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Function with goto jumps around memmove */
static void test_goto_memmove(char* dest, char* src, size_t n) {
    int use_copy = 1;
    
    if (n == 0) goto skip_copy;
    
    /* Jump into memory operation block */
    goto do_copy;
    
copy_block:
    /* Force builtin_memmove with goto control flow */
    __builtin_memmove(dest, src, n);
    goto after_copy;
    
do_copy:
    if (use_copy) {
        goto copy_block;
    }
    
skip_copy:
    /* Empty path */
    
after_copy:
    /* Verify the copy */
    for (size_t i = 0; i < n; i++) {
        if (dest[i] != src[i]) {
            printf("Memmove verification failed at index %zu\n", i);
            break;
        }
    }
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_ast_node(int id) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern using memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (id % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    return node;
}

static void copy_ast_data(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Copy data between nodes using builtins */
    __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    
    /* Recursive copy of children */
    if (src->left && dest->left) {
        copy_ast_data(dest->left, src->left);
    }
    if (src->right && dest->right) {
        copy_ast_data(dest->right, src->right);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    size_t local_size = g_mem_size / num_threads;
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread allocates and operates on its own buffer */
        buffers[tid] = (char*)malloc(local_size);
        if (!buffers[tid]) {
            #pragma omp critical
            printf("Thread %d: Allocation failed\n", tid);
            return;
        }
        
        /* Pattern initialization with memset */
        __builtin_memset(buffers[tid], tid + '0', local_size);
        
        /* Barrier to ensure all threads have initialized */
        #pragma omp barrier
        
        /* Rotate buffers between threads using memcpy */
        int next_tid = (tid + 1) % num_threads;
        char* temp = (char*)malloc(local_size);
        if (temp) {
            __builtin_memcpy(temp, buffers[tid], local_size);
            __builtin_memcpy(buffers[tid], buffers[next_tid], local_size);
            __builtin_memcpy(buffers[next_tid], temp, local_size);
            free(temp);
        }
        
        /* Verify the rotation */
        #pragma omp barrier
        for (size_t i = 0; i < local_size; i++) {
            if (buffers[tid][i] != ((tid + num_threads - 1) % num_threads) + '0') {
                #pragma omp critical
                printf("Thread %d: Verification failed at %zu\n", tid, i);
                break;
            }
        }
        
        free(buffers[tid]);
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Direct builtin calls with volatile sizes */
    volatile size_t copy_size = g_mem_size;
    char* src = (char*)malloc(copy_size);
    char* dest = (char*)malloc(copy_size);
    
    if (!src || !dest) {
        printf("Initial allocation failed\n");
        return 1;
    }
    
    /* Force all three builtins to be called */
    __builtin_memset(src, 0x42, copy_size);
    __builtin_memcpy(dest, src, copy_size);
    __builtin_memmove(src + 10, src, copy_size - 10);
    
    /* Phase 2: Goto-based control flow with memmove */
    test_goto_memmove(dest, src, copy_size / 2);
    
    /* Phase 3: AST structure operations */
    ASTNode* tree1 = create_ast_node(1);
    ASTNode* tree2 = create_ast_node(2);
    
    if (tree1 && tree2) {
        /* Create child nodes */
        tree1->left = create_ast_node(3);
        tree1->right = create_ast_node(4);
        tree2->left = create_ast_node(5);
        tree2->right = create_ast_node(6);
        
        /* Copy entire tree structure */
        copy_ast_data(tree2, tree1);
        
        /* Verify copy */
        if (__builtin_memcmp(tree1->data, tree2->data, sizeof(tree1->data)) == 0) {
            printf("AST copy successful\n");
        }
        
        /* Cleanup */
        free(tree1->left);
        free(tree1->right);
        free(tree2->left);
        free(tree2->right);
    }
    
    free(tree1);
    free(tree2);
    
    /* Phase 4: OpenMP parallel operations */
    printf("Starting parallel memory operations...\n");
    parallel_memory_ops();
    
    /* Phase 5: Edge cases with small and zero sizes */
    char small_buf[16];
    __builtin_memset(small_buf, 0, 0);  /* Zero-size memset */
    __builtin_memcpy(small_buf, small_buf, 1);  /* 1-byte copy */
    __builtin_memmove(small_buf + 1, small_buf, 0);  /* Zero-size memmove */
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < copy_size && i < 100; i++) {
        hash = hash * 31 + (unsigned char)dest[i];
    }
    printf("Final hash: %lu\n", hash);
    
    free(src);
    free(dest);
    
    printf("=== Test Complete ===\n");
    return 0;
}
