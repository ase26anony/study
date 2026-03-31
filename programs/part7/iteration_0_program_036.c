/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force early initialization of ASAN runtime */
    volatile char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_late(void) {
    /* Final memory operation to ensure coverage */
    volatile char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_tree(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Create pattern data with memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) pattern[i] = (char)(i + depth);
    __builtin_memcpy(node->data, pattern, 64);
    
    node->size = 64;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_left = depth % 2;
        
        if (use_left) {
            node->left = create_tree(depth - 1);
            goto skip_right;
        }
        
        node->right = create_tree(depth - 1);
        skip_right:
        /* memmove between nodes if both exist */
        if (node->left && node->right) {
            __builtin_memmove(node->left->data + 16, 
                            node->right->data + 8, 32);
        }
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void goto_mem_operations(char* dest, char* src, size_t n) {
    int do_copy = 1;
    
    if (n > 100) {
        goto large_copy;
    } else {
        goto small_copy;
    }
    
large_copy:
    {
        /* This block should trigger memcpy redirection */
        __builtin_memcpy(dest, src, n);
        goto end;
    }
    
small_copy:
    {
        /* Different path for smaller copies */
        if (g_use_memmove) {
            __builtin_memmove(dest, src, n);
        } else {
            __builtin_memcpy(dest, src, n);
        }
    }
    
end:
    return;
}

/* OpenMP parallel region with memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[128];
        char shared_buf[256];
        
        /* Thread-specific initialization */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Collective memory operation */
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            size_t size = g_mem_size / (i + 1);
            __builtin_memcpy(shared_buf + i * 64, 
                           local_buf, 
                           size > 64 ? 64 : size);
        }
        
        /* Conditional memmove */
        if (tid % 2 == 0) {
            __builtin_memmove(local_buf + 32, local_buf, 32);
        }
    }
}

/* Multi-stage processing function */
static size_t process_ast(ASTNode* root) {
    if (!root) return 0;
    
    size_t hash = 0;
    char temp[64];
    
    /* Process current node */
    __builtin_memcpy(temp, root->data, root->size);
    
    for (size_t i = 0; i < root->size; i++) {
        hash = hash * 31 + temp[i];
    }
    
    /* Recursive processing with goto */
    if (root->left) {
        hash += process_ast(root->left);
        if (root->right) {
            goto process_right;
        }
        return hash;
    }
    
process_right:
    if (root->right) {
        /* Use memmove to shift data before processing */
        char shifted[64];
        __builtin_memmove(shifted, root->right->data + 8, 56);
        __builtin_memset(shifted + 56, 0, 8);
        
        for (int i = 0; i < 56; i++) {
            hash += shifted[i];
        }
        
        hash += process_ast(root->right);
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Initialize data structures */
    ASTNode* tree = create_tree(4);
    if (!tree) {
        fprintf(stderr, "Failed to create tree\n");
        return 1;
    }
    
    /* Phase 2: Perform goto-based memory operations */
    char src_buffer[512];
    char dst_buffer[512];
    
    /* Initialize source with pattern */
    for (int i = 0; i < 512; i++) {
        src_buffer[i] = (char)(i % 256);
    }
    
    /* Test various memory operation patterns */
    goto_mem_operations(dst_buffer, src_buffer, 200);
    goto_mem_operations(dst_buffer + 100, src_buffer + 50, 75);
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Phase 4: Process AST with recursive memory operations */
    size_t result_hash = process_ast(tree);
    
    /* Phase 5: Final builtin calls to ensure coverage */
    volatile char final_src[128];
    volatile char final_dst[128];
    
    __builtin_memset(final_src, 0xAA, sizeof(final_src));
    __builtin_memcpy(final_dst, final_src, sizeof(final_dst));
    __builtin_memmove(final_dst + 32, final_dst, 64);
    
    /* Cleanup */
    /* Note: In real ASAN, memory would be freed automatically */
    
    printf("Processing complete. Hash: %zu\n", result_hash);
    printf("All memory operations executed\n");
    
    return 0;
}
