/* ISO C99-compliant test program for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Token array for parser simulation */
typedef struct {
    char tokens[32][16];
    int count;
} TokenArray;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force memcpy redirection in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    g_use_hwasan = (buffer[0] == 0xAA) ? 1 : 0;
}

/* Destructor to test cleanup paths */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char final_buffer[64];
    __builtin_memset(final_buffer, 0xFF, sizeof(final_buffer));
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(TokenArray* tokens, int* pos) {
    if (*pos >= tokens->count) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data with memcpy */
    size_t copy_len = strlen(tokens->tokens[*pos]) + 1;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, tokens->tokens[*pos], copy_len);
    
    node->id = (*pos)++;
    
    /* Control flow with goto for edge cases */
    if (node->id % 3 == 0) {
        goto skip_left;
    }
    
    node->left = parse_expression(tokens, pos);
    
skip_left:
    /* Jump back into normal flow */
    if (node->id % 2 == 0) {
        node->right = parse_expression(tokens, pos);
        goto finish_node;
    }
    
    /* Another memory operation after goto */
    volatile char temp[32];
    __builtin_memmove(temp, node->data, sizeof(temp));
    
finish_node:
    return node;
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        volatile char local_buf[256];
        volatile int thread_id = 0;
        
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Different memory operations per thread */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(local_buf, thread_id, g_mem_size);
                break;
            case 1:
                __builtin_memcpy(local_buf + 64, local_buf, 128);
                break;
            case 2:
                __builtin_memmove(local_buf, local_buf + 32, 192);
                break;
        }
        
        /* Cross-thread memory operation simulation */
        #pragma omp barrier
        
        volatile char shared_buf[512];
        #pragma omp single
        {
            __builtin_memset(shared_buf, 0xCC, sizeof(shared_buf));
        }
        
        #pragma omp for
        for (int i = 0; i < 8; i++) {
            volatile char thread_buf[64];
            __builtin_memcpy(thread_buf, shared_buf + i * 64, 64);
            __builtin_memset(thread_buf + 32, i, 32);
            __builtin_memmove(shared_buf + i * 64, thread_buf, 64);
        }
    }
}

/* Calculate hash from AST tree */
static int calculate_tree_hash(ASTNode* node) {
    if (!node) return 0;
    
    int hash = node->id;
    volatile char temp[64];
    
    /* Use memcpy in recursive calculation */
    __builtin_memcpy(temp, node->data, sizeof(temp));
    
    for (size_t i = 0; i < sizeof(temp) && temp[i]; i++) {
        hash = (hash * 31 + temp[i]) % 1000000007;
    }
    
    /* Complex control flow with goto */
    if (hash % 7 == 0) {
        goto skip_children;
    }
    
    hash += calculate_tree_hash(node->left);
    
skip_children:
    if (hash % 5 != 0) {
        hash += calculate_tree_hash(node->right);
    } else {
        /* Another memory operation in goto path */
        volatile char dummy[16];
        __builtin_memset(dummy, hash & 0xFF, sizeof(dummy));
    }
    
    return hash;
}

/* Main execution flow */
int main(void) {
    /* Initialize token array */
    TokenArray tokens;
    tokens.count = 10;
    
    const char* sample_tokens[] = {
        "IDENT", "NUMBER", "PLUS", "MINUS", "TIMES",
        "DIVIDE", "LPAREN", "RPAREN", "ASSIGN", "SEMI"
    };
    
    for (int i = 0; i < tokens.count; i++) {
        size_t len = strlen(sample_tokens[i]) + 1;
        if (len > sizeof(tokens.tokens[i])) len = sizeof(tokens.tokens[i]);
        __builtin_memcpy(tokens.tokens[i], sample_tokens[i], len);
    }
    
    /* Parse expression tree */
    int pos = 0;
    ASTNode* root = parse_expression(&tokens, &pos);
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Additional memory operations in main */
    volatile char main_buffer[1024];
    volatile size_t dynamic_size = g_mem_size * 4;
    
    /* Test all three builtins in sequence */
    __builtin_memset(main_buffer, 0x11, dynamic_size);
    __builtin_memcpy(main_buffer + 256, main_buffer, 512);
    __builtin_memmove(main_buffer, main_buffer + 128, 768);
    
    /* Calculate and print result */
    int result_hash = 0;
    if (root) {
        result_hash = calculate_tree_hash(root);
        
        /* Cleanup with memory operations */
        volatile char cleanup_buf[128];
        __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
        __builtin_memcpy(cleanup_buf, root->data, sizeof(cleanup_buf));
        
        free(root);
    }
    
    printf("Result hash: %d\n", result_hash);
    
    /* Final memory operation to ensure all paths are taken */
    volatile char final_check[256];
    __builtin_memset(final_check, result_hash & 0xFF, sizeof(final_check));
    
    return 0;
}
