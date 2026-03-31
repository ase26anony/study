/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char global_tokens[1024];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize with memset builtin */
    __builtin_memset(global_tokens, 0, sizeof(global_tokens));
    
    /* Fill with pattern using memcpy builtin */
    char pattern[] = "ASAN_TEST_PATTERN_";
    for (int i = 0; i < sizeof(global_tokens) - sizeof(pattern); i += sizeof(pattern)) {
        __builtin_memcpy(&global_tokens[i], pattern, sizeof(pattern) - 1);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Clear sensitive data */
    __builtin_memset(global_tokens, 0xAA, sizeof(global_tokens));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Format data string */
    char buffer[128];
    __builtin_memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "Node_%d_Depth_%d", id, depth);
    
    /* Copy using memcpy with goto control flow */
    int copy_success = 0;
    
    copy_start:
    if (volatile_flag) {
        __builtin_memcpy(node->data, buffer, strlen(buffer) + 1);
        copy_success = 1;
    }
    
    if (!copy_success) {
        volatile_flag = 1;
        goto copy_start;  /* Jump back to force re-execution */
    }
    
    /* Create children recursively */
    node->left = create_ast_node(depth - 1, id * 2);
    node->right = create_ast_node(depth - 1, id * 2 + 1);
    
    /* Copy between nodes if both exist */
    if (node->left && node->right) {
        size_t copy_len = volatile_len % 128;
        
        copy_between_nodes:
        __builtin_memmove(node->right->data, node->left->data, copy_len);
        
        /* Verify copy with another memmove */
        if (volatile_flag) {
            __builtin_memmove(node->left->data + 64, node->right->data, copy_len / 2);
        }
    }
    
    return node;
}

/* Process AST with memory operations */
static int process_ast(ASTNode* node, int* sum) {
    if (!node) return 0;
    
    int local_sum = 0;
    
    /* Process data with memcpy operations */
    char temp[256];
    
    /* Use goto for complex control flow around memmove */
    int retry_count = 0;
    
    retry_memcpy:
    __builtin_memcpy(temp, node->data, sizeof(temp));
    
    /* Calculate hash/sum */
    for (size_t i = 0; i < sizeof(temp) && temp[i]; i++) {
        local_sum += temp[i];
    }
    
    /* Conditional memmove with goto */
    if (local_sum % 3 == 0 && retry_count < 2) {
        retry_count++;
        __builtin_memmove(node->data, temp, volatile_len % 128);
        goto retry_memcpy;
    }
    
    /* Recursive processing */
    local_sum += process_ast(node->left, sum);
    local_sum += process_ast(node->right, sum);
    
    *sum += local_sum;
    return local_sum;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf1[512];
        char local_buf2[512];
        
        /* Initialize with memset */
        __builtin_memset(local_buf1, thread_id, sizeof(local_buf1));
        __builtin_memset(local_buf2, 0xFF, sizeof(local_buf2));
        
        /* Copy between buffers */
        size_t copy_size = (volatile_len + thread_id) % 256 + 64;
        
        copy_parallel:
        __builtin_memcpy(local_buf2, local_buf1, copy_size);
        
        /* Move data around */
        if (thread_id % 2 == 0) {
            __builtin_memmove(local_buf1 + 128, local_buf2, copy_size / 2);
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* Global memory operation */
        #pragma omp critical
        {
            size_t offset = (thread_id * 64) % sizeof(global_tokens);
            size_t len = volatile_len % 128;
            
            if (offset + len < sizeof(global_tokens)) {
                __builtin_memcpy(&global_tokens[offset], local_buf1, len);
            }
        }
    }
}

/* Main test driver */
int main(void) {
    int total_sum = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Create and process AST */
    ASTNode* root = create_ast_node(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    process_ast(root, &total_sum);
    
    /* Phase 2: Parallel operations */
    parallel_memory_operations();
    
    /* Phase 3: Additional built-in tests */
    char test_buf1[1024];
    char test_buf2[1024];
    
    /* Test all three builtins in sequence */
    __builtin_memset(test_buf1, 0x42, sizeof(test_buf1));
    
    jump_memcpy:
    __builtin_memcpy(test_buf2, test_buf1, volatile_len % 512);
    
    /* Conditional jump around memmove */
    if (total_sum % 7 != 0) {
        goto skip_memmove;
    }
    
    __builtin_memmove(test_buf1 + 256, test_buf2, volatile_len % 256);
    
    skip_memmove:
    /* Verify with another memcpy */
    if (volatile_flag) {
        __builtin_memcpy(&global_tokens[512], test_buf1, 256);
    }
    
    /* Calculate final hash from global tokens */
    unsigned long final_hash = 0;
    for (size_t i = 0; i < sizeof(global_tokens); i++) {
        final_hash = final_hash * 31 + global_tokens[i];
    }
    
    printf("AST processing sum: %d\n", total_sum);
    printf("Global tokens hash: 0x%08lx\n", final_hash);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free_ast(root);
    
    return 0;
}
