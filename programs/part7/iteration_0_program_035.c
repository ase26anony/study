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
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "test", "data"};
static const int token_count = 5;

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
}

__attribute__((destructor)) static void cleanup_globals(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive function using builtins */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    node->left = create_ast(depth - 1, counter);
    node->right = create_ast(depth - 1, counter);
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill data with pattern */
    snprintf(node->data, sizeof(node->data), 
             "Node%d_%s", node->id, tokens[node->id % token_count]);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst, size_t size) {
    int use_copy = 1;
    
    if (src == NULL || dst == NULL) {
        use_copy = 0;
        goto skip_copy;
    }
    
copy_block:
    /* Force __builtin_memmove usage */
    if (g_use_memmove) {
        __builtin_memmove(dst->data, src->data, 
                         size < sizeof(dst->data) ? size : sizeof(dst->data));
    }
    
skip_copy:
    if (use_copy && dst != NULL) {
        /* Modify after copy */
        __builtin_memset(dst->data + 10, 'X', 5);
        goto finalize;
    }
    
finalize:
    return;
}

/* Parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count - 1; i++) {
        volatile size_t local_size = g_mem_size % 128;
        
        /* Mix of builtins in parallel region */
        if (i % 3 == 0) {
            __builtin_memcpy(nodes[i+1]->data, nodes[i]->data, local_size);
        } else if (i % 3 == 1) {
            __builtin_memset(nodes[i]->data + 20, i, local_size / 2);
        } else {
            /* Create overlapping regions for memmove */
            char temp[256];
            __builtin_memcpy(temp, nodes[i]->data, sizeof(temp));
            __builtin_memmove(nodes[i]->data + 10, nodes[i]->data, 50);
            __builtin_memcpy(nodes[i]->data, temp + 10, 50);
        }
    }
}

/* Compute verification hash */
static unsigned long compute_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* str = node->data;
    
    while (*str) {
        hash = ((hash << 5) + hash) + *str++;
    }
    
    hash += compute_hash(node->left);
    hash += compute_hash(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN builtin redirection test\n");
    
    /* Create AST structure */
    int counter = 0;
    ASTNode* root = create_ast(4, &counter);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    printf("Created AST with %d nodes\n", counter);
    
    /* Array of nodes for parallel processing */
    ASTNode* node_array[100];
    int array_count = 0;
    
    /* Collect nodes into array */
    void collect_nodes(ASTNode* node) {
        if (!node || array_count >= 100) return;
        node_array[array_count++] = node;
        collect_nodes(node->left);
        collect_nodes(node->right);
    }
    collect_nodes(root);
    
    /* Test goto flow with memmove */
    if (array_count >= 2) {
        process_with_goto(node_array[0], node_array[1], 32);
        
        /* Jump back to copy block */
        g_use_memmove = 0;
        process_with_goto(node_array[1], node_array[2], 16);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, array_count);
    
    /* Additional builtin calls in main */
    char buffer1[256], buffer2[256];
    
    __builtin_memset(buffer1, 'A', sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer2));
    
    /* Overlapping memmove */
    __builtin_memmove(buffer1 + 50, buffer1 + 30, 100);
    
    /* Complex pattern with volatile size */
    volatile size_t dynamic_size = g_mem_size % 64;
    for (int i = 0; i < 10; i++) {
        __builtin_memset(buffer2 + i * 10, '0' + i, dynamic_size + i);
    }
    
    /* Compute and print verification result */
    unsigned long total_hash = compute_hash(root);
    
    /* Mix in buffer hashes */
    for (size_t i = 0; i < sizeof(buffer1); i++) {
        total_hash = ((total_hash << 3) + total_hash) + buffer1[i];
    }
    
    printf("Verification hash: %lu\n", total_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    void free_ast(ASTNode* node) {
        if (!node) return;
        free_ast(node->left);
        free_ast(node->right);
        free(node);
    }
    free_ast(root);
    
    return 0;
}
