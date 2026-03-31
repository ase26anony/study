/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_token_array[1024];
static volatile int g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 31) & 0xFF);
    }
    printf("Constructor: Token array initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill data with pattern using volatile size */
    size_t copy_size = g_mem_size % sizeof(node->data);
    if (copy_size > 0) {
        /* Use __builtin_memcpy with volatile control */
        __builtin_memcpy(node->data, &g_token_array[g_token_index], copy_size);
        g_token_index = (g_token_index + (int)copy_size) % sizeof(g_token_array);
    }
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, counter);
        
        create_left:
        node->right = create_ast(depth - 1, counter);
        
        /* Copy between nodes using __builtin_memmove with goto */
        if (node->left && node->right) {
            int should_move = 1;
            if (depth % 2 == 0) {
                goto skip_move;
            }
            
            should_move = 0;
            skip_move:
            
            if (should_move) {
                /* Force memmove redirection */
                __builtin_memmove(node->left->data, node->right->data, 
                                 sizeof(node->data) / 2);
            }
        }
    }
    
    return node;
}

/* Calculate hash of AST */
static unsigned long hash_ast(const ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    
    /* Hash node data */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + (unsigned long)node->data[i];
    }
    
    /* Recursive hash */
    hash ^= hash_ast(node->left);
    hash ^= hash_ast(node->right);
    hash ^= (unsigned long)node->id;
    
    return hash;
}

/* Free AST */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

/* Parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char src_buf[128];
        char dst_buf[128];
        
        /* Initialize with pattern */
        for (int i = 0; i < sizeof(src_buf); i++) {
            src_buf[i] = (char)((i + thread_id * 17) & 0xFF);
        }
        
        /* Use all three builtins in parallel region */
        __builtin_memset(dst_buf, 0, sizeof(dst_buf));
        
        /* Conditional memcpy with volatile size */
        size_t copy_len = g_mem_size % sizeof(src_buf);
        if (copy_len > 0) {
            __builtin_memcpy(dst_buf, src_buf, copy_len);
        }
        
        /* Self-overlapping memmove */
        if (thread_id % 2 == 0) {
            __builtin_memmove(dst_buf + 32, dst_buf, 64);
        }
        
        /* Verify by hashing result */
        unsigned long local_hash = 0;
        for (size_t i = 0; i < sizeof(dst_buf); i++) {
            local_hash = ((local_hash << 3) + local_hash) + (unsigned long)dst_buf[i];
        }
        
        #pragma omp critical
        {
            printf("Thread %d: local hash = 0x%lx\n", thread_id, local_hash);
        }
    }
}

/* Multi-stage test with control flow */
static void memory_dispatch_test(void) {
    /* Stage 1: Direct builtin calls */
    char buffer1[512];
    char buffer2[512];
    
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    
    /* Stage 2: Goto-based control flow */
    int stage = 0;
    
    start_stage_2:
    if (stage == 0) {
        /* Overlap test with memmove */
        __builtin_memmove(buffer1 + 128, buffer1, 256);
        stage = 1;
        goto start_stage_2;
    }
    
    /* Stage 3: Volatile-controlled operations */
    volatile int do_copy = 1;
    volatile size_t len = g_mem_size % 128;
    
    if (do_copy) {
        goto perform_copy;
    }
    
    /* This should be skipped */
    __builtin_memset(buffer2, 0, sizeof(buffer2));
    
    perform_copy:
    if (len > 0) {
        __builtin_memcpy(buffer1, buffer2, len);
    }
    
    /* Stage 4: Nested calls */
    char temp[64];
    __builtin_memset(temp, 0xCC, sizeof(temp));
    __builtin_memcpy(buffer1 + 384, temp, sizeof(temp));
    __builtin_memmove(buffer2 + 384, buffer1 + 384, sizeof(temp));
}

int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Create recursive AST */
    int counter = 0;
    ASTNode* root = create_ast(4, &counter);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    printf("Created AST with %d nodes\n", counter);
    
    /* Calculate and print hash */
    unsigned long ast_hash = hash_ast(root);
    printf("AST hash: 0x%lx\n", ast_hash);
    
    /* Execute parallel memory operations */
    printf("\nParallel memory operations:\n");
    parallel_mem_ops();
    
    /* Multi-stage dispatch test */
    printf("\nMulti-stage dispatch test:\n");
    memory_dispatch_test();
    
    /* Additional stress: repeated builtin calls */
    printf("\nStress test with repeated builtins:\n");
    char stress_buf[1024];
    for (int i = 0; i < 100; i++) {
        size_t offset = (i * 7) % sizeof(stress_buf);
        size_t len = (g_mem_size + i) % (sizeof(stress_buf) - offset);
        
        if (len > 0) {
            if (i % 3 == 0) {
                __builtin_memset(stress_buf + offset, i, len);
            } else if (i % 3 == 1) {
                __builtin_memcpy(stress_buf + offset, g_token_array, len);
            } else {
                __builtin_memmove(stress_buf + offset, stress_buf, len);
            }
        }
    }
    
    /* Verify final state */
    unsigned long final_hash = 0;
    for (size_t i = 0; i < sizeof(stress_buf); i++) {
        final_hash = ((final_hash << 7) + final_hash) + (unsigned long)stress_buf[i];
    }
    printf("Final buffer hash: 0x%lx\n", final_hash);
    
    /* Cleanup */
    free_ast(root);
    
    printf("\nTest completed successfully\n");
    return 0;
}
