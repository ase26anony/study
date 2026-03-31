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
    unsigned char redzone[32]; /* Simulate ASAN redzone */
} ASTNode;

/* Global token array */
static const char *tokens[] = {"memcpy", "memset", "memmove", "test", "data"};
static const size_t token_count = sizeof(tokens)/sizeof(tokens[0]);

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_global(void) {
    g_init_flag = 1;
    fprintf(stderr, "Constructor: Global initialized\n");
}

__attribute__((destructor)) static void cleanup_global(void) {
    fprintf(stderr, "Destructor: Cleaning up\n");
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth, const char **token_ptr) {
    if (depth <= 0 || **token_ptr == '\0') return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->len = g_mem_size % 128 + 64;
    node->data = malloc(node->len);
    
    if (node->data) {
        /* Use __builtin_memcpy with volatile size */
        volatile size_t copy_len = node->len - 8;
        __builtin_memcpy(node->data, *token_ptr, copy_len > 0 ? copy_len : 8);
        
        /* Jump label for goto testing */
        memcpy_label:
        if (depth > 1) {
            /* Use __builtin_memmove with goto */
            char temp[128];
            __builtin_memmove(temp, node->data, node->len % 64);
            __builtin_memcpy(node->data, temp, node->len % 64);
        }
    }
    
    /* Recursive calls */
    (*token_ptr)++;
    node->left = parse_expression(depth - 1, token_ptr);
    node->right = parse_expression(depth - 1, token_ptr);
    
    return node;
}

/* Parallel memory dispatch */
static void parallel_mem_operations(ASTNode *root) {
    if (!root) return;
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            char buffer[512];
            volatile size_t op_size = g_mem_size % 256 + 64;
            
            switch (i % 3) {
                case 0:
                    __builtin_memset(buffer + tid * 64, tid, op_size);
                    break;
                case 1:
                    if (root->data && root->len > 0) {
                        __builtin_memcpy(buffer, root->data, 
                                       op_size < root->len ? op_size : root->len);
                    }
                    break;
                case 2:
                    __builtin_memmove(buffer + 128, buffer, op_size % 128);
                    break;
            }
            
            /* Goto to test flow sensitivity */
            if (tid == 0 && i == 2) {
                goto parallel_mem_label;
            }
        }
        
        parallel_mem_label:
        #pragma omp barrier
    }
}

/* Tree traversal with memory operations */
static size_t compute_tree_hash(ASTNode *node) {
    if (!node) return 0;
    
    size_t hash = 0;
    char temp_buf[256];
    
    if (node->data && node->len > 0) {
        /* Use all three builtins in sequence */
        __builtin_memset(temp_buf, 0, sizeof(temp_buf));
        __builtin_memcpy(temp_buf, node->data, 
                        node->len < sizeof(temp_buf) ? node->len : sizeof(temp_buf));
        __builtin_memmove(temp_buf + 128, temp_buf, 64);
        
        for (size_t i = 0; i < node->len && i < sizeof(temp_buf); i++) {
            hash = hash * 31 + temp_buf[i];
        }
    }
    
    /* Goto into block with memmove */
    if (hash % 2 == 0) {
        goto hash_calc_label;
    }
    
    return hash + compute_tree_hash(node->left) + compute_tree_hash(node->right);
    
    hash_calc_label:
    {
        char local_buf[64];
        __builtin_memmove(local_buf, temp_buf, 32);
        return hash + compute_tree_hash(node->left);
    }
}

/* Main execution flow */
int main(void) {
    const char *token_ptr = tokens[0];
    size_t final_hash = 0;
    
    /* Create recursive structure */
    ASTNode *root = parse_expression(3, &token_ptr);
    
    /* Force initialization of asan_memfn_rtls cache */
    char init_buf[1024];
    volatile size_t sizes[] = {32, 64, 128};
    
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                __builtin_memset(init_buf, 0xAA, sizes[i % 3]);
                break;
            case 1:
                __builtin_memcpy(init_buf + 256, init_buf, sizes[i % 3]);
                break;
            case 2:
                __builtin_memmove(init_buf + 512, init_buf + 256, sizes[i % 3]);
                break;
        }
    }
    
    /* Execute parallel operations */
    parallel_mem_operations(root);
    
    /* Compute verification hash */
    final_hash = compute_tree_hash(root);
    
    /* Cleanup */
    #pragma omp parallel
    {
        char thread_buf[64];
        __builtin_memset(thread_buf, omp_get_thread_num(), sizeof(thread_buf));
    }
    
    printf("Result hash: %zu\n", final_hash);
    printf("Verification: %s\n", final_hash != 0 ? "PASS" : "FAIL");
    
    /* Free resources */
    free(root->data);
    free(root);
    
    return final_hash != 0 ? 0 : 1;
}
