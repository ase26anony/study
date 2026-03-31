/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* AST-like recursive structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    size_t data_len;
} ASTNode;

/* Global token array */
static char g_token_array[4096];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token array with pattern */
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 31) & 0xFF);
    }
    printf("Constructor: Token array initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast(int depth, volatile int* counter) {
    if (depth <= 0 || *counter >= 100) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with builtin memcpy using volatile size */
    size_t copy_size = (g_mem_size % 64) + 1;
    __builtin_memcpy(node->data, &g_token_array[g_token_idx], copy_size);
    node->data_len = copy_size;
    
    /* Update volatile index with goto for flow control */
    update_index:
    g_token_idx = (g_token_idx + copy_size) % sizeof(g_token_array);
    
    (*counter)++;
    
    /* Recursive creation with conditional goto */
    if (depth > 1) {
        node->left = create_ast(depth - 1, counter);
        if (depth % 3 == 0) {
            goto skip_right;  /* Test goto into block */
        }
        node->right = create_ast(depth - 1, counter);
        skip_right:;
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* AST copy function with builtin memmove */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use goto to jump into memmove block */
    if (src->data_len > 32) {
        goto do_copy;
    }
    
    /* Small copy path */
    __builtin_memcpy(dest->data, src->data, src->data_len);
    dest->data_len = src->data_len;
    return;
    
do_copy:
    /* Large copy with memmove (handles overlap) */
    volatile size_t move_size = src->data_len;
    __builtin_memmove(dest->data, src->data, move_size);
    dest->data_len = move_size;
    
    /* Jump out of block */
    goto copy_done;
    
copy_done:
    return;
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
        char local_buf1[128];
        char local_buf2[128];
        
        /* Initialize with builtin memset */
        __builtin_memset(local_buf1, thread_id, sizeof(local_buf1));
        
        /* Copy with builtin memcpy */
        volatile size_t copy_len = (g_mem_size % 64) + 64;
        __builtin_memcpy(local_buf2, local_buf1, copy_len);
        
        /* Move with builtin memmove (potential overlap) */
        if (thread_id % 2 == 0) {
            __builtin_memmove(local_buf1 + 32, local_buf1, 64);
        }
        
        #pragma omp barrier
        
        /* Verify copy */
        int errors = 0;
        for (size_t i = 0; i < copy_len; i++) {
            if (local_buf2[i] != (char)thread_id) errors++;
        }
        
        #pragma omp critical
        {
            printf("Thread %d: %d errors in memcpy verification\n", 
                   thread_id, errors);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    volatile int node_counter = 0;
    
    /* Phase 1: Create AST with recursive memory ops */
    ASTNode* root = create_ast(5, &node_counter);
    printf("Created AST with %d nodes\n", node_counter);
    
    /* Phase 2: Copy AST data with goto flow control */
    if (root) {
        ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
        if (copy) {
            /* Test goto jumping into memmove block */
            if (root->data_len > 40) {
                goto perform_copy;
            }
            
            __builtin_memset(copy, 0, sizeof(ASTNode));
            
        perform_copy:
            copy_ast_data(copy, root);
            
            /* Verify copy */
            int match = __builtin_memcmp(root->data, copy->data, 
                                        root->data_len) == 0;
            printf("AST copy verification: %s\n", 
                   match ? "PASS" : "FAIL");
            
            free(copy);
        }
    }
    
    /* Phase 3: Parallel memory operations */
    printf("\n=== Parallel Memory Operations ===\n");
    parallel_mem_ops();
    
    /* Phase 4: Direct builtin calls with volatile control */
    printf("\n=== Direct Built-in Calls ===\n");
    {
        char buffer1[256];
        char buffer2[256];
        
        /* Force all three builtins with volatile sizes */
        volatile size_t op_size = g_mem_size % 128;
        
        __builtin_memset(buffer1, 0xAA, op_size);
        __builtin_memcpy(buffer2, buffer1, op_size);
        
        /* Create overlap for memmove */
        __builtin_memmove(buffer1 + 64, buffer1, 128);
        
        /* Compute hash to prevent optimization */
        unsigned long hash = 0;
        for (size_t i = 0; i < sizeof(buffer1); i++) {
            hash = hash * 31 + (unsigned char)buffer1[i];
        }
        printf("Final buffer hash: 0x%08lx\n", hash);
    }
    
    /* Cleanup */
    /* Note: In real code, you'd need to free the AST recursively */
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
