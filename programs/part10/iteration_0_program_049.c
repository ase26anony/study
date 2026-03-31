#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Complex AST-like structure for data access patterns */
typedef struct ASTNode {
    int type;
    int value;
    volatile int flags;  /* volatile to prevent optimization */
    struct ASTNode *left;
    struct ASTNode *right;
    char padding[32];    /* Ensure size for redzone testing */
} ASTNode;

/* Global volatile variables to prevent constant folding */
volatile size_t g_mem_size = 64;
volatile int g_init_value = 0x42;
volatile int g_copy_offset = 8;

/* Function attributes for initialization ordering */
__attribute__((constructor)) 
static void init_constructor(void) {
    printf("Constructor: Initializing ASAN environment\n");
}

__attribute__((destructor))
static void cleanup_destructor(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast(int depth, int value) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = value;
    node->flags = g_init_value;  /* volatile access */
    
    /* Recursive creation with goto for control flow testing */
    if (depth > 1) {
        int left_val = value * 2;
        int right_val = value * 2 + 1;
        
        /* Jump label for goto testing */
        create_left:
        node->left = create_ast(depth - 1, left_val);
        
        /* Jump to skip right creation in some cases */
        if (value % 3 == 0) goto skip_right;
        
        node->right = create_ast(depth - 1, right_val);
        goto done;
        
        skip_right:
        node->right = NULL;
        
        done:;
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_memmove = 0;
    
    /* Goto jumping into memmove block */
    if (src->value > 100) {
        goto do_memmove;
    }
    
    /* Normal memcpy path */
    __builtin_memcpy(&dst->value, &src->value, sizeof(int));
    return;
    
    do_memmove:
    /* This tests the memmove redirection */
    __builtin_memmove(&dst->value, &src->value, sizeof(int));
    
    /* Jump out of the block */
    goto finish;
    
    /* Unreachable code that might confuse flow analysis */
    __builtin_memset(&dst->value, 0, sizeof(int));
    
    finish:
    dst->flags = src->flags;
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Force memcpy redirection in parallel context */
            ASTNode temp;
            __builtin_memcpy(&temp, nodes[i], sizeof(ASTNode));
            
            /* Modify with memset */
            __builtin_memset(&temp.padding[g_copy_offset], i, 16);
            
            /* Copy back with memmove (overlapping test) */
            __builtin_memmove(&nodes[i]->padding[4], &temp.padding[8], 24);
            
            /* Volatile operation to prevent dead code elimination */
            nodes[i]->flags += i;
        }
    }
}

/* Complex token processing with varied memory operations */
static int process_token_array(const char* tokens[], int token_count) {
    int hash = 0;
    char* buffers[4];
    volatile int buffer_index = 0;
    
    /* Allocate buffers with different sizes */
    for (int i = 0; i < 4; i++) {
        size_t size = g_mem_size * (i + 1);
        buffers[i] = (char*)malloc(size);
        if (buffers[i]) {
            /* Initialize each buffer differently */
            if (i % 2 == 0) {
                __builtin_memset(buffers[i], g_init_value + i, size);
            } else {
                __builtin_memcpy(buffers[i], tokens[i % token_count], 
                               size > 16 ? 16 : size);
            }
        }
    }
    
    /* Process tokens with memory operations */
    for (int i = 0; i < token_count; i++) {
        buffer_index = i % 4;
        
        if (buffers[buffer_index]) {
            /* Mix memcpy and memmove operations */
            if (i % 3 == 0) {
                /* Overlapping memmove */
                __builtin_memmove(buffers[buffer_index] + 8,
                                buffers[buffer_index] + 4, 32);
            }
            
            /* Always do memcpy */
            char temp[64];
            __builtin_memcpy(temp, buffers[buffer_index], 64);
            
            /* Compute hash from buffer contents */
            for (int j = 0; j < 64 && j < g_mem_size; j++) {
                hash += temp[j] * (i + 1);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        if (buffers[i]) {
            /* Final memset before free */
            __builtin_memset(buffers[i], 0, g_mem_size * (i + 1));
            free(buffers[i]);
        }
    }
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN memory operation tests\n");
    
    /* Create complex token array */
    const char* tokens[] = {
        "TOKEN_ALPHA", "TOKEN_BETA", "TOKEN_GAMMA",
        "TOKEN_DELTA", "TOKEN_EPSILON", "TOKEN_ZETA"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Create AST structures */
    ASTNode* ast1 = create_ast(4, 1);
    ASTNode* ast2 = create_ast(3, 50);
    ASTNode* ast3 = create_ast(2, 150);  /* Will trigger memmove path */
    
    /* Test goto control flow with memory operations */
    if (ast1 && ast2) {
        process_with_goto(ast1, ast2);
    }
    
    if (ast3 && ast2) {
        process_with_goto(ast3, ast2);  /* This uses memmove */
    }
    
    /* Prepare array for parallel processing */
    ASTNode* node_array[] = {ast1, ast2, ast3, NULL, ast1};
    int array_size = sizeof(node_array) / sizeof(node_array[0]);
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, array_size);
    
    /* Process token array with varied memory ops */
    int final_hash = process_token_array(tokens, token_count);
    
    /* Verify operations by computing checksum */
    int checksum = 0;
    if (ast1) checksum += ast1->value + ast1->flags;
    if (ast2) checksum += ast2->value + ast2->flags;
    if (ast3) checksum += ast3->value + ast3->flags;
    
    checksum += final_hash;
    
    printf("Result checksum: %d\n", checksum);
    printf("Final hash: %d\n", final_hash);
    
    /* Cleanup */
    if (ast1) free(ast1);
    if (ast2) free(ast2);
    if (ast3) free(ast3);
    
    return 0;
}
