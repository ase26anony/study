/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    char data[256];
    struct ast_node* left;
    struct ast_node* right;
    int depth;
};

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_memory_pool(void) {
    volatile char buffer[128];
    /* Force memcpy redirection early */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    printf("Constructor: Initialized memory pool\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_memory(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Use volatile to prevent folding */
    volatile size_t copy_len = g_mem_size % 128;
    
    /* Built-in memset with non-constant size */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Built-in memcpy with variable length */
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->depth = depth;
    
    /* Recursive construction with goto for flow control */
    if (depth > 1) {
        char new_data[256];
        __builtin_memset(new_data, 'A' + depth, sizeof(new_data));
        
        /* Jump label for goto testing */
        build_left:
        node->left = build_ast(depth - 1, new_data);
        
        /* Skip right branch with goto */
        if (depth % 2 == 0) goto skip_right;
        
        build_right:
        node->right = build_ast(depth - 1, new_data);
        goto done;
        
        skip_right:
        node->right = NULL;
        goto done;
    } else {
        node->left = node->right = NULL;
    }
    
done:
    return node;
}

/* Function with complex goto patterns around memmove */
static void rearrange_ast(struct ast_node* node) {
    if (!node) return;
    
    volatile int should_move = 1;
    
    /* Goto into block containing memmove */
    if (node->left && node->right) {
        goto move_block;
    }
    
    normal_path:
    __builtin_memcpy(node->data, "Normal", 7);
    return;
    
    move_block:
    {
        char temp[256];
        /* Built-in memmove with overlap */
        __builtin_memmove(temp, node->left->data, sizeof(temp));
        __builtin_memmove(node->left->data, node->right->data, sizeof(node->left->data));
        __builtin_memmove(node->right->data, temp, sizeof(node->right->data));
        
        /* Jump out of block */
        if (should_move) goto normal_path;
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(struct ast_node** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            volatile size_t op_size = (g_mem_size + i) % 128;
            
            /* All three built-ins in parallel context */
            __builtin_memset(nodes[i]->data + 64, i, op_size);
            
            if (i > 0) {
                __builtin_memcpy(nodes[i]->data, nodes[i-1]->data, op_size);
            }
            
            if (i < count - 1) {
                char buffer[256];
                __builtin_memmove(buffer, nodes[i]->data, op_size);
                __builtin_memmove(nodes[i]->data + 32, buffer, op_size);
            }
        }
    }
}

/* Calculate hash of AST structure */
static unsigned long ast_hash(struct ast_node* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    int i;
    
    /* Hash the data */
    for (i = 0; i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Recursive hash calculation */
    hash += ast_hash(node->left);
    hash += ast_hash(node->right);
    
    return hash;
}

int main(void) {
    struct ast_node* ast = NULL;
    struct ast_node* node_array[8] = {0};
    unsigned long total_hash = 0;
    int i;
    
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Build recursive AST structure */
    ast = build_ast(4, "BaseASTData");
    
    /* Fill array with AST nodes */
    for (i = 0; i < 8; i++) {
        char data[256];
        __builtin_memset(data, '0' + i, sizeof(data));
        node_array[i] = build_ast(3, data);
    }
    
    /* Test goto flow control with memmove */
    rearrange_ast(ast);
    
    /* Parallel memory operations */
    #pragma omp parallel
    {
        #pragma omp single
        printf("OpenMP threads: %d\n", omp_get_num_threads());
    }
    
    parallel_memory_ops(node_array, 8);
    
    /* Calculate verification hash */
    total_hash = ast_hash(ast);
    for (i = 0; i < 8; i++) {
        total_hash ^= ast_hash(node_array[i]);
    }
    
    printf("Verification hash: %lu\n", total_hash);
    
    /* Additional built-in calls in different contexts */
    {
        volatile char src[256], dst[256];
        volatile size_t len = g_mem_size;
        
        __builtin_memset(src, 0xCC, sizeof(src));
        __builtin_memcpy(dst, src, len);
        __builtin_memmove(src + 128, dst, len);
    }
    
    /* Test with variable that might trigger HWASAN branch */
    if (g_use_hwasan) {
        volatile char buffer[512];
        __builtin_memset(buffer, 0, sizeof(buffer));
    }
    
    printf("Test completed successfully\n");
    return 0;
}
