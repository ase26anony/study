#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* AST-like recursive structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    uint32_t hash;
} ASTNode;

/* Volatile variables to prevent optimization */
volatile size_t mem_size = 64;
volatile int use_memmove = 1;

/* Token array for parser simulation */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "data", "hash", "node",
    "left", "right", "ptr", "buffer", "asan", "hwasan"
};
static const int token_count = sizeof(tokens) / sizeof(tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_hooks(void) {
    volatile char init_buf[32];
    /* Force builtin usage in constructor */
    __builtin_memset(init_buf, 0xAA, sizeof(init_buf));
    __builtin_memcpy(init_buf + 16, init_buf, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_hooks(void) {
    volatile char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token into node data */
    const char* token = tokens[depth % token_count];
    size_t len = __builtin_strlen(token);
    __builtin_memcpy(node->data, token, len < 63 ? len : 63);
    
    /* Create children recursively */
    node->left = create_ast(depth + 1, max_depth);
    node->right = create_ast(depth + 2, max_depth);
    
    return node;
}

/* Complex memory operation with goto flow control */
static void process_ast_with_goto(ASTNode* node) {
    if (!node) return;
    
    volatile int use_copy = 1;
    char temp_buf[64];
    
    /* Jump into memory operation block */
    goto start_block;
    
copy_block:
    /* This block contains the critical builtin calls */
    if (use_copy) {
        __builtin_memcpy(temp_buf, node->data, mem_size % 64);
    } else {
        __builtin_memset(temp_buf, 0xCC, mem_size % 64);
    }
    goto after_block;
    
start_block:
    /* Control flow with goto into memory ops */
    if (node->left) {
        use_copy = 0;
        goto copy_block;
    }
    
after_block:
    /* Memmove with potential overlap */
    if (use_memmove && node->right) {
        __builtin_memmove(node->data, temp_buf, 32);
    }
    
    /* Process children */
    process_ast_with_goto(node->left);
    process_ast_with_goto(node->right);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            char buffer[128];
            volatile int iter = i % 3;
            
            switch (iter) {
                case 0:
                    __builtin_memcpy(buffer, nodes[i]->data, 64);
                    __builtin_memcpy(nodes[i]->data, buffer, 64);
                    break;
                case 1:
                    __builtin_memset(buffer, i, 128);
                    __builtin_memcpy(nodes[i]->data, buffer, 64);
                    break;
                case 2:
                    __builtin_memmove(buffer, buffer + 32, 64);
                    __builtin_memcpy(nodes[i]->data, buffer, 32);
                    break;
            }
            
            /* Compute hash */
            uint32_t hash = 0;
            for (int j = 0; j < 64; j++) {
                hash = (hash * 31) + nodes[i]->data[j];
            }
            nodes[i]->hash = hash;
        }
    }
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    volatile char* ptr = (char*)node;
    __builtin_memset(ptr, 0, sizeof(ASTNode));
    free(node);
}

int main(void) {
    const int ast_depth = 4;
    const int node_count = 8;
    
    printf("Starting ASAN memory operation test...\n");
    
    /* Create multiple ASTs */
    ASTNode* nodes[node_count];
    for (int i = 0; i < node_count; i++) {
        nodes[i] = create_ast(0, ast_depth);
    }
    
    /* Process with goto flow control */
    for (int i = 0; i < node_count; i++) {
        process_ast_with_goto(nodes[i]);
    }
    
    /* Parallel memory operations */
    parallel_memory_ops(nodes, node_count);
    
    /* Verify and print results */
    uint64_t total_hash = 0;
    for (int i = 0; i < node_count; i++) {
        if (nodes[i]) {
            total_hash += nodes[i]->hash;
            printf("Node %d hash: %08x\n", i, nodes[i]->hash);
        }
    }
    
    printf("Total hash sum: %016llx\n", (unsigned long long)total_hash);
    
    /* Cleanup */
    for (int i = 0; i < node_count; i++) {
        free_ast(nodes[i]);
    }
    
    /* Final builtin calls */
    volatile char final_buf[256];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf + 128, final_buf, 128);
    __builtin_memmove(final_buf, final_buf + 64, 128);
    
    printf("Test completed successfully.\n");
    return 0;
}
