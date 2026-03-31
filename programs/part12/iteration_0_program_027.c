/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor))
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 256 + (rand() % 128);
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("Destructor: ASAN test completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with volatile size */
    size_t copy_size = (g_mem_size % 64) + 1;
    __builtin_memcpy(node->data, base_data, copy_size);
    node->size = copy_size;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 2 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, "left_branch");
        node->right = create_ast(depth - 1, "right_branch");
        
        create_children:
        /* Jump target with __builtin_memmove */
        if (depth > 2) {
            char temp[64];
            __builtin_memmove(temp, node->data, node->size);
            __builtin_memmove(node->data + 10, temp, node->size - 10);
        }
    }
    
    return node;
}

/* Function with complex memory operations and goto */
static void process_ast_with_goto(ASTNode* root) {
    if (!root) return;
    
    char buffer[256];
    volatile int flag = 1;
    
    /* Goto jumping into memory operation block */
    if (flag) {
        goto mem_operation_block;
    }
    
    normal_path:
    __builtin_memcpy(buffer, root->data, root->size);
    return;
    
    mem_operation_block:
    {
        /* This block should trigger ASAN instrumentation */
        char local_buf[128];
        volatile size_t op_size = g_mem_size % 128;
        
        /* Use all three builtins with volatile sizes */
        __builtin_memset(local_buf, 0xAA, op_size);
        __builtin_memcpy(buffer, local_buf, op_size);
        
        /* Overlapping memory move */
        __builtin_memmove(buffer + 32, buffer, 64);
        
        /* Jump out of block */
        goto normal_path;
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    const int num_threads = 4;
    char shared_buffer[1024];
    volatile long results[4] = {0};
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        char thread_buffer[256];
        
        /* Each thread uses builtins with different patterns */
        __builtin_memset(thread_buffer, tid, 256);
        
        /* Copy to shared buffer with offset */
        size_t offset = (tid * 64) % 1024;
        __builtin_memcpy(shared_buffer + offset, thread_buffer, 64);
        
        /* Move data within shared buffer */
        if (tid % 2 == 0) {
            __builtin_memmove(shared_buffer + offset + 32, 
                            shared_buffer + offset, 32);
        }
        
        /* Compute checksum */
        for (int i = 0; i < 64; i++) {
            results[tid] += shared_buffer[offset + i];
        }
    }
    
    /* Verify parallel operations */
    long total = 0;
    for (int i = 0; i < 4; i++) {
        total += results[i];
    }
    printf("Parallel checksum: %ld\n", total);
}

/* Multi-stage initialization function */
static void initialize_complex_buffer(char* buf, size_t size) {
    char pattern[] = "ASAN_TEST_PATTERN_1234567890";
    size_t pattern_len = sizeof(pattern) - 1;
    
    /* Fill buffer using __builtin_memset */
    __builtin_memset(buf, 0, size);
    
    /* Copy pattern repeatedly */
    for (size_t i = 0; i < size; i += pattern_len) {
        size_t chunk = (size - i) < pattern_len ? (size - i) : pattern_len;
        __builtin_memcpy(buf + i, pattern, chunk);
    }
    
    /* Move data around */
    if (size > 256) {
        __builtin_memmove(buf + 128, buf, 128);
    }
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast(4, "root_data");
    if (root) {
        process_ast_with_goto(root);
        
        /* Additional memory operations on AST */
        if (root->left && root->right) {
            char merge_buf[128];
            size_t copy_len = root->left->size;
            if (copy_len > root->right->size) {
                copy_len = root->right->size;
            }
            
            __builtin_memcpy(merge_buf, root->left->data, copy_len);
            __builtin_memcpy(merge_buf + copy_len, root->right->data, copy_len);
            __builtin_memset(merge_buf + copy_len * 2, 0xFF, 64);
        }
        
        free(root);
    }
    
    /* Phase 2: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 3: Large buffer operations */
    char large_buffer[2048];
    initialize_complex_buffer(large_buffer, sizeof(large_buffer));
    
    /* Verify buffer content */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(large_buffer); i++) {
        hash = (hash * 31) + large_buffer[i];
    }
    printf("Buffer hash: %lu\n", hash);
    
    /* Phase 4: Edge case with minimal size */
    char tiny_buf[8];
    volatile size_t tiny_size = 4;
    __builtin_memset(tiny_buf, 0xCC, tiny_size);
    __builtin_memcpy(tiny_buf + 2, tiny_buf, 2);
    
    printf("ASAN test completed successfully\n");
    return 0;
}
