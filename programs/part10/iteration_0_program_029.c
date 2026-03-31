/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
static const size_t g_token_count = sizeof(g_tokens) / sizeof(g_tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    /* Force initialization of ASAN runtime */
    volatile char buffer[128];
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    printf("Destructor: ASAN environment cleanup\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* data, size_t depth) {
    if (depth > 3) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with goto for flow control */
    size_t copy_len = strlen(data);
    if (copy_len > 255) copy_len = 255;
    
    /* Jump label for goto testing */
    copy_start:
    __builtin_memcpy(node->data, data, copy_len);
    node->data[copy_len] = '\0';
    node->size = copy_len;
    
    /* Recursive creation with goto out of block */
    if (depth < 3) {
        node->left = create_ast_node(g_tokens[depth % g_token_count], depth + 1);
        goto skip_right;  /* Jump to skip right creation */
        
        right_create:
        node->right = create_ast_node(g_tokens[(depth + 1) % g_token_count], depth + 1);
    }
    
    skip_right:
    if (depth == 1) goto right_create;  /* Jump back into block */
    
    return node;
}

/* Memory operation dispatcher with OpenMP */
static void dispatch_memory_operations(ASTNode* nodes[], size_t count) {
    #pragma omp parallel
    {
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->left) {
                /* Test all three built-ins in parallel */
                char temp[256];
                volatile size_t op_size = g_mem_size;
                
                /* __builtin_memcpy between nodes */
                __builtin_memcpy(temp, nodes[i]->data, op_size);
                
                /* __builtin_memmove with overlapping regions */
                if (nodes[i]->right) {
                    __builtin_memmove(nodes[i]->data + 32, 
                                     nodes[i]->data, 
                                     op_size > 32 ? 32 : op_size);
                }
                
                /* __builtin_memset to clear partial data */
                __builtin_memset(nodes[i]->data + 64, 0xFF, 
                                op_size > 64 ? 64 : op_size);
                
                /* Copy back using goto for control flow */
                if (i % 2 == 0) {
                    goto copy_back;
                } else {
                    /* Alternative path */
                    __builtin_memcpy(nodes[i]->left->data, temp, op_size);
                    continue;
                }
                
                copy_back:
                __builtin_memcpy(nodes[i]->data, temp, op_size);
            }
        }
    }
}

/* Calculate hash of AST structure */
static size_t calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 5381;
    for (size_t i = 0; i < node->size && i < 256; i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Recursive hash calculation */
    hash += calculate_ast_hash(node->left);
    hash += calculate_ast_hash(node->right);
    
    return hash;
}

/* Free AST memory */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create array of AST nodes */
    const size_t node_count = 8;
    ASTNode* nodes[node_count];
    
    /* Initialize nodes with different tokens */
    for (size_t i = 0; i < node_count; i++) {
        const char* token = g_tokens[i % g_token_count];
        nodes[i] = create_ast_node(token, 0);
        
        /* Additional __builtin_memset on each node */
        if (nodes[i]) {
            volatile size_t clear_size = g_mem_size;
            __builtin_memset(nodes[i]->data + 128, 0, 
                            clear_size > 128 ? 128 : clear_size);
        }
    }
    
    /* Test control flow with goto around memmove */
    volatile int use_memmove = 1;
    if (use_memmove) {
        goto perform_memmove;
    } else {
        printf("Skipping memmove test\n");
        goto skip_memmove;
    }
    
    perform_memmove:
    if (nodes[0] && nodes[1]) {
        /* Force __builtin_memmove with goto */
        __builtin_memmove(nodes[0]->data, nodes[1]->data, 32);
    }
    
    skip_memmove:
    
    /* Dispatch parallel memory operations */
    dispatch_memory_operations(nodes, node_count);
    
    /* Calculate and print verification hash */
    size_t total_hash = 0;
    for (size_t i = 0; i < node_count; i++) {
        if (nodes[i]) {
            total_hash ^= calculate_ast_hash(nodes[i]);
        }
    }
    
    printf("Verification hash: %zu\n", total_hash);
    
    /* Additional built-in calls in cleanup */
    for (size_t i = 0; i < node_count; i++) {
        if (nodes[i] && nodes[i+1 < node_count ? i+1 : 0]) {
            /* Final memory operations */
            __builtin_memcpy(nodes[i]->data, 
                           nodes[i+1 < node_count ? i+1 : 0]->data, 
                           16);
            __builtin_memset(nodes[i]->data + 240, 0, 16);
        }
    }
    
    /* Cleanup */
    for (size_t i = 0; i < node_count; i++) {
        free_ast(nodes[i]);
    }
    
    printf("Test completed successfully\n");
    return 0;
}
