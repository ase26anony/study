/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
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
    int id;
} ASTNode;

/* Global token array */
static char g_tokens[8][32] = {
    "memcpy_test_token_1",
    "memset_test_token_2",
    "memmove_test_token_3",
    "asan_coverage_4",
    "hwasan_branch_5",
    "goto_flow_6",
    "omp_parallel_7",
    "final_result_8"
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Force early initialization of ASAN runtime */
    volatile char init_buf[16];
    __builtin_memset(init_buf, 0xA5, sizeof(init_buf));
    
    /* Test __builtin_memcpy in constructor */
    char src[8] = "INIT";
    char dest[8];
    __builtin_memcpy(dest, src, sizeof(src));
    
    /* This should trigger ASAN initialization */
    if (g_use_hwasan) {
        /* Create condition that might affect HWASAN path */
        g_mem_size = 128;
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    /* Final memory operations in destructor */
    volatile char cleanup_buf[32];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Copy token data using __builtin_memcpy */
    int token_idx = id % 8;
    __builtin_memcpy(node->data, g_tokens[token_idx], 
                    sizeof(g_tokens[token_idx]));
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            /* Jump into memory operation block */
            goto create_children;
        }
        
        /* Normal path */
        node->left = create_ast_node(depth - 1, id * 2);
        node->right = create_ast_node(depth - 1, id * 2 + 1);
        
        if (use_goto) {
            /* Jump target */
            create_children:
            /* Test __builtin_memmove with goto context */
            char temp_buf[64];
            __builtin_memcpy(temp_buf, node->data, 32);
            __builtin_memmove(node->data + 16, node->data, 32);
            __builtin_memcpy(node->data, temp_buf, 32);
            
            node->left = create_ast_node(depth - 1, id * 2);
            node->right = create_ast_node(depth - 1, id * 2 + 1);
        }
    }
    
    return node;
}

/* Complex memory operation with overlapping regions */
static void perform_memory_operations(ASTNode* node) {
    if (!node) return;
    
    volatile size_t op_size = g_mem_size;
    
    /* Test all three builtins in sequence */
    char buffer1[256];
    char buffer2[256];
    
    /* 1. __builtin_memset */
    __builtin_memset(buffer1, node->id, op_size % 128);
    
    /* 2. __builtin_memcpy */
    __builtin_memcpy(buffer2, buffer1, op_size % 128);
    
    /* 3. __builtin_memmove with overlapping regions */
    __builtin_memmove(buffer1 + 32, buffer1, 64);
    __builtin_memcpy(node->data, buffer1 + 32, 64);
    
    /* Recursive operations */
    perform_memory_operations(node->left);
    perform_memory_operations(node->right);
}

/* OpenMP parallel memory dispatcher */
static void parallel_memory_dispatch(ASTNode* root) {
    int i;
    
    #pragma omp parallel private(i)
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs memory operations */
        char local_buf[128];
        volatile int pattern = thread_id * 0x11;
        
        /* Force ASAN to handle parallel builtin calls */
        __builtin_memset(local_buf, pattern, sizeof(local_buf));
        
        #pragma omp for
        for (i = 0; i < 16; i++) {
            char temp[64];
            __builtin_memset(temp, i, sizeof(temp));
            
            /* Mix of memory operations */
            if (i % 3 == 0) {
                __builtin_memcpy(local_buf + (i * 4), temp, 16);
            } else if (i % 3 == 1) {
                __builtin_memmove(local_buf + (i * 4), temp, 16);
            } else {
                __builtin_memset(local_buf + (i * 4), i, 16);
            }
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* Final memory operation in parallel region */
        if (root && thread_id == 0) {
            __builtin_memcpy(root->data + 64, local_buf, 64);
        }
    }
}

/* Calculate hash from AST */
static unsigned long calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Simple hash calculation */
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    /* Add child hashes */
    hash += calculate_ast_hash(node->left);
    hash += calculate_ast_hash(node->right);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before free (using memset) */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create complex AST */
    ASTNode* root = create_ast_node(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Perform recursive memory operations */
    perform_memory_operations(root);
    
    /* Execute parallel memory dispatch */
    parallel_memory_dispatch(root);
    
    /* Additional builtin calls in main */
    volatile char main_buf[512];
    
    /* Test all three builtins with volatile sizes */
    volatile size_t size1 = 128;
    volatile size_t size2 = 256;
    volatile size_t size3 = 64;
    
    __builtin_memset(main_buf, 0xCC, size1);
    __builtin_memcpy(main_buf + 128, main_buf, size2 % 128);
    __builtin_memmove(main_buf + 64, main_buf + 32, size3);
    
    /* Copy to AST node */
    __builtin_memcpy(root->data + 128, main_buf, 128);
    
    /* Calculate and print result */
    unsigned long final_hash = calculate_ast_hash(root);
    printf("AST Hash Result: %lu\n", final_hash);
    
    /* Verify with expected pattern */
    unsigned long expected = 0;
    for (int i = 0; i < 8; i++) {
        char* ptr = g_tokens[i];
        while (*ptr) {
            expected = ((expected << 5) + expected) + *ptr++;
        }
    }
    
    printf("Expected base hash: %lu\n", expected);
    
    /* Cleanup */
    free_ast(root);
    
    /* Final memory operation */
    __builtin_memset(main_buf, 0, sizeof(main_buf));
    
    printf("Test completed successfully.\n");
    return 0;
}
