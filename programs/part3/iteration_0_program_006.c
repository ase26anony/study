/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char buffer[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
    unsigned long hash;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    /* Force initialization of ASAN runtime */
    char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    /* Final memory operation to ensure cleanup paths are taken */
    volatile char final_buf[8];
    __builtin_memset((void*)final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast_tree(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with volatile size */
    size_t copy_len = g_mem_size % 128;
    if (copy_len > sizeof(node->buffer) - 1)
        copy_len = sizeof(node->buffer) - 1;
    
    __builtin_memcpy(node->buffer, base_data, copy_len);
    node->buffer[copy_len] = '\0';
    
    /* Calculate hash using memory operations */
    for (size_t i = 0; i < copy_len; i++) {
        node->hash = (node->hash * 31) + node->buffer[i];
    }
    
    node->size = copy_len;
    
    /* Recursive creation with goto for control flow testing */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast_tree(depth - 1, node->buffer);
        node->right = NULL;
        
        create_children:
        /* Jump into block with __builtin_memmove */
        {
            char temp[256];
            __builtin_memcpy(temp, node->buffer, node->size);
            
            /* Test memmove with overlapping regions */
            __builtin_memmove(node->buffer + 10, node->buffer, 
                            node->size > 20 ? 20 : node->size);
            
            /* Restore original */
            __builtin_memmove(node->buffer, temp, node->size);
        }
        
        if (!use_goto) {
            node->right = create_ast_tree(depth - 2, node->buffer);
        } else {
            node->right = create_ast_tree(depth - 1, node->buffer + 10);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with complex memory pattern */
static void process_ast_nodes(ASTNode* node, unsigned long* total_hash) {
    if (!node) return;
    
    /* Process current node */
    *total_hash ^= node->hash;
    
    /* Memory operations between nodes */
    if (node->left && node->right) {
        size_t copy_size = (node->left->size < node->right->size) ? 
                          node->left->size : node->right->size;
        
        if (copy_size > 0) {
            /* Use all three builtins in sequence */
            char temp[256];
            __builtin_memcpy(temp, node->left->buffer, copy_size);
            __builtin_memset(node->left->buffer + copy_size/2, '.', copy_size/4);
            __builtin_memmove(node->right->buffer, temp, copy_size);
        }
    }
    
    /* Recursive processing */
    process_ast_nodes(node->left, total_hash);
    process_ast_nodes(node->right, total_hash);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_operations(void) {
    const int num_threads = 4;
    unsigned long thread_results[4] = {0};
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        char local_buf[128];
        volatile size_t local_size = g_mem_size + tid * 16;
        
        /* Each thread uses different memory builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(local_buf, tid, local_size % sizeof(local_buf));
                break;
            case 1:
                {
                    char src[128];
                    for (int i = 0; i < sizeof(src); i++) src[i] = i + tid;
                    __builtin_memcpy(local_buf, src, local_size % sizeof(local_buf));
                }
                break;
            case 2:
                {
                    /* Create overlapping memmove scenario */
                    for (int i = 0; i < sizeof(local_buf); i++) 
                        local_buf[i] = i;
                    __builtin_memmove(local_buf + 32, local_buf, 64);
                }
                break;
        }
        
        /* Compute hash from buffer */
        for (size_t i = 0; i < (local_size % sizeof(local_buf)); i++) {
            thread_results[tid] = thread_results[tid] * 31 + local_buf[i];
        }
    }
    
    /* Combine results with memory operations */
    unsigned long final_result = 0;
    for (int i = 0; i < num_threads; i++) {
        char temp[8];
        __builtin_memcpy(temp, &thread_results[i], sizeof(unsigned long));
        for (int j = 0; j < 8; j++) {
            final_result = final_result * 31 + temp[j];
        }
    }
    
    printf("Parallel result: %lu\n", final_result);
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize with various memory operations */
    char init_buf[1024];
    volatile size_t dynamic_size = g_mem_size * 2;
    
    /* Force all three builtins to be used */
    __builtin_memset(init_buf, 0xAA, dynamic_size % sizeof(init_buf));
    __builtin_memcpy(init_buf + 128, init_buf, 64);
    __builtin_memmove(init_buf + 256, init_buf + 128, 128);
    
    /* Create recursive structure */
    ASTNode* root = create_ast_tree(5, "BaseASTNodeDataForTesting");
    if (!root) {
        fprintf(stderr, "Failed to create AST tree\n");
        return 1;
    }
    
    /* Process tree */
    unsigned long tree_hash = 0;
    process_ast_nodes(root, &tree_hash);
    printf("Tree hash: %lu\n", tree_hash);
    
    /* Execute parallel section */
    parallel_memory_operations();
    
    /* Additional edge case: goto jumping over memory operations */
    {
        int use_memcpy = 1;
        char edge_buf1[64], edge_buf2[64];
        
        if (use_memcpy) {
            goto skip_setup;
        }
        
        __builtin_memset(edge_buf1, 0, sizeof(edge_buf1));
        
        skip_setup:
        /* Jump here with uninitialized buffer */
        for (int i = 0; i < sizeof(edge_buf1); i++) {
            edge_buf1[i] = i;
        }
        
        __builtin_memcpy(edge_buf2, edge_buf1, sizeof(edge_buf1));
        __builtin_memmove(edge_buf1, edge_buf1 + 16, 32);
    }
    
    /* Final verification */
    {
        char verify_buf[32];
        unsigned long verify_hash = 0;
        
        __builtin_memset(verify_buf, 0xCC, sizeof(verify_buf));
        __builtin_memcpy(verify_buf + 8, verify_buf, 16);
        
        for (int i = 0; i < sizeof(verify_buf); i++) {
            verify_hash = verify_hash * 31 + verify_buf[i];
        }
        
        printf("Verification hash: %lu\n", verify_hash);
    }
    
    /* Cleanup */
    /* Note: In real code, you'd need to free the AST tree properly */
    
    printf("Test completed successfully\n");
    return 0;
}
