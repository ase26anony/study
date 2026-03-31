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
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_global_data(void) {
    g_init_flag = 1;
    printf("Constructor: Initializing ASAN environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_global_data(void) {
    printf("Destructor: Cleaning up ASAN environment\n");
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_tree(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = (*counter)++;
    
    /* Create pattern in data using __builtin_memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (depth % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int left_counter = *counter;
        goto create_left;
        
    create_left:
        node->left = create_tree(depth - 1, &left_counter);
        *counter = left_counter;
        
        int right_counter = *counter;
        if (depth % 2 == 0) {
            goto create_right;
        }
        
    create_right:
        node->right = create_tree(depth - 1, &right_counter);
        *counter = right_counter;
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    volatile int use_memmove = 1;
    
    /* Jump into memory operation block */
    if (src->id % 3 == 0) {
        goto block1;
    } else {
        goto block2;
    }
    
block1:
    {
        char temp[64];
        /* Force __builtin_memcpy */
        __builtin_memcpy(temp, src->data, sizeof(temp));
        
        if (use_memmove) {
            goto use_memmove_block;
        }
        
        __builtin_memcpy(dst->data, temp, sizeof(dst->data));
        goto after_copy;
    }

use_memmove_block:
    {
        /* Force __builtin_memmove with overlapping regions */
        char buffer[128];
        __builtin_memcpy(buffer, src->data, sizeof(src->data));
        __builtin_memmove(buffer + 32, buffer, sizeof(src->data));
        __builtin_memcpy(dst->data, buffer + 32, sizeof(dst->data));
    }
    goto after_copy;

block2:
    {
        /* Force __builtin_memset */
        __builtin_memset(dst->data, 'X', sizeof(dst->data));
    }

after_copy:
    /* Continue processing */
    if (src->left && dst->left) {
        process_with_goto(src->left, dst->left);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            volatile size_t local_size = g_mem_size;
            char* buffer = (char*)malloc(local_size);
            
            if (buffer) {
                /* Mix of memory built-ins in parallel region */
                __builtin_memset(buffer, i % 256, local_size);
                
                if (i % 2 == 0) {
                    __builtin_memcpy(buffer + 16, nodes[i]->data, 
                                   sizeof(nodes[i]->data));
                } else {
                    __builtin_memmove(buffer, buffer + 8, local_size - 8);
                }
                
                /* Verify by computing checksum */
                unsigned long sum = 0;
                for (size_t j = 0; j < local_size; j++) {
                    sum += (unsigned char)buffer[j];
                }
                
                #pragma omp critical
                {
                    printf("Thread %d: Node %d checksum = %lu\n",
                           omp_get_thread_num(), nodes[i]->id, sum);
                }
                
                free(buffer);
            }
        }
    }
}

/* Multi-stage initialization */
static ASTNode** create_node_array(int* total_count) {
    const int depths[] = {3, 4, 2, 5, 3};
    const int num_trees = sizeof(depths)/sizeof(depths[0]);
    static ASTNode* nodes[32];
    int counter = 1;
    int idx = 0;
    
    for (int i = 0; i < num_trees; i++) {
        ASTNode* root = create_tree(depths[i], &counter);
        if (root) {
            nodes[idx++] = root;
            
            /* Create duplicate tree using memory operations */
            ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
            if (copy) {
                __builtin_memcpy(copy, root, sizeof(ASTNode));
                copy->left = copy->right = NULL;
                nodes[idx++] = copy;
                
                /* Process with goto flow */
                process_with_goto(root, copy);
            }
        }
    }
    
    *total_count = idx;
    return nodes;
}

int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Stage 1: Create complex AST structures */
    int node_count = 0;
    ASTNode** nodes = create_node_array(&node_count);
    
    printf("Created %d AST nodes\n", node_count);
    
    /* Stage 2: Parallel memory operations */
    parallel_memory_ops(nodes, node_count);
    
    /* Stage 3: Additional built-in usage in main flow */
    volatile char final_buffer[512];
    volatile char source_buffer[512];
    
    /* Initialize source with pattern */
    for (int i = 0; i < sizeof(source_buffer); i++) {
        source_buffer[i] = (char)(i % 256);
    }
    
    /* Force all three built-ins in sequence */
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, source_buffer, sizeof(final_buffer));
    __builtin_memmove(final_buffer + 128, final_buffer, 256);
    
    /* Compute verification hash */
    unsigned long final_hash = 0;
    for (int i = 0; i < sizeof(final_buffer); i++) {
        final_hash = final_hash * 31 + (unsigned char)final_buffer[i];
    }
    
    printf("Final buffer hash: %lu\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    for (int i = 0; i < node_count; i++) {
        free(nodes[i]);
    }
    
    return 0;
}
