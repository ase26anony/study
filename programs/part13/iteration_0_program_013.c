/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_tokens[8][128] = {
    "memcpy_test_token_1",
    "memset_test_token_2",
    "memmove_test_token_3",
    "recursive_ast_node_4",
    "openmp_parallel_5",
    "volatile_control_6",
    "goto_flow_test_7",
    "final_verification_8"
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Force early initialization of ASAN runtime */
    volatile char init_buf[16];
    __builtin_memset(init_buf, 0, sizeof(init_buf));
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    printf("Destructor: ASAN environment cleaned up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast_recursive(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Use __builtin_memcpy to copy data with volatile size */
    volatile size_t copy_size = (g_mem_size < 256) ? g_mem_size : 256;
    __builtin_memcpy(node->data, base_data, copy_size);
    node->size = copy_size;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        char child_data[256];
        __builtin_snprintf(child_data, sizeof(child_data), 
                          "%s_child_%d", base_data, depth);
        
        /* Goto label for flow control */
        create_left:
        node->left = create_ast_recursive(depth - 1, child_data);
        
        /* Jump around right creation in some cases */
        if (depth % 2 == 0) {
            goto skip_right;
        }
        
        node->right = create_ast_recursive(depth - 1, child_data);
        goto after_children;
        
        skip_right:
        node->right = NULL;
        
        after_children:;
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with complex memory operations and goto */
static void process_ast_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int use_memmove = 1;
    
    /* Goto-based flow control around memory operations */
    if (src->size > 128) {
        goto use_memcpy_path;
    }
    
    /* Path with __builtin_memmove */
    if (use_memmove) {
        __builtin_memmove(dst->data, src->data, src->size);
        goto after_memory_op;
    }
    
    use_memcpy_path:
    /* Path with __builtin_memcpy */
    __builtin_memcpy(dst->data, src->data, src->size);
    
    after_memory_op:
    /* Additional memset on processed data */
    __builtin_memset(dst->data + src->size, 0xAA, 
                    sizeof(dst->data) - src->size);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        char local_buf[256];
        char result_buf[256];
        
        /* Each thread uses builtins */
        #pragma omp for
        for (int i = 0; i < 8; i++) {
            /* Initialize with memset */
            __builtin_memset(local_buf, i, sizeof(local_buf));
            
            /* Copy token data */
            __builtin_memcpy(local_buf, g_tokens[i], 
                           strlen(g_tokens[i]) + 1);
            
            /* Move data around */
            __builtin_memmove(result_buf, local_buf, 
                            sizeof(local_buf));
            
            /* Verify with another memset */
            __builtin_memset(local_buf, 0xFF, 32);
        }
        
        #pragma omp barrier
        
        /* Additional memory operation after barrier */
        #pragma omp single
        {
            char sync_buf[128];
            __builtin_memset(sync_buf, 0xCC, sizeof(sync_buf));
            __builtin_memcpy(sync_buf + 64, sync_buf, 64);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Basic built-in usage */
    char buffer1[512];
    char buffer2[512];
    
    __builtin_memset(buffer1, 0x11, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 256, buffer1, 256);
    
    /* Phase 2: Recursive AST operations */
    ASTNode* ast1 = create_ast_recursive(4, g_tokens[0]);
    ASTNode* ast2 = create_ast_recursive(3, g_tokens[1]);
    
    if (ast1 && ast2) {
        process_ast_with_goto(ast1, ast2);
        
        /* Cross-copy between AST nodes */
        __builtin_memcpy(ast1->data + 128, ast2->data, 128);
        __builtin_memmove(ast2->data, ast1->data, 128);
    }
    
    /* Phase 3: OpenMP parallel section */
    parallel_memory_operations();
    
    /* Phase 4: Volatile-controlled operations */
    volatile size_t dynamic_size = g_mem_size;
    for (int i = 0; i < 4; i++) {
        char temp[512];
        __builtin_memset(temp, i * 0x33, dynamic_size);
        __builtin_memcpy(buffer1 + i * 64, temp, dynamic_size / 2);
        __builtin_memmove(temp + 32, temp, 32);
        dynamic_size += 16;
    }
    
    /* Verification: Compute hash/sum */
    unsigned long long hash = 0;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        hash += (unsigned long long)buffer1[i] * (i + 1);
    }
    
    if (ast1) {
        for (size_t i = 0; i < ast1->size; i++) {
            hash += (unsigned long long)ast1->data[i] * (i + 257);
        }
        free(ast1);
    }
    
    if (ast2) {
        for (size_t i = 0; i < ast2->size; i++) {
            hash += (unsigned long long)ast2->data[i] * (i + 513);
        }
        free(ast2);
    }
    
    printf("Test completed. Verification hash: %llu\n", hash);
    printf("Expected ASAN coverage:\n");
    printf("  - BUILT_IN_MEMCPY/MEMSET/MEMMOVE redirection\n");
    printf("  - asan_memfn_rtls[] cache initialization\n");
    printf("  - __asan_ vs __hwasan_ prefix selection\n");
    printf("  - Flow-sensitive RTL modification logic\n");
    
    return (hash != 0) ? 0 : 1;
}
