/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t hash;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 128; /* Force non-constant size */
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Use __builtin_memcpy to copy base data */
    size_t copy_len = strlen(base) < 31 ? strlen(base) : 31;
    __builtin_memcpy(node->data, base, copy_len);
    node->data[copy_len] = '\0';
    
    /* Create children with goto-based control flow */
    int create_left = 1;
    
    /* Jump into memory operation block */
    if (depth > 2) {
        goto create_children;
    }
    
    skip_children:
    node->left = NULL;
    node->right = NULL;
    goto compute_hash;
    
    create_children:
    {
        char left_name[32];
        char right_name[32];
        
        /* Use __builtin_memcpy with volatile size */
        volatile size_t name_len = 8;
        __builtin_memcpy(left_name, "left_", 5);
        __builtin_memcpy(right_name, "right_", 6);
        
        node->left = create_ast(depth - 1, left_name);
        node->right = create_ast(depth - 1, right_name);
    }
    
    compute_hash:
    /* Compute hash using memory operations */
    node->hash = 0;
    for (size_t i = 0; i < sizeof(node->data); i++) {
        node->hash = (node->hash * 31) + node->data[i];
    }
    
    return node;
}

/* Function with goto jumping around memmove */
static void manipulate_ast_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_memmove = 1;
    
    if (src == dst) {
        goto skip_copy;
    }
    
    copy_block:
    {
        /* Use __builtin_memmove for overlapping regions */
        char buffer[64];
        volatile size_t copy_size = g_mem_size > 64 ? 64 : g_mem_size;
        
        /* Copy src->data to buffer */
        __builtin_memcpy(buffer, src->data, 32);
        
        /* Move from buffer to dst->data (simulating overlap scenario) */
        __builtin_memmove(dst->data, buffer, 32);
        
        /* Jump out of the block */
        goto after_copy;
    }
    
    skip_copy:
    printf("Skipping copy (src == dst)\n");
    goto finish;
    
    after_copy:
    /* Modify using memset */
    __builtin_memset(dst->data + 16, 'X', 8);
    
    finish:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    const int num_buffers = 8;
    char* buffers[num_buffers];
    
    /* Allocate buffers */
    for (int i = 0; i < num_buffers; i++) {
        buffers[i] = (char*)malloc(g_mem_size);
        if (buffers[i]) {
            __builtin_memset(buffers[i], i, g_mem_size);
        }
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < num_buffers; i++) {
            if (buffers[i]) {
                /* Each thread performs memory operations */
                char temp[64];
                volatile size_t op_size = (g_mem_size + thread_id) % 64 + 1;
                
                /* Force builtin calls in parallel region */
                __builtin_memcpy(temp, buffers[i], op_size);
                __builtin_memset(buffers[i] + op_size/2, thread_id, op_size/2);
                __builtin_memmove(buffers[i], temp, op_size);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_buffers; i++) {
        if (buffers[i]) {
            free(buffers[i]);
        }
    }
}

/* Multi-stage initialization with memory builtins */
static uint32_t process_ast_tree(ASTNode* root) {
    if (!root) return 0;
    
    uint32_t total_hash = 0;
    ASTNode* nodes[4];
    int node_count = 0;
    
    /* Collect nodes */
    nodes[node_count++] = root;
    if (root->left) nodes[node_count++] = root->left;
    if (root->right) nodes[node_count++] = root->right;
    if (root->left && root->left->left) nodes[node_count++] = root->left->left;
    
    /* Process nodes with memory operations */
    for (int i = 0; i < node_count; i++) {
        ASTNode* current = nodes[i];
        
        /* Create temporary copy */
        ASTNode temp;
        __builtin_memcpy(&temp, current, sizeof(ASTNode));
        
        /* Modify copy */
        __builtin_memset(temp.data + 8, 'M', 4);
        
        /* Move back if not root */
        if (i > 0) {
            __builtin_memmove(current->data, temp.data, 32);
        }
        
        total_hash ^= current->hash;
    }
    
    return total_hash;
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Initialize and create AST */
    ASTNode* root = create_ast(4, "root_node");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Test goto-based control flow with memmove */
    printf("Testing goto-based memmove...\n");
    manipulate_ast_with_goto(root, root->left);
    manipulate_ast_with_goto(root->left, root->right);
    
    /* Phase 3: Parallel memory operations */
    printf("Testing parallel memory operations...\n");
    parallel_memory_operations();
    
    /* Phase 4: Multi-stage processing */
    printf("Processing AST tree...\n");
    uint32_t final_hash = process_ast_tree(root);
    
    /* Phase 5: Additional builtin stress tests */
    {
        char buffer1[256];
        char buffer2[256];
        volatile size_t size1 = g_mem_size;
        volatile size_t size2 = g_mem_size / 2;
        
        /* Chain of memory operations */
        __builtin_memset(buffer1, 0xAA, size1);
        __builtin_memcpy(buffer2, buffer1, size2);
        __builtin_memset(buffer1 + size2, 0xBB, size1 - size2);
        __builtin_memmove(buffer2 + 64, buffer1, 128);
        
        /* Verify by computing checksum */
        uint32_t checksum = 0;
        for (size_t i = 0; i < sizeof(buffer2); i++) {
            checksum += buffer2[i];
        }
        final_hash ^= checksum;
    }
    
    printf("Final hash: 0x%08X\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real ASAN, this would detect leaks */
    
    return 0;
}
