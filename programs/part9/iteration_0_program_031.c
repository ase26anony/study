/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* __attribute__((constructor)) function */
static void __attribute__((constructor)) init_asan_test(void) {
    printf("ASAN test constructor initialized\n");
}

/* __attribute__((destructor)) function */
static void __attribute__((destructor)) cleanup_asan_test(void) {
    printf("ASAN test destructor cleaning up\n");
}

/* Function with goto jumps around memmove */
static void test_goto_memmove(char* dest, char* src, size_t n) {
    int use_copy = 1;
    
    if (n == 0) goto skip_copy;
    
    /* Jump into memory operation block */
    goto do_copy;
    
copy_block:
    /* Force builtin memmove with goto control flow */
    __builtin_memmove(dest, src, n);
    goto after_copy;
    
do_copy:
    if (use_copy) {
        goto copy_block;
    }
    
skip_copy:
    /* Alternative path */
    dest[0] = 'X';
    
after_copy:
    /* Jump out of block */
    goto finish;
    
finish:
    return;
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset to initialize */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with builtin memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) {
        copy_len = sizeof(node->data);
    }
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->size = copy_len;
    
    /* Recursive creation with different data */
    char child_data[64];
    __builtin_snprintf(child_data, sizeof(child_data), "%s-%d", base_data, depth);
    
    node->left = create_ast(depth - 1, child_data);
    node->right = create_ast(depth - 1, child_data);
    
    return node;
}

/* Function to process AST with memory operations */
static size_t process_ast(ASTNode* node, char* buffer) {
    if (!node) return 0;
    
    size_t total = 0;
    
    /* Copy node data to buffer */
    __builtin_memcpy(buffer, node->data, node->size);
    total += node->size;
    
    /* Process children */
    total += process_ast(node->left, buffer + total);
    total += process_ast(node->right, buffer + total);
    
    return total;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread allocates and operates on memory */
        buffers[tid] = (char*)malloc(g_mem_size);
        if (buffers[tid]) {
            /* Use volatile to prevent optimization */
            volatile size_t local_size = g_mem_size;
            
            /* Force builtin memset */
            __builtin_memset(buffers[tid], tid + 'A', local_size);
            
            /* Force builtin memcpy between thread buffers */
            if (tid > 0) {
                __builtin_memcpy(buffers[tid], buffers[tid-1], local_size / 2);
            }
            
            /* Complex memmove with overlapping regions */
            if (local_size > 32) {
                __builtin_memmove(buffers[tid] + 16, buffers[tid] + 8, local_size - 32);
            }
        }
        
        #pragma omp barrier
        
        /* Verify and modify */
        if (buffers[tid]) {
            buffers[tid][0] = 'Z' - tid;
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        if (buffers[i]) {
            free(buffers[i]);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Test 1: Basic builtin calls with volatile control */
    char src[512], dest[512];
    volatile size_t copy_size = 128;
    
    __builtin_memset(src, 'S', sizeof(src));
    __builtin_memcpy(dest, src, copy_size);
    __builtin_memmove(dest + 64, dest, copy_size);
    
    /* Test 2: Goto control flow with memmove */
    test_goto_memmove(dest + 128, src, 64);
    
    /* Test 3: Recursive AST operations */
    ASTNode* root = create_ast(3, "ROOT");
    if (root) {
        char ast_buffer[1024];
        size_t total = process_ast(root, ast_buffer);
        
        /* Verify with builtin memcmp */
        int cmp = __builtin_memcmp(ast_buffer, "ROOT", 4);
        printf("AST processed %zu bytes, cmp result: %d\n", total, cmp);
        
        /* Cleanup AST */
        /* Note: Proper tree deletion omitted for brevity */
        free(root);
    }
    
    /* Test 4: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Test 5: Variable-sized operations */
    for (volatile int i = 1; i <= 4; i++) {
        size_t size = 64 * i;
        char* buf1 = (char*)malloc(size);
        char* buf2 = (char*)malloc(size);
        
        if (buf1 && buf2) {
            __builtin_memset(buf1, i, size);
            __builtin_memcpy(buf2, buf1, size);
            
            /* Overlapping memmove */
            if (size > 16) {
                __builtin_memmove(buf1 + 8, buf1, size - 16);
            }
        }
        
        free(buf1);
        free(buf2);
    }
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(dest); i++) {
        hash = hash * 31 + dest[i];
    }
    printf("Final hash: %lu\n", hash);
    
    return 0;
}
