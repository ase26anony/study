/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int use_memmove = 1;

/* Recursive tree structure for AST-like operations */
typedef struct TreeNode {
    char buffer[128];
    struct TreeNode* left;
    struct TreeNode* right;
    int id;
} TreeNode;

/* Global token array */
static char global_tokens[256];

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_global_data(void) {
    /* Initialize with pattern using builtin memset */
    __builtin_memset(global_tokens, 0xAA, sizeof(global_tokens));
    
    /* Force memset redirection in constructor context */
    volatile char local_buf[32];
    __builtin_memset(local_buf, 0x55, sizeof(local_buf));
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_check(void) {
    /* Verify pattern was modified */
    int sum = 0;
    for (size_t i = 0; i < sizeof(global_tokens); i++) {
        sum += global_tokens[i];
    }
    printf("Destructor: Global token checksum = %d\n", sum);
}

/* Recursive tree manipulation with memory operations */
static TreeNode* create_tree(int depth, int id) {
    if (depth <= 0) return NULL;
    
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    if (!node) return NULL;
    
    /* Initialize node buffer with builtin memset */
    __builtin_memset(node->buffer, id, sizeof(node->buffer));
    
    node->id = id;
    node->left = create_tree(depth - 1, id * 2);
    node->right = create_tree(depth - 1, id * 2 + 1);
    
    return node;
}

/* Function with goto jumps around memmove operations */
static void goto_memmove_operations(char* dest, char* src, size_t len) {
    int condition = (len > 32);
    
    if (condition) {
        goto use_memmove;
    } else {
        /* Use memcpy for small copies */
        __builtin_memcpy(dest, src, len);
        goto after_copy;
    }
    
use_memmove:
    /* Jump target containing builtin memmove */
    __builtin_memmove(dest, src, len);
    goto after_copy;
    
after_copy:
    /* Verify copy */
    for (size_t i = 0; i < len; i++) {
        if (dest[i] != src[i]) {
            printf("Copy mismatch at index %zu\n", i);
            break;
        }
    }
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[128];
        char src_buf[128];
        
        /* Initialize source with thread-specific pattern */
        __builtin_memset(src_buf, thread_id, sizeof(src_buf));
        
        /* Copy using builtin memcpy */
        __builtin_memcpy(local_buf, src_buf, sizeof(local_buf));
        
        /* Conditional memmove based on thread ID */
        if (thread_id % 2 == 0) {
            __builtin_memmove(local_buf + 32, local_buf, 64);
        }
        
        #pragma omp barrier
        
        /* Verify operation */
        int valid = 1;
        for (size_t i = 0; i < sizeof(local_buf); i++) {
            if (local_buf[i] != (char)thread_id) {
                valid = 0;
                break;
            }
        }
        
        #pragma omp critical
        {
            printf("Thread %d: Memory verification %s\n", 
                   thread_id, valid ? "PASSED" : "FAILED");
        }
    }
}

/* Complex function mixing all patterns */
static int process_tokens_recursive(TreeNode* node, int depth) {
    if (!node) return 0;
    
    int result = node->id;
    
    /* Copy node buffer to global tokens using volatile length */
    size_t copy_len = volatile_len % sizeof(node->buffer);
    if (copy_len > 0) {
        if (use_memmove) {
            __builtin_memmove(global_tokens + node->id, 
                             node->buffer, 
                             copy_len);
        } else {
            __builtin_memcpy(global_tokens + node->id, 
                            node->buffer, 
                            copy_len);
        }
    }
    
    /* Recursive processing */
    result += process_tokens_recursive(node->left, depth + 1);
    result += process_tokens_recursive(node->right, depth + 1);
    
    return result;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Tree creation and recursive processing */
    TreeNode* root = create_tree(3, 1);
    if (!root) {
        fprintf(stderr, "Failed to create tree\n");
        return 1;
    }
    
    int tree_sum = process_tokens_recursive(root, 0);
    printf("Tree processing sum: %d\n", tree_sum);
    
    /* Phase 2: Goto-based memory operations */
    char src_data[100];
    char dest_data[100];
    
    for (int i = 0; i < 100; i++) {
        src_data[i] = (char)(i % 256);
    }
    
    goto_memmove_operations(dest_data, src_data, 80);
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 4: Direct builtin calls with volatile control */
    volatile int mode = 0;
    char final_buffer[256];
    
    for (int i = 0; i < 5; i++) {
        mode = i % 3;
        
        switch (mode) {
            case 0:
                __builtin_memset(final_buffer, i, sizeof(final_buffer));
                break;
            case 1:
                __builtin_memcpy(final_buffer, src_data, 
                                sizeof(final_buffer) < 100 ? 
                                sizeof(final_buffer) : 100);
                break;
            case 2:
                __builtin_memmove(final_buffer + 50, final_buffer, 100);
                break;
        }
    }
    
    /* Calculate final checksum */
    unsigned long checksum = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        checksum += (unsigned char)final_buffer[i];
    }
    
    printf("Final buffer checksum: %lu\n", checksum);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(root);
    
    return 0;
}
