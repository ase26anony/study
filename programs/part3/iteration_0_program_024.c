/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array for parser simulation */
static const char* tokens[] = {"memcpy", "memset", "memmove", "data", "node"};
static const int token_count = 5;

/* Constructor/destructor for initialization coordination */
void __attribute__((constructor)) init_asan_hooks(void) {
    volatile char init_buf[32];
    __builtin_memset(init_buf, 0, sizeof(init_buf));
}

void __attribute__((destructor)) cleanup_asan_hooks(void) {
    volatile char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
ASTNode* parse_expression(int depth, int* token_idx) {
    if (depth <= 0 || *token_idx >= token_count) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = depth * 100 + *token_idx;
    
    /* Copy token name using builtin memcpy */
    const char* token = tokens[*token_idx];
    size_t len = strlen(token);
    if (len > sizeof(node->data) - 1) len = sizeof(node->data) - 1;
    __builtin_memcpy(node->data, token, len);
    node->data[len] = '\0';
    
    (*token_idx)++;
    
    /* Recursive calls with goto for flow control */
    int use_goto = (depth % 2 == 0);
    
    if (use_goto) {
        goto parse_left;
    }
    
    node->left = parse_expression(depth - 1, token_idx);
    
parse_left:
    if (use_goto) {
        node->left = parse_expression(depth - 1, token_idx);
    }
    
    /* Jump over right subtree initialization */
    if (depth == 3) {
        goto skip_right_init;
    }
    
    node->right = parse_expression(depth - 2, token_idx);
    goto after_right;
    
skip_right_init:
    node->right = NULL;
    
after_right:
    return node;
}

/* Copy AST structure with memmove between nodes */
void copy_ast_structure(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    volatile size_t copy_size = sizeof(ASTNode);
    
    /* Use memmove for potential overlapping copies */
    if (g_use_memmove) {
        __builtin_memmove(dest, src, copy_size);
    } else {
        __builtin_memcpy(dest, src, copy_size);
    }
    
    /* Recursive copy of children */
    if (src->left) {
        dest->left = (ASTNode*)malloc(sizeof(ASTNode));
        if (dest->left) {
            copy_ast_structure(dest->left, src->left);
        }
    }
    
    if (src->right) {
        dest->right = (ASTNode*)malloc(sizeof(ASTNode));
        if (dest->right) {
            copy_ast_structure(dest->right, src->right);
        }
    }
}

/* Parallel memory operations with OpenMP */
unsigned long long parallel_memory_operations(void) {
    unsigned long long hash = 0;
    const int num_threads = 4;
    volatile size_t block_size = g_mem_size / num_threads;
    
    #pragma omp parallel num_threads(num_threads) reduction(+:hash)
    {
        int tid = omp_get_thread_num();
        char* buffer = (char*)malloc(block_size);
        char* buffer2 = (char*)malloc(block_size);
        
        if (buffer && buffer2) {
            /* Initialize with builtin memset */
            __builtin_memset(buffer, tid, block_size);
            
            /* Copy with builtin memcpy */
            __builtin_memcpy(buffer2, buffer, block_size);
            
            /* Move with builtin memmove (potentially overlapping) */
            size_t move_size = block_size / 2;
            __builtin_memmove(buffer + move_size, buffer, move_size);
            
            /* Compute thread-local hash */
            for (size_t i = 0; i < block_size; i++) {
                hash += (unsigned char)buffer[i];
                hash += (unsigned char)buffer2[i];
            }
        }
        
        free(buffer);
        free(buffer2);
    }
    
    return hash;
}

/* Free AST recursively */
void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Recursive parser with memory operations */
    int token_idx = 0;
    ASTNode* ast = parse_expression(4, &token_idx);
    
    if (!ast) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Copy AST structure */
    ASTNode* ast_copy = (ASTNode*)malloc(sizeof(ASTNode));
    if (ast_copy) {
        copy_ast_structure(ast_copy, ast);
    }
    
    /* Phase 3: Parallel memory operations */
    unsigned long long hash = parallel_memory_operations();
    
    /* Phase 4: Additional builtin usage in varied contexts */
    volatile char dynamic_buf[512];
    volatile char dynamic_buf2[512];
    
    /* Force all three builtins in a loop */
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                __builtin_memset(dynamic_buf, i, sizeof(dynamic_buf));
                break;
            case 1:
                __builtin_memcpy(dynamic_buf2, dynamic_buf, sizeof(dynamic_buf) / 2);
                break;
            case 2:
                __builtin_memmove(dynamic_buf + 128, dynamic_buf, 256);
                break;
        }
    }
    
    /* Compute final verification hash */
    unsigned long long final_hash = hash;
    
    if (ast) {
        for (int i = 0; i < 64 && i < sizeof(ast->data); i++) {
            final_hash += (unsigned char)ast->data[i];
        }
        final_hash += ast->id;
    }
    
    if (ast_copy) {
        final_hash += ast_copy->id * 3;
    }
    
    /* Add dynamic buffer content */
    for (int i = 0; i < 64 && i < sizeof(dynamic_buf); i++) {
        final_hash += (unsigned char)dynamic_buf[i];
    }
    
    printf("Final verification hash: %llu\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free_ast(ast);
    free_ast(ast_copy);
    
    return 0;
}
