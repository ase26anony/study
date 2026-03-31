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
    char *data;
    size_t size;
    struct ASTNode *left;
    struct ASTNode *right;
    int id;
} ASTNode;

/* Global token array */
static char g_tokens[][16] = {
    "token1", "token2", "token3", "token4",
    "token5", "token6", "token7", "token8"
};

/* Constructor attribute for early initialization */
__attribute__((constructor))
static void init_asan_early(void) {
    volatile char buffer[64];
    /* Force __builtin_memset redirection */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    
    /* Initialize global memory size with non-foldable pattern */
    g_mem_size = (size_t)((unsigned long)&g_mem_size & 0xFF) + 128;
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char final_check[32];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->size = g_mem_size / (depth + 1);
    node->data = (char*)malloc(node->size);
    node->id = depth;
    node->left = NULL;
    node->right = NULL;
    
    if (node->data) {
        /* Use __builtin_memset with volatile length */
        volatile size_t fill_size = node->size;
        __builtin_memset(node->data, depth + 0x30, fill_size);
        
        /* Copy token into beginning of buffer */
        const char *token = g_tokens[depth % 8];
        volatile size_t copy_len = strlen(token) + 1;
        __builtin_memcpy(node->data, token, copy_len);
    }
    
    /* Recursive creation with goto for flow control */
    if (depth < max_depth - 1) {
        goto create_children;
    }
    
    return node;
    
create_children:
    node->left = create_ast(depth + 1, max_depth);
    
    /* Jump back from goto block */
    if (node->left && depth < max_depth - 2) {
        node->right = create_ast(depth + 2, max_depth);
    }
    
    return node;
}

/* Memory operation between AST nodes */
static void copy_ast_data(ASTNode *dest, ASTNode *src) {
    if (!dest || !src || !dest->data || !src->data) return;
    
    volatile size_t copy_size = dest->size < src->size ? dest->size : src->size;
    
    /* Conditional memmove with goto */
    if (g_use_memmove) {
        goto use_memmove;
    } else {
        __builtin_memcpy(dest->data, src->data, copy_size);
        return;
    }
    
use_memmove:
    /* This goto tests flow-sensitivity of asan_memfn_rtls retrieval */
    __builtin_memmove(dest->data, src->data, copy_size);
}

/* Parallel memory dispatch */
static void parallel_memory_ops(ASTNode **nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i] && nodes[i]->data) {
            volatile char temp_buf[128];
            volatile size_t op_size = nodes[i]->size < 128 ? nodes[i]->size : 128;
            
            /* Mix of memory operations in parallel region */
            __builtin_memcpy(temp_buf, nodes[i]->data, op_size);
            
            /* Modify and copy back */
            temp_buf[0] = (char)('A' + (i % 26));
            __builtin_memcpy(nodes[i]->data, temp_buf, op_size);
            
            /* Conditional memset */
            if (i % 3 == 0) {
                __builtin_memset(nodes[i]->data + op_size/2, 0xCC, op_size/4);
            }
        }
    }
}

/* Free AST recursively */
static void free_ast(ASTNode *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node->data);
    free(node);
}

int main(void) {
    const int ast_depth = 5;
    const int node_count = 8;
    ASTNode *nodes[node_count];
    unsigned long hash = 0;
    int i;
    
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create multiple ASTs */
    for (i = 0; i < node_count; i++) {
        nodes[i] = create_ast(0, ast_depth);
    }
    
    /* Perform memory operations between nodes */
    for (i = 0; i < node_count - 1; i++) {
        if (nodes[i] && nodes[i+1]) {
            copy_ast_data(nodes[i], nodes[i+1]);
            
            /* Toggle memmove usage */
            g_use_memmove = !g_use_memmove;
            
            /* Use goto to skip every other copy */
            if (i % 2 == 0) {
                goto skip_copy;
            }
            copy_ast_data(nodes[i+1], nodes[i]);
        }
    skip_copy:
        continue;
    }
    
    /* Parallel operations */
    parallel_memory_ops(nodes, node_count);
    
    /* Compute verification hash */
    for (i = 0; i < node_count; i++) {
        if (nodes[i] && nodes[i]->data) {
            for (size_t j = 0; j < (nodes[i]->size < 16 ? nodes[i]->size : 16); j++) {
                hash += (unsigned long)nodes[i]->data[j] * (i + 1);
            }
        }
    }
    
    printf("Verification hash: %lu\n", hash);
    
    /* Cleanup */
    for (i = 0; i < node_count; i++) {
        free_ast(nodes[i]);
    }
    
    /* Final memory operation in main */
    volatile char final_buffer[64];
    volatile size_t final_size = sizeof(final_buffer);
    __builtin_memset(final_buffer, hash & 0xFF, final_size);
    
    return (int)(hash % 256);
}
