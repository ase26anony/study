/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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
static char g_tokens[][32] = {
    "memcpy_test_token_1",
    "memset_test_token_2", 
    "memmove_test_token_3",
    "asan_coverage_token_4"
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char init_buf[128];
    /* Force __builtin_memset in constructor */
    __builtin_memset(init_buf, 0xAA, sizeof(init_buf));
    
    /* Copy initialization pattern */
    const char* pattern = "ASAN_INIT_PATTERN";
    __builtin_memcpy(init_buf + 16, pattern, strlen(pattern) + 1);
    
    /* Move data around */
    __builtin_memmove(init_buf + 32, init_buf + 16, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ASTNode* parse_tokens_recursive(int depth, int token_idx) {
    if (depth <= 0 || token_idx >= 4) {
        return NULL;
    }
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = depth * 1000 + token_idx;
    
    /* Copy token data with goto-controlled flow */
    int copy_complete = 0;
    
copy_start:
    if (!copy_complete) {
        /* Use __builtin_memcpy with volatile size */
        size_t copy_len = g_mem_size % 32;
        if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
        
        __builtin_memcpy(node->data, g_tokens[token_idx], copy_len);
        copy_complete = 1;
        
        /* Jump back to test flow sensitivity */
        goto copy_start;
    }
    
    /* Recursive parsing with different memory operations */
    node->left = parse_tokens_recursive(depth - 1, (token_idx + 1) % 4);
    node->right = parse_tokens_recursive(depth - 1, (token_idx + 2) % 4);
    
    /* Move data between nodes if both children exist */
    if (node->left && node->right) {
        volatile size_t move_size = g_mem_size % 16;
        
        /* Complex goto pattern around memmove */
        int move_phase = 0;
        
    move_phase_1:
        if (move_phase == 0) {
            __builtin_memmove(node->left->data + 8, node->right->data, move_size);
            move_phase = 1;
            goto move_phase_2;
        }
        
    move_phase_2:
        if (move_phase == 1) {
            /* Move in opposite direction */
            __builtin_memmove(node->right->data, node->left->data + 8, move_size);
            move_phase = 2;
        }
    }
    
    return node;
}

/* Calculate hash of AST */
static unsigned long hash_ast(const ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    const char* p = node->data;
    
    /* Simple djb2 hash */
    while (*p) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    hash ^= hash_ast(node->left);
    hash ^= hash_ast(node->right);
    hash ^= node->id;
    
    return hash;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear sensitive data before free */
    volatile char* data_ptr = node->data;
    __builtin_memset(data_ptr, 0, sizeof(node->data));
    
    free(node);
}

/* Parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf1[256];
        char local_buf2[256];
        
        /* Initialize with builtins */
        __builtin_memset(local_buf1, thread_id, sizeof(local_buf1));
        
        /* Copy between buffers */
        volatile size_t op_size = (g_mem_size + thread_id) % 128;
        __builtin_memcpy(local_buf2, local_buf1, op_size);
        
        /* Move data around */
        __builtin_memmove(local_buf1 + 64, local_buf2, op_size / 2);
        
        /* Cross-thread pattern (simulated) */
        if (thread_id % 2 == 0) {
            __builtin_memset(local_buf1 + 128, 0xCC, 64);
        } else {
            __builtin_memcpy(local_buf2 + 128, local_buf1, 32);
        }
        
        /* Force memory function calls in critical section */
        #pragma omp critical
        {
            static char shared_buf[512];
            __builtin_memcpy(shared_buf + thread_id * 64, local_buf1, 32);
            __builtin_memset(shared_buf + 256 + thread_id * 32, thread_id, 16);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive AST parsing */
    printf("Phase 1: Building recursive AST...\n");
    ASTNode* root = parse_tokens_recursive(3, 0);
    
    if (!root) {
        fprintf(stderr, "Failed to build AST\n");
        return 1;
    }
    
    /* Phase 2: Parallel memory operations */
    printf("Phase 2: Executing parallel memory operations...\n");
    parallel_memory_operations();
    
    /* Phase 3: Complex memory pattern with gotos */
    printf("Phase 3: Complex goto-based memory operations...\n");
    {
        char pattern_buf[1024];
        char temp_buf[512];
        
        /* Initialize pattern */
        for (int i = 0; i < sizeof(pattern_buf); i++) {
            pattern_buf[i] = i % 256;
        }
        
        int stage = 0;
        
    stage_0:
        if (stage == 0) {
            /* Large memcpy */
            __builtin_memcpy(temp_buf, pattern_buf, 256);
            stage = 1;
            goto stage_1;
        }
        
    stage_1:
        if (stage == 1) {
            /* Overlapping memmove */
            __builtin_memmove(pattern_buf + 128, pattern_buf, 384);
            stage = 2;
            goto stage_2;
        }
        
    stage_2:
        if (stage == 2) {
            /* memset with volatile size */
            volatile size_t set_size = g_mem_size % 256;
            __builtin_memset(pattern_buf + 512, 0xAB, set_size);
            stage = 3;
        }
        
        /* Copy back with different size */
        __builtin_memcpy(temp_buf + 128, pattern_buf + 512, 128);
    }
    
    /* Phase 4: Calculate and verify result */
    printf("Phase 4: Verifying results...\n");
    unsigned long ast_hash = hash_ast(root);
    printf("AST hash: 0x%08lx\n", ast_hash);
    
    /* Additional builtin calls in cleanup path */
    char verify_buf[64];
    __builtin_memset(verify_buf, 0, sizeof(verify_buf));
    __builtin_memcpy(verify_buf, &ast_hash, sizeof(ast_hash));
    __builtin_memmove(verify_buf + 32, verify_buf, 8);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully.\n");
    return 0;
}
