/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
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
    g_mem_size = 256 + (rand() % 768);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with __builtin_memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) 
        copy_len = sizeof(node->data);
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->size = copy_len;
    
    /* Create children with modified data */
    char left_data[256];
    char right_data[256];
    
    __builtin_memset(left_data, 0, sizeof(left_data));
    __builtin_memset(right_data, 0, sizeof(right_data));
    
    __builtin_memcpy(left_data, base_data, copy_len);
    __builtin_memcpy(right_data, base_data, copy_len);
    
    /* Modify data for children */
    left_data[0] = 'L';
    right_data[0] = 'R';
    
    /* Goto-based control flow for memmove testing */
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth - 1, left_data);
    node->right = create_ast(depth - 1, right_data);
    
    /* Use __builtin_memmove to shift data between nodes */
    if (node->left && node->right) {
        volatile size_t move_size = node->left->size;
        if (move_size > sizeof(node->right->data))
            move_size = sizeof(node->right->data);
        
        __builtin_memmove(node->right->data, node->left->data, move_size);
    }
    
done:
    return node;
}

/* Function with goto jumping into memory block */
static void test_goto_memmove(void* dest, void* src, size_t n) {
    volatile int use_memmove = 1;
    
    if (use_memmove) {
        goto perform_move;
    }
    
    /* This block should be jumped into */
    __builtin_memset(dest, 0, n);
    goto end;
    
perform_move:
    __builtin_memmove(dest, src, n);
    
end:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char buffer[512];
        char src_buffer[512];
        
        /* Initialize with memset */
        __builtin_memset(buffer, thread_id, sizeof(buffer));
        __builtin_memset(src_buffer, 255 - thread_id, sizeof(src_buffer));
        
        #pragma omp barrier
        
        /* Copy between buffers */
        volatile size_t copy_size = g_mem_size % 512;
        __builtin_memcpy(buffer, src_buffer, copy_size);
        
        #pragma omp barrier
        
        /* Move data within buffer */
        if (thread_id % 2 == 0) {
            __builtin_memmove(buffer + 128, buffer, 256);
        }
    }
}

/* Calculate hash of AST tree */
static size_t hash_ast(const ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 5381;
    for (size_t i = 0; i < node->size && i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    return hash + hash_ast(node->left) + hash_ast(node->right);
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Test 1: Direct built-in calls with volatile sizes */
    {
        char buffer1[1024];
        char buffer2[1024];
        volatile size_t op_size = g_mem_size % 512;
        
        __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
        __builtin_memcpy(buffer2, buffer1, op_size);
        __builtin_memmove(buffer1 + 128, buffer1, 256);
        
        printf("Test 1: Direct built-ins completed\n");
    }
    
    /* Test 2: Goto-based control flow */
    {
        char data_block[256];
        char temp_block[256];
        
        for (int i = 0; i < sizeof(data_block); i++) {
            data_block[i] = i % 256;
        }
        
        test_goto_memmove(temp_block, data_block, 128);
        printf("Test 2: Goto memmove completed\n");
    }
    
    /* Test 3: Recursive AST operations */
    ASTNode* root = create_ast(4, "RootNodeData");
    if (root) {
        size_t tree_hash = hash_ast(root);
        printf("Test 3: AST hash = %zu\n", tree_hash);
        
        /* Cleanup would normally be here, but we want to test memory */
    }
    
    /* Test 4: OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_ops();
    printf("Test 4: OpenMP parallel ops completed\n");
    #endif
    
    /* Test 5: Mixed operations in loop */
    {
        char* dynamic_buf = (char*)malloc(2048);
        if (dynamic_buf) {
            volatile int iterations = 10;
            
            for (int i = 0; i < iterations; i++) {
                size_t offset = (i * 137) % 2048;
                size_t len = 64 + (i * 23) % 192;
                
                if (i % 3 == 0) {
                    __builtin_memset(dynamic_buf + offset, i, len);
                } else if (i % 3 == 1) {
                    __builtin_memcpy(dynamic_buf + offset, dynamic_buf, len);
                } else {
                    __builtin_memmove(dynamic_buf + offset, dynamic_buf + 512, len);
                }
            }
            
            free(dynamic_buf);
            printf("Test 5: Mixed loop operations completed\n");
        }
    }
    
    printf("All ASAN tests completed successfully\n");
    return 0;
}
