/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _OPENMP
#include <omp.h>
#endif

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char* data;
    size_t size;
    uint32_t hash;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 256 + (rand() % 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with volatile-controlled size */
    size_t data_size = g_mem_size / (depth + 1);
    node->size = data_size;
    node->data = (char*)malloc(data_size);
    
    if (node->data) {
        /* Use __builtin_memset with volatile size */
        __builtin_memset(node->data, 0, data_size);
        
        /* Copy base data using __builtin_memcpy */
        size_t copy_len = data_size < strlen(base_data) ? data_size : strlen(base_data);
        __builtin_memcpy(node->data, base_data, copy_len);
        
        /* Calculate hash */
        node->hash = 0;
        for (size_t i = 0; i < copy_len; i++) {
            node->hash = (node->hash * 31) + node->data[i];
        }
    }
    
    /* Recursive creation with goto for control flow */
    int create_left = 1;
    if (depth > 2) {
        create_left = (rand() % 2);
    }
    
    if (create_left) {
        node->left = create_ast(depth - 1, base_data);
    } else {
        node->left = NULL;
        goto skip_right;
    }
    
    node->right = create_ast(depth - 2, base_data);
    
skip_right:
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst || !src->data || !dst->data) return;
    
    int use_memmove = 0;
    
    /* Jump into memory operation block */
    if (src->size > 128) {
        use_memmove = 1;
        goto mem_operation;
    }
    
    /* Regular path */
    if (src->data != dst->data) {
        __builtin_memcpy(dst->data, src->data, 
                        src->size < dst->size ? src->size : dst->size);
    }
    return;
    
mem_operation:
    /* This block is entered via goto */
    if (use_memmove && src->data && dst->data) {
        /* Use __builtin_memmove with overlapping regions */
        size_t min_size = src->size < dst->size ? src->size : dst->size;
        __builtin_memmove(dst->data, src->data, min_size);
        
        /* Jump back out */
        goto finish;
    }
    
finish:
    /* Update hash */
    dst->hash = src->hash;
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        #pragma omp for
        for (int i = 0; i < count - 1; i++) {
            if (nodes[i] && nodes[i+1]) {
                /* Mix of memory operations */
                if (thread_id % 3 == 0) {
                    /* memset pattern */
                    __builtin_memset(nodes[i]->data, thread_id, 
                                   nodes[i]->size / 2);
                } else if (thread_id % 3 == 1) {
                    /* memcpy between nodes */
                    size_t copy_size = nodes[i]->size < nodes[i+1]->size ? 
                                      nodes[i]->size : nodes[i+1]->size;
                    __builtin_memcpy(nodes[i+1]->data, nodes[i]->data, copy_size);
                } else {
                    /* memmove with potential overlap */
                    size_t move_size = nodes[i]->size / 3;
                    if (nodes[i]->data + move_size < nodes[i]->data + nodes[i]->size) {
                        __builtin_memmove(nodes[i]->data + move_size, 
                                         nodes[i]->data, move_size);
                    }
                }
            }
        }
    }
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node->data);
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create complex token array */
    const char* tokens[] = {
        "memcpy_test", "memset_pattern", "memmove_overlap",
        "asan_redirect", "hwasan_check", "builtin_flow"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Create AST structures */
    ASTNode* ast_pool[10];
    int ast_count = 0;
    
    for (int i = 0; i < 6 && i < token_count; i++) {
        ast_pool[ast_count] = create_ast(3 + (i % 3), tokens[i]);
        if (ast_pool[ast_count]) {
            ast_count++;
        }
    }
    
    /* Process with goto control flow */
    for (int i = 0; i < ast_count - 1; i++) {
        process_with_goto(ast_pool[i], ast_pool[i+1]);
    }
    
    /* Execute parallelized memory dispatch */
    parallel_memory_ops(ast_pool, ast_count);
    
    /* Verify operations with hash sum */
    uint64_t total_hash = 0;
    for (int i = 0; i < ast_count; i++) {
        if (ast_pool[i]) {
            total_hash += ast_pool[i]->hash;
            
            /* Additional memory operation in verification */
            char verify_buf[64];
            __builtin_memset(verify_buf, 0, sizeof(verify_buf));
            __builtin_memcpy(verify_buf, ast_pool[i]->data, 
                           ast_pool[i]->size < 63 ? ast_pool[i]->size : 63);
            
            /* Use memmove for verification */
            char verify_buf2[64];
            __builtin_memmove(verify_buf2, verify_buf, sizeof(verify_buf));
        }
    }
    
    printf("Total hash sum: %llu\n", (unsigned long long)total_hash);
    printf("AST nodes processed: %d\n", ast_count);
    
    /* Cleanup */
    for (int i = 0; i < ast_count; i++) {
        free_ast(ast_pool[i]);
    }
    
    /* Final memory operation to ensure all builtins are used */
    volatile char final_buf[128];
    volatile char final_src[128];
    
    for (int i = 0; i < 128; i++) {
        final_src[i] = (char)(i % 256);
    }
    
    __builtin_memcpy((void*)final_buf, (void*)final_src, 128);
    __builtin_memset((void*)(final_buf + 64), 0xFF, 64);
    __builtin_memmove((void*)(final_buf + 32), (void*)final_buf, 64);
    
    printf("ASAN test completed successfully\n");
    return 0;
}
