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
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
    unsigned long hash;
} ASTNode;

/* Global token array */
static const char *tokens[] = {
    "memcpy_test", "memset_test", "memmove_test",
    "asan_redirect", "hwasan_check", "parallel_exec"
};
static const int token_count = sizeof(tokens)/sizeof(tokens[0]);

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_globals(void) {
    g_init_flag = 1;
    printf("Constructor: Global init flag set\n");
}

__attribute__((destructor)) static void cleanup(void) {
    printf("Destructor: Program cleanup\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_node(const char *src, size_t len) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->len = len;
    node->data = malloc(len + 1);
    node->left = node->right = NULL;
    
    /* Force __builtin_memcpy usage with volatile length */
    volatile size_t copy_len = len;
    if (node->data) {
        __builtin_memcpy(node->data, src, copy_len);
        node->data[len] = '\0';
    }
    
    /* Initialize with __builtin_memset */
    volatile int init_val = 0;
    node->hash = 0;
    __builtin_memset(&node->hash, init_val, sizeof(node->hash));
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode *dest, ASTNode *src) {
    int use_memmove = 1;
    
    if (dest && src && dest->data && src->data) {
        volatile size_t move_len = dest->len < src->len ? dest->len : src->len;
        
    jump_point:
        if (use_memmove) {
            /* This should trigger the memmove redirection */
            __builtin_memmove(dest->data, src->data, move_len);
            use_memmove = 0;
            goto jump_point;
        }
    }
}

/* Parallel memory dispatch with OpenMP */
static void parallel_memory_ops(ASTNode **nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i] && nodes[i]->data) {
                /* Volatile control for memset length */
                volatile size_t op_len = g_mem_size % (nodes[i]->len + 1);
                
                /* Force all three builtins in parallel context */
                char temp[512];
                __builtin_memset(temp, tid, op_len);
                __builtin_memcpy(temp + op_len, nodes[i]->data, op_len);
                
                /* Conditional memmove with goto */
                if (i > 0 && nodes[i-1]) {
                    volatile int do_move = (tid % 2 == 0);
                    if (do_move) {
                        __builtin_memmove(nodes[i]->data, temp, op_len);
                    }
                }
                
                /* Compute hash */
                for (size_t j = 0; j < op_len; j++) {
                    nodes[i]->hash = nodes[i]->hash * 31 + temp[j];
                }
            }
        }
    }
}

/* Recursive tree processing */
static unsigned long process_tree(ASTNode *root, int depth) {
    if (!root || depth <= 0) return 0;
    
    unsigned long hash = root->hash;
    
    /* Process children with memory operations */
    if (root->left && root->right) {
        volatile size_t swap_len = root->left->len < root->right->len ? 
                                  root->left->len : root->right->len;
        
        /* Use temp buffer for memcpy */
        char temp[1024];
        __builtin_memcpy(temp, root->left->data, swap_len);
        __builtin_memcpy(root->left->data, root->right->data, swap_len);
        __builtin_memcpy(root->right->data, temp, swap_len);
        
        /* Clear temp with memset */
        __builtin_memset(temp, 0, swap_len);
    }
    
    /* Recursive calls */
    hash += process_tree(root->left, depth - 1);
    hash += process_tree(root->right, depth - 1);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create AST nodes from tokens */
    ASTNode *nodes[token_count];
    for (int i = 0; i < token_count; i++) {
        nodes[i] = create_node(tokens[i], strlen(tokens[i]));
    }
    
    /* Test goto with memmove */
    if (token_count >= 2) {
        process_with_goto(nodes[0], nodes[1]);
    }
    
    /* Parallel memory operations */
    parallel_memory_ops(nodes, token_count);
    
    /* Process tree structure */
    ASTNode *root = nodes[0];
    if (root && token_count >= 3) {
        root->left = nodes[1];
        root->right = nodes[2];
        
        if (token_count >= 5) {
            nodes[1]->left = nodes[3];
            nodes[1]->right = nodes[4];
        }
    }
    
    unsigned long total_hash = 0;
    if (root) {
        total_hash = process_tree(root, 3);
    }
    
    /* Final memory operations to ensure all builtins are used */
    char final_buffer[1024];
    volatile size_t final_size = g_mem_size % 512;
    
    __builtin_memset(final_buffer, 0xFF, final_size);
    __builtin_memcpy(final_buffer + 128, root ? root->data : "default", 64);
    __builtin_memmove(final_buffer, final_buffer + 64, 128);
    
    /* Compute final result */
    for (size_t i = 0; i < final_size && i < 256; i++) {
        total_hash = total_hash * 17 + final_buffer[i];
    }
    
    printf("Result hash: %lu\n", total_hash);
    
    /* Cleanup */
    for (int i = 0; i < token_count; i++) {
        if (nodes[i]) {
            free(nodes[i]->data);
            free(nodes[i]);
        }
    }
    
    return (int)(total_hash % 256);
}
