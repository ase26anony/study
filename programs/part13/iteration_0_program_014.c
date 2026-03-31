/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static volatile size_t token_offset = 0;

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_tokens(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)((i * 31) & 0xFF);
    }
}

__attribute__((destructor)) static void cleanup(void) {
    /* Verify token pool wasn't corrupted */
    size_t sum = 0;
    for (size_t i = 0; i < sizeof(token_pool); i++) {
        sum += (unsigned char)token_pool[i];
    }
    printf("Token pool checksum: %zu\n", sum);
}

/* Recursive function with memory operations */
static ASTNode* create_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy pattern into node using __builtin_memcpy */
    size_t copy_len = volatile_len % sizeof(node->data);
    __builtin_memcpy(node->data, token_pool + token_offset, copy_len);
    token_offset = (token_offset + copy_len) % sizeof(token_pool);
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int create_left = 1;
        
        /* Goto jumping into block with memory operation */
        if (id % 3 == 0) goto create_children;
        
        create_left = 0;
        
    create_children:
        if (create_left) {
            node->left = create_node(depth - 1, id * 2);
            
            /* Use __builtin_memmove between nodes if they exist */
            if (node->left && depth > 2) {
                ASTNode* temp = create_node(1, -1);
                if (temp) {
                    __builtin_memmove(temp->data, node->left->data, 
                                     sizeof(node->left->data));
                    free(temp);
                }
            }
        }
        
        node->right = create_node(depth - 1, id * 2 + 1);
        
        /* Jump out of block */
        if (id % 5 == 0) goto finish_node;
    }
    
    /* Another goto target */
    if (id % 7 == 0) {
        /* Additional memory operation after jump */
        char buffer[128];
        __builtin_memset(buffer, id, sizeof(buffer));
    }
    
finish_node:
    return node;
}

/* Function with OpenMP parallel section */
static void parallel_memory_operations(void) {
    const int num_ops = 100;
    char* buffers[100];
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < num_ops; i++) {
            buffers[i] = (char*)malloc(volatile_len + i);
            if (buffers[i]) {
                /* Force all three builtins in parallel region */
                __builtin_memset(buffers[i], thread_id, volatile_len + i);
                
                if (i > 0) {
                    __builtin_memcpy(buffers[i], buffers[i-1], 
                                   volatile_len + (i < volatile_len ? i : 0));
                }
                
                if (use_memmove && i % 3 == 0) {
                    __builtin_memmove(buffers[i] + 1, buffers[i], 
                                     volatile_len + i - 1);
                }
            }
        }
        
        #pragma omp for
        for (int i = 0; i < num_ops; i++) {
            if (buffers[i]) {
                free(buffers[i]);
            }
        }
    }
}

/* Complex function with varied memory operations */
static size_t process_ast(ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 0;
    char temp_buffer[512];
    
    /* Mix of memory operations */
    __builtin_memset(temp_buffer, 0, sizeof(temp_buffer));
    __builtin_memcpy(temp_buffer, node->data, sizeof(node->data));
    
    /* Conditional memmove with goto */
    if (node->id % 4 == 0) {
        goto do_memmove;
    }
    
    /* Skip memmove for some nodes */
    if (node->id % 2 == 0) {
        goto skip_memmove;
    }
    
do_memmove:
    if (node->left && node->right) {
        size_t move_len = volatile_len % sizeof(node->left->data);
        __builtin_memmove(node->right->data, node->left->data, move_len);
    }
    
skip_memmove:
    /* Compute hash from data */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        hash = hash * 31 + (unsigned char)node->data[i];
    }
    
    /* Recursive processing */
    hash += process_ast(node->left);
    hash += process_ast(node->right);
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create complex AST */
    ASTNode* root = create_node(5, 1);
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Process AST with memory operations */
    size_t total_hash = process_ast(root);
    printf("AST hash: %zu\n", total_hash);
    
    /* Additional memory operations in main */
    char final_buffer[1024];
    volatile size_t final_len = volatile_len % sizeof(final_buffer);
    
    __builtin_memset(final_buffer, 0xFF, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, token_pool, final_len);
    
    if (use_memmove) {
        __builtin_memmove(final_buffer + 256, final_buffer, 256);
    }
    
    /* Verify final buffer */
    size_t final_sum = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        final_sum += (unsigned char)final_buffer[i];
    }
    printf("Final buffer sum: %zu\n", final_sum);
    
    /* Cleanup */
    /* Note: Proper AST cleanup omitted for brevity */
    
    return 0;
}
