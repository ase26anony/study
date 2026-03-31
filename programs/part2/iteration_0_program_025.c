#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Complex AST-like structure for data structure access */
typedef struct ASTNode {
    int type;
    int value;
    volatile int flags;  /* volatile to prevent optimization */
    char *data;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *parent;
} ASTNode;

/* Global token array for initialization */
static volatile int global_token_array[256];

/* Constructor function to force initialization logic */
__attribute__((constructor)) 
static void init_constructor(void) {
    volatile int i;
    char buffer[64];
    
    /* Use __builtin_memset in constructor */
    __builtin_memset(buffer, 0, sizeof(buffer));
    
    /* Initialize token array with non-foldable pattern */
    for (i = 0; i < 256; i++) {
        global_token_array[i] = (i * 1103515245 + 12345) & 0x7fffffff;
    }
}

/* Destructor function for cleanup coordination */
__attribute__((destructor))
static void cleanup_destructor(void) {
    volatile char cleanup_buf[128];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) {
        return NULL;
    }
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Volatile control of sizes */
    volatile size_t data_size = (depth * 16 + 32) & 0xFF;
    
    /* Initialize node with __builtin_memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = global_token_array[depth];
    node->flags = (depth % 2) ? 1 : 0;
    
    /* Allocate and initialize data with memcpy */
    node->data = malloc(data_size + 1);
    if (node->data) {
        char temp[256];
        volatile int pattern = depth * 0xABCD;
        
        /* Use __builtin_memset and __builtin_memcpy */
        __builtin_memset(temp, pattern & 0xFF, sizeof(temp));
        __builtin_memcpy(node->data, temp, data_size);
        node->data[data_size] = '\0';
    }
    
    /* Recursive creation with goto for control flow */
    if (depth < max_depth - 1) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    return node;
    
create_children:
    node->left = create_ast(depth + 1, max_depth);
    
    /* Jump back from goto block */
    if (node->left) {
        volatile char temp_buf[64];
        /* __builtin_memmove inside goto block */
        __builtin_memmove(temp_buf, node->left->data, 
                         (node->left->flags ? 32 : 16));
        goto create_right;
    }
    
create_right:
    node->right = create_ast(depth + 2, max_depth);
    return node;
}

/* Function with complex control flow using goto */
static void process_with_goto(ASTNode *node, int depth) {
    if (!node) return;
    
    volatile char local_buf[128];
    volatile int use_memmove = node->flags;
    
    if (depth % 3 == 0) {
        goto memcpy_block;
    } else if (depth % 3 == 1) {
        goto memset_block;
    } else {
        goto memmove_block;
    }
    
memcpy_block:
    /* Force __builtin_memcpy with volatile size */
    {
        volatile size_t copy_size = (node->value % 64) + 16;
        __builtin_memcpy(local_buf, node->data, copy_size);
    }
    goto process_children;
    
memset_block:
    /* Force __builtin_memset with volatile pattern */
    {
        volatile char pattern = node->value & 0xFF;
        volatile size_t set_size = (node->type * 8) & 0x7F;
        __builtin_memset(local_buf, pattern, set_size);
    }
    goto process_children;
    
memmove_block:
    /* Force __builtin_memmove with goto in/out */
    if (use_memmove) {
        volatile size_t move_size = 48;
        char temp[64];
        __builtin_memset(temp, 0xAA, sizeof(temp));
        __builtin_memmove(local_buf, temp, move_size);
        goto overlap_move;
    }
    goto process_children;
    
overlap_move:
    /* Overlapping memmove */
    __builtin_memmove(local_buf + 16, local_buf, 32);
    goto process_children;
    
process_children:
    process_with_goto(node->left, depth + 1);
    process_with_goto(node->right, depth + 2);
}

/* Parallel memory dispatch with OpenMP */
static long long parallel_memory_operations(ASTNode **nodes, int count) {
    long long total_hash = 0;
    volatile int i;
    
    #pragma omp parallel reduction(+:total_hash)
    {
        int thread_id = omp_get_thread_num();
        volatile char thread_buf[256];
        
        #pragma omp for
        for (i = 0; i < count; i++) {
            if (nodes[i]) {
                volatile size_t op_size = (thread_id * 32 + i * 16) & 0xFF;
                
                /* Mixed builtin usage in parallel region */
                if (i % 3 == 0) {
                    __builtin_memset(thread_buf, nodes[i]->value & 0xFF, op_size);
                    __builtin_memcpy(nodes[i]->data, thread_buf, 
                                   (op_size > 64) ? 64 : op_size);
                } else if (i % 3 == 1) {
                    char temp[128];
                    __builtin_memset(temp, 0xCC, sizeof(temp));
                    __builtin_memmove(thread_buf, temp, op_size);
                    __builtin_memcpy(nodes[i]->data + 16, thread_buf, 32);
                } else {
                    __builtin_memmove(thread_buf, nodes[i]->data, op_size);
                    __builtin_memset(nodes[i]->data, 0, op_size / 2);
                    __builtin_memcpy(nodes[i]->data + op_size / 2, 
                                   thread_buf, op_size / 2);
                }
                
                /* Compute hash from processed data */
                for (int j = 0; j < 32 && j < op_size; j++) {
                    total_hash += thread_buf[j] * (i + 1);
                }
            }
        }
    }
    
    return total_hash;
}

/* Main execution flow */
int main(void) {
    const int ast_depth = 5;
    const int node_count = 8;
    ASTNode *nodes[node_count];
    long long final_hash = 0;
    volatile int i;
    
    printf("Starting ASAN coverage test...\n");
    
    /* Create multiple ASTs */
    for (i = 0; i < node_count; i++) {
        nodes[i] = create_ast(0, ast_depth);
    }
    
    /* Process with goto control flow */
    for (i = 0; i < node_count; i++) {
        process_with_goto(nodes[i], 0);
    }
    
    /* Execute parallel memory operations */
    final_hash = parallel_memory_operations(nodes, node_count);
    
    /* Additional memory operations in main */
    volatile char main_buf[512];
    volatile size_t final_size = 256;
    
    __builtin_memset(main_buf, 0xFE, final_size);
    __builtin_memcpy(main_buf + 128, main_buf, 128);
    __builtin_memmove(main_buf + 64, main_buf + 32, 96);
    
    /* Verify by computing final checksum */
    for (i = 0; i < 256; i++) {
        final_hash += main_buf[i];
    }
    
    printf("Final hash: %lld\n", final_hash);
    
    /* Cleanup */
    for (i = 0; i < node_count; i++) {
        if (nodes[i]) {
            if (nodes[i]->data) free(nodes[i]->data);
            free(nodes[i]);
        }
    }
    
    return (final_hash != 0) ? 0 : 1;
}
