#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Complex AST-like structure for data structure access */
typedef struct ASTNode {
    int type;
    int value;
    volatile int volatile_field;  /* Prevent optimization */
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
} ASTNode;

/* Global token array for initialization */
static volatile int token_array[256];
static int array_initialized = 0;

/* Constructor function for early initialization */
__attribute__((constructor)) 
static void init_token_array(void) {
    volatile int i;
    for (i = 0; i < 256; i++) {
        token_array[i] = i * 3 + 7;
    }
    array_initialized = 1;
}

/* Destructor function */
__attribute__((destructor))
static void cleanup_resources(void) {
    /* Explicit memory clearing using builtin */
    __builtin_memset((void*)token_array, 0, sizeof(token_array));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int max_depth) {
    if (depth >= max_depth) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = token_array[depth % 256];
    node->volatile_field = depth * 2;
    
    /* Fill data with pattern using builtin memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) {
        pattern[i] = (char)((i + depth) % 256);
    }
    __builtin_memcpy(node->data, pattern, 64);
    
    /* Recursive creation with goto for control flow */
    if (depth % 3 == 0) {
        goto create_left;
    } else {
        goto create_right;
    }
    
create_left:
    node->left = create_ast_node(depth + 1, max_depth);
    if (depth % 2 == 1) goto skip_right;
    
create_right:
    node->right = create_ast_node(depth + 2, max_depth);
    
skip_right:
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    volatile int use_memmove = 0;
    
    if (src && dst) {
        use_memmove = 1;
        goto perform_operation;
    }
    
    /* This block should be jumped into */
perform_operation:
    if (use_memmove) {
        /* Force memmove usage with overlapping regions */
        __builtin_memmove(dst->data, src->data, 32);
        
        /* Jump out to different context */
        goto cleanup;
    }
    
    /* Normal path */
    if (src) {
        __builtin_memset(src->data, 0xFF, 16);
    }
    
cleanup:
    return;
}

/* Parallel memory dispatch logic */
static long long parallel_memory_operations(ASTNode** nodes, int count) {
    volatile long long total_hash = 0;
    volatile int block_size = 64;  /* Prevent constant folding */
    
    #pragma omp parallel reduction(+:total_hash)
    {
        int thread_id = omp_get_thread_num();
        volatile int local_offset = thread_id * 16;
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Mixed memory operations in parallel region */
                if (i % 3 == 0) {
                    /* memcpy between nodes */
                    if (i + 1 < count && nodes[i + 1]) {
                        __builtin_memcpy(nodes[i + 1]->data, 
                                       nodes[i]->data, 
                                       block_size);
                    }
                } else if (i % 3 == 1) {
                    /* memset pattern */
                    __builtin_memset(nodes[i]->data, 
                                   (char)(i % 256), 
                                   block_size);
                } else {
                    /* memmove with overlap */
                    if (i > 0 && nodes[i - 1]) {
                        __builtin_memmove(nodes[i]->data + 16,
                                        nodes[i]->data,
                                        32);
                    }
                }
                
                /* Compute hash from node data */
                long long node_hash = 0;
                for (int j = 0; j < 64; j++) {
                    node_hash = node_hash * 31 + nodes[i]->data[j];
                }
                total_hash += node_hash;
            }
        }
        
        /* Additional memory operation outside loop */
        volatile char temp_buffer[128];
        __builtin_memset(temp_buffer, thread_id, sizeof(temp_buffer));
        
        /* Cross-thread memory operation simulation */
        if (thread_id == 0) {
            #pragma omp barrier
            for (int t = 1; t < omp_get_num_threads(); t++) {
                /* Symbolic memory operation */
                volatile int dummy = 0;
                (void)dummy;
            }
        }
    }
    
    return total_hash;
}

/* Multi-stage initialization */
static ASTNode** create_node_array(int size) {
    ASTNode** array = (ASTNode**)malloc(size * sizeof(ASTNode*));
    if (!array) return NULL;
    
    /* Initialize array with builtin memset */
    __builtin_memset(array, 0, size * sizeof(ASTNode*));
    
    for (int i = 0; i < size; i++) {
        array[i] = create_ast_node(i % 5, 5);
    }
    
    return array;
}

static void free_node_array(ASTNode** array, int size) {
    if (!array) return;
    
    for (int i = 0; i < size; i++) {
        if (array[i]) {
            free(array[i]);
        }
    }
    free(array);
}

int main(void) {
    /* Verify constructor ran */
    if (!array_initialized) {
        fprintf(stderr, "Constructor not called\n");
        return 1;
    }
    
    /* Create complex AST structure */
    ASTNode* root = create_ast_node(0, 4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Test goto control flow with memory operations */
    ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
    if (copy) {
        __builtin_memset(copy, 0, sizeof(ASTNode));
        process_with_goto(root, copy);
        free(copy);
    }
    
    /* Create array for parallel processing */
    const int array_size = 32;
    ASTNode** node_array = create_node_array(array_size);
    if (!node_array) {
        fprintf(stderr, "Failed to create node array\n");
        free(root);
        return 1;
    }
    
    /* Execute parallel memory operations */
    long long result_hash = parallel_memory_operations(node_array, array_size);
    
    /* Additional memory operation in main */
    volatile char final_buffer[256];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, root->data, 64);
    __builtin_memmove(final_buffer + 128, final_buffer, 64);
    
    /* Print verification result */
    printf("Result hash: %lld\n", result_hash);
    printf("Root value: %d\n", root->value);
    
    /* Cleanup */
    free(root);
    free_node_array(node_array, array_size);
    
    return 0;
}
