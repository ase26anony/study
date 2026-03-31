/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) static void init_globals(void) {
    g_init_flag = 1;
}

/* Destructor for cleanup verification */
__attribute__((destructor)) static void cleanup_check(void) {
    printf("Cleanup: g_init_flag = %d\n", g_init_flag);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Build node data with __builtin_memcpy */
    char temp[64];
    __builtin_memcpy(temp, base, strlen(base) + 1);
    __builtin_memcpy(node->data, temp, sizeof(node->data));
    node->size = sizeof(node->data);
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    if (depth > 2) {
        goto create_children;
    }
    
create_children:
    node->left = create_ast(depth - 1, "left");
    node->right = create_ast(depth - 1, "right");
    
    /* Jump back to avoid optimization */
    if (create_left) {
        create_left = 0;
        goto skip_memmove;
    }
    
    /* Use __builtin_memmove with goto edge case */
    if (node->left && node->right) {
        char buffer[128];
        __builtin_memcpy(buffer, node->left->data, node->left->size);
        __builtin_memmove(buffer + 32, buffer, 32);
        __builtin_memcpy(node->right->data, buffer + 32, 32);
    }
    
skip_memmove:
    return node;
}

/* Parallel memory operation dispatcher */
static void parallel_mem_ops(ASTNode** nodes, int count) {
    #pragma omp parallel for
    for (int i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Force built-in usage with volatile sizes */
            size_t op_size = g_mem_size;
            char* dest = nodes[i]->data;
            char* src = nodes[i]->data + 16;
            
            /* All three built-ins in parallel context */
            __builtin_memset(dest, i, op_size % 64);
            __builtin_memcpy(dest + 8, src, 16);
            __builtin_memmove(src, dest, 8);
            
            /* Additional goto for flow complexity */
            if (i % 3 == 0) {
                goto extra_op;
            }
            continue;
            
        extra_op:
            __builtin_memcpy(nodes[i]->data + 32, "EXTRA", 6);
        }
    }
}

/* Token processing with memory operations */
static size_t process_tokens(const char** tokens, int token_count) {
    size_t hash = 0;
    char buffer[512];
    volatile size_t buf_size = sizeof(buffer);
    
    for (int i = 0; i < token_count; i++) {
        /* Mixed built-in usage pattern */
        if (i % 2 == 0) {
            __builtin_memset(buffer, 0, buf_size);
            __builtin_memcpy(buffer, tokens[i], strlen(tokens[i]));
        } else {
            __builtin_memmove(buffer + 128, buffer, 64);
            __builtin_memcpy(buffer, tokens[i], strlen(tokens[i]));
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < buf_size && buffer[j]; j++) {
            hash = hash * 31 + buffer[j];
        }
        
        /* Goto-based control flow */
        if (hash % 7 == 0) {
            goto reset_buffer;
        }
        continue;
        
    reset_buffer:
        __builtin_memset(buffer, 0, 64);
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Initialize token array */
    const char* tokens[] = {
        "MEMCPY_TEST", "MEMSET_OPERATION", "MEMMOVE_DATA",
        "BUILTIN_REDIRECT", "GOTO_FLOW", "VOLATILE_SIZE",
        "PARALLEL_EXEC", "RECURSIVE_AST", "CONSTRUCTOR_INIT"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Create AST structures */
    ASTNode* root = create_ast(4, "ROOT");
    ASTNode* nodes[6];
    
    nodes[0] = root;
    for (int i = 1; i < 6; i++) {
        nodes[i] = create_ast(3, "NODE");
    }
    
    /* Execute parallel memory operations */
    parallel_mem_ops(nodes, 6);
    
    /* Process tokens with memory built-ins */
    size_t final_hash = process_tokens(tokens, token_count);
    
    /* Verify operations */
    printf("Result hash: %zu\n", final_hash);
    printf("Global flag: %d\n", g_init_flag);
    
    /* Cleanup */
    for (int i = 0; i < 6; i++) {
        if (nodes[i]) free(nodes[i]);
    }
    
    return final_hash != 0 ? 0 : 1;
}
