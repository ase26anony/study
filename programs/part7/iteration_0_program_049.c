/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Function with goto statements for control flow testing */
static void test_memmove_with_goto(void* dest, const void* src, size_t n) {
    int use_builtin = 1;
    
    if (n == 0) goto skip_memmove;
    
    if (use_builtin) {
        /* Force builtin memmove with goto */
        goto do_memmove;
    }
    
skip_memmove:
    return;
    
do_memmove:
    /* This should trigger ASAN's built-in redirection */
    __builtin_memmove(dest, src, n);
    goto skip_memmove;
}

/* Recursive function using memory builtins on AST nodes */
static ASTNode* create_ast_node(int id, const char* data) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset to initialize */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->id = id;
    
    /* Use builtin memcpy for string data */
    size_t len = __builtin_strlen(data);
    if (len > 255) len = 255;
    __builtin_memcpy(node->data, data, len);
    node->data[len] = '\0';
    
    return node;
}

static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Test memcpy between AST nodes */
    __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    
    /* Recursive copy */
    if (src->left) {
        if (!dest->left) dest->left = create_ast_node(src->left->id * 10, "");
        copy_ast_data(dest->left, src->left);
    }
    if (src->right) {
        if (!dest->right) dest->right = create_ast_node(src->right->id * 10, "");
        copy_ast_data(dest->right, src->right);
    }
}

/* Function with complex memory operations */
static unsigned long compute_ast_hash(const ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    const char* p = node->data;
    
    /* Process string with volatile control */
    volatile int i = 0;
    while (*p && i < 256) {
        hash = ((hash << 5) + hash) + *p++;
        i++;
    }
    
    /* Recursive hash computation */
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

/* Main test function with OpenMP parallel section */
static void parallel_memory_operations(void) {
    const size_t buffer_size = g_mem_size;
    char* buffer1 = (char*)malloc(buffer_size);
    char* buffer2 = (char*)malloc(buffer_size);
    
    if (!buffer1 || !buffer2) {
        free(buffer1);
        free(buffer2);
        return;
    }
    
    /* Initialize with builtin memset */
    __builtin_memset(buffer1, 0xAA, buffer_size);
    __builtin_memset(buffer2, 0x55, buffer_size);
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                /* Test memcpy in parallel */
                __builtin_memcpy(buffer1 + thread_id * 8, 
                               buffer2 + thread_id * 8, 
                               16);
                break;
            case 1:
                /* Test memset in parallel */
                __builtin_memset(buffer1 + thread_id * 8, 
                               thread_id, 
                               16);
                break;
            case 2:
                /* Test memmove in parallel */
                if (g_use_memmove) {
                    __builtin_memmove(buffer2 + thread_id * 8,
                                    buffer1 + thread_id * 8,
                                    16);
                }
                break;
        }
    }
    
    /* Verify with another memcpy */
    __builtin_memcpy(buffer2, buffer1, buffer_size / 2);
    
    free(buffer1);
    free(buffer2);
}

/* Function with mixed builtin usage */
static void mixed_builtin_test(void) {
    char temp[128];
    volatile int pattern = 0xDEADBEEF;
    
    /* Chain of memory operations */
    __builtin_memset(temp, pattern & 0xFF, sizeof(temp));
    
    char temp2[128];
    __builtin_memcpy(temp2, temp, sizeof(temp));
    
    /* Overlapping memmove */
    __builtin_memmove(temp + 32, temp + 16, 64);
    
    /* Another memcpy */
    __builtin_memcpy(temp + 96, temp2 + 32, 32);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create complex AST */
    ASTNode* root = create_ast_node(1, "Root node data");
    if (root) {
        root->left = create_ast_node(2, "Left child data with some content");
        root->right = create_ast_node(3, "Right child data with different content");
        
        if (root->left) {
            root->left->left = create_ast_node(4, "Left-left grandchild");
            root->left->right = create_ast_node(5, "Left-right grandchild");
        }
        
        /* Test AST copy with memory operations */
        ASTNode* copy = create_ast_node(100, "Copy root");
        if (copy) {
            copy_ast_data(copy, root);
            
            /* Compute hash to verify data */
            unsigned long hash1 = compute_ast_hash(root);
            unsigned long hash2 = compute_ast_hash(copy);
            printf("AST hash comparison: original=%lu, copy=%lu\n", hash1, hash2);
            
            free(copy);
        }
        
        /* Test with goto control flow */
        test_memmove_with_goto(root->data, root->left->data, 32);
        
        free(root->left->left);
        free(root->left->right);
        free(root->left);
        free(root->right);
        free(root);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Additional mixed tests */
    mixed_builtin_test();
    
    /* Force different memory sizes */
    for (volatile size_t i = 0; i < 4; i++) {
        g_mem_size = 32 << i;  /* 32, 64, 128, 256 */
        parallel_memory_operations();
    }
    
    printf("ASAN test completed successfully\n");
    return 0;
}
