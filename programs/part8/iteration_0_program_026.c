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
char global_tokens[1024];
int token_index = 0;

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_global_tokens(void) {
    /* Use __builtin_memset in constructor */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    global_tokens[sizeof(global_tokens)-1] = '\0';
}

__attribute__((destructor)) static void cleanup_global_tokens(void) {
    /* Use __builtin_memset in destructor */
    __builtin_memset(global_tokens, 0, sizeof(global_tokens));
}

/* Recursive function with memory operations */
ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with __builtin_memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Create pattern in data using __builtin_memcpy */
    char pattern[32];
    __builtin_memset(pattern, '0' + (id % 10), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(pattern));
    
    /* Recursive creation with goto for control flow */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth-1, id*2);
    node->right = create_ast(depth-1, id*2+1);
    
    /* Copy between child nodes using __builtin_memmove with goto */
    if (node->left && node->right && use_memmove) {
        goto perform_memmove;
    }
    
done:
    return node;

perform_memmove:
    /* This tests the memmove redirection with goto jumping into block */
    __builtin_memmove(node->left->data + 32, node->right->data, 32);
    goto done;
}

/* Function with OpenMP parallel section */
void parallel_memory_operations(ASTNode* root) {
    if (!root) return;
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buffer[128];
        
        /* Each thread uses builtins with volatile lengths */
        size_t len = volatile_len / (thread_id + 1);
        if (len > sizeof(local_buffer)) len = sizeof(local_buffer);
        
        /* Force memcpy redirection */
        __builtin_memcpy(local_buffer, root->data, len);
        
        /* Force memset redirection */
        __builtin_memset(local_buffer + len/2, thread_id + '0', len/2);
        
        /* Conditional memmove with goto */
        if (thread_id % 2 == 0) {
            goto use_memmove;
        }
        
        #pragma omp barrier
        
        /* Copy back to global */
        #pragma omp critical
        {
            __builtin_memcpy(global_tokens + token_index, local_buffer, len);
            token_index += len;
        }
        
        continue_normal:
        return;
        
    use_memmove:
        /* Jump target for memmove testing */
        char temp[128];
        __builtin_memmove(temp, local_buffer, len);
        __builtin_memmove(local_buffer, temp, len);
        goto continue_normal;
    }
}

/* Complex dispatch function */
void memory_dispatch_engine(ASTNode* nodes[], int count) {
    for (int i = 0; i < count; i++) {
        if (!nodes[i]) continue;
        
        /* Vary memory operations based on node properties */
        switch (nodes[i]->id % 3) {
            case 0:
                __builtin_memset(nodes[i]->data, 'X', 64);
                break;
            case 1:
                if (i > 0 && nodes[i-1]) {
                    __builtin_memcpy(nodes[i]->data, nodes[i-1]->data, 64);
                }
                break;
            case 2:
                if (i > 0 && nodes[i-1]) {
                    __builtin_memmove(nodes[i]->data + 32, nodes[i]->data, 32);
                    __builtin_memmove(nodes[i]->data, nodes[i-1]->data, 32);
                }
                break;
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create AST structure */
    ASTNode* root = create_ast(4, 1);
    
    /* Create array of nodes for dispatch */
    ASTNode* node_array[8];
    node_array[0] = root;
    for (int i = 1; i < 8; i++) {
        node_array[i] = create_ast(3, i+1);
    }
    
    /* Execute parallel operations */
    parallel_memory_operations(root);
    
    /* Execute dispatch engine */
    memory_dispatch_engine(node_array, 8);
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < token_index && i < sizeof(global_tokens); i++) {
        hash = hash * 31 + global_tokens[i];
    }
    
    /* Also hash node data */
    for (int i = 0; i < 8; i++) {
        if (node_array[i]) {
            for (int j = 0; j < 64; j++) {
                hash = hash * 31 + node_array[i]->data[j];
            }
        }
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Token index: %d\n", token_index);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free(node_array[i]);
    }
    
    return 0;
}
