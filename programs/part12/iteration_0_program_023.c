/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Token array for parser simulation */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "data", "test", "asan", "hwasan"
};
static const int token_count = sizeof(tokens) / sizeof(tokens[0]);

/* Constructor function - forces early initialization */
__attribute__((constructor))
static void init_asan_early(void) {
    volatile char buffer1[128];
    volatile char buffer2[128];
    
    /* Force memcpy built-in in constructor */
    __builtin_memcpy((void*)buffer1, "constructor_init", 16);
    
    /* Use volatile to prevent dead code elimination */
    if (volatile_flag) {
        __builtin_memset((void*)buffer2, 0xAA, 32);
    }
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0 || *counter >= 100) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    node->left = NULL;
    node->right = NULL;
    
    /* Initialize node data with memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy token based on depth */
    int token_idx = depth % token_count;
    size_t len = strlen(tokens[token_idx]);
    if (len < sizeof(node->data)) {
        __builtin_memcpy(node->data, tokens[token_idx], len + 1);
    }
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    if (depth % 3 == 0) {
        goto skip_left;
    }
    
    create_left = 0;
    
skip_left:
    if (!create_left) {
        node->left = create_ast(depth - 1, counter);
    }
    
    /* Another goto for right subtree */
    if (depth % 4 == 0) {
        goto create_right;
    } else {
        goto skip_right;
    }
    
create_right:
    node->right = create_ast(depth - 2, counter);
    
skip_right:
    return node;
}

/* Function with goto jumping into memmove block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_memmove = volatile_flag & 1;
    
    if (use_memmove) {
        goto do_memmove;
    } else {
        goto do_memcpy;
    }
    
do_memmove:
    /* This block is entered via goto */
    __builtin_memmove(dst->data, src->data, volatile_len % 256);
    goto after_ops;
    
do_memcpy:
    __builtin_memcpy(dst->data, src->data, volatile_len % 256);
    goto after_ops;
    
after_ops:
    /* Clear part of destination */
    __builtin_memset(dst->data + 128, 0, 64);
}

/* Parallel processing with OpenMP */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Mix of memory operations in parallel region */
            char temp[256];
            
            __builtin_memcpy(temp, nodes[i]->data, 128);
            __builtin_memset(nodes[i]->data + 64, i, 32);
            __builtin_memmove(nodes[i]->data, temp, 128);
            
            /* Volatile-dependent operation */
            if (volatile_flag) {
                __builtin_memset(nodes[i]->data + 192, 0xFF, 32);
            }
        }
    }
}

/* Calculate hash from AST */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* p = node->data;
    
    /* Simple hash calculation */
    while (*p) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    hash += node->id;
    
    return hash;
}

/* Destructor for cleanup */
__attribute__((destructor))
static void cleanup_asan(void) {
    /* Final memory operation in destructor */
    volatile char final_buf[64];
    __builtin_memset((void*)final_buf, 0, sizeof(final_buf));
}

int main(void) {
    int counter = 0;
    unsigned long total_hash = 0;
    
    printf("Starting ASAN/HWASAN built-in redirection test...\n");
    
    /* Create complex AST */
    ASTNode* root = create_ast(7, &counter);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    printf("Created AST with %d nodes\n", counter);
    
    /* Process with goto flow control */
    if (root->left) {
        process_with_goto(root, root->left);
    }
    
    /* Create array for parallel processing */
    ASTNode* node_array[100];
    int array_count = 0;
    
    /* Flatten AST into array */
    ASTNode* stack[100];
    int stack_top = 0;
    stack[stack_top++] = root;
    
    while (stack_top > 0 && array_count < 100) {
        ASTNode* current = stack[--stack_top];
        node_array[array_count++] = current;
        
        if (current->right) stack[stack_top++] = current->right;
        if (current->left) stack[stack_top++] = current->left;
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, array_count);
    
    /* Compute verification hash */
    total_hash = compute_ast_hash(root);
    
    printf("Total hash: %lu\n", total_hash);
    printf("Test completed successfully\n");
    
    /* Final built-in calls in main */
    char main_buffer[256];
    __builtin_memset(main_buffer, 0, sizeof(main_buffer));
    __builtin_memcpy(main_buffer + 128, "test_complete", 13);
    __builtin_memmove(main_buffer, main_buffer + 128, 13);
    
    return 0;
}
