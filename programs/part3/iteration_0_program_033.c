/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t hash;
} ASTNode;

/* Token array for parser simulation */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "data", "test", "asan", "hwasan"
};
static const int token_count = 7;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    printf("ASAN test constructor initialized\n");
    /* Force initialization of memory areas */
    volatile char init_buf[128];
    __builtin_memset(init_buf, 0xA5, sizeof(init_buf));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    printf("ASAN test destructor cleaning up\n");
}

/* Recursive parser with goto control flow */
static ASTNode* parse_expression(int depth, int* index) {
    if (depth >= 3 || *index >= token_count) {
        return NULL;
    }
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data with memcpy */
    const char* token = tokens[(*index)++];
    size_t len = strlen(token);
    if (len > sizeof(node->data) - 1) {
        len = sizeof(node->data) - 1;
    }
    
    /* Complex control flow with goto */
    if (len > 0) {
        copy_data:
        __builtin_memcpy(node->data, token, len);
        node->data[len] = '\0';
        
        /* Jump to hash calculation */
        goto calculate_hash;
    } else {
        __builtin_memset(node->data, 'X', sizeof(node->data) - 1);
        node->data[sizeof(node->data) - 1] = '\0';
    }
    
    calculate_hash:
    /* Compute simple hash */
    uint32_t hash = 0;
    for (int i = 0; i < len; i++) {
        hash = (hash * 31) + node->data[i];
    }
    node->hash = hash;
    
    /* Recursive parsing with goto */
    if (depth < 2) {
        node->left = parse_expression(depth + 1, index);
        
        /* Jump over right child creation if index exhausted */
        if (*index >= token_count) {
            goto skip_right;
        }
        
        node->right = parse_expression(depth + 1, index);
        skip_right:;
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Memory operation dispatcher with OpenMP */
static void dispatch_memory_operations(ASTNode* nodes[], int count) {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i] && nodes[(i + 1) % count]) {
                volatile char temp_buf[128];
                
                /* Use all three builtins in different contexts */
                __builtin_memset(temp_buf, i, sizeof(temp_buf));
                
                /* Copy between AST nodes */
                __builtin_memcpy(nodes[i]->data, 
                               nodes[(i + 1) % count]->data,
                               sizeof(nodes[i]->data));
                
                /* Move data around with memmove (overlapping regions) */
                if (i % 2 == 0) {
                    __builtin_memmove(nodes[i]->data + 10,
                                     nodes[i]->data,
                                     sizeof(nodes[i]->data) - 10);
                }
                
                /* Force volatile access */
                g_mem_size = sizeof(temp_buf);
            }
        }
    }
}

/* Tree traversal with memory operations */
static uint32_t traverse_and_hash(ASTNode* root) {
    if (!root) return 0;
    
    uint32_t total_hash = root->hash;
    char buffer[128];
    
    /* Use memcpy in conditional blocks */
    if (root->left) {
        __builtin_memcpy(buffer, root->data, sizeof(root->data));
        total_hash ^= traverse_and_hash(root->left);
        
        /* Use memmove for overlapping copy */
        __builtin_memmove(buffer + 32, buffer, 64);
    }
    
    /* Jump label for control flow testing */
    process_right:
    if (root->right) {
        /* Use memset before copying */
        __builtin_memset(buffer, 0, sizeof(buffer));
        __builtin_memcpy(buffer, root->right->data, sizeof(root->right->data));
        total_hash ^= traverse_and_hash(root->right);
    }
    
    return total_hash;
}

/* Free tree with volatile operations */
static void free_tree(ASTNode* root) {
    if (!root) return;
    
    /* Use goto for non-linear control flow */
    if (root->left) {
        free_tree(root->left);
        goto free_right;
    }
    
    free_right:
    if (root->right) {
        free_tree(root->right);
    }
    
    /* Clear memory before free */
    volatile char* data = (volatile char*)root;
    for (size_t i = 0; i < sizeof(ASTNode); i += 64) {
        size_t len = sizeof(ASTNode) - i;
        if (len > 64) len = 64;
        __builtin_memset((char*)data + i, 0, len);
    }
    
    free(root);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Parse tokens into AST */
    int index = 0;
    ASTNode* root = parse_expression(0, &index);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    ASTNode* node_array[10];
    node_array[0] = root;
    
    /* Build additional nodes */
    for (int i = 1; i < 10; i++) {
        node_array[i] = malloc(sizeof(ASTNode));
        if (node_array[i]) {
            __builtin_memset(node_array[i], 0, sizeof(ASTNode));
            __builtin_memcpy(node_array[i]->data, 
                           tokens[i % token_count],
                           strlen(tokens[i % token_count]));
        }
    }
    
    /* Execute parallel memory operations */
    dispatch_memory_operations(node_array, 10);
    
    /* Traverse and compute final hash */
    uint32_t final_hash = traverse_and_hash(root);
    
    /* Additional memory operations in main */
    volatile char main_buf[512];
    __builtin_memset(main_buf, 0xCC, sizeof(main_buf));
    
    /* Test overlapping memmove */
    __builtin_memmove(main_buf + 100, main_buf, 200);
    
    /* Final memcpy with volatile size */
    size_t copy_size = g_mem_size;
    if (copy_size > sizeof(main_buf)) {
        copy_size = sizeof(main_buf);
    }
    __builtin_memcpy(main_buf + 300, main_buf, copy_size);
    
    printf("Final hash: 0x%08X\n", final_hash);
    printf("Memory operations completed\n");
    
    /* Cleanup */
    for (int i = 1; i < 10; i++) {
        if (node_array[i]) {
            free(node_array[i]);
        }
    }
    free_tree(root);
    
    return 0;
}
