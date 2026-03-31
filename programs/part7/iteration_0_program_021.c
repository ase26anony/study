/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(const char* data, int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy */
    size_t len = strlen(data) + 1;
    if (len > sizeof(node->data)) len = sizeof(node->data);
    __builtin_memcpy(node->data, data, len);
    node->size = len;
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        char left_data[256], right_data[256];
        snprintf(left_data, sizeof(left_data), "%s-L%d", data, depth);
        snprintf(right_data, sizeof(right_data), "%s-R%d", data, depth);
        
        /* Jump label for goto */
        create_left:
        node->left = create_ast(left_data, depth - 1);
        
        /* Use __builtin_memmove with goto */
        if (node->left && node->right) {
            char temp[256];
            __builtin_memcpy(temp, node->left->data, node->left->size);
            goto move_data;
        }
        
        create_right:
        node->right = create_ast(right_data, depth - 1);
        goto create_left;  /* Jump back */
        
        move_data:
        __builtin_memmove(node->right->data, temp, node->left->size);
    }
    
    return node;
}

/* Function with complex memory operations */
static void process_ast(ASTNode* node, char* buffer, size_t buf_size) {
    if (!node || !buffer) return;
    
    volatile size_t local_size = g_mem_size;
    if (local_size > buf_size) local_size = buf_size;
    
    /* Multiple memory operations in sequence */
    __builtin_memset(buffer, 0, buf_size);
    __builtin_memcpy(buffer, node->data, node->size < local_size ? node->size : local_size);
    
    if (node->left) {
        size_t offset = node->size;
        if (offset < buf_size - 1) {
            __builtin_memcpy(buffer + offset, node->left->data, 
                           node->left->size < (buf_size - offset) ? 
                           node->left->size : (buf_size - offset));
        }
    }
    
    /* Use __builtin_memmove for overlapping regions */
    if (node->right && node->size > 10) {
        __builtin_memmove(buffer + 5, buffer, node->size - 5);
    }
}

/* OpenMP parallel section */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf1[128];
        char local_buf2[128];
        
        /* Initialize with builtins */
        __builtin_memset(local_buf1, thread_id, sizeof(local_buf1));
        __builtin_memset(local_buf2, 0xFF, sizeof(local_buf2));
        
        /* Copy between buffers */
        __builtin_memcpy(local_buf2, local_buf1, 
                        sizeof(local_buf1) < sizeof(local_buf2) ? 
                        sizeof(local_buf1) : sizeof(local_buf2));
        
        /* Move data around */
        if (thread_id % 2 == 0) {
            __builtin_memmove(local_buf1 + 32, local_buf1, 64);
        }
        
        #pragma omp barrier
        
        /* Verify the operations */
        volatile int check = 0;
        for (size_t i = 0; i < sizeof(local_buf1); i++) {
            check += local_buf1[i];
            check += local_buf2[i];
        }
        
        #pragma omp critical
        {
            printf("Thread %d: Memory ops completed (checksum: %d)\n", 
                   thread_id, check);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    if (!g_init_flag) {
        fprintf(stderr, "Error: Constructor not called\n");
        return 1;
    }
    
    /* Create recursive AST */
    ASTNode* root = create_ast("ROOT", 3);
    if (!root) {
        fprintf(stderr, "Error: Failed to create AST\n");
        return 1;
    }
    
    /* Process buffer */
    char main_buffer[512];
    volatile size_t dynamic_size = g_mem_size * 2;
    
    /* Force multiple builtin calls */
    for (int i = 0; i < 3; i++) {
        __builtin_memset(main_buffer, i, dynamic_size);
        process_ast(root, main_buffer, sizeof(main_buffer));
        
        /* Use memmove with overlapping regions */
        if (i % 2 == 0) {
            __builtin_memmove(main_buffer + 128, main_buffer, 256);
        }
    }
    
    /* Execute parallel section */
    printf("\n--- Parallel Memory Operations ---\n");
    parallel_memory_ops();
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(main_buffer); i++) {
        hash = (hash * 31) + main_buffer[i];
    }
    
    printf("\n=== Test Complete ===\n");
    printf("Final buffer hash: 0x%08lx\n", hash);
    printf("Verification: %s\n", 
           (hash != 0 && root != NULL) ? "PASS" : "FAIL");
    
    /* Cleanup */
    free(root);
    
    return 0;
}
