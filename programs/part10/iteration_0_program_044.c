/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_tokens[][32] = {
    "memcpy_test_token_1",
    "memset_test_token_2", 
    "memmove_test_token_3",
    "asan_coverage_token_4"
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    /* Force initialization of ASAN globals */
    char buffer[128];
    __builtin_memset(buffer, 0, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    /* Ensure destructor path is exercised */
    volatile char cleanup_buf[64];
    __builtin_memset((void*)cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_recursive(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with builtin memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) 
        copy_len = sizeof(node->data);
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->size = copy_len;
    
    /* Create children with goto-based control flow */
    if (depth > 1) {
        int use_left = depth % 2;
        
        if (use_left) {
            /* Jump into block with memmove */
            goto create_left;
        } else {
            /* Jump around memmove */
            goto skip_memmove;
        }
        
    create_left:
        {
            char temp[64];
            __builtin_memmove(temp, node->data, node->size);
            node->left = create_ast_recursive(depth - 1, temp);
        }
        
    skip_memmove:
        node->right = create_ast_recursive(depth - 1, "right_branch");
    }
    
    return node;
}

/* Calculate hash from AST */
static size_t hash_ast(const ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 0;
    for (size_t i = 0; i < node->size && i < sizeof(node->data); i++) {
        hash = hash * 31 + node->data[i];
    }
    
    return hash + hash_ast(node->left) + hash_ast(node->right);
}

/* Parallel memory operations */
static void parallel_memory_operations(void) {
    const int num_ops = 4;
    char* buffers[num_ops];
    
    /* Allocate buffers with volatile size */
    for (int i = 0; i < num_ops; i++) {
        buffers[i] = (char*)malloc(g_mem_size);
        if (!buffers[i]) return;
    }
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0: {
                /* Use builtin memcpy */
                char pattern[32];
                __builtin_memset(pattern, thread_id + 'A', sizeof(pattern));
                __builtin_memcpy(buffers[thread_id], pattern, 
                               sizeof(pattern) < g_mem_size ? sizeof(pattern) : g_mem_size);
                break;
            }
            case 1: {
                /* Use builtin memset with goto */
                if (thread_id > 0) {
                    goto do_memset;
                } else {
                    goto do_memmove;
                }
                
            do_memset:
                __builtin_memset(buffers[thread_id], thread_id + '0', g_mem_size);
                break;
                
            do_memmove:
                __builtin_memmove(buffers[thread_id], buffers[(thread_id + 1) % num_ops], 
                                g_mem_size / 2);
                break;
            }
            case 2: {
                /* Use builtin memmove between buffers */
                int src_idx = (thread_id + 1) % num_ops;
                __builtin_memmove(buffers[thread_id], buffers[src_idx], g_mem_size);
                break;
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_ops; i++) {
        free(buffers[i]);
    }
}

/* Main test driver */
int main(void) {
    size_t total_hash = 0;
    
    /* Phase 1: Initialize and verify constructor ran */
    if (!g_init_flag) {
        fprintf(stderr, "Constructor not executed\n");
        return 1;
    }
    
    /* Phase 2: Create recursive AST structures */
    ASTNode* root = NULL;
    for (int i = 0; i < 4; i++) {
        ASTNode* tree = create_ast_recursive(3, g_tokens[i]);
        if (tree) {
            total_hash += hash_ast(tree);
            
            /* Free tree with memory operations */
            ASTNode* temp = tree;
            while (temp) {
                ASTNode* next = temp->right;
                __builtin_memset(temp, 0, sizeof(ASTNode));
                free(temp);
                temp = next;
            }
        }
    }
    
    /* Phase 3: Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 4: Complex memory pattern with gotos */
    {
        char buffer1[256];
        char buffer2[256];
        volatile int use_memmove = 1;
        
        __builtin_memset(buffer1, 'X', sizeof(buffer1));
        
        if (use_memmove) {
            goto perform_memmove;
        } else {
            goto perform_memcpy;
        }
        
    perform_memmove:
        __builtin_memmove(buffer2, buffer1, sizeof(buffer1));
        goto after_ops;
        
    perform_memcpy:
        __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
        
    after_ops:
        /* Verify the copy */
        for (size_t i = 0; i < sizeof(buffer1); i++) {
            total_hash += buffer2[i];
        }
    }
    
    /* Phase 5: Variable-length memory operations */
    {
        volatile size_t sizes[] = {16, 32, 64, 128};
        char* dyn_buf = (char*)malloc(256);
        
        if (dyn_buf) {
            for (int i = 0; i < 4; i++) {
                char pattern[128];
                __builtin_memset(pattern, 'A' + i, sizeof(pattern));
                
                /* Alternate between memcpy and memmove */
                if (i % 2 == 0) {
                    __builtin_memcpy(dyn_buf, pattern, sizes[i]);
                } else {
                    __builtin_memmove(dyn_buf, pattern, sizes[i]);
                }
                
                /* Add to hash */
                for (size_t j = 0; j < sizes[i]; j++) {
                    total_hash += dyn_buf[j];
                }
            }
            free(dyn_buf);
        }
    }
    
    printf("Total hash: %zu\n", total_hash % 1000000);
    return 0;
}
