/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char global_tokens[4096];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_constructor(void) {
    /* Initialize with memset builtin */
    __builtin_memset(global_tokens, 0, sizeof(global_tokens));
    
    /* Fill with pattern using memcpy builtin */
    char pattern[64];
    __builtin_memset(pattern, 'A', sizeof(pattern));
    
    for (int i = 0; i < 64; i++) {
        __builtin_memcpy(&global_tokens[i * 64], pattern, 64);
    }
    
    volatile_flag = 0;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Cleanup with memset builtin */
    __builtin_memset(global_tokens, 0xFF, sizeof(global_tokens));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset builtin */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with memcpy from global tokens */
    size_t copy_len = (id % 256) + 1;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    
    __builtin_memcpy(node->data, &global_tokens[id * 64], copy_len);
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast_node(depth - 1, id * 2);
        node->right = create_ast_node(depth - 1, id * 2 + 1);
        
        if (0) {
create_children:
            /* Alternative path with memmove builtin */
            ASTNode temp;
            __builtin_memcpy(&temp, node, sizeof(ASTNode));
            __builtin_memmove(node->data, temp.data, sizeof(node->data));
            
            node->left = create_ast_node(depth - 1, id * 2 + 100);
            node->right = create_ast_node(depth - 1, id * 2 + 101);
        }
    }
    
    return node;
}

/* Copy between AST nodes with complex flow */
static void copy_ast_data(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    volatile int copy_direction = volatile_flag;
    
    /* Complex flow with goto around memmove */
    if (copy_direction) {
        goto forward_copy;
    } else {
        goto reverse_copy;
    }
    
forward_copy:
    /* Normal memcpy path */
    __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    return;
    
reverse_copy:
    /* Reverse copy using memmove for overlap */
    char buffer[sizeof(dest->data)];
    __builtin_memcpy(buffer, src->data, sizeof(buffer));
    __builtin_memmove(dest->data, buffer, sizeof(dest->data));
    
    /* Jump back */
    goto forward_copy;
}

/* Calculate hash of AST */
static unsigned long hash_ast(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Process data with volatile length */
    size_t len = volatile_len % sizeof(node->data);
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + ptr[i];
    }
    
    hash ^= hash_ast(node->left);
    hash ^= hash_ast(node->right);
    
    return hash;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear sensitive data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

/* Main parallel processing function */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf1[1024];
        char local_buf2[1024];
        
        /* Initialize with memset builtin */
        __builtin_memset(local_buf1, thread_id, sizeof(local_buf1));
        __builtin_memset(local_buf2, 0, sizeof(local_buf2));
        
        /* Copy with memcpy builtin */
        size_t copy_size = (thread_id * 64 + 128) % sizeof(local_buf1);
        __builtin_memcpy(local_buf2, local_buf1, copy_size);
        
        /* Overlapping move with memmove builtin */
        if (copy_size > 256) {
            __builtin_memmove(local_buf1 + 128, local_buf1, 256);
        }
        
        /* Atomic update of global token */
        #pragma omp atomic
        token_index++;
        
        /* Copy to global array with volatile length */
        size_t global_offset = (thread_id * 128) % sizeof(global_tokens);
        size_t copy_len = volatile_len % 512;
        
        if (global_offset + copy_len <= sizeof(global_tokens)) {
            __builtin_memcpy(&global_tokens[global_offset], 
                           local_buf2, copy_len);
        }
    }
}

/* Varied functional contexts for builtins */
static void test_builtin_variants(void) {
    /* Test 1: Small buffers */
    char buf1[32], buf2[32];
    __builtin_memset(buf1, 0xAA, 32);
    __builtin_memcpy(buf2, buf1, 32);
    __builtin_memmove(buf1 + 8, buf1, 16);
    
    /* Test 2: Medium buffers with volatile */
    char* medium1 = malloc(volatile_len % 4096 + 1);
    char* medium2 = malloc(volatile_len % 4096 + 1);
    
    if (medium1 && medium2) {
        __builtin_memset(medium1, 0xCC, volatile_len % 4096);
        __builtin_memcpy(medium2, medium1, volatile_len % 4096);
        __builtin_memmove(medium1 + 128, medium1, 256);
    }
    
    free(medium1);
    free(medium2);
    
    /* Test 3: Large overlapping regions */
    static char large_buf[8192];
    __builtin_memset(large_buf, 0x11, sizeof(large_buf));
    __builtin_memmove(large_buf + 4096, large_buf, 4096);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize and test basic builtins */
    test_builtin_variants();
    
    /* Phase 2: Create recursive AST structure */
    ASTNode* root = create_ast_node(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 4: Copy between AST nodes */
    if (root->left && root->right) {
        copy_ast_data(root->left, root->right);
        copy_ast_data(root->right, root->left);
    }
    
    /* Phase 5: Calculate and print verification hash */
    unsigned long hash = hash_ast(root);
    printf("AST Hash: %lu\n", hash);
    printf("Token index: %d\n", token_index);
    
    /* Phase 6: Cleanup */
    free_ast(root);
    
    /* Final builtin test */
    char final_buf[256];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf, "Test Complete", 14);
    
    printf("Test completed successfully.\n");
    return 0;
}
