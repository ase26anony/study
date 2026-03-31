/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile int volatile_flag = 1;

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
__attribute__((constructor)) static void init_constructor(void) {
    /* Initialize global tokens with pattern */
    for (int i = 0; i < sizeof(global_tokens); i++) {
        global_tokens[i] = (char)((i % 26) + 'A');
    }
    
    /* Use __builtin_memset in constructor */
    char local_buf[128];
    __builtin_memset(local_buf, 0xCC, sizeof(local_buf));
    
    /* Copy to global using __builtin_memcpy */
    __builtin_memcpy(global_tokens + 512, local_buf, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_destructor(void) {
    /* Final memory operation in destructor */
    char final_buf[256];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = NULL;
    node->right = NULL;
    
    /* Initialize node data with __builtin_memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy pattern from global tokens */
    int copy_len = volatile_len % 128;
    if (copy_len > 0) {
        __builtin_memcpy(node->data, global_tokens + token_index, copy_len);
        token_index = (token_index + copy_len) % 512;
    }
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = volatile_flag;
        
        if (use_goto) {
            goto create_left;
        }
        
        node->left = create_ast_node(depth - 1, id * 2);
        
    create_left:
        node->right = create_ast_node(depth - 1, id * 2 + 1);
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int mode = volatile_flag % 3;
    
    switch (mode) {
        case 0:
            goto normal_copy;
        case 1:
            goto overlapping_copy;
        default:
            goto zero_fill;
    }
    
normal_copy:
    /* Use __builtin_memcpy with goto entry */
    if (src && dst) {
        __builtin_memcpy(dst->data, src->data, sizeof(src->data));
    }
    goto end;
    
overlapping_copy:
    /* Use __builtin_memmove for overlapping regions */
    if (src && dst) {
        char temp[256];
        __builtin_memcpy(temp, src->data, sizeof(src->data));
        __builtin_memmove(dst->data, temp, sizeof(temp));
    }
    goto end;
    
zero_fill:
    /* Use __builtin_memset */
    if (dst) {
        __builtin_memset(dst->data, 0, sizeof(dst->data));
    }
    goto end;
    
end:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char thread_buf[512];
        char thread_dst[512];
        
        /* Initialize with __builtin_memset */
        __builtin_memset(thread_buf, thread_id, sizeof(thread_buf));
        
        /* Copy with __builtin_memcpy */
        __builtin_memcpy(thread_dst, thread_buf, sizeof(thread_buf));
        
        /* Move with __builtin_memmove (overlapping) */
        __builtin_memmove(thread_buf + 128, thread_buf, 256);
        
        #pragma omp barrier
        
        /* Synchronized operation */
        #pragma omp single
        {
            char sync_buf[1024];
            __builtin_memset(sync_buf, 0xAA, sizeof(sync_buf));
            __builtin_memcpy(global_tokens, sync_buf, 512);
        }
    }
}

/* Multi-stage processing */
static unsigned long long process_ast_tree(ASTNode* root) {
    if (!root) return 0;
    
    unsigned long long hash = 0;
    
    /* Process node data */
    for (size_t i = 0; i < sizeof(root->data); i++) {
        hash = hash * 31 + (unsigned char)root->data[i];
    }
    
    /* Recursive processing */
    hash += process_ast_tree(root->left);
    hash += process_ast_tree(root->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create AST structure */
    ASTNode* ast_root = create_ast_node(4, 1);
    
    /* Create duplicate for memory operations */
    ASTNode* ast_copy = (ASTNode*)malloc(sizeof(ASTNode));
    if (ast_copy) {
        __builtin_memset(ast_copy, 0, sizeof(ASTNode));
        
        /* Test goto flow with memory operations */
        process_with_goto(ast_root, ast_copy);
        
        /* Additional memory operation without goto */
        if (volatile_flag) {
            __builtin_memcpy(ast_copy->data + 128, ast_root->data, 128);
        }
    }
    
    /* Execute OpenMP parallel section */
    #ifdef _OPENMP
    parallel_memory_operations();
    #endif
    
    /* Process tree and compute result */
    unsigned long long result = 0;
    if (ast_root) {
        result = process_ast_tree(ast_root);
        
        /* Additional memory operation on result */
        char result_buf[sizeof(unsigned long long)];
        __builtin_memcpy(result_buf, &result, sizeof(result));
        
        /* Overwrite with memset */
        __builtin_memset(result_buf, 0x42, sizeof(result_buf));
    }
    
    /* Cleanup */
    free(ast_copy);
    
    /* Note: In real code, we would free the AST tree recursively */
    
    printf("Test completed. Result hash: 0x%016llX\n", result);
    printf("(Note: Actual value depends on memory layout and timing)\n");
    
    return 0;
}
