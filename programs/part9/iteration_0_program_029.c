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
    char buffer[128];
    int value;
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "data", "test"};
static const int token_count = 5;

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
}

__attribute__((destructor)) static void cleanup_globals(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive parser function */
static ASTNode* create_ast(int depth, int index) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill buffer with pattern using __builtin_memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (index % 26), 63);
    pattern[63] = '\0';
    __builtin_memcpy(node->buffer, pattern, 64);
    
    node->value = index;
    node->left = create_ast(depth - 1, index * 2);
    node->right = create_ast(depth - 1, index * 2 + 1);
    
    return node;
}

/* Function with goto statements for flow control */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int use_memmove = g_use_memmove;
    
    if (use_memmove) {
        goto use_memmove_block;
    } else {
        goto use_memcpy_block;
    }
    
use_memmove_block:
    {
        /* This block tests memmove redirection with goto */
        __builtin_memmove(dst->buffer, src->buffer, 64);
        goto after_copy;
    }
    
use_memcpy_block:
    {
        __builtin_memcpy(dst->buffer, src->buffer, 64);
        /* fall through */
    }
    
after_copy:
    /* Additional processing */
    __builtin_memset(src->buffer + 32, 'Z', 16);
}

/* Parallel memory operation dispatcher */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count - 1; i++) {
            size_t op_size = g_mem_size % 128;
            
            /* Force all three builtins to be used */
            if (tid % 3 == 0) {
                __builtin_memcpy(nodes[i+1]->buffer, 
                               nodes[i]->buffer, 
                               op_size);
            } else if (tid % 3 == 1) {
                __builtin_memset(nodes[i]->buffer + 16, 
                               '0' + (tid % 10), 
                               op_size / 2);
            } else {
                /* Create overlapping regions for memmove */
                char temp[256];
                __builtin_memcpy(temp, nodes[i]->buffer, 128);
                __builtin_memmove(nodes[i]->buffer + 32, 
                                nodes[i]->buffer, 
                                96);
                __builtin_memcpy(nodes[i]->buffer, temp, 32);
            }
        }
    }
}

/* Compute hash from AST */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* p = node->buffer;
    
    /* Process buffer with volatile control */
    volatile int i = 0;
    while (i < 64 && *p) {
        hash = ((hash << 5) + hash) + *p;
        p++;
        i++;
    }
    
    hash ^= compute_ast_hash(node->left);
    hash ^= compute_ast_hash(node->right);
    hash ^= node->value;
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create AST structures */
    ASTNode* ast1 = create_ast(3, 1);
    ASTNode* ast2 = create_ast(3, 100);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST structures\n");
        return 1;
    }
    
    /* Test goto flow control with memory operations */
    process_with_goto(ast1, ast2);
    
    /* Create array of nodes for parallel processing */
    ASTNode* node_array[8];
    node_array[0] = ast1;
    node_array[1] = ast2;
    
    for (int i = 2; i < 8; i++) {
        node_array[i] = create_ast(2, i * 50);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Additional explicit builtin calls */
    char extra_buf1[256], extra_buf2[256];
    
    __builtin_memset(extra_buf1, 'X', 256);
    __builtin_memcpy(extra_buf2, extra_buf1, 256);
    
    /* Force memmove with overlapping regions */
    __builtin_memmove(extra_buf1 + 128, extra_buf1, 128);
    
    /* Compute and print verification hash */
    unsigned long total_hash = 0;
    for (int i = 0; i < 8; i++) {
        if (node_array[i]) {
            total_hash ^= compute_ast_hash(node_array[i]);
        }
    }
    
    /* Process token array */
    for (int i = 0; i < token_count - 1; i++) {
        char token_copy[32];
        size_t len = strlen(tokens[i]) + 1;
        __builtin_memcpy(token_copy, tokens[i], len);
        __builtin_memset(token_copy + len - 1, '_', 1);
        total_hash += (unsigned long)token_copy[0];
    }
    
    printf("Verification hash: 0x%016lx\n", total_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    for (int i = 2; i < 8; i++) {
        free(node_array[i]);
    }
    free(ast1);
    free(ast2);
    
    return 0;
}
