/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force early initialization of memory builtins */
    char buffer1[128];
    char buffer2[128];
    
    /* Use all three builtins in constructor */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    
    if (g_use_memmove) {
        __builtin_memmove(buffer1 + 32, buffer1, 64);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_late(void) {
    /* Final memory operations in destructor */
    volatile char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_tree(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Initialize node data with memset */
    __builtin_memset(node->data, node->id, sizeof(node->data));
    
    /* Create left subtree */
    node->left = create_tree(depth - 1, counter);
    
    /* Copy data between nodes if both children exist */
    if (node->left && depth > 2) {
        ASTNode* temp = create_tree(1, counter);
        if (temp) {
            /* Use memcpy between structures */
            __builtin_memcpy(&temp->data, &node->left->data, sizeof(node->data));
            free(temp);
        }
    }
    
    /* Create right subtree with goto for flow control */
    int use_goto = (depth % 3 == 0);
    
    if (use_goto) {
        goto create_right;
    }
    
    node->right = NULL;
    goto skip_right;
    
create_right:
    node->right = create_tree(depth - 2, counter);
    
    /* Use memmove with overlapping regions */
    if (node->right) {
        __builtin_memmove(node->data + 16, node->data, 32);
    }
    
skip_right:
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_ops(char* buffer, size_t size) {
    volatile size_t chunk_size = size / 4;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[128];
        
        /* Each thread uses different builtins */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(local_buf, thread_id, sizeof(local_buf));
                __builtin_memcpy(buffer + thread_id * chunk_size, 
                               local_buf, 
                               chunk_size);
                break;
            case 1:
                __builtin_memset(local_buf, 0xCC, sizeof(local_buf));
                /* Overlapping memmove */
                __builtin_memmove(buffer + thread_id * chunk_size + 8,
                                buffer + thread_id * chunk_size,
                                chunk_size - 8);
                break;
            case 2:
                /* Mixed operations */
                __builtin_memset(local_buf, 0xDD, sizeof(local_buf));
                __builtin_memcpy(buffer + thread_id * chunk_size,
                               local_buf,
                               chunk_size);
                __builtin_memmove(local_buf + 16, local_buf, 48);
                break;
        }
        
        /* Barrier to ensure all memory ops complete */
        #pragma omp barrier
        
        /* Verify with another memcpy */
        if (thread_id == 0) {
            char verify_buf[64];
            __builtin_memcpy(verify_buf, buffer, sizeof(verify_buf));
        }
    }
}

/* Token array initialization */
static void init_token_array(char tokens[][32], int count) {
    for (int i = 0; i < count; i++) {
        /* Use memset for initialization */
        __builtin_memset(tokens[i], i, 32);
        
        /* Conditional memcpy between tokens */
        if (i > 0) {
            __builtin_memcpy(tokens[i] + 16, tokens[i-1], 16);
        }
        
        /* Goto-based flow control around memmove */
        if (i % 5 == 0) {
            goto do_memmove;
        }
        continue;
        
    do_memmove:
        __builtin_memmove(tokens[i], tokens[i] + 8, 24);
    }
}

/* Main execution flow */
int main(void) {
    /* Initialize complex token array */
    char tokens[10][32];
    init_token_array(tokens, 10);
    
    /* Create recursive AST structure */
    int counter = 1;
    ASTNode* root = create_tree(4, &counter);
    
    /* Allocate buffer for parallel operations */
    size_t buffer_size = (size_t)g_mem_size;
    char* main_buffer = (char*)malloc(buffer_size);
    if (!main_buffer) return 1;
    
    /* Initialize buffer with memset */
    __builtin_memset(main_buffer, 0xAB, buffer_size);
    
    /* Perform parallel memory operations */
    parallel_memory_ops(main_buffer, buffer_size);
    
    /* Process AST with memory copies between nodes */
    unsigned long hash = 0;
    ASTNode* current = root;
    ASTNode* prev = NULL;
    
    while (current) {
        /* Accumulate hash from node data */
        for (int i = 0; i < 64; i++) {
            hash += (unsigned long)current->data[i];
        }
        
        /* Copy data to previous node if exists */
        if (prev) {
            __builtin_memcpy(prev->data + 32, current->data, 32);
        }
        
        /* Move to next node with memmove on current data */
        __builtin_memmove(current->data, current->data + 16, 48);
        
        prev = current;
        current = current->left ? current->left : current->right;
    }
    
    /* Final memory operations with all three builtins */
    char final_buffer[256];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, main_buffer, buffer_size < 256 ? buffer_size : 256);
    __builtin_memmove(final_buffer + 128, final_buffer, 128);
    
    /* Compute verification result */
    unsigned long result = hash;
    for (int i = 0; i < 256; i++) {
        result ^= (unsigned long)final_buffer[i] << (i % 8);
    }
    
    /* Cleanup */
    free(main_buffer);
    
    /* Free tree recursively */
    ASTNode* nodes[100];
    int node_count = 0;
    current = root;
    
    while (current || node_count > 0) {
        while (current) {
            nodes[node_count++] = current;
            current = current->left;
        }
        
        if (node_count > 0) {
            current = nodes[--node_count];
            ASTNode* right = current->right;
            free(current);
            current = right;
        }
    }
    
    printf("Result: 0x%lx\n", result);
    return 0;
}
