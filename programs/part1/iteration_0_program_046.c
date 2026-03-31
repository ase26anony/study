#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Global token array */
volatile int token_array[256];
volatile size_t token_idx = 0;

/* Volatile variables to prevent optimization */
volatile size_t mem_size = 64;
volatile int should_copy = 1;

/* Constructor function */
__attribute__((constructor)) void init_tokens() {
    for (int i = 0; i < 256; i++) {
        token_array[i] = i * 3 + 7;
    }
}

/* Destructor function */
__attribute__((destructor)) void cleanup() {
    printf("Cleanup completed\n");
}

/* Recursive parser with memory operations */
ASTNode* create_node(int type, int value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = type;
    node->value = value;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memset to initialize data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    return node;
}

void copy_node_data(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Force __builtin_memcpy usage with volatile size */
    size_t copy_size = mem_size;
    if (copy_size > sizeof(dest->data))
        copy_size = sizeof(dest->data);
    
    __builtin_memcpy(dest->data, src->data, copy_size);
}

/* Function with goto statements for flow control */
void process_with_goto(ASTNode* nodes[], int count) {
    int i = 0;
    
start_loop:
    if (i >= count) goto end_processing;
    
    if (i % 2 == 0) {
        /* Jump into memory operation block */
        goto mem_operation;
    } else {
        i++;
        goto start_loop;
    }
    
mem_operation:
    {
        volatile char buffer[128];
        volatile char buffer2[128];
        
        /* __builtin_memmove with goto context */
        __builtin_memset(buffer, i, sizeof(buffer));
        
        if (should_copy) {
            __builtin_memmove(buffer2, buffer, sizeof(buffer));
            
            /* Another goto to exit block */
            goto after_memmove;
        }
        
        __builtin_memcpy(buffer, "test", 5);
        
    after_memmove:
        i++;
        goto start_loop;
    }
    
end_processing:
    return;
}

/* Parallel memory dispatch logic */
void parallel_memory_operations() {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        volatile char local_buf[256];
        volatile char shared_buf[256];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        if (thread_id == 0) {
            /* Master thread copies from all */
            for (int i = 0; i < omp_get_num_threads(); i++) {
                volatile char temp[256];
                __builtin_memcpy(temp, local_buf, sizeof(temp));
                __builtin_memmove(shared_buf, temp, sizeof(shared_buf));
            }
        }
        
        #pragma omp barrier
        
        /* Verify with memset */
        __builtin_memset(local_buf, 0xFF, sizeof(local_buf));
    }
}

/* Complex initialization with recursion */
ASTNode* build_tree(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = create_node(depth, (*counter)++);
    
    /* Fill data with pattern */
    for (int i = 0; i < (int)sizeof(node->data); i++) {
        node->data[i] = (char)((depth * 17 + i * 13) % 256);
    }
    
    node->left = build_tree(depth - 1, counter);
    node->right = build_tree(depth - 1, counter);
    
    return node;
}

/* Tree traversal with memory operations */
int traverse_and_process(ASTNode* root, int* sum) {
    if (!root) return 0;
    
    int left_sum = traverse_and_process(root->left, sum);
    int right_sum = traverse_and_process(root->right, sum);
    
    /* Process current node with memory builtins */
    volatile char temp[64];
    __builtin_memcpy(temp, root->data, sizeof(temp));
    
    /* Modify with memset */
    __builtin_memset(root->data + 32, root->value, 16);
    
    /* Move data around */
    __builtin_memmove(temp + 16, root->data, 32);
    
    *sum += root->value + left_sum + right_sum;
    return root->value;
}

void free_tree(ASTNode* root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

int main() {
    printf("Starting ASAN coverage test...\n");
    
    /* Initialize complex structures */
    int counter = 0;
    ASTNode* tree = build_tree(4, &counter);
    
    /* Create array of nodes for goto testing */
    ASTNode* node_array[8];
    for (int i = 0; i < 8; i++) {
        node_array[i] = create_node(i, i * 100);
        __builtin_memset(node_array[i]->data, i * 10, sizeof(node_array[i]->data));
    }
    
    /* Test goto flow control with memory operations */
    process_with_goto(node_array, 8);
    
    /* Copy data between nodes */
    for (int i = 0; i < 7; i++) {
        copy_node_data(node_array[i + 1], node_array[i]);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Process tree and compute result */
    int total_sum = 0;
    traverse_and_process(tree, &total_sum);
    
    /* Additional memory builtin calls in main */
    volatile char final_buffer[1024];
    volatile char source_buffer[1024];
    
    /* Chain of memory operations */
    __builtin_memset(source_buffer, 0xAA, sizeof(source_buffer));
    __builtin_memcpy(final_buffer, source_buffer, sizeof(final_buffer));
    __builtin_memmove(final_buffer + 512, final_buffer, 512);
    __builtin_memset(final_buffer + 256, 0xBB, 128);
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 256; i++) {
        hash = hash * 31 + token_array[i];
    }
    hash += total_sum;
    
    printf("Result hash: %lu\n", hash);
    printf("Total tree sum: %d\n", total_sum);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free(node_array[i]);
    }
    free_tree(tree);
    
    printf("Test completed successfully\n");
    return 0;
}
