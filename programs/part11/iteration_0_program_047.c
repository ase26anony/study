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
        token_pool[i] = (i % 26) + 'a';
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    printf("Destructor: Cleanup completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
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
    for (int i = 0; i < len && i < sizeof(node->data) - 1; i++) {
        node->data[i] = 'A' + (id + i) % 26;
    }
    
    if (depth > 1) {
        /* Create children with goto-based control flow */
        int use_goto = volatile_flag;
        
        if (use_goto) {
            goto create_left;
        }
        
        /* Normal path */
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        return node;
        
    create_left:
        /* Goto target - test flow sensitivity */
        node->left = create_ast(depth - 1, id * 2);
        
        /* Jump back */
        if (volatile_flag) {
            goto create_right;
        }
        
        node->right = NULL;
        return node;
        
    create_right:
        node->right = create_ast(depth - 1, id * 2 + 1);
    }
    
    return node;
}

/* Function with complex memory operations and goto */
static void process_ast_with_goto(ASTNode* root) {
    if (!root) return;
    
    char buffer[512];
    int jump_target = volatile_flag % 3;
    
    /* Goto-based dispatch */
    switch (jump_target) {
        case 0:
            goto memcpy_block;
        case 1:
            goto memset_block;
        case 2:
            goto memmove_block;
        default:
            return;
    }
    
memcpy_block:
    {
        /* Force __builtin_memcpy redirection */
        __builtin_memcpy(buffer, root->data, 
                        volatile_len % sizeof(buffer));
        
        /* Jump to next operation */
        if (volatile_flag) goto process_children;
    }
    
memset_block:
    {
        /* Force __builtin_memset redirection */
        __builtin_memset(buffer, 0xCC, 
                        volatile_len % sizeof(buffer));
        
        /* Copy back to node */
        __builtin_memcpy(root->data, buffer, 
                        volatile_len % sizeof(root->data));
        
        if (volatile_flag) goto memmove_block;
    }
    
memmove_block:
    {
        /* Force __builtin_memmove redirection with overlap */
        char temp[256];
        __builtin_memcpy(temp, root->data, 128);
        __builtin_memmove(root->data, root->data + 64, 128);
        __builtin_memmove(root->data + 128, temp, 128);
    }
    
process_children:
    /* Recursive processing */
    process_ast_with_goto(root->left);
    process_ast_with_goto(root->right);
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
        char* shared_ptr = token_pool + (thread_id * 512);
        
        /* Each thread performs different built-in operations */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(local_buf, thread_id, 
                                volatile_len % sizeof(local_buf));
                __builtin_memcpy(shared_ptr, local_buf, 256);
                break;
            case 1:
                __builtin_memcpy(local_buf, shared_ptr, 256);
                __builtin_memset(shared_ptr, 0, 256);
                break;
            case 2:
                __builtin_memmove(shared_ptr, shared_ptr + 128, 256);
                __builtin_memcpy(local_buf, shared_ptr, 256);
                break;
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* Collective operation */
        #pragma omp for
        for (int i = 0; i < 8; i++) {
            char temp[64];
            __builtin_memset(temp, i, sizeof(temp));
            __builtin_memcpy(token_pool + (i * 64), temp, sizeof(temp));
        }
    }
}

/* Compute hash of AST for verification */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* p = node->data;
    
    /* Simple hash computation */
    while (*p) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

/* Free AST memory */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Create and process AST with goto */
    ASTNode* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    printf("AST created, processing with goto jumps...\n");
    process_ast_with_goto(root);
    
    /* Phase 2: OpenMP parallel operations */
    printf("Starting OpenMP parallel memory operations...\n");
    parallel_memory_operations();
    
    /* Phase 3: Additional built-in stress tests */
    printf("Performing built-in function stress tests...\n");
    
    /* Test all three built-ins in sequence */
    char test_buf1[1024];
    char test_buf2[1024];
    
    __builtin_memset(test_buf1, 0xAA, sizeof(test_buf1));
    __builtin_memcpy(test_buf2, test_buf1, sizeof(test_buf1));
    __builtin_memmove(test_buf1, test_buf1 + 256, 512);
    
    /* Use volatile to control which built-in is called */
    for (int i = 0; i < 3; i++) {
        volatile int op_type = i;
        char temp[128];
        
        if (op_type == 0) {
            __builtin_memset(temp, i, sizeof(temp));
        } else if (op_type == 1) {
            __builtin_memcpy(temp, test_buf1, 64);
        } else {
            __builtin_memmove(temp, temp + 32, 64);
        }
        
        /* Copy to token pool */
        __builtin_memcpy(token_pool + (i * 128), temp, 64);
    }
    
    /* Phase 4: Verification */
    unsigned long hash = compute_ast_hash(root);
    unsigned long token_hash = 0;
    
    for (size_t i = 0; i < sizeof(token_pool); i++) {
        token_hash = token_hash * 31 + token_pool[i];
    }
    
    printf("Results:\n");
    printf("  AST hash: %lu\n", hash);
    printf("  Token pool hash: %lu\n", token_hash);
    printf("  Final sum: %lu\n", hash + token_hash);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
