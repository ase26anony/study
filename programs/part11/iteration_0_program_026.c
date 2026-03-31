#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* AST-like recursive structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    volatile int visited;  /* Prevent optimization */
} ASTNode;

/* Global volatile variables to prevent constant folding */
volatile size_t g_mem_size = 64;
volatile int g_init_value = 0x42;
volatile int g_copy_offset = 8;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_hooks(void) {
    volatile char buffer[128];
    /* Force initialization of memcpy redirection */
    __builtin_memcpy(buffer, "constructor_init", 16);
    __builtin_memset(buffer + 16, g_init_value, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_hooks(void) {
    volatile char cleanup_buf[256];
    /* Force memmove redirection in destructor */
    __builtin_memmove(cleanup_buf, "cleanup_data", 12);
}

/* Recursive AST creation with memory operations */
ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with offset using builtin memcpy */
    size_t copy_len = (depth * 8) % 64;
    if (copy_len > 0) {
        __builtin_memcpy(node->data, base_data, copy_len);
    }
    
    node->visited = depth;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, base_data + 1);
        node->right = NULL;
        
        create_children:
        /* Jump into block with memmove operation */
        volatile char temp[128];
        __builtin_memmove(temp, node->data, copy_len);
        node->right = create_ast(depth - 2, base_data + 2);
        
        /* Jump out of block */
        if (depth > 3) {
            goto finish_node;
        }
    }
    
    finish_node:
    return node;
}

/* Process AST with memory operations between nodes */
uint64_t process_ast(ASTNode* node, int level) {
    if (!node) return 0;
    
    uint64_t hash = 0;
    volatile char buffer[256];
    
    /* Copy node data to buffer */
    __builtin_memcpy(buffer, node->data, sizeof(node->data));
    
    /* Process left subtree */
    hash += process_ast(node->left, level + 1);
    
    /* Move data around with memmove */
    if (node->right) {
        __builtin_memmove(buffer + 128, node->right->data, sizeof(node->right->data));
        
        /* Complex goto pattern */
        if (level % 4 == 0) {
            goto skip_memset;
        }
        
        /* Clear part of buffer */
        __builtin_memset(buffer + 64, g_init_value, 32);
        
        skip_memset:
        /* Copy back modified data */
        __builtin_memcpy(node->right->data, buffer + 128, 32);
    }
    
    /* Calculate hash from buffer */
    for (int i = 0; i < 64; i++) {
        hash = (hash * 31) + buffer[i];
    }
    
    /* Process right subtree */
    hash += process_ast(node->right, level + 1);
    
    return hash;
}

/* Free AST with memory operations */
void free_ast(ASTNode* node) {
    if (!node) return;
    
    volatile char cleanup[128];
    
    /* Copy node data before freeing */
    __builtin_memcpy(cleanup, node->data, 32);
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

/* Main execution with OpenMP parallel section */
int main(void) {
    const char* init_data = "AST_Node_Initialization_Data_String_For_Memory_Operations_1234567890";
    uint64_t total_hash = 0;
    
    /* Create complex AST */
    ASTNode* root = create_ast(5, init_data);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* OpenMP parallel region to trigger thread-aware ASAN paths */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        volatile char thread_buffer[512];
        
        /* Each thread performs memory operations */
        #pragma omp for reduction(+:total_hash)
        for (int i = 0; i < 4; i++) {
            /* Initialize thread buffer */
            __builtin_memset(thread_buffer, thread_id + i, 256);
            
            /* Copy AST data to thread buffer with goto */
            if (i % 2 == 0) {
                goto copy_operation;
            }
            
            __builtin_memcpy(thread_buffer + 256, root->data, 64);
            
            copy_operation:
            /* Move data around */
            __builtin_memmove(thread_buffer, thread_buffer + 128, 128);
            
            /* Calculate thread-local hash */
            uint64_t local_hash = 0;
            for (int j = 0; j < 256; j++) {
                local_hash = (local_hash * 17) + thread_buffer[j];
            }
            
            total_hash += local_hash;
        }
        
        /* Barrier with memory operation */
        #pragma omp barrier
        
        /* Post-barrier memory operation */
        volatile char sync_buffer[64];
        __builtin_memset(sync_buffer, 0xFF, sizeof(sync_buffer));
    }
    
    /* Process AST sequentially */
    uint64_t ast_hash = process_ast(root, 0);
    total_hash += ast_hash;
    
    /* Additional memory operations in main */
    volatile char final_buffer[1024];
    
    /* Chain of memory operations */
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, init_data, strlen(init_data));
    __builtin_memmove(final_buffer + 512, final_buffer, 256);
    __builtin_memset(final_buffer + 768, g_init_value, 128);
    
    /* Final hash calculation */
    for (int i = 0; i < 256; i++) {
        total_hash = (total_hash * 13) + final_buffer[i];
    }
    
    /* Cleanup */
    free_ast(root);
    
    /* Print result to prevent optimization */
    printf("Total hash: %llu\n", (unsigned long long)total_hash);
    
    return 0;
}
