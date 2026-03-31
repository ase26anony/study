#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Complex AST-like structure for data structure access */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* volatile to prevent optimization */
    struct ASTNode* left;
    struct ASTNode* right;
    char buffer[256];
} ASTNode;

/* Global volatile variables for non-foldable constants */
volatile size_t g_memcpy_len = 128;
volatile size_t g_memset_len = 64;
volatile size_t g_memmove_len = 96;

/* Token array for parser simulation */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "data", "test", "asan", "hwasan"
};
#define TOKEN_COUNT (sizeof(tokens)/sizeof(tokens[0]))

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_hooks(void) {
    volatile char init_buf[32];
    
    /* Force initialization of builtin redirection */
    __builtin_memset(init_buf, 0, sizeof(init_buf));
    __builtin_memcpy(init_buf, "init", 5);
    __builtin_memmove(init_buf + 1, init_buf, 4);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_sanitizer_hooks(void) {
    volatile char cleanup_buf[16];
    
    /* More builtin calls in destructor */
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    __builtin_memcpy(cleanup_buf, "clean", 6);
}

/* Recursive parser with goto statements for control flow */
static int parse_tokens_recursive(const char** token_list, int index, int depth, ASTNode* node) {
    if (index >= TOKEN_COUNT || depth >= 3) {
        return 0;
    }
    
    int processed = 0;
    
    /* Goto label for jumping into memory operation block */
    process_token:
    if (token_list[index] != NULL) {
        size_t len = strlen(token_list[index]);
        
        /* Use volatile to prevent optimization */
        volatile size_t copy_len = len < 255 ? len : 255;
        
        /* Builtin memcpy with goto edge case */
        __builtin_memcpy(node->buffer, token_list[index], copy_len);
        node->buffer[copy_len] = '\0';
        processed++;
        
        /* Jump to different processing based on token */
        if (strcmp(token_list[index], "memmove") == 0) {
            goto handle_memmove;
        }
    }
    
    /* Normal recursion */
    if (node->left != NULL) {
        processed += parse_tokens_recursive(token_list, index + 1, depth + 1, node->left);
    }
    
    if (node->right != NULL) {
        processed += parse_tokens_recursive(token_list, index + 2, depth + 1, node->right);
    }
    
    return processed;
    
    /* Goto target for memmove-specific handling */
    handle_memmove:
    {
        volatile char temp_buf[256];
        
        /* Builtin memmove with overlapping regions */
        __builtin_memcpy(temp_buf, node->buffer, g_memmove_len);
        __builtin_memmove(node->buffer + 10, node->buffer, g_memmove_len - 10);
        __builtin_memmove(node->buffer, temp_buf, g_memmove_len);
        
        /* Jump back to normal flow */
        goto process_token;
    }
}

/* Function with OpenMP parallel region */
static void parallel_memory_operations(ASTNode** nodes, int node_count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, node_count)
    for (i = 0; i < node_count; i++) {
        volatile char local_buf[512];
        ASTNode* current = nodes[i];
        
        if (current != NULL) {
            /* Stress builtin calls in parallel context */
            __builtin_memset(local_buf, i, sizeof(local_buf));
            
            /* Use all three builtins with volatile lengths */
            __builtin_memcpy(current->buffer, local_buf, g_memcpy_len);
            __builtin_memset(current->buffer + g_memcpy_len, 0, g_memset_len);
            
            /* Create overlapping region for memmove */
            if (i > 0 && nodes[i-1] != NULL) {
                __builtin_memmove(current->buffer, nodes[i-1]->buffer, g_memmove_len);
            }
            
            /* Additional builtin calls with different patterns */
            __builtin_memcpy(local_buf + 128, current->buffer, 64);
            __builtin_memset(current->buffer + 192, 0xFF, 32);
        }
    }
}

/* Create AST tree */
static ASTNode* create_ast_tree(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (node == NULL) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    node->value = depth * 10;
    node->size = (size_t)(depth * 50);
    
    /* Create pattern in buffer */
    volatile char pattern[32];
    __builtin_memset(pattern, 'A' + depth, sizeof(pattern));
    __builtin_memcpy(node->buffer, pattern, sizeof(pattern) < 256 ? sizeof(pattern) : 255);
    
    /* Recursive children */
    node->left = create_ast_tree(depth - 1);
    node->right = create_ast_tree(depth - 2);
    
    return node;
}

/* Free AST tree */
static void free_ast_tree(ASTNode* node) {
    if (node == NULL) return;
    
    free_ast_tree(node->left);
    free_ast_tree(node->right);
    
    /* Clear memory before free */
    volatile char clear_buf[sizeof(ASTNode)];
    __builtin_memcpy(clear_buf, node, sizeof(ASTNode));
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    free(node);
}

/* Compute hash of AST tree */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (node == NULL) return 0;
    
    unsigned long hash = 5381;
    int i;
    
    /* Hash buffer contents */
    for (i = 0; i < 256 && node->buffer[i] != '\0'; i++) {
        hash = ((hash << 5) + hash) + node->buffer[i];
    }
    
    /* Hash structure fields */
    hash = ((hash << 5) + hash) + node->type;
    hash = ((hash << 5) + hash) + node->value;
    hash = ((hash << 5) + hash) + (unsigned long)node->size;
    
    /* Recursive hash */
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    ASTNode* root = NULL;
    ASTNode* node_array[8] = {0};
    int i;
    unsigned long final_hash = 0;
    
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Create complex AST structure */
    root = create_ast_tree(5);
    if (root == NULL) {
        fprintf(stderr, "Failed to create AST tree\n");
        return 1;
    }
    
    /* Fill node array for parallel processing */
    for (i = 0; i < 8; i++) {
        node_array[i] = create_ast_tree(3 + (i % 3));
    }
    
    /* Phase 1: Recursive parsing with goto control flow */
    int processed = parse_tokens_recursive(tokens, 0, 0, root);
    printf("Processed %d tokens recursively\n", processed);
    
    /* Phase 2: OpenMP parallel memory operations */
    parallel_memory_operations(node_array, 8);
    printf("Completed parallel memory operations\n");
    
    /* Phase 3: Additional builtin calls in main */
    volatile char main_buf[1024];
    
    __builtin_memset(main_buf, 0, sizeof(main_buf));
    __builtin_memcpy(main_buf, "Main buffer data", 17);
    
    /* Overlapping memmove */
    __builtin_memmove(main_buf + 100, main_buf, 200);
    __builtin_memmove(main_buf, main_buf + 50, 150);
    
    /* Copy to AST nodes */
    for (i = 0; i < 4 && i < 8; i++) {
        if (node_array[i] != NULL) {
            __builtin_memcpy(node_array[i]->buffer + 128, main_buf + i * 32, 64);
        }
    }
    
    /* Compute verification hash */
    final_hash = compute_ast_hash(root);
    for (i = 0; i < 8; i++) {
        if (node_array[i] != NULL) {
            final_hash ^= compute_ast_hash(node_array[i]);
        }
    }
    
    printf("Final verification hash: 0x%08lx\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free_ast_tree(root);
    for (i = 0; i < 8; i++) {
        free_ast_tree(node_array[i]);
    }
    
    return 0;
}
