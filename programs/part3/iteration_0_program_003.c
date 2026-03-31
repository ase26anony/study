/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_selector = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
char global_tokens[1024];
int token_hash = 0;

/* Constructor/destructor functions */
__attribute__((constructor)) void init_asan_environment() {
    /* Force initialization of ASAN runtime */
    volatile char buffer[128];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
}

__attribute__((destructor)) void cleanup_asan_environment() {
    /* Trigger finalization logic */
    volatile char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
}

/* Recursive parser with memory operations */
ASTNode* build_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = depth;
    
    /* Copy data using builtin memcpy */
    char source[256];
    __builtin_memset(source, 'A' + depth, sizeof(source));
    __builtin_memcpy(node->data, source, volatile_len % 256);
    
    /* Recursive construction */
    node->left = build_ast(depth + 1, max_depth);
    node->right = build_ast(depth + 2, max_depth);
    
    return node;
}

/* Function with goto edge cases */
void process_with_goto(ASTNode* node1, ASTNode* node2) {
    if (!node1 || !node2) return;
    
    int use_memmove = 0;
    
    /* Jump into memory operation block */
    goto entry_point;
    
memmove_block:
    {
        /* Force memmove redirection */
        char temp[256];
        __builtin_memmove(temp, node1->data, volatile_len % 256);
        __builtin_memmove(node1->data, node2->data, volatile_len % 256);
        __builtin_memmove(node2->data, temp, volatile_len % 256);
    }
    goto after_operation;
    
entry_point:
    if (volatile_selector & 1) {
        use_memmove = 1;
        goto memmove_block;
    }
    
    /* Regular memcpy path */
    __builtin_memcpy(node1->data, node2->data, volatile_len % 256);
    
after_operation:
    /* Additional operation after goto */
    __builtin_memset(node1->data + 128, 0xBB, 32);
}

/* OpenMP parallel memory operations */
void parallel_memory_operations(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Mixed builtin usage in parallel region */
                char buffer[512];
                
                /* Force all three builtins */
                __builtin_memset(buffer, tid, sizeof(buffer));
                __builtin_memcpy(nodes[i]->data, buffer, volatile_len % 256);
                
                if (i > 0 && nodes[i-1]) {
                    __builtin_memmove(nodes[i-1]->data + 64, 
                                     nodes[i]->data, 
                                     volatile_len % 128);
                }
                
                /* Update hash */
                #pragma omp atomic
                token_hash += nodes[i]->data[0];
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Initialize token array with builtin memset */
    __builtin_memset(global_tokens, 0xCC, sizeof(global_tokens));
    
    /* Build recursive structures */
    ASTNode* root = build_ast(0, 4);
    ASTNode* alt_root = build_ast(1, 3);
    
    if (!root || !alt_root) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Test goto edge cases */
    for (volatile_selector = 0; volatile_selector < 4; volatile_selector++) {
        process_with_goto(root, alt_root);
    }
    
    /* Prepare array for parallel operations */
    ASTNode* node_array[8];
    node_array[0] = root;
    node_array[1] = alt_root;
    
    for (int i = 2; i < 8; i++) {
        node_array[i] = build_ast(i, 3);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations(node_array, 8);
    
    /* Final verification with all three builtins */
    char final_buffer[1024];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, root->data, volatile_len % 256);
    __builtin_memmove(final_buffer + 256, alt_root->data, volatile_len % 256);
    
    /* Compute final hash */
    int final_hash = token_hash;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        final_hash += final_buffer[i];
    }
    
    printf("Final hash: %d\n", final_hash);
    printf("Test completed\n");
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        if (node_array[i]) free(node_array[i]);
    }
    
    return 0;
}
