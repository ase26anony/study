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
static char g_token_pool[4096];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_constructor(void) {
    /* Force early initialization of memory functions */
    char buffer[32];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(&g_init_flag, &buffer[0], sizeof(g_init_flag));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan_destructor(void) {
    /* Final memory operation in destructor */
    char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_recursive(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile size */
    size_t copy_size = (g_mem_size % 64) + 1;
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, copy_size);
    
    node->size = copy_size;
    
    /* Create children with goto-controlled flow */
    int create_left = 1;
    
    /* Goto into memory operation block */
    if (depth > 2) {
        create_left = 0;
        goto skip_left;
    }
    
    node->left = create_ast_recursive(depth - 1, node->data);
    
skip_left:
    /* Goto out of block */
    if (create_left) {
        node->right = create_ast_recursive(depth - 1, node->data);
    } else {
        char temp[64];
        __builtin_memmove(temp, node->data, node->size);
        __builtin_memcpy(node->data, temp, node->size);
        node->right = NULL;
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void complex_mem_operations(void) {
    char src[512], dst[512];
    volatile size_t op_size = g_mem_size % 256;
    
    /* Initialize with builtin */
    __builtin_memset(src, 0xCC, sizeof(src));
    
    /* Goto into memmove block */
    int do_memmove = (op_size > 128);
    
    if (!do_memmove) goto skip_memmove;
    
    /* This block should trigger memmove redirection */
    __builtin_memmove(dst, src, op_size);
    
skip_memmove:
    /* Always do memcpy */
    __builtin_memcpy(dst + 128, src, op_size);
    
    /* Jump back for second memmove */
    if (do_memmove) {
        goto do_second_memmove;
    }
    
    __builtin_memset(dst, 0xDD, op_size);
    return;
    
do_second_memmove:
    __builtin_memmove(src, dst, op_size / 2);
}

/* OpenMP parallel memory dispatcher */
static void parallel_mem_dispatch(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buf[128];
        volatile size_t local_size = (g_mem_size + thread_id) % 64;
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Memcpy between thread-specific patterns */
        char shared_buf[256];
        __builtin_memcpy(shared_buf, local_buf, local_size);
        
        #pragma omp barrier
        
        /* Memmove within shared buffer */
        if (thread_id % 2 == 0) {
            __builtin_memmove(shared_buf + 64, shared_buf, local_size);
        }
    }
}

/* Token array initialization with builtins */
static void init_token_array(void) {
    char pattern[] = "ASAN_TEST_PATTERN_1234567890";
    
    for (size_t i = 0; i < sizeof(g_token_pool); i += 32) {
        size_t copy_len = 32;
        if (i + copy_len > sizeof(g_token_pool)) {
            copy_len = sizeof(g_token_pool) - i;
        }
        
        /* Alternate between memcpy and memmove */
        if (i % 64 == 0) {
            __builtin_memmove(&g_token_pool[i], pattern, copy_len);
        } else {
            __builtin_memcpy(&g_token_pool[i], pattern, copy_len);
        }
    }
    
    /* Final memset to mark completion */
    __builtin_memset(&g_token_idx, 0xFF, sizeof(g_token_idx));
}

/* Compute verification hash */
static unsigned long compute_hash(const char* data, size_t len) {
    unsigned long hash = 5381;
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + data[i];
    }
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Stage 1: Initialize token array */
    init_token_array();
    
    /* Stage 2: Create recursive AST */
    ASTNode* root = create_ast_recursive(4, "ROOT_NODE_DATA");
    
    /* Stage 3: Perform complex memory operations */
    complex_mem_operations();
    
    /* Stage 4: Execute parallel memory dispatch */
    #ifdef _OPENMP
    parallel_mem_dispatch();
    #endif
    
    /* Stage 5: Compute verification results */
    unsigned long token_hash = compute_hash(g_token_pool, sizeof(g_token_pool));
    unsigned long ast_hash = 0;
    
    if (root) {
        /* Copy AST data for verification */
        char ast_data[256];
        size_t total_copied = 0;
        
        ASTNode* current = root;
        while (current && total_copied < sizeof(ast_data) - 64) {
            size_t copy_size = current->size;
            if (copy_size > 64) copy_size = 64;
            
            __builtin_memcpy(ast_data + total_copied, current->data, copy_size);
            total_copied += copy_size;
            
            /* Move between nodes */
            __builtin_memmove(current->data, ast_data + total_copied - copy_size, copy_size);
            
            current = current->left;
        }
        
        ast_hash = compute_hash(ast_data, total_copied);
        
        /* Cleanup with final memory operation */
        __builtin_memset(root, 0, sizeof(ASTNode));
        free(root);
    }
    
    /* Final verification output */
    printf("Verification results:\n");
    printf("  Token array hash: 0x%08lx\n", token_hash);
    printf("  AST structure hash: 0x%08lx\n", ast_hash);
    printf("  Init flag: %d\n", (int)g_init_flag);
    
    /* Force one more builtin call before exit */
    char exit_buf[32];
    __builtin_memset(exit_buf, 0x55, sizeof(exit_buf));
    __builtin_memcpy(&g_init_flag, exit_buf, sizeof(g_init_flag));
    
    return 0;
}
