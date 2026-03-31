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
static const char* tokens[] = {"memcpy", "memset", "memmove", "test", "data"};
static const int token_count = 5;

/* Constructor/destructor functions */
__attribute__((constructor)) static void init_globals(void) {
    printf("Initializing ASAN test environment\n");
}

__attribute__((destructor)) static void cleanup_globals(void) {
    printf("Cleaning up ASAN test environment\n");
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_node(int id) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = node->right = NULL;
    
    /* Use __builtin_memset to initialize */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill with pattern */
    snprintf(node->data, sizeof(node->data), "Node_%d_Data", id);
    
    return node;
}

static void copy_node_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Force __builtin_memcpy usage */
    __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    
    /* Conditional memmove with goto */
    if (g_use_memmove) {
        char buffer[128];
        __builtin_memset(buffer, 'X', sizeof(buffer));
        
        /* Jump into block with memmove */
        goto memmove_block;
        
        memmove_block:
        __builtin_memmove(buffer + 32, buffer, 64);
        __builtin_memcpy(dest->data + 16, buffer + 32, 32);
    }
}

static int traverse_and_hash(ASTNode* node) {
    if (!node) return 0;
    
    int hash = node->id;
    char temp[64];
    
    /* Use volatile-controlled size */
    size_t copy_size = g_mem_size % sizeof(temp);
    if (copy_size > 0) {
        __builtin_memcpy(temp, node->data, copy_size);
        
        /* XOR each character */
        for (size_t i = 0; i < copy_size; i++) {
            hash ^= temp[i];
        }
    }
    
    hash ^= traverse_and_hash(node->left);
    hash ^= traverse_and_hash(node->right);
    
    return hash;
}

/* Parallel memory operation dispatcher */
static void parallel_mem_operations(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[512];
        char src_buf[512];
        
        /* Initialize source with pattern */
        for (int i = 0; i < sizeof(src_buf); i++) {
            src_buf[i] = (i + tid) & 0xFF;
        }
        
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            /* Mix different builtins */
            switch (i % 3) {
                case 0:
                    __builtin_memcpy(local_buf, src_buf, 
                                    (g_mem_size + i) % sizeof(local_buf));
                    break;
                case 1:
                    __builtin_memset(local_buf + i, tid, 
                                    (g_mem_size + tid) % 128);
                    break;
                case 2:
                    __builtin_memmove(local_buf, local_buf + 64, 128);
                    break;
            }
            
            /* Conditional goto around memmove */
            if (i % 7 == 0) {
                goto skip_memmove;
            }
            
            __builtin_memmove(local_buf + 128, local_buf, 64);
            
            skip_memmove:
            /* Use result to prevent elimination */
            local_buf[0] ^= local_buf[255];
        }
    }
}

/* Complex initialization with token processing */
static ASTNode* build_tree_from_tokens(int depth, int* token_idx) {
    if (depth <= 0 || *token_idx >= token_count) {
        return NULL;
    }
    
    ASTNode* node = create_node(depth * 100 + *token_idx);
    if (!node) return NULL;
    
    /* Copy token into node data */
    const char* token = tokens[*token_idx];
    size_t len = strlen(token);
    __builtin_memcpy(node->data, token, len < sizeof(node->data) ? len : sizeof(node->data)-1);
    
    (*token_idx)++;
    
    /* Build left/right subtrees */
    node->left = build_tree_from_tokens(depth - 1, token_idx);
    node->right = build_tree_from_tokens(depth - 1, token_idx);
    
    /* Copy between nodes if both exist */
    if (node->left && node->right) {
        copy_node_data(node->left, node->right);
        
        /* Also test reverse copy */
        char temp[64];
        __builtin_memcpy(temp, node->left->data, sizeof(temp));
        __builtin_memcpy(node->right->data, temp, sizeof(temp));
    }
    
    return node;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Tree construction and manipulation */
    int token_idx = 0;
    ASTNode* root = build_tree_from_tokens(4, &token_idx);
    
    if (!root) {
        fprintf(stderr, "Failed to build tree\n");
        return 1;
    }
    
    /* Phase 2: Parallel memory operations */
    parallel_mem_operations();
    
    /* Phase 3: Verification via tree traversal */
    int final_hash = traverse_and_hash(root);
    
    /* Additional memory operations with volatile control */
    char* dynamic_buf = (char*)malloc(1024);
    if (dynamic_buf) {
        volatile size_t op_size = g_mem_size;
        
        __builtin_memset(dynamic_buf, 0xAA, op_size % 1024);
        
        char* dynamic_buf2 = (char*)malloc(512);
        if (dynamic_buf2) {
            __builtin_memcpy(dynamic_buf2, dynamic_buf, 256);
            
            /* Force memmove with overlapping regions */
            __builtin_memmove(dynamic_buf + 128, dynamic_buf, 384);
            
            free(dynamic_buf2);
        }
        
        free(dynamic_buf);
    }
    
    /* Print verification result */
    printf("Final hash: %d\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real code, would need proper tree freeing */
    
    return 0;
}
