/* ISO C99-compliant test program for ASAN/HWASAN built-in redirection */
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
        token_pool[i] = (char)(i % 256);
    }
    
    /* Force early built-in usage in constructor */
    char local_buf[128];
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    __builtin_memcpy(&token_pool[0], local_buf, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Final memory operation in destructor */
    char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = NULL;
    node->right = NULL;
    
    /* Use volatile length for memory operations */
    int len = volatile_len % 128;
    
    /* Pattern 1: Direct built-in calls */
    __builtin_memset(node->data, depth, len);
    
    /* Pattern 2: Built-in with computed source */
    char src_buf[256];
    for (int i = 0; i < sizeof(src_buf); i++) {
        src_buf[i] = (char)((i + id) % 256);
    }
    __builtin_memcpy(&node->data[64], src_buf, len);
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = volatile_flag;
        
        if (use_goto) {
            /* Jump into block with memmove */
            goto create_left;
        } else {
            node->left = create_ast(depth - 1, id * 2);
            node->right = create_ast(depth - 1, id * 2 + 1);
        }
        
        create_left:
        {
            /* Block with memmove accessed via goto */
            char temp[128];
            __builtin_memcpy(temp, node->data, 64);
            __builtin_memmove(node->data, &node->data[64], 32);
            __builtin_memmove(&node->data[32], temp, 32);
            
            if (use_goto) {
                node->left = create_ast(depth - 1, id * 2);
                goto create_right;
            }
        }
        
        create_right:
        node->right = create_ast(depth - 1, id * 2 + 1);
    }
    
    return node;
}

/* Function with complex memory operations and OpenMP */
static void parallel_memory_operations(ASTNode* root) {
    if (!root) return;
    
    /* OpenMP parallel region */
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread operates on different memory regions */
        char thread_buf[512];
        volatile int local_len = volatile_len;
        
        /* Pattern 3: Built-in with volatile length */
        __builtin_memset(thread_buf, thread_id, local_len % 256);
        
        /* Copy between AST nodes with built-ins */
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            if (root->left && root->right) {
                /* Use memcpy for left-to-right copy */
                __builtin_memcpy(root->right->data, 
                               root->left->data, 
                               128);
                
                /* Use memmove for overlapping regions */
                __builtin_memmove(&root->right->data[64],
                                root->right->data,
                                64);
            }
        }
        
        /* Additional built-in usage in parallel region */
        char temp[256];
        __builtin_memset(temp, 0xCC, sizeof(temp));
        __builtin_memcpy(&thread_buf[128], temp, 128);
    }
}

/* Function with goto jumping around memory operations */
static void goto_memory_patterns(void) {
    char buffer_a[256];
    char buffer_b[256];
    int pattern = volatile_flag;
    
    /* Initialize buffers */
    __builtin_memset(buffer_a, 'A', sizeof(buffer_a));
    __builtin_memset(buffer_b, 'B', sizeof(buffer_b));
    
    if (pattern == 0) {
        goto pattern1;
    } else if (pattern == 1) {
        goto pattern2;
    } else {
        goto pattern3;
    }
    
pattern1:
    /* Jump target with memcpy */
    __builtin_memcpy(buffer_a, buffer_b, 128);
    goto cleanup;
    
pattern2:
    /* Jump target with memmove */
    __builtin_memmove(buffer_a, buffer_b, 128);
    goto cleanup;
    
pattern3:
    /* Jump target with memset */
    __builtin_memset(buffer_a, 'C', 128);
    /* Fall through */
    
cleanup:
    /* Final memory operation after goto */
    __builtin_memset(buffer_b, 0, sizeof(buffer_b));
}

/* Compute hash of AST for verification */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* data = node->data;
    
    /* Simple hash computation */
    for (int i = 0; i < 256; i++) {
        hash = ((hash << 5) + hash) + data[i];
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Create recursive AST */
    ASTNode* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Perform parallel memory operations */
    parallel_memory_operations(root);
    
    /* Phase 3: Execute goto patterns */
    goto_memory_patterns();
    
    /* Phase 4: Additional built-in usage in main */
    char main_buffer[1024];
    __builtin_memset(main_buffer, 0, sizeof(main_buffer));
    
    /* Copy from token pool using built-ins */
    __builtin_memcpy(main_buffer, token_pool, 512);
    __builtin_memmove(&main_buffer[256], main_buffer, 256);
    
    /* Phase 5: Compute and print verification result */
    unsigned long hash = compute_ast_hash(root);
    printf("AST Hash: %lu\n", hash);
    
    /* Phase 6: Cleanup with final memory operations */
    __builtin_memset(root->data, 0, sizeof(root->data));
    
    /* Note: In real code, you would free the AST here */
    /* free_ast(root); */
    
    printf("Test completed successfully\n");
    return 0;
}
