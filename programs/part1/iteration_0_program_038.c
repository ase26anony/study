/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array for parser simulation */
static const char* tokens[] = {"memcpy", "memset", "memmove", "data", "node"};
static const size_t token_count = sizeof(tokens)/sizeof(tokens[0]);

/* Constructor attribute forces early initialization */
__attribute__((constructor))
static void init_global_buffer(void) {
    g_init_flag = 1;
    printf("Constructor: Global buffer initialized\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_global_buffer(void) {
    printf("Destructor: Program cleanup completed\n");
}

/* Recursive parser with goto control flow */
static ASTNode* parse_expression(int depth, int* index) {
    if (depth <= 0 || *index >= token_count) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with volatile size */
    node->size = g_mem_size;
    node->left = NULL;
    node->right = NULL;
    
    /* Use goto for flow control around memmove */
    int use_memmove = 0;
    if (depth % 2 == 0) {
        use_memmove = 1;
        goto memmove_block;
    }
    
    /* Regular initialization path */
    __builtin_memset(node->data, 0, sizeof(node->data));
    goto parse_children;
    
memmove_block:
    {
        /* Create source buffer */
        char src[256];
        __builtin_memset(src, 'A', sizeof(src));
        
        /* Force memmove with goto entry/exit */
        __builtin_memmove(node->data, src, node->size);
        
        /* Jump out of memmove block */
        goto parse_children;
    }
    
parse_children:
    /* Copy token into node using memcpy */
    const char* current_token = tokens[*index];
    size_t token_len = strlen(current_token);
    __builtin_memcpy(node->data, current_token, 
                    token_len < sizeof(node->data) ? token_len : sizeof(node->data)-1);
    
    /* Recursive parsing */
    (*index)++;
    node->left = parse_expression(depth-1, index);
    node->right = parse_expression(depth-1, index);
    
    return node;
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(ASTNode* nodes[], size_t count) {
    #pragma omp parallel for
    for (size_t i = 0; i < count; i++) {
        if (nodes[i] && nodes[i]->left && nodes[i]->right) {
            /* Inter-node memory operations */
            __builtin_memcpy(nodes[i]->data, nodes[i]->left->data, 
                           nodes[i]->size < sizeof(nodes[i]->data) ? 
                           nodes[i]->size : sizeof(nodes[i]->data));
            
            /* Conditional memset */
            if (i % 3 == 0) {
                __builtin_memset(nodes[i]->right->data, i % 256, 
                               nodes[i]->size < sizeof(nodes[i]->right->data) ? 
                               nodes[i]->size : sizeof(nodes[i]->right->data));
            }
            
            /* Complex memmove with overlapping regions */
            if (i % 2 == 0 && i+1 < count && nodes[i+1]) {
                size_t move_size = nodes[i]->size / 2;
                if (move_size > 0) {
                    __builtin_memmove(nodes[i]->data + move_size/2, 
                                    nodes[i]->data, move_size);
                }
            }
        }
    }
}

/* Calculate hash from AST structure */
static size_t compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 5381;
    for (size_t i = 0; i < node->size && i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    return hash + compute_ast_hash(node->left) + compute_ast_hash(node->right);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Force initialization of global volatile */
    if (!g_init_flag) {
        printf("Warning: Constructor not called\n");
        return 1;
    }
    
    /* Create AST tree */
    int index = 0;
    ASTNode* root = parse_expression(4, &index);
    
    if (!root) {
        printf("Error: Failed to create AST\n");
        return 1;
    }
    
    /* Create node array for parallel processing */
    ASTNode* node_array[8];
    node_array[0] = root;
    
    /* Build additional nodes with different memory patterns */
    for (size_t i = 1; i < 8; i++) {
        node_array[i] = malloc(sizeof(ASTNode));
        if (node_array[i]) {
            node_array[i]->size = g_mem_size + i * 16;
            node_array[i]->left = NULL;
            node_array[i]->right = NULL;
            
            /* Varied memory operations */
            switch (i % 3) {
                case 0:
                    __builtin_memset(node_array[i]->data, '0' + i, 
                                   sizeof(node_array[i]->data));
                    break;
                case 1:
                    __builtin_memcpy(node_array[i]->data, tokens[i % token_count],
                                   strlen(tokens[i % token_count]));
                    break;
                case 2:
                    if (i > 1) {
                        __builtin_memmove(node_array[i]->data, 
                                        node_array[i-1]->data,
                                        node_array[i]->size < sizeof(node_array[i]->data) ?
                                        node_array[i]->size : sizeof(node_array[i]->data));
                    }
                    break;
            }
        }
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Compute verification hash */
    size_t total_hash = 0;
    for (size_t i = 0; i < 8; i++) {
        if (node_array[i]) {
            total_hash ^= compute_ast_hash(node_array[i]);
        }
    }
    
    printf("Verification hash: 0x%08zx\n", total_hash);
    printf("Built-in redirection test completed\n");
    
    /* Cleanup */
    for (size_t i = 0; i < 8; i++) {
        free(node_array[i]);
    }
    
    return 0;
}
