/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int value;
    uint8_t padding[32];  /* Redzone testing */
} ASTNode;

/* Global token array for initialization */
static char g_token_array[4096];
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
    /* Verify memory wasn't corrupted */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += g_token_array[i];
    }
    printf("Destructor: Token array checksum = %d\n", sum);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with volatile size */
    size_t copy_len = g_mem_size % 64;
    if (copy_len > 63) copy_len = 63;
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data[copy_len] = '\0';
    
    node->value = depth;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, node->data);
        node->right = NULL;
        
        create_children:
        /* Jump target with __builtin_memmove */
        if (node->left) {
            ASTNode temp;
            __builtin_memcpy(&temp, node->left, sizeof(ASTNode));
            __builtin_memmove(node->right, &temp, sizeof(ASTNode));
        } else {
            node->right = create_ast(depth - 2, "right_branch");
        }
    }
    
    return node;
}

/* Function with complex memory operations */
static void process_ast(ASTNode* root, int* result) {
    if (!root) return;
    
    ASTNode local_copy;
    volatile size_t copy_size = sizeof(ASTNode);
    
    /* Use all three builtins in sequence */
    __builtin_memset(&local_copy, 0xAA, sizeof(ASTNode));
    __builtin_memcpy(&local_copy, root, copy_size);
    
    /* Conditional memmove with goto */
    if (root->left && root->right) {
        int do_move = (root->value % 2 == 0);
        
        if (do_move) {
            goto perform_memmove;
        }
        
        /* Normal path */
        __builtin_memcpy(root->left, root->right, sizeof(ASTNode));
        
        perform_memmove:
        /* Jump target */
        __builtin_memmove(root->right->data, root->left->data, 32);
    }
    
    *result += root->value;
    process_ast(root->left, result);
    process_ast(root->right, result);
}

/* OpenMP parallel section */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char buffer1[256];
        char buffer2[256];
        volatile int buf_size = 128 + (thread_id * 16);
        
        /* Force builtin calls in parallel region */
        __builtin_memset(buffer1, thread_id, buf_size);
        __builtin_memcpy(buffer2, buffer1, buf_size);
        
        /* Conditional memmove with volatile control */
        volatile int should_move = (thread_id % 3 == 0);
        if (should_move) {
            __builtin_memmove(buffer1 + 64, buffer2, 64);
        }
        
        #pragma omp critical
        {
            /* Verify the operations */
            int sum = 0;
            for (int i = 0; i < 64; i++) {
                sum += buffer1[i];
            }
            printf("Thread %d: buffer checksum = %d\n", thread_id, sum);
        }
    }
}

/* Multi-stage initialization */
static void initialize_system(void) {
    /* Stage 1: Global array operations */
    char temp[512];
    volatile size_t stage1_size = 256;
    
    __builtin_memcpy(temp, g_token_array, stage1_size);
    __builtin_memset(g_token_array + 512, 0xCC, 256);
    __builtin_memmove(g_token_array + 256, temp, stage1_size);
    
    /* Stage 2: Recursive structure */
    ASTNode* ast = create_ast(4, "initial_data");
    if (ast) {
        int ast_sum = 0;
        process_ast(ast, &ast_sum);
        printf("AST traversal sum: %d\n", ast_sum);
        
        /* Cleanup */
        free(ast);
    }
    
    /* Stage 3: Parallel operations */
    parallel_memory_ops();
}

int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Force initialization of asan_memfn_rtls cache */
    char test_buf[128];
    char src_buf[128];
    
    /* Initial builtin calls to trigger cache initialization */
    __builtin_memset(test_buf, 0x42, sizeof(test_buf));
    __builtin_memcpy(src_buf, test_buf, sizeof(test_buf));
    __builtin_memmove(test_buf + 32, src_buf, 64);
    
    /* Multi-stage execution */
    initialize_system();
    
    /* Final verification */
    uint64_t final_hash = 0;
    for (int i = 0; i < sizeof(g_token_array); i += 64) {
        volatile size_t chunk_size = 64;
        char chunk[64];
        __builtin_memcpy(chunk, g_token_array + i, chunk_size);
        
        for (int j = 0; j < 64; j++) {
            final_hash = (final_hash * 31) + chunk[j];
        }
    }
    
    printf("Final hash: 0x%016llx\n", (unsigned long long)final_hash);
    printf("Test completed successfully\n");
    
    return 0;
}
