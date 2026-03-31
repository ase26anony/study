/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

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
    g_mem_size = 128;  /* Force non-constant size */
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Function with goto statements for flow control */
static void test_memmove_with_goto(char* dest, const char* src, size_t n) {
    int use_fast_path = 0;
    
    /* Jump into memory operation block */
    goto start_copy;
    
copy_block:
    /* Force __builtin_memmove call */
    __builtin_memmove(dest, src, n);
    goto after_copy;
    
start_copy:
    if (n > 100) {
        use_fast_path = 1;
        goto copy_block;
    }
    
    /* Alternative path */
    __builtin_memmove(dest + 10, src, n - 10);
    
after_copy:
    /* Jump out of scope */
    if (use_fast_path) {
        goto finalize;
    }
    
    /* Additional memory operation */
    __builtin_memmove(dest, dest + 5, 20);
    
finalize:
    return;
}

/* Recursive function using AST structures */
static ASTNode* create_ast_recursive(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->size = copy_len;
    
    /* Recursive creation with different data */
    char child_data[256];
    snprintf(child_data, sizeof(child_data), "%s.%d", base_data, depth);
    
    node->left = create_ast_recursive(depth - 1, child_data);
    node->right = create_ast_recursive(depth - 1, child_data);
    
    /* Copy between nodes if both exist */
    if (node->left && node->right) {
        size_t min_size = (node->left->size < node->right->size) ? 
                          node->left->size : node->right->size;
        __builtin_memcpy(node->right->data, node->left->data, min_size);
    }
    
    return node;
}

/* Complex memory operation function */
static void perform_memory_operations(void) {
    volatile size_t local_size = g_mem_size;
    char buffer1[256];
    char buffer2[256];
    char buffer3[256];
    
    /* Initialize buffers with different patterns */
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        buffer1[i] = (char)(i % 256);
        buffer2[i] = (char)((i + 128) % 256);
    }
    
    /* Test all three builtins in sequence */
    __builtin_memset(buffer3, 0xAA, local_size);
    __builtin_memcpy(buffer1 + 32, buffer2, local_size / 2);
    __builtin_memmove(buffer2, buffer1, local_size);
    
    /* Nested memory operations */
    char* ptr = buffer3;
    for (int i = 0; i < 3; i++) {
        __builtin_memcpy(ptr, buffer1 + i * 16, 32);
        __builtin_memset(ptr + 16, i, 16);
        ptr += 32;
    }
}

/* OpenMP parallel section */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char tlbuf1[128];
        char tlbuf2[128];
        
        /* Initialize with thread-specific pattern */
        __builtin_memset(tlbuf1, thread_id, sizeof(tlbuf1));
        __builtin_memset(tlbuf2, 255 - thread_id, sizeof(tlbuf2));
        
        /* Cross-thread like operations */
        #pragma omp barrier
        
        /* Memory operations that should trigger ASAN */
        __builtin_memcpy(tlbuf1 + 32, tlbuf2, 64);
        __builtin_memmove(tlbuf2, tlbuf1, 48);
        
        /* Clear part of buffer */
        __builtin_memset(tlbuf1 + 16, 0, 32);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Basic built-in calls */
    printf("Phase 1: Testing basic built-ins\n");
    perform_memory_operations();
    
    /* Phase 2: AST structure operations */
    printf("Phase 2: Testing AST operations\n");
    ASTNode* root = create_ast_recursive(3, "root");
    
    if (root) {
        /* Perform memory operations on AST */
        char temp[256];
        __builtin_memcpy(temp, root->data, root->size);
        __builtin_memset(root->data, 0xCC, root->size / 2);
        __builtin_memmove(root->data + 10, temp, root->size - 10);
        
        /* TODO: Add AST cleanup */
    }
    
    /* Phase 3: Goto flow control */
    printf("Phase 3: Testing goto flow control\n");
    char src[100], dst[100];
    for (int i = 0; i < 100; i++) src[i] = (char)i;
    
    test_memmove_with_goto(dst, src, 100);
    test_memmove_with_goto(dst, src, 50);
    
    /* Phase 4: OpenMP parallel operations */
    printf("Phase 4: Testing OpenMP operations\n");
    #ifdef _OPENMP
    parallel_memory_operations();
    #else
    printf("OpenMP not available, skipping parallel tests\n");
    #endif
    
    /* Phase 5: Variable-sized operations */
    printf("Phase 5: Testing variable-sized operations\n");
    volatile size_t dynamic_size = g_mem_size + 64;
    char* dyn_buf1 = (char*)malloc(dynamic_size);
    char* dyn_buf2 = (char*)malloc(dynamic_size);
    
    if (dyn_buf1 && dyn_buf2) {
        __builtin_memset(dyn_buf1, 0x11, dynamic_size);
        __builtin_memcpy(dyn_buf2, dyn_buf1, dynamic_size);
        __builtin_memmove(dyn_buf1 + 16, dyn_buf2, dynamic_size - 32);
        
        free(dyn_buf1);
        free(dyn_buf2);
    }
    
    /* Verification hash */
    unsigned long hash = 0;
    char verify_buf[256];
    __builtin_memset(verify_buf, 0, sizeof(verify_buf));
    __builtin_memcpy(verify_buf, "ASAN_TEST_COMPLETE", 19);
    
    for (size_t i = 0; i < sizeof(verify_buf); i++) {
        hash = hash * 31 + verify_buf[i];
    }
    
    printf("Test completed. Verification hash: %lu\n", hash);
    printf("Exit code: %d\n", (int)(hash % 256));
    
    return (int)(hash % 256);
}
