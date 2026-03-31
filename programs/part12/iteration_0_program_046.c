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
static void init_asan_environment(void) {
    printf("Initializing ASAN test environment...\n");
    /* Force early initialization of memory functions */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Cleaning up ASAN test environment...\n");
}

/* Function with goto control flow */
static void test_memmove_with_goto(char* dest, const char* src, size_t n) {
    int use_copy = 1;
    
    if (n == 0) goto skip_copy;
    
    /* Jump into memory operation block */
    goto perform_copy;
    
perform_copy:
    /* This tests flow-sensitivity of asan_memfn_rtls retrieval */
    __builtin_memmove(dest, src, n);
    
    if (use_copy) {
        /* Another memory operation after goto */
        __builtin_memset(dest + n/2, 'X', n/4);
        goto skip_copy;
    }
    
skip_copy:
    return;
}

/* Recursive function using memory built-ins */
static ASTNode* create_ast_node(const char* data, size_t depth) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to control size */
    size_t copy_size = g_mem_size % 256;
    
    /* Force built-in memcpy usage */
    __builtin_memcpy(node->data, data, copy_size);
    __builtin_memset(node->data + copy_size, 0, sizeof(node->data) - copy_size);
    
    node->size = copy_size;
    
    if (depth > 0) {
        char child_data[256];
        __builtin_snprintf(child_data, sizeof(child_data), 
                          "Child-%zu-%s", depth, data);
        
        node->left = create_ast_node(child_data, depth - 1);
        node->right = create_ast_node(child_data, depth - 1);
        
        /* Copy between nodes */
        if (node->left && node->right) {
            size_t min_size = node->left->size < node->right->size ? 
                            node->left->size : node->right->size;
            __builtin_memcpy(node->right->data, node->left->data, min_size);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with complex memory operations */
static size_t process_ast_tree(ASTNode* root) {
    if (!root) return 0;
    
    size_t hash = 0;
    char temp_buffer[512];
    
    /* Test memmove with overlapping regions */
    __builtin_memcpy(temp_buffer, root->data, root->size);
    __builtin_memmove(temp_buffer + 128, temp_buffer + 64, 64);
    __builtin_memset(temp_buffer + 192, 0, 64);
    
    /* Calculate simple hash */
    for (size_t i = 0; i < root->size && i < 256; i++) {
        hash = (hash * 31) + temp_buffer[i];
    }
    
    /* Process children recursively */
    hash += process_ast_tree(root->left);
    hash += process_ast_tree(root->right);
    
    return hash;
}

/* OpenMP parallel section */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buffer[1024];
        char shared_buffer[1024];
        
        /* Each thread uses memory built-ins */
        __builtin_memset(local_buffer, thread_id, sizeof(local_buffer));
        
        #pragma omp barrier
        
        #pragma omp master
        {
            /* Master thread collects data */
            __builtin_memset(shared_buffer, 0, sizeof(shared_buffer));
        }
        
        #pragma omp barrier
        
        /* Copy thread data to shared buffer with offset */
        size_t offset = (thread_id * 64) % 512;
        __builtin_memcpy(shared_buffer + offset, 
                        local_buffer + offset, 
                        64);
        
        #pragma omp barrier
        
        /* Move data around in shared buffer */
        if (thread_id % 2 == 0) {
            __builtin_memmove(shared_buffer + 256, 
                            shared_buffer + 128, 
                            128);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create complex AST structure */
    ASTNode* root = create_ast_node("RootNode", 3);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process tree with memory operations */
    size_t tree_hash = process_ast_tree(root);
    printf("AST tree hash: %zu\n", tree_hash);
    
    /* Test goto control flow */
    char src_buffer[256];
    char dest_buffer[256];
    
    for (size_t i = 0; i < sizeof(src_buffer); i++) {
        src_buffer[i] = (char)(i % 256);
    }
    
    test_memmove_with_goto(dest_buffer, src_buffer, sizeof(dest_buffer));
    
    /* Verify the copy */
    int copy_ok = 1;
    for (size_t i = 0; i < sizeof(dest_buffer); i++) {
        if (dest_buffer[i] != src_buffer[i] && 
            !(i >= sizeof(dest_buffer)/2 && i < sizeof(dest_buffer)/2 + sizeof(dest_buffer)/4)) {
            copy_ok = 0;
            break;
        }
    }
    printf("Memmove with goto test: %s\n", copy_ok ? "PASS" : "FAIL");
    
    /* Execute parallel operations */
    #ifdef _OPENMP
    printf("Running OpenMP memory operations...\n");
    parallel_memory_operations();
    printf("OpenMP operations completed\n");
    #endif
    
    /* Test various memory operation sizes */
    char* dynamic_buffer = malloc(4096);
    if (dynamic_buffer) {
        /* Use all three built-ins with dynamic sizes */
        __builtin_memset(dynamic_buffer, 0xAA, 1024);
        __builtin_memcpy(dynamic_buffer + 1024, dynamic_buffer, 1024);
        __builtin_memmove(dynamic_buffer + 2048, dynamic_buffer + 512, 1536);
        
        /* Verify pattern */
        int pattern_ok = 1;
        for (size_t i = 0; i < 1024; i++) {
            if (dynamic_buffer[i] != (char)0xAA) {
                pattern_ok = 0;
                break;
            }
        }
        printf("Dynamic buffer pattern test: %s\n", pattern_ok ? "PASS" : "FAIL");
        
        free(dynamic_buffer);
    }
    
    /* Cleanup */
    /* Note: In real code, you'd need a proper tree deletion function */
    
    printf("ASAN test completed successfully\n");
    return 0;
}
