/* ISO C99-compliant test program for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "test", "data", "node"
};
static const int g_num_tokens = 6;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    printf("ASAN/HWASAN test constructor initialized\n");
    /* Force initialization of built-in redirection cache */
    char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("ASAN/HWASAN test destructor cleaning up\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, int* token_idx) {
    if (depth <= 0 || *token_idx >= g_num_tokens) {
        return NULL;
    }
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = *token_idx;
    node->value = depth;
    
    /* Copy token name using __builtin_memcpy */
    const char* token = g_tokens[*token_idx];
    size_t len = strlen(token);
    if (len > 31) len = 31;
    __builtin_memcpy(node->data, token, len);
    node->data[len] = '\0';
    
    /* Recursive calls with goto for flow control */
    int original_idx = *token_idx;
    
    if (depth > 1) {
        (*token_idx)++;
        if (*token_idx >= g_num_tokens) *token_idx = 0;
        
        node->left = parse_expression(depth - 1, token_idx);
        
        /* Jump label for goto testing */
        skip_right:
        (*token_idx)++;
        if (*token_idx >= g_num_tokens) *token_idx = 0;
        
        node->right = parse_expression(depth - 2, token_idx);
        
        /* Use goto to create unusual control flow */
        if (node->type == 2) { /* memmove token */
            goto skip_copy;
        }
        
        /* Copy data between nodes using __builtin_memmove */
        if (node->left && node->right) {
            __builtin_memmove(node->right->data, 
                            node->left->data, 
                            sizeof(node->left->data));
        }
        
        skip_copy:
        ;
    }
    
    /* Another goto target */
    if (node->type == 1 && depth == 3) { /* memset token at depth 3 */
        goto skip_right;
    }
    
    *token_idx = original_idx + 1;
    return node;
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_operations(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Use volatile-controlled sizes */
            size_t op_size = g_mem_size;
            if (op_size > sizeof(nodes[i]->data)) {
                op_size = sizeof(nodes[i]->data);
            }
            
            /* Mix of memory operations */
            switch (i % 3) {
                case 0:
                    __builtin_memset(nodes[i]->data, i, op_size);
                    break;
                case 1:
                    if (i > 0 && nodes[i-1]) {
                        __builtin_memcpy(nodes[i]->data, 
                                       nodes[i-1]->data, 
                                       op_size);
                    }
                    break;
                case 2:
                    if (i > 0 && nodes[i-1]) {
                        __builtin_memmove(nodes[i]->data, 
                                        nodes[i-1]->data, 
                                        op_size);
                    }
                    break;
            }
        }
    }
}

/* Calculate hash of AST tree */
static int calculate_tree_hash(ASTNode* node) {
    if (!node) return 0;
    
    int hash = node->type * 31 + node->value;
    
    /* Hash the data using memory operations */
    char temp[32];
    __builtin_memset(temp, 0, sizeof(temp));
    __builtin_memcpy(temp, node->data, sizeof(node->data));
    
    for (size_t i = 0; i < sizeof(temp); i++) {
        hash = hash * 17 + temp[i];
    }
    
    hash += calculate_tree_hash(node->left);
    hash += calculate_tree_hash(node->right);
    
    return hash;
}

/* Free AST tree */
static void free_tree(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before freeing */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Parse recursive structure */
    int token_idx = 0;
    ASTNode* root = parse_expression(5, &token_idx);
    
    if (!root) {
        fprintf(stderr, "Failed to parse expression\n");
        return 1;
    }
    
    /* Create array of nodes for parallel operations */
    ASTNode* nodes[10];
    nodes[0] = root;
    
    /* Build node array from tree */
    int node_count = 1;
    ASTNode* stack[20];
    int stack_ptr = 0;
    
    if (root->left) stack[stack_ptr++] = root->left;
    if (root->right) stack[stack_ptr++] = root->right;
    
    while (stack_ptr > 0 && node_count < 10) {
        ASTNode* node = stack[--stack_ptr];
        nodes[node_count++] = node;
        
        if (node->left && node_count < 20) 
            stack[stack_ptr++] = node->left;
        if (node->right && node_count < 20) 
            stack[stack_ptr++] = node->right;
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations(nodes, node_count);
    
    /* Calculate and print verification hash */
    int final_hash = calculate_tree_hash(root);
    printf("Final tree hash: %d\n", final_hash);
    
    /* Additional memory operation with goto */
    char buffer1[128];
    char buffer2[128];
    
    __builtin_memset(buffer1, 'A', sizeof(buffer1));
    
    /* Complex goto pattern */
    int use_memmove = 1;
    
    if (use_memmove) {
        goto do_memmove;
    }
    
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    goto skip_memmove;
    
    do_memmove:
    __builtin_memmove(buffer2, buffer1, sizeof(buffer1));
    
    skip_memmove:
    
    /* Verify the copy/move */
    int verify = 1;
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        if (buffer1[i] != buffer2[i]) {
            verify = 0;
            break;
        }
    }
    
    printf("Memory verification: %s\n", verify ? "PASS" : "FAIL");
    
    /* Cleanup */
    free_tree(root);
    
    printf("Test completed successfully\n");
    return 0;
}
