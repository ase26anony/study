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
static char global_tokens[1024];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    printf("Constructor: Initialized global tokens\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtin memcpy in destructor */
    char temp[1024];
    __builtin_memcpy(temp, global_tokens, sizeof(global_tokens));
    printf("Destructor: Cleaned up %zu bytes\n", sizeof(global_tokens));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for node initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = depth;
    
    /* Copy data with builtin memcpy */
    size_t copy_len = volatile_len % 256;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Recursive creation with goto for control flow */
    int use_goto = volatile_flag;
    
    if (use_goto) {
        goto create_left;
    }
    
    node->right = create_ast(depth - 1, base_data);
    
create_left:
    node->left = create_ast(depth - 1, base_data);
    
    if (!use_goto) {
        goto finish_node;
    }
    
    node->right = create_ast(depth - 1, base_data);
    
finish_node:
    return node;
}

/* Memory operation between AST nodes */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use builtin memmove with potential overlap */
    char buffer[512];
    
    /* Forward copy */
    __builtin_memcpy(buffer, src->data, sizeof(src->data));
    
    /* Backward copy with memmove for overlap handling */
    __builtin_memmove(dest->data, buffer, sizeof(dest->data));
    
    /* Recursive copy */
    copy_ast_data(dest->left, src->left);
    copy_ast_data(dest->right, src->right);
}

/* Parallel memory dispatch logic */
static void parallel_memory_ops(void) {
    int i;
    char buffers[4][256];
    volatile int sizes[4] = {32, 64, 128, 256};
    
    #pragma omp parallel for private(i)
    for (i = 0; i < 4; i++) {
        int thread_id = omp_get_thread_num();
        
        /* Initialize buffer with builtin memset */
        __builtin_memset(buffers[i], thread_id + '0', sizes[i]);
        
        /* Copy between buffers with builtin memcpy */
        if (i > 0) {
            __builtin_memcpy(buffers[i], buffers[i-1], sizes[i-1]);
        }
        
        /* Move data around with builtin memmove */
        __builtin_memmove(buffers[i] + 64, buffers[i], 128);
    }
}

/* Complex token processing with goto jumps */
static int process_tokens(void) {
    char local_buffer[512];
    int result = 0;
    int i = 0;
    
    /* Initial memset */
    __builtin_memset(local_buffer, 0, sizeof(local_buffer));
    
    /* Jump into memory operation block */
    if (volatile_flag) {
        goto mem_operation_block;
    }
    
normal_path:
    for (i = 0; i < 100; i++) {
        local_buffer[i] = (char)(i % 26 + 'A');
    }
    goto continue_processing;
    
mem_operation_block:
    {
        /* Builtin memmove inside goto block */
        char temp[256];
        __builtin_memset(temp, 'X', sizeof(temp));
        __builtin_memmove(local_buffer, temp, 128);
        
        /* Jump out to normal processing */
        if (volatile_flag) {
            goto normal_path;
        }
    }
    
continue_processing:
    /* Final memcpy */
    __builtin_memcpy(global_tokens + 256, local_buffer, 256);
    
    /* Calculate hash/sum */
    for (i = 0; i < 512; i++) {
        result += (int)global_tokens[i];
    }
    
    return result;
}

/* Main execution flow */
int main(void) {
    int final_result = 0;
    
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Recursive AST creation */
    printf("Phase 1: Creating AST structures...\n");
    ASTNode* ast1 = create_ast(3, "BaseData123");
    ASTNode* ast2 = create_ast(3, "DifferentBase");
    
    if (ast1 && ast2) {
        /* Phase 2: AST data copying */
        printf("Phase 2: Copying AST data...\n");
        copy_ast_data(ast2, ast1);
        
        /* Phase 3: Parallel operations */
        printf("Phase 3: Parallel memory operations...\n");
        parallel_memory_ops();
        
        /* Phase 4: Token processing with goto */
        printf("Phase 4: Token processing...\n");
        final_result = process_tokens();
        
        /* Cleanup */
        free(ast1);
        free(ast2);
    }
    
    printf("Final result (hash/sum): %d\n", final_result);
    printf("Test completed successfully.\n");
    
    return 0;
}
