/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "asan", "hwasan"};
static const int token_count = sizeof(tokens) / sizeof(tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_environment(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < 256; i++) {
        volatile_src[i] = (char)(i % 256);
    }
    
    /* Force early built-in usage in constructor */
    __builtin_memset(volatile_dest, 0xAA, sizeof(volatile_dest));
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan_environment(void) {
    /* Final built-in usage in destructor */
    __builtin_memset(volatile_dest, 0xFF, 16);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with built-ins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, tokens[id % token_count], 
                    strlen(tokens[id % token_count]));
    
    node->id = id;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
done:
    return node;
}

/* Function with goto jumping into memmove block */
static void process_with_goto(ASTNode* dest, ASTNode* src) {
    int use_memmove = 1;
    
    if (dest == src) {
        goto skip_copy;
    }
    
    /* Jump into memmove block */
    if (use_memmove) {
        goto do_memmove;
    }
    
skip_copy:
    return;
    
do_memmove:
    /* This should trigger the memmove redirection */
    __builtin_memmove(dest->data, src->data, sizeof(dest->data));
    
    /* Jump out */
    goto finish;
    
finish:
    return;
}

/* Parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buf[128];
        char src_buf[128];
        
        /* Initialize with built-ins */
        __builtin_memset(src_buf, thread_id, sizeof(src_buf));
        
        /* Force all three built-ins in parallel region */
        __builtin_memcpy(local_buf, src_buf, volatile_len % 64);
        __builtin_memset(local_buf + 32, 0xCC, 32);
        __builtin_memmove(local_buf + 16, local_buf, 48);
        
        /* Use result to prevent optimization */
        volatile_dest[thread_id % 256] = local_buf[0];
    }
}

/* Complex memory dispatch logic */
static unsigned long execute_memory_dispatch(ASTNode* root) {
    if (!root) return 0;
    
    unsigned long hash = 0;
    ASTNode temp_node;
    
    /* Test all three built-ins in sequence */
    __builtin_memset(&temp_node, 0, sizeof(temp_node));
    __builtin_memcpy(&temp_node, root, sizeof(temp_node));
    __builtin_memmove(root->data, temp_node.data, sizeof(root->data));
    
    /* Process with goto flow */
    process_with_goto(root, &temp_node);
    
    /* Calculate hash from node data */
    for (int i = 0; i < 32; i++) {
        hash = hash * 31 + (unsigned char)root->data[i];
    }
    
    /* Recursive processing */
    hash += execute_memory_dispatch(root->left);
    hash += execute_memory_dispatch(root->right);
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create complex AST */
    ASTNode* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Execute parallel operations */
    parallel_memory_operations();
    
    /* Execute memory dispatch */
    unsigned long result = execute_memory_dispatch(root);
    
    /* Additional built-in usage in main */
    char final_buf[256];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf, root->data, sizeof(root->data));
    __builtin_memmove(final_buf + 128, final_buf, 64);
    
    /* Use volatile to ensure operations aren't optimized away */
    volatile_dest[0] = final_buf[0];
    
    printf("Result hash: %lu\n", result);
    printf("Volatile check: %d\n", (int)volatile_dest[0]);
    
    /* Cleanup */
    /* Note: In real code, you'd want to properly free the AST */
    
    return 0;
}
