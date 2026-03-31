/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int value;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor to test cleanup paths */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile int cleanup_buf[8];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* prefix) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Build node data with memcpy */
    char temp[64];
    __builtin_memset(temp, 0, sizeof(temp));
    __builtin_memcpy(temp, prefix, strlen(prefix));
    __builtin_memcpy(node->data, temp, sizeof(node->data));
    
    node->value = depth;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth - 1, "left_");
    
    /* Use memmove between nodes */
    if (node->left) {
        ASTNode temp_node;
        __builtin_memcpy(&temp_node, node->left, sizeof(ASTNode));
        __builtin_memmove(node->left, &temp_node, sizeof(ASTNode));
    }
    
    node->right = create_ast(depth - 2, "right_");
    
done:
    return node;
}

/* Function with goto jumping around memmove */
static void test_goto_memmove(void* dest, void* src, size_t n) {
    volatile int use_goto = 1;
    
    if (use_goto) {
        goto jump_point;
    }
    
    /* This should be skipped by goto */
    __builtin_memset(dest, 0, n);
    
jump_point:
    /* Execute memmove after jump */
    __builtin_memmove(dest, src, n);
    
    /* Jump back */
    if (n > 100) {
        goto after_memmove;
    }
    
    __builtin_memset(src, 0, n);
    
after_memmove:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        volatile char local_buf[128];
        volatile char shared_buf[128];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp critical
        {
            __builtin_memcpy(shared_buf, local_buf, sizeof(local_buf));
            __builtin_memmove(local_buf, shared_buf, sizeof(shared_buf));
        }
    }
}

/* Multi-stage initialization */
static void init_complex_buffer(char* buf, size_t size) {
    volatile size_t half = size / 2;
    volatile size_t quarter = size / 4;
    
    /* Stage 1: Clear buffer */
    __builtin_memset(buf, 0, size);
    
    /* Stage 2: Fill first half */
    char pattern[32];
    __builtin_memset(pattern, 0xCC, sizeof(pattern));
    __builtin_memcpy(buf, pattern, sizeof(pattern));
    
    /* Stage 3: Move data around */
    __builtin_memmove(buf + quarter, buf, quarter);
    
    /* Stage 4: Final copy */
    __builtin_memcpy(buf + half, buf, quarter);
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Basic builtin usage */
    volatile char buffer1[512];
    volatile char buffer2[512];
    
    __builtin_memset(buffer1, 0x11, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1, buffer2, sizeof(buffer1));
    
    /* Phase 2: Recursive AST operations */
    ASTNode* root = create_ast(4, "root_");
    
    if (root) {
        /* Copy between AST nodes */
        ASTNode node_copy;
        __builtin_memcpy(&node_copy, root, sizeof(ASTNode));
        __builtin_memmove(root, &node_copy, sizeof(ASTNode));
        
        /* Test goto with memmove */
        test_goto_memmove(root->data, node_copy.data, 32);
    }
    
    /* Phase 3: OpenMP parallel section */
    #ifdef _OPENMP
    parallel_mem_ops();
    #endif
    
    /* Phase 4: Complex buffer initialization */
    char complex_buf[1024];
    init_complex_buffer(complex_buf, sizeof(complex_buf));
    
    /* Phase 5: Variable-sized operations */
    volatile size_t dynamic_size = g_mem_size;
    char* dynamic_buf = (char*)malloc(dynamic_size);
    if (dynamic_buf) {
        __builtin_memset(dynamic_buf, 0xAA, dynamic_size);
        
        char* dynamic_buf2 = (char*)malloc(dynamic_size);
        if (dynamic_buf2) {
            __builtin_memcpy(dynamic_buf2, dynamic_buf, dynamic_size);
            __builtin_memmove(dynamic_buf, dynamic_buf2, dynamic_size);
            free(dynamic_buf2);
        }
        
        free(dynamic_buf);
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash += buffer1[i];
    }
    
    if (root) {
        /* Cleanup AST */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    printf("Test completed. Hash: %lu\n", hash);
    printf("Builtin calls executed: memcpy, memset, memmove\n");
    
    return 0;
}
