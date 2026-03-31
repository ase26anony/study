/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

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
    volatile char buffer[128];
    /* Force builtin initialization early */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 64, buffer, 32);
}

/* Destructor function */
__attribute__((destructor)) 
static void cleanup_asan(void) {
    volatile char final_check[16];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_tree(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use builtins with volatile size */
    __builtin_memset(node->data, node->id, sizeof(node->data));
    
    if (depth > 1) {
        /* Create children with goto for flow control */
        int left_counter = *counter;
        node->left = create_tree(depth - 1, counter);
        
        /* Jump label for flow sensitivity test */
        if (node->left) {
            void* target = node->left->data;
            size_t copy_size = g_mem_size % 32;
            
            /* Goto-based control flow */
            if (g_use_memmove) {
                goto use_memmove_block;
            } else {
                goto use_memcpy_block;
            }
            
use_memmove_block:
            /* This block tests memmove redirection */
            __builtin_memmove(node->data + 16, target, copy_size);
            goto after_copy;
            
use_memcpy_block:
            __builtin_memcpy(node->data + 16, target, copy_size);
            goto after_copy;
            
after_copy:
            /* Continue normal execution */
            node->right = create_tree(depth - 1, counter);
        }
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Parallel memory operation dispatcher */
static void parallel_mem_ops(ASTNode* root, int depth) {
    if (!root) return;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        volatile char local_buf[256];
        
        /* Each thread uses builtins independently */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp for
        for (int i = 0; i < depth * 4; i++) {
            size_t offset = (i * 17) % 192;
            size_t len = (i * 13) % 64 + 1;
            
            /* Mix memcpy and memmove based on thread parity */
            if (thread_id % 2 == 0) {
                __builtin_memcpy(local_buf + offset, 
                               root->data + (i % 56), 
                               len);
            } else {
                /* Potential overlap tests memmove logic */
                __builtin_memmove(local_buf + offset,
                                local_buf + offset - 8,
                                len);
            }
        }
        
        /* Copy results back to shared structure */
        #pragma omp critical
        {
            __builtin_memcpy(root->data + (thread_id * 8),
                           local_buf + 128,
                           32);
        }
    }
}

/* Tree traversal with memory operations */
static unsigned long traverse_and_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    volatile char temp[64];
    
    /* Copy node data to volatile buffer */
    __builtin_memcpy(temp, node->data, sizeof(node->data));
    
    /* Process each byte */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + temp[i];
    }
    
    /* Recursive traversal */
    hash += traverse_and_hash(node->left);
    hash += traverse_and_hash(node->right);
    
    return hash;
}

/* Free tree with memory clearing */
static void free_tree(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    free_tree(node->left);
    free_tree(node->right);
    
    /* Final clear of node memory */
    volatile char* node_mem = (char*)node;
    __builtin_memset(node_mem, 0, sizeof(ASTNode));
    free(node);
}

int main(void) {
    int counter = 1;
    unsigned long final_hash = 0;
    
    printf("Starting ASAN/HWASAN builtin redirection test...\n");
    
    /* Phase 1: Create recursive structure */
    ASTNode* root = create_tree(4, &counter);
    if (!root) {
        fprintf(stderr, "Failed to create tree\n");
        return 1;
    }
    
    /* Phase 2: Parallel memory operations */
    parallel_mem_ops(root, 4);
    
    /* Phase 3: Additional builtin calls in main */
    volatile char main_buffer[512];
    size_t sizes[] = {128, 256, 384};
    
    for (int i = 0; i < 3; i++) {
        /* Exercise all three builtins */
        __builtin_memset(main_buffer, i, sizes[i]);
        
        if (i % 2 == 0) {
            __builtin_memcpy(main_buffer + 64, 
                           main_buffer, 
                           sizes[i] / 2);
        } else {
            __builtin_memmove(main_buffer + 32,
                            main_buffer + 16,
                            sizes[i] / 4);
        }
    }
    
    /* Phase 4: Compute verification hash */
    final_hash = traverse_and_hash(root);
    printf("Result hash: %lu\n", final_hash);
    
    /* Phase 5: Cleanup with memory operations */
    free_tree(root);
    
    /* Final builtin call */
    __builtin_memset(main_buffer, 0, sizeof(main_buffer));
    
    printf("Test completed successfully\n");
    return 0;
}
