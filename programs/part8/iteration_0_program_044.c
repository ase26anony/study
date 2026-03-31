/* ISO C99-compliant test program for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    int type;
    char data[64];
    struct ast_node *left;
    struct ast_node *right;
    struct ast_node *next;
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    printf("ASAN test constructor initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("ASAN test destructor cleaning up\n");
}

/* Function with goto jumps around memory operations */
static void test_goto_memmove(struct ast_node *dst, struct ast_node *src) {
    int use_builtin = 1;
    
    if (use_builtin) goto use_builtin_path;
    
    regular_path:
    memmove(dst->data, src->data, sizeof(dst->data));
    return;
    
    use_builtin_path:
    /* Force __builtin_memmove usage with goto */
    __builtin_memmove(dst->data, src->data, sizeof(dst->data));
    goto regular_path;
}

/* Recursive function with memory operations */
static struct ast_node* create_ast(int depth, int max_depth) {
    struct ast_node *node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    node->type = depth;
    node->left = NULL;
    node->right = NULL;
    node->next = NULL;
    
    /* Initialize data with __builtin_memset */
    __builtin_memset(node->data, depth % 256, sizeof(node->data));
    
    if (depth < max_depth) {
        node->left = create_ast(depth + 1, max_depth);
        node->right = create_ast(depth + 1, max_depth);
        
        /* Copy data between nodes using __builtin_memcpy */
        if (node->left && node->right) {
            __builtin_memcpy(node->right->data, node->left->data, 
                           sizeof(node->data));
        }
    }
    
    return node;
}

/* Function with complex memory operations */
static void process_ast(struct ast_node *root) {
    struct ast_node *current = root;
    struct ast_node temp;
    
    while (current) {
        /* Use volatile to control operation size */
        size_t copy_size = g_mem_size % sizeof(current->data);
        if (copy_size > 0) {
            /* Mix of builtin and library calls */
            if (current->type % 2 == 0) {
                __builtin_memcpy(temp.data, current->data, copy_size);
            } else {
                memcpy(temp.data, current->data, copy_size);
            }
        }
        
        /* Test memmove with goto jumps */
        if (current->left && current->right) {
            test_goto_memmove(current->left, current->right);
        }
        
        current = current->next;
    }
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(struct ast_node **nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Force __builtin_memset in parallel context */
            __builtin_memset(nodes[i]->data, i % 256, sizeof(nodes[i]->data));
            
            /* Create chain of nodes */
            if (i > 0) {
                nodes[i-1]->next = nodes[i];
                
                /* Use __builtin_memmove in parallel */
                __builtin_memmove(nodes[i]->data, nodes[i-1]->data,
                                sizeof(nodes[i]->data));
            }
        }
    }
}

/* Calculate hash of AST structure */
static unsigned long ast_hash(struct ast_node *root) {
    unsigned long hash = 5381;
    struct ast_node *current = root;
    
    while (current) {
        int i;
        for (i = 0; i < sizeof(current->data); i++) {
            hash = ((hash << 5) + hash) + current->data[i];
        }
        hash = ((hash << 5) + hash) + current->type;
        current = current->next;
    }
    
    return hash;
}

/* Main test driver */
int main(void) {
    struct ast_node *root = NULL;
    struct ast_node *node_array[10];
    int i;
    unsigned long final_hash = 0;
    
    printf("Starting ASAN built-in redirection test\n");
    
    /* Create recursive AST structure */
    root = create_ast(0, 3);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    for (i = 0; i < 10; i++) {
        node_array[i] = create_ast(i % 3, 2);
    }
    
    /* Process with OpenMP */
    parallel_memory_ops(node_array, 10);
    
    /* Link all nodes into a list */
    struct ast_node *last = root;
    for (i = 0; i < 10; i++) {
        if (node_array[i]) {
            last->next = node_array[i];
            while (last->next) last = last->next;
        }
    }
    
    /* Process the complete chain */
    process_ast(root);
    
    /* Calculate and print verification hash */
    final_hash = ast_hash(root);
    printf("Verification hash: %lu\n", final_hash);
    
    /* Test all three builtins in one operation */
    {
        char buffer1[256];
        char buffer2[256];
        volatile size_t op_size = g_mem_size % sizeof(buffer1);
        
        /* Sequence using all three builtins */
        __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
        __builtin_memcpy(buffer2, buffer1, op_size);
        __builtin_memmove(buffer1, buffer2, op_size);
        
        /* Mix with regular calls */
        memset(buffer1 + 128, 0xBB, 64);
        memcpy(buffer2 + 64, buffer1, 32);
        memmove(buffer1, buffer2, 16);
    }
    
    printf("ASAN test completed successfully\n");
    return 0;
}
