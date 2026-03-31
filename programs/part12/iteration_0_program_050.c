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

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "asan", "hwasan"};
static const int token_count = sizeof(tokens) / sizeof(tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_environment(void) {
    printf("Initializing ASAN environment...\n");
    /* Force early initialization of memory functions */
    char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan_environment(void) {
    printf("Cleaning up ASAN environment...\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern using memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (id % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        
        if (0) {
create_children:
            /* This block tests goto into memory operation context */
            volatile int flag = g_use_memmove;
            if (flag) {
                ASTNode temp;
                __builtin_memcpy(&temp, node, sizeof(ASTNode));
                __builtin_memmove(node->data, temp.data, sizeof(node->data));
            }
            node->left = create_ast(depth - 1, id * 2);
            node->right = NULL;
        }
    }
    
    return node;
}

/* Function with OpenMP parallel memory operations */
static void parallel_memory_operations(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Varied memory operations based on thread and index */
                size_t op_size = g_mem_size % 128;
                
                switch ((i + thread_id) % 3) {
                    case 0:
                        __builtin_memset(nodes[i]->data, thread_id, op_size);
                        break;
                    case 1: {
                        char buffer[128];
                        __builtin_memset(buffer, i, sizeof(buffer));
                        __builtin_memcpy(nodes[i]->data, buffer, op_size);
                        break;
                    }
                    case 2:
                        if (i > 0 && nodes[i-1]) {
                            __builtin_memmove(nodes[i]->data, 
                                            nodes[i-1]->data, 
                                            op_size);
                        }
                        break;
                }
            }
        }
        
        /* Barrier to ensure all memory ops complete */
        #pragma omp barrier
        
        /* Additional memory operations after barrier */
        #pragma omp single
        {
            char shared_buffer[256];
            __builtin_memset(shared_buffer, 0xFF, sizeof(shared_buffer));
            
            for (int i = 0; i < count && i < 10; i++) {
                if (nodes[i]) {
                    __builtin_memcpy(nodes[i]->data, 
                                   shared_buffer, 
                                   sizeof(nodes[i]->data));
                }
            }
        }
    }
}

/* Complex token processing with memory operations */
static int process_tokens(ASTNode* node) {
    int hash = 0;
    
    for (int i = 0; i < token_count; i++) {
        /* Copy token to temporary buffer */
        char buffer[32];
        size_t len = strlen(tokens[i]);
        
        __builtin_memset(buffer, 0, sizeof(buffer));
        __builtin_memcpy(buffer, tokens[i], len);
        
        /* Process token with potential memmove */
        if (i > 0 && len > 2) {
            __builtin_memmove(buffer + 1, buffer, len - 1);
        }
        
        /* Update hash */
        for (size_t j = 0; j < len && j < sizeof(buffer); j++) {
            hash = (hash * 31 + buffer[j]) % 1000000;
        }
        
        /* Conditional goto for flow control */
        if (hash % 7 == 0) {
            goto apply_to_node;
        }
        
        continue;
        
apply_to_node:
        if (node) {
            __builtin_memcpy(node->data + (i % 32), 
                           buffer, 
                           len % 32);
        }
    }
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create AST structure */
    ASTNode* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create array of nodes for parallel processing */
    ASTNode* nodes[8];
    nodes[0] = root;
    for (int i = 1; i < 8; i++) {
        nodes[i] = create_ast(3, i + 1);
    }
    
    /* Process tokens */
    int token_hash = process_tokens(root);
    printf("Token hash: %d\n", token_hash);
    
    /* Parallel memory operations */
    parallel_memory_operations(nodes, 8);
    
    /* Final verification with memory operations */
    int final_sum = 0;
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            /* Use all three builtins in verification */
            char verify_buffer[64];
            
            __builtin_memset(verify_buffer, 0, sizeof(verify_buffer));
            __builtin_memcpy(verify_buffer, nodes[i]->data, 32);
            
            if (i % 2 == 0) {
                __builtin_memmove(verify_buffer + 16, verify_buffer, 16);
            }
            
            for (int j = 0; j < 32; j++) {
                final_sum += verify_buffer[j];
            }
            
            free(nodes[i]);
        }
    }
    
    printf("Final checksum: %d\n", final_sum % 1000000);
    printf("Test completed successfully.\n");
    
    return 0;
}
