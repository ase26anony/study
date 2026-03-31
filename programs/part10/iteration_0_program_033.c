/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) static void init_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor)) static void cleanup_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(const char *token, int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->len = g_mem_size;
    node->data = (char*)malloc(node->len);
    if (!node->data) {
        free(node);
        return NULL;
    }
    
    /* Use __builtin_memcpy with volatile size */
    volatile size_t copy_len = node->len < strlen(token) ? node->len : strlen(token);
    __builtin_memcpy(node->data, token, copy_len);
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        create_left = 0;
        goto skip_left;
    }
    
    node->left = create_ast("left", depth - 1);
    
skip_left:
    node->right = create_ast("right", depth - 1);
    
    return node;
}

/* Function with goto jumping into memmove block */
static void process_with_goto(ASTNode *src, ASTNode *dst) {
    int use_memmove = g_use_memmove;
    
    if (!src || !dst || !src->data || !dst->data) return;
    
    if (use_memmove) {
        goto do_memmove;
    } else {
        /* Use __builtin_memcpy */
        __builtin_memcpy(dst->data, src->data, 
                        src->len < dst->len ? src->len : dst->len);
        return;
    }
    
do_memmove:
    /* This goto target contains __builtin_memmove */
    __builtin_memmove(dst->data, src->data, 
                     src->len < dst->len ? src->len : dst->len);
    
    /* Jump back out */
    goto finish;
    
finish:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode **nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count - 1; i++) {
            if (nodes[i] && nodes[i+1]) {
                /* Mix of memory operations */
                if (i % 3 == 0) {
                    __builtin_memcpy(nodes[i+1]->data, nodes[i]->data,
                                    nodes[i]->len < nodes[i+1]->len ? 
                                    nodes[i]->len : nodes[i+1]->len);
                } else if (i % 3 == 1) {
                    __builtin_memset(nodes[i]->data, tid, 
                                    nodes[i]->len > 32 ? 32 : nodes[i]->len);
                } else {
                    __builtin_memmove(nodes[i+1]->data, nodes[i]->data,
                                     nodes[i]->len < nodes[i+1]->len ? 
                                     nodes[i]->len : nodes[i+1]->len);
                }
            }
        }
    }
}

/* Compute hash from AST data */
static unsigned long compute_ast_hash(ASTNode *node) {
    if (!node || !node->data) return 0;
    
    unsigned long hash = 5381;
    for (size_t i = 0; i < node->len && i < 256; i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    if (node->left) hash ^= compute_ast_hash(node->left);
    if (node->right) hash ^= compute_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create token array */
    const char *tokens[] = {"func", "var", "expr", "stmt", "decl"};
    const int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Create AST nodes */
    ASTNode *nodes[10];
    int node_count = 0;
    
    for (int i = 0; i < 5 && i < token_count; i++) {
        nodes[node_count] = create_ast(tokens[i], 3);
        if (nodes[node_count]) node_count++;
    }
    
    if (node_count < 2) {
        fprintf(stderr, "Failed to create enough AST nodes\n");
        return 1;
    }
    
    /* Test goto with memmove */
    printf("Testing goto flow control with memmove...\n");
    process_with_goto(nodes[0], nodes[1]);
    
    /* Force initialization of all three builtins */
    volatile char buffer1[128];
    volatile char buffer2[128];
    
    /* Ensure all three builtins are called */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1, buffer2, sizeof(buffer1));
    
    /* Parallel operations */
    printf("Running parallel memory operations...\n");
    parallel_memory_ops(nodes, node_count);
    
    /* Compute verification hash */
    unsigned long total_hash = 0;
    for (int i = 0; i < node_count; i++) {
        if (nodes[i]) {
            total_hash += compute_ast_hash(nodes[i]);
        }
    }
    
    printf("Verification hash: %lu\n", total_hash);
    
    /* Cleanup */
    for (int i = 0; i < node_count; i++) {
        if (nodes[i]) {
            if (nodes[i]->data) free(nodes[i]->data);
            free(nodes[i]);
        }
    }
    
    printf("Test completed successfully\n");
    return 0;
}
