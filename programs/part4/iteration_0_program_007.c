/* ISO C99-compliant test program for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    char data[256];
    struct ast_node *left;
    struct ast_node *right;
    int id;
};

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_memory_pool(void) {
    /* Force initialization of memory functions */
    char buffer[128];
    char target[128];
    
    /* Use all three builtins in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(target, buffer, 64);
    __builtin_memmove(buffer + 32, buffer, 32);
    
    printf("Constructor: Memory pool initialized\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_memory_pool(void) {
    printf("Destructor: Memory pool cleaned up\n");
}

/* Recursive function with memory operations */
static struct ast_node* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize node data with builtins */
    __builtin_memset(node->data, id, sizeof(node->data));
    node->id = id;
    
    /* Create pattern in left half */
    char pattern[128];
    __builtin_memset(pattern, 0xCC, sizeof(pattern));
    __builtin_memcpy(node->data, pattern, 128);
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    /* Copy data between nodes if both exist */
    if (node->left && node->right) {
        __builtin_memcpy(node->right->data + 64, 
                        node->left->data, 
                        64);
    }
    
done:
    return node;
}

/* Function with goto jumping around memmove */
static void process_with_goto(struct ast_node* node) {
    if (!node) return;
    
    volatile int use_memmove = 1;
    char temp[256];
    
    if (node->id % 3 == 0) {
        goto skip_memmove;
    }
    
    /* This memmove should be intercepted */
    __builtin_memmove(temp, node->data, g_mem_size);
    goto process_data;
    
skip_memmove:
    __builtin_memset(temp, 0xFF, g_mem_size);
    
process_data:
    /* Copy back with memcpy */
    __builtin_memcpy(node->data + 128, temp, g_mem_size);
    
    /* Jump back for recursive processing */
    if (node->left) {
        goto process_left;
    }
    return;
    
process_left:
    process_with_goto(node->left);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(struct ast_node** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Force all three builtins in parallel region */
            char buffer[512];
            volatile size_t local_size = g_mem_size + i;
            
            __builtin_memset(buffer, i, sizeof(buffer));
            __builtin_memcpy(nodes[i]->data, buffer, local_size);
            
            /* Conditional memmove */
            if (i % 2 == 0) {
                __builtin_memmove(nodes[i]->data + 128, 
                                nodes[i]->data, 
                                local_size / 2);
            }
        }
    }
}

/* Complex token processing with varied memory operations */
static unsigned long process_tokens(const char* tokens[], int token_count) {
    unsigned long hash = 0xDEADBEEF;
    char accum[1024];
    volatile int offset = 0;
    
    /* Initialize accumulator */
    __builtin_memset(accum, 0, sizeof(accum));
    
    for (int i = 0; i < token_count; i++) {
        size_t token_len = strlen(tokens[i]);
        volatile size_t copy_len = token_len;
        
        if (copy_len > 256) copy_len = 256;
        
        /* Mix of memory operations */
        if (i % 3 == 0) {
            __builtin_memcpy(accum + offset, tokens[i], copy_len);
        } else if (i % 3 == 1) {
            __builtin_memmove(accum + offset, tokens[i], copy_len);
        } else {
            __builtin_memset(accum + offset, tokens[i][0], copy_len);
        }
        
        offset += copy_len;
        if (offset > 512) {
            __builtin_memmove(accum, accum + 256, 256);
            offset = 256;
        }
    }
    
    /* Compute simple hash */
    for (int i = 0; i < 256; i++) {
        hash = (hash << 5) + hash + accum[i];
    }
    
    return hash;
}

int main(void) {
    /* Initialize complex token array */
    const char* tokens[] = {
        "MEMCPY_TEST_1234567890",
        "MEMSET_ABCDEFGHIJKLMNOP",
        "MEMMOVE_XYZ1234567890",
        "ASAN_INSTRUMENTATION",
        "HWASAN_KERNEL_MODE",
        "BUILTIN_REDIRECTION",
        "GOTO_FLOW_CONTROL",
        "OPENMP_PARALLEL",
        "VOLATILE_VARIABLES",
        "RECURSIVE_AST_NODES"
    };
    
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create recursive AST structure */
    struct ast_node* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto statements */
    process_with_goto(root);
    
    /* Create array of nodes for parallel processing */
    struct ast_node* nodes[8];
    nodes[0] = root;
    for (int i = 1; i < 8; i++) {
        nodes[i] = create_ast(3, i + 10);
    }
    
    /* Execute parallelized memory operations */
    parallel_memory_ops(nodes, 8);
    
    /* Process tokens with varied memory operations */
    unsigned long result_hash = process_tokens(tokens, token_count);
    
    printf("Result hash: 0x%08lX\n", result_hash);
    
    /* Cleanup */
    for (int i = 1; i < 8; i++) {
        if (nodes[i]) free(nodes[i]);
    }
    if (root) free(root);
    
    printf("Test completed successfully\n");
    return 0;
}
