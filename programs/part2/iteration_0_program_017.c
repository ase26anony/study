/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
};

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "asan", "hwasan"};
static const int num_tokens = sizeof(tokens)/sizeof(tokens[0]);

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_asan_env(void) {
    /* Force early initialization */
    volatile char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
}

__attribute__((destructor)) static void cleanup_asan_env(void) {
    /* Ensure destructor path is taken */
    volatile char dummy[8];
    __builtin_memset(dummy, 0xFF, sizeof(dummy));
}

/* Recursive parser with memory operations */
static struct ASTNode* parse_expression(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    struct ASTNode* node = (struct ASTNode*)malloc(sizeof(struct ASTNode));
    if (!node) return NULL;
    
    /* Initialize with volatile size */
    __builtin_memset(node->data, 0, sizeof(node->data));
    node->id = depth;
    
    /* Build string using memcpy */
    char buffer[256];
    int token_idx = depth % num_tokens;
    size_t len = strlen(tokens[token_idx]);
    
    /* Force memcpy with non-constant size */
    __builtin_memcpy(buffer, tokens[token_idx], len);
    buffer[len] = '\0';
    
    /* Copy to node data */
    __builtin_memcpy(node->data, buffer, len + 1);
    
    /* Recursive calls */
    node->left = parse_expression(depth + 1, max_depth);
    node->right = parse_expression(depth + 2, max_depth);
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(struct ASTNode* src, struct ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_memmove = g_use_memmove;
    
    /* Jump into memory operation block */
    goto entry_point;
    
memmove_block:
    /* This tests flow sensitivity */
    __builtin_memmove(dst->data, src->data, g_mem_size % 256);
    goto after_copy;
    
entry_point:
    if (use_memmove) {
        goto memmove_block;
    } else {
        /* Regular memcpy */
        __builtin_memcpy(dst->data, src->data, g_mem_size % 256);
    }
    
after_copy:
    /* Jump out and back in */
    if (dst->id % 2 == 0) {
        goto verify_block;
    }
    
    /* Additional memset */
    __builtin_memset(dst->data + 128, 0xAA, 32);
    
verify_block:
    /* Verify copy with another memcpy to volatile */
    volatile char verify_buf[256];
    __builtin_memcpy((char*)verify_buf, dst->data, 64);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(struct ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count - 1; i++) {
            /* Inter-thread memory operations */
            if (nodes[i] && nodes[i+1]) {
                /* Mixed built-in usage */
                __builtin_memset(nodes[i]->data, tid, 64);
                
                /* Conditional memcpy/memmove */
                if (i % 3 == 0) {
                    __builtin_memcpy(nodes[i+1]->data, nodes[i]->data, 64);
                } else if (i % 3 == 1) {
                    __builtin_memmove(nodes[i+1]->data, nodes[i]->data, 64);
                }
            }
        }
        
        /* Thread-private memset */
        char private_buf[128];
        __builtin_memset(private_buf, tid, sizeof(private_buf));
    }
}

/* Free AST recursively */
static void free_ast(struct ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    const int num_nodes = 8;
    struct ASTNode* nodes[num_nodes];
    unsigned long hash = 0;
    
    /* Initialize AST */
    for (int i = 0; i < num_nodes; i++) {
        nodes[i] = parse_expression(i, 4);
    }
    
    /* Test goto edge cases */
    for (int i = 0; i < num_nodes - 1; i++) {
        process_with_goto(nodes[i], nodes[i+1]);
    }
    
    /* Force asan_memfn_rtls initialization with all built-ins */
    volatile char src[256], dst[256];
    
    /* Explicit calls to all three built-ins */
    __builtin_memset(src, 0xCC, sizeof(src));
    __builtin_memcpy(dst, src, sizeof(src));
    __builtin_memmove(src + 128, src, 128);
    
    /* OpenMP parallel section */
    parallel_memory_ops(nodes, num_nodes);
    
    /* Compute verification hash */
    for (int i = 0; i < num_nodes; i++) {
        if (nodes[i]) {
            for (int j = 0; j < 64; j++) {
                hash = (hash * 31 + nodes[i]->data[j]) % 1000000007;
            }
        }
    }
    
    /* Additional memory operations in different scopes */
    {
        char local_buf[512];
        __builtin_memset(local_buf, 0xDD, sizeof(local_buf));
        __builtin_memcpy(local_buf + 256, local_buf, 256);
        
        /* Nested memmove with goto */
        int k = 0;
        goto nested_move;
        
    nested_move:
        __builtin_memmove(local_buf + 128, local_buf + 64, 128);
        k++;
        
        if (k < 2) {
            goto nested_move;
        }
    }
    
    printf("Result hash: %lu\n", hash);
    
    /* Cleanup */
    for (int i = 0; i < num_nodes; i++) {
        free_ast(nodes[i]);
    }
    
    return 0;
}
