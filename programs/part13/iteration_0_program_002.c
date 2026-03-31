/* ISO C99-compliant test program for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int value;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Initializing ASAN environment...\n");
    /* Force early initialization of memory functions */
    char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Cleaning up ASAN environment...\n");
}

/* Complex token array for parser */
enum TokenType { TOKEN_NUM, TOKEN_OP, TOKEN_END };
typedef struct Token {
    enum TokenType type;
    union {
        int num;
        char op;
    } value;
} Token;

/* Recursive parser with memory operations */
static ASTNode* parse_expression(const Token** tokens) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with volatile-controlled size */
    size_t copy_size = g_mem_size % 64;
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Use goto for control flow edge cases */
    if ((*tokens)->type == TOKEN_NUM) {
        node->value = (*tokens)->value.num;
        (*tokens)++;
        goto skip_operation;
    }
    
    /* Memory copy between nodes */
    ASTNode temp_node;
    __builtin_memcpy(&temp_node, node, sizeof(ASTNode));
    node->left = parse_expression(tokens);
    
skip_operation:
    /* Another goto jumping into block with memmove */
    if ((*tokens)->type == TOKEN_OP) {
        char op_buffer[8];
        __builtin_memmove(op_buffer, &(*tokens)->value.op, 1);
        (*tokens)++;
    }
    
    return node;
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
        char src_buffer[256];
        char dst_buffer[256];
        
        /* Initialize with builtin memset */
        __builtin_memset(src_buffer, thread_id, sizeof(src_buffer));
        
        /* Copy with volatile size */
        volatile size_t copy_len = g_mem_size % 128 + 128;
        __builtin_memcpy(dst_buffer, src_buffer, copy_len);
        
        /* Move data around */
        __builtin_memmove(src_buffer + 64, src_buffer, 128);
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize token array */
    Token tokens[] = {
        {TOKEN_NUM, .value.num = 42},
        {TOKEN_OP, .value.op = '+'},
        {TOKEN_NUM, .value.num = 23},
        {TOKEN_END, .value.num = 0}
    };
    const Token* token_ptr = tokens;
    
    /* Create AST with recursive parsing */
    ASTNode* root = parse_expression(&token_ptr);
    
    if (root) {
        /* Perform memory operations on AST */
        ASTNode node_copy;
        __builtin_memcpy(&node_copy, root, sizeof(ASTNode));
        
        /* Move data within node */
        __builtin_memmove(root->data + 32, root->data, 32);
        
        /* Clear part of node */
        __builtin_memset(root->data + 48, 0, 16);
        
        free(root);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Complex memory pattern with all three builtins */
    char* dynamic_buf1 = malloc(512);
    char* dynamic_buf2 = malloc(512);
    
    if (dynamic_buf1 && dynamic_buf2) {
        /* Chain of memory operations */
        __builtin_memset(dynamic_buf1, 0xAA, 512);
        __builtin_memcpy(dynamic_buf2, dynamic_buf1, 256);
        __builtin_memmove(dynamic_buf1 + 128, dynamic_buf1, 384);
        
        /* Verify with checksum */
        uint64_t checksum = 0;
        for (int i = 0; i < 256; i++) {
            checksum += dynamic_buf2[i];
        }
        printf("Memory operations completed. Checksum: %llu\n", 
               (unsigned long long)checksum);
    }
    
    free(dynamic_buf1);
    free(dynamic_buf2);
    
    printf("Test completed successfully.\n");
    return 0;
}
