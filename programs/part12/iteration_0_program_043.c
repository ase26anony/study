#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Complex AST-like structure for recursive operations */
typedef struct ASTNode {
    int type;
    int value;
    volatile int volatile_marker;  /* Prevent optimization */
    struct ASTNode *left;
    struct ASTNode *right;
    char padding[32];  /* Ensure size for memcpy operations */
} ASTNode;

/* Global token array for initialization */
static volatile int global_token_array[256];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    volatile int i;
    
    /* Initialize with __builtin_memset */
    for (i = 0; i < 256; i++) {
        __builtin_memset((void*)&global_token_array[i], i & 0xFF, sizeof(int));
    }
    
    /* Early __builtin_memcpy in constructor */
    volatile int temp[4];
    __builtin_memcpy((void*)temp, (void*)global_token_array, 4 * sizeof(int));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    volatile int cleanup_buf[16];
    __builtin_memset((void*)cleanup_buf, 0, 16 * sizeof(int));
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast(int depth, int value) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = value;
    node->volatile_marker = depth * 1000 + value;
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = (value % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, value * 2);
        node->right = create_ast(depth - 1, value * 2 + 1);
        
        if (use_goto) {
            create_children:
            /* This goto block contains __builtin_memmove */
            ASTNode temp_node;
            __builtin_memmove(&temp_node, node, sizeof(ASTNode));
            node->left = create_ast(depth - 1, value * 3);
            node->right = create_ast(depth - 1, value * 3 + 1);
            __builtin_memmove(node, &temp_node, sizeof(ASTNode));
        }
    }
    
    return node;
}

/* Complex memory operation between AST nodes */
static void copy_ast_data(ASTNode* dest, ASTNode* src) {
    volatile int size = sizeof(ASTNode);
    
    /* Force __builtin_memcpy with volatile size */
    __builtin_memcpy(dest, src, size);
    
    /* Additional __builtin_memmove with goto */
    if (src->left && dest->left) {
        int use_goto = (src->value % 2 == 0);
        
        if (use_goto) {
            goto copy_children;
        }
        
        __builtin_memcpy(dest->left, src->left, sizeof(ASTNode));
        
        if (use_goto) {
            copy_children:
            ASTNode temp;
            __builtin_memmove(&temp, src->right, sizeof(ASTNode));
            __builtin_memmove(dest->right, &temp, sizeof(ASTNode));
        }
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    volatile int buffer_size = 1024;
    char* src_buffer = (char*)malloc(buffer_size);
    char* dest_buffer = (char*)malloc(buffer_size);
    
    if (!src_buffer || !dest_buffer) {
        free(src_buffer);
        free(dest_buffer);
        return;
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        volatile int chunk_size = buffer_size / omp_get_num_threads();
        volatile int offset = thread_id * chunk_size;
        
        /* Each thread uses __builtin_memset */
        __builtin_memset(src_buffer + offset, thread_id, chunk_size);
        
        #pragma omp barrier
        
        /* __builtin_memcpy between buffers */
        __builtin_memcpy(dest_buffer + offset, src_buffer + offset, chunk_size);
        
        /* Conditional __builtin_memmove with goto */
        if (thread_id % 2 == 0) {
            goto move_data;
        }
        
        __builtin_memcpy(dest_buffer + offset + chunk_size/2, 
                        src_buffer + offset, 
                        chunk_size/2);
        
        if (thread_id % 2 == 0) {
            move_data:
            __builtin_memmove(dest_buffer + offset, 
                             src_buffer + offset + chunk_size/2, 
                             chunk_size/2);
        }
    }
    
    /* Final verification __builtin_memcmp */
    int result = __builtin_memcmp(src_buffer, dest_buffer, buffer_size);
    
    free(src_buffer);
    free(dest_buffer);
}

/* Main execution with complex flow */
int main(void) {
    volatile int depth = 4;
    volatile int seed = 42;
    
    /* Initialize global array with __builtin_memset */
    __builtin_memset((void*)global_token_array, 0, sizeof(global_token_array));
    
    /* Create AST structures */
    ASTNode* ast1 = create_ast(depth, seed);
    ASTNode* ast2 = create_ast(depth, seed + 1);
    
    if (!ast1 || !ast2) {
        free(ast1);
        free(ast2);
        return 1;
    }
    
    /* Copy between ASTs using all three builtins */
    __builtin_memcpy(ast2, ast1, sizeof(ASTNode));
    copy_ast_data(ast1, ast2);
    
    /* Complex __builtin_memmove with goto */
    volatile int use_complex_move = 1;
    
    if (use_complex_move) {
        ASTNode temp_ast;
        
        goto perform_move;
        
        __builtin_memset(&temp_ast, 0, sizeof(ASTNode));
        
        perform_move:
        __builtin_memmove(&temp_ast, ast1, sizeof(ASTNode));
        __builtin_memmove(ast1, ast2, sizeof(ASTNode));
        __builtin_memmove(ast2, &temp_ast, sizeof(ASTNode));
    }
    
    /* Execute parallel operations */
    parallel_memory_operations();
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (volatile int i = 0; i < 256; i++) {
        hash = (hash * 31) + global_token_array[i];
    }
    
    /* Additional __builtin_memset before exit */
    char exit_buffer[64];
    __builtin_memset(exit_buffer, 0, sizeof(exit_buffer));
    __builtin_memcpy(exit_buffer, &hash, sizeof(hash));
    
    printf("Verification hash: %lu\n", hash);
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    return 0;
}
