/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile int volatile_offset = 16;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array for parser simulation */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "recursive", "parallel",
    "asan", "hwasan", "instrument", "redzone", "builtin"
};
static const int token_count = sizeof(tokens) / sizeof(tokens[0]);

/* Constructor/destructor for initialization coordination */
__attribute__((constructor))
static void init_asan_hooks(void) {
    printf("Constructor: Initializing ASAN/HWASAN hooks\n");
}

__attribute__((destructor))
static void cleanup_asan_hooks(void) {
    printf("Destructor: Cleaning up ASAN/HWASAN state\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, int* index) {
    if (depth <= 0 || *index >= token_count) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = (*index)++;
    
    /* Copy token data with builtin memcpy */
    const char* token = tokens[node->id % token_count];
    size_t len = strlen(token) + 1;
    if (len > sizeof(node->data)) len = sizeof(node->data);
    __builtin_memcpy(node->data, token, len);
    
    /* Recursive parsing with goto for flow control */
    int use_goto = (node->id % 3 == 0);
    
    if (use_goto) {
        goto parse_left;
    }
    
    node->left = parse_expression(depth - 1, index);
    
parse_left:
    if (!use_goto) {
        goto parse_right;
    }
    
    /* Memory move operation with goto */
    char temp[256];
    __builtin_memcpy(temp, node->data, sizeof(temp));
    __builtin_memmove(node->data + volatile_offset, temp, 
                     volatile_len < 256 ? volatile_len : 256);
    
parse_right:
    node->right = parse_expression(depth - 1, index);
    
    return node;
}

/* Parallel memory operation dispatcher */
static void parallel_memory_operations(ASTNode* nodes[], int count) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->left && nodes[i]->right) {
                /* Inter-node memory copies */
                __builtin_memcpy(nodes[i]->left->data, 
                               nodes[i]->data, 
                               volatile_len);
                
                /* Conditional memory set */
                if (thread_id % 2 == 0) {
                    __builtin_memset(nodes[i]->right->data + 32, 
                                   thread_id, 
                                   volatile_len / 2);
                }
                
                /* Memory move with overlap */
                __builtin_memmove(nodes[i]->data + 16,
                                nodes[i]->data,
                                volatile_len - 16);
            }
        }
    }
}

/* Calculate hash of AST structure */
static unsigned long calculate_ast_hash(ASTNode* root) {
    if (!root) return 0;
    
    unsigned long hash = 5381;
    char* ptr = root->data;
    
    /* DJB2 hash algorithm */
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    hash ^= calculate_ast_hash(root->left);
    hash ^= calculate_ast_hash(root->right);
    hash ^= (unsigned long)root->id;
    
    return hash;
}

/* Free AST memory */
static void free_ast(ASTNode* root) {
    if (!root) return;
    free_ast(root->left);
    free_ast(root->right);
    free(root);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Recursive parsing with memory operations */
    int index = 0;
    ASTNode* ast = parse_expression(4, &index);
    
    if (!ast) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Create node array for parallel operations */
    ASTNode* node_array[10];
    int array_index = 0;
    
    /* Fill array with AST nodes using depth-first traversal */
    ASTNode* stack[20];
    int stack_ptr = 0;
    stack[stack_ptr++] = ast;
    
    while (stack_ptr > 0 && array_index < 10) {
        ASTNode* current = stack[--stack_ptr];
        node_array[array_index++] = current;
        
        if (current->right) stack[stack_ptr++] = current->right;
        if (current->left) stack[stack_ptr++] = current->left;
    }
    
    /* Phase 3: Execute parallel memory operations */
    printf("Executing parallel memory operations...\n");
    parallel_memory_operations(node_array, array_index);
    
    /* Phase 4: Direct builtin calls with volatile control */
    char buffer1[256];
    char buffer2[256];
    
    /* Force all three builtins with volatile lengths */
    __builtin_memset(buffer1, 0xAA, volatile_len);
    __builtin_memcpy(buffer2, buffer1, volatile_len);
    __builtin_memmove(buffer1 + 32, buffer2, volatile_len / 2);
    
    /* Additional complex pattern */
    for (int i = 0; i < 5; i++) {
        volatile int local_len = volatile_len + i * 8;
        __builtin_memset(buffer1 + i * 16, i, local_len % 128);
        __builtin_memcpy(buffer2 + i * 16, buffer1 + i * 16, local_len % 128);
    }
    
    /* Phase 5: Calculate and verify result */
    unsigned long hash = calculate_ast_hash(ast);
    hash ^= (unsigned long)buffer1[0];
    hash ^= (unsigned long)buffer2[volatile_len % 256];
    
    printf("Result hash: 0x%08lx\n", hash);
    printf("Verification: %s\n", (hash != 0) ? "PASS" : "FAIL");
    
    /* Cleanup */
    free_ast(ast);
    
    return (hash != 0) ? 0 : 1;
}
