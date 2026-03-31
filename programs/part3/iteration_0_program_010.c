/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
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
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)((i * 13) & 0xFF);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill data with pattern using volatile length */
    int len = volatile_len % 128;
    for (int i = 0; i < len; i++) {
        node->data[i] = (char)((id + i) & 0xFF);
    }
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = volatile_flag & 1;
        
        if (use_goto) {
            goto create_left;
        } else {
            node->left = create_ast_node(depth - 1, id * 2);
            node->right = create_ast_node(depth - 1, id * 2 + 1);
        }
        
        create_left:
        if (use_goto) {
            node->left = create_ast_node(depth - 1, id * 2);
            
            /* Jump back with __builtin_memmove in the path */
            char temp[128];
            __builtin_memmove(temp, node->data, len);
            __builtin_memmove(node->data + 64, temp, len > 64 ? 64 : len);
            
            node->right = create_ast_node(depth - 1, id * 2 + 1);
        }
    }
    
    return node;
}

/* Function with complex memory operations and goto */
static void process_ast_nodes(ASTNode* node1, ASTNode* node2) {
    if (!node1 || !node2) return;
    
    char buffer1[512], buffer2[512];
    int offset = 0;
    
    /* Label for goto into memory operation block */
    memory_operations:
    
    /* Copy data between nodes using __builtin_memcpy */
    __builtin_memcpy(buffer1, node1->data, sizeof(node1->data));
    __builtin_memcpy(buffer2, node2->data, sizeof(node2->data));
    
    /* Mix data with __builtin_memmove */
    if (volatile_flag & 2) {
        __builtin_memmove(buffer1 + 128, buffer2 + 64, 64);
        __builtin_memmove(buffer2 + 128, buffer1 + 64, 64);
    }
    
    /* Conditional goto out of block */
    if (offset < 256) {
        offset += 64;
        
        /* More memory operations before jumping back */
        __builtin_memset(buffer1 + offset, 0xAA, 32);
        __builtin_memcpy(node1->data + offset, buffer1 + offset, 32);
        
        goto memory_operations;
    }
    
    /* Process children recursively */
    process_ast_nodes(node1->left, node2->right);
    process_ast_nodes(node1->right, node2->left);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buf[1024];
        char shared_buf[1024];
        
        /* Initialize with __builtin_memset */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Copy between buffers using all three builtins */
        if (thread_id % 3 == 0) {
            __builtin_memcpy(shared_buf, local_buf, 256);
        } else if (thread_id % 3 == 1) {
            __builtin_memset(shared_buf + 256, 0xCC, 256);
        } else {
            __builtin_memmove(shared_buf + 512, local_buf + 256, 256);
        }
        
        #pragma omp barrier
        
        /* Verify with another memory operation */
        __builtin_memcpy(local_buf + 512, shared_buf + 256, 256);
    }
}

/* Multi-stage initialization function */
static void initialize_system(void) {
    /* Stage 1: Setup token array */
    char setup_buf[1024];
    __builtin_memset(setup_buf, 0, sizeof(setup_buf));
    
    /* Stage 2: Copy from token pool */
    int copy_len = volatile_len % 512;
    __builtin_memcpy(setup_buf, token_pool, copy_len);
    
    /* Stage 3: Move data around */
    __builtin_memmove(setup_buf + 512, setup_buf, copy_len > 512 ? 512 : copy_len);
    
    /* Stage 4: Clear part of buffer */
    __builtin_memset(setup_buf + 256, 0, 128);
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: System initialization */
    initialize_system();
    
    /* Phase 2: Create recursive AST structures */
    ASTNode* ast1 = create_ast_node(4, 1);
    ASTNode* ast2 = create_ast_node(3, 100);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Phase 3: Process AST nodes with goto flow */
    process_ast_nodes(ast1, ast2);
    
    /* Phase 4: Parallel memory operations */
    #ifdef _OPENMP
    printf("Running OpenMP parallel section\n");
    #endif
    parallel_memory_operations();
    
    /* Phase 5: Final verification */
    unsigned long hash = 0;
    for (int i = 0; i < sizeof(token_pool); i++) {
        hash = (hash * 31) + (unsigned long)token_pool[i];
    }
    
    /* Use builtins in verification */
    char verify_buf[256];
    __builtin_memset(verify_buf, 0, sizeof(verify_buf));
    __builtin_memcpy(verify_buf, &hash, sizeof(hash));
    
    printf("Test completed. Final hash: %lu\n", hash);
    
    /* Cleanup */
    /* Note: In real code, you'd need proper AST cleanup */
    
    return 0;
}
