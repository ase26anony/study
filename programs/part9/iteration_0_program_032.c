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
    int type;
    char data[32];
    struct ASTNode *left;
    struct ASTNode *right;
};

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force initialization of ASAN runtime before main */
    volatile char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    /* Final memory operation in destructor */
    volatile char final_buf[8];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static struct ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    struct ASTNode* node = malloc(sizeof(struct ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile size */
    __builtin_memset(node, 0, sizeof(*node));
    node->type = depth;
    
    /* Format data string */
    __builtin_snprintf(node->data, sizeof(node->data), 
                      "Node%d", depth);
    
    /* Create children */
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(struct ASTNode* src, struct ASTNode* dst) {
    int use_copy = 1;
    
    if (src == NULL || dst == NULL) {
        goto cleanup;
    }
    
copy_block:
    /* Jump target containing memmove */
    if (use_copy) {
        __builtin_memmove(dst->data, src->data, 
                         sizeof(src->data));
        use_copy = 0;
        goto verify_block;
    }
    
verify_block:
    /* Verify copy */
    if (__builtin_memcmp(dst->data, src->data, 
                        sizeof(src->data)) != 0) {
        goto copy_block;  /* Jump back */
    }
    
cleanup:
    /* Empty cleanup for goto structure */
    return;
}

/* OpenMP parallel memory operations */
static void parallel_mem_ops(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread allocates and initializes */
        buffers[tid] = malloc(g_mem_size);
        if (buffers[tid]) {
            /* Use all three builtins */
            __builtin_memset(buffers[tid], tid, g_mem_size);
            
            if (tid > 0) {
                __builtin_memcpy(buffers[tid], 
                               buffers[tid-1], 
                               g_mem_size / 2);
            }
            
            if (g_use_memmove) {
                __builtin_memmove(buffers[tid] + 10,
                                buffers[tid],
                                g_mem_size - 20);
            }
        }
        
        #pragma omp barrier
        
        /* Verify across threads */
        if (tid == 0) {
            for (int i = 1; i < num_threads; i++) {
                if (buffers[i]) {
                    __builtin_memcpy(buffers[0],
                                   buffers[i],
                                   g_mem_size / 4);
                }
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        free(buffers[i]);
    }
}

/* Complex token processing */
static unsigned long process_tokens(const char** tokens, int count) {
    unsigned long hash = 5381;
    char buffer[256];
    
    for (int i = 0; i < count; i++) {
        /* Clear buffer with memset */
        __builtin_memset(buffer, 0, sizeof(buffer));
        
        /* Copy token with memcpy */
        size_t len = __builtin_strlen(tokens[i]);
        if (len > sizeof(buffer) - 1)
            len = sizeof(buffer) - 1;
        
        __builtin_memcpy(buffer, tokens[i], len);
        
        /* Update hash */
        for (size_t j = 0; j < len; j++) {
            hash = ((hash << 5) + hash) + buffer[j];
        }
        
        /* Conditional memmove */
        if (i > 0 && (hash % 3 == 0)) {
            __builtin_memmove(buffer + 10, buffer, 20);
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in test\n");
    
    /* Phase 1: Recursive AST operations */
    struct ASTNode* root = create_ast(3);
    if (root) {
        struct ASTNode* copy = malloc(sizeof(struct ASTNode));
        if (copy) {
            /* Direct memcpy between structures */
            __builtin_memcpy(copy, root, sizeof(*root));
            
            /* Goto-based processing */
            process_with_goto(root, copy);
            
            free(copy);
        }
        
        /* Free AST recursively */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    /* Phase 2: OpenMP parallel operations */
    parallel_mem_ops();
    
    /* Phase 3: Token processing */
    const char* tokens[] = {
        "memcpy", "memset", "memmove",
        "asan", "hwasan", "instrumentation",
        "redzone", "shadow", "granule"
    };
    
    unsigned long result = process_tokens(tokens, 
                                        sizeof(tokens)/sizeof(tokens[0]));
    
    printf("Processing result: %lu\n", result);
    printf("Test completed\n");
    
    return 0;
}
