/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[64];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 32);
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char final_check[32];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->len = g_mem_size % 128 + 64;
    node->data = malloc(node->len);
    
    /* Use all three builtins with volatile control */
    __builtin_memset(node->data, depth, node->len);
    
    if (base_data) {
        size_t copy_len = node->len < strlen(base_data) ? node->len : strlen(base_data);
        __builtin_memcpy(node->data, base_data, copy_len);
    }
    
    /* Conditional memmove with goto for flow complexity */
    if (g_use_memmove && node->len > 32) {
        char *temp = malloc(node->len);
        if (temp) {
            goto do_memmove;
memmove_block:
            __builtin_memmove(temp, node->data, node->len);
            __builtin_memcpy(node->data, temp, node->len);
            free(temp);
            goto after_memmove;
        }
do_memmove:
        /* Jump into memmove block */
        goto memmove_block;
after_memmove:
        ; /* Empty statement for label */
    }
    
    node->left = create_ast(depth - 1, node->data);
    node->right = create_ast(depth - 1, node->data + node->len/2);
    
    return node;
}

/* Calculate hash from AST */
static size_t ast_hash(ASTNode *node) {
    if (!node) return 0;
    
    size_t hash = 0;
    for (size_t i = 0; i < node->len; i++) {
        hash = hash * 31 + node->data[i];
    }
    
    return hash + ast_hash(node->left) + ast_hash(node->right);
}

/* Free AST recursively */
static void free_ast(ASTNode *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node->data);
    free(node);
}

int main(void) {
    const char *test_data = "ASAN_TEST_BUILTINS_REDIRECTION";
    size_t total_hash = 0;
    
    /* Create AST with memory operations */
    ASTNode *root = create_ast(4, test_data);
    
    /* OpenMP parallel section with memory operations */
    #pragma omp parallel reduction(+:total_hash)
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers for builtin operations */
        char local_buf[256];
        char src_buf[256];
        
        /* Initialize with builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        __builtin_memset(src_buf, 0xCC, sizeof(src_buf));
        
        /* Mixed builtin usage pattern */
        for (int i = 0; i < 3; i++) {
            switch (i) {
                case 0:
                    __builtin_memcpy(local_buf + 64, src_buf, 128);
                    break;
                case 1:
                    __builtin_memset(local_buf + 128, 0xDD, 64);
                    break;
                case 2:
                    if (g_use_memmove) {
                        __builtin_memmove(local_buf, local_buf + 32, 96);
                    }
                    break;
            }
        }
        
        /* Process AST in parallel */
        #pragma omp for
        for (int i = 0; i < 8; i++) {
            char tmp[128];
            __builtin_memset(tmp, i, sizeof(tmp));
            
            if (root && root->data) {
                size_t copy_len = root->len < sizeof(tmp) ? root->len : sizeof(tmp);
                __builtin_memcpy(tmp, root->data, copy_len);
            }
            
            for (size_t j = 0; j < sizeof(tmp); j++) {
                total_hash += tmp[j] * (j + 1);
            }
        }
    }
    
    /* Additional serial builtin usage */
    if (root) {
        total_hash += ast_hash(root);
        
        /* Final overlapping memory operation */
        if (root->data && root->len > 64) {
            __builtin_memmove(root->data + 16, root->data, 48);
        }
    }
    
    /* Print verification result */
    printf("Total hash: %zu\n", total_hash);
    
    /* Cleanup */
    free_ast(root);
    
    return total_hash != 0 ? 0 : 1;
}
