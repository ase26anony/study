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
static const int token_count = sizeof(tokens)/sizeof(tokens[0]);

/* Constructor/destructor functions */
__attribute__((constructor))
static void init_asan_env(void) {
    printf("Initializing ASAN environment...\n");
}

__attribute__((destructor))
static void cleanup_asan_env(void) {
    printf("Cleaning up ASAN environment...\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (id % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth-1, id*2);
        node->right = create_ast(depth-1, id*2+1);
        
        if (use_goto) {
            create_children:
            /* This goto tests flow sensitivity around memmove */
            volatile int flag = 1;
            if (flag) {
                ASTNode* temp = create_ast(depth-2, id*3);
                if (temp) {
                    /* Use __builtin_memmove with goto context */
                    __builtin_memmove(&node->left, &temp, sizeof(ASTNode*));
                    free(temp);
                }
                node->right = create_ast(depth-1, id*2+1);
            }
        }
    }
    
    return node;
}

/* Function with complex memory operations */
static void process_ast(ASTNode* root, char* buffer, size_t buf_size) {
    if (!root || !buffer) return;
    
    /* Copy node data using builtins */
    __builtin_memcpy(buffer, root->data, 64);
    
    if (root->left) {
        /* Conditional memmove with volatile control */
        volatile int do_move = g_use_memmove;
        if (do_move) {
            char temp[64];
            __builtin_memcpy(temp, root->left->data, 64);
            __builtin_memmove(buffer + 64, temp, 64);
        }
    }
    
    /* Process children recursively */
    process_ast(root->left, buffer + 128, buf_size - 128);
    process_ast(root->right, buffer + 192, buf_size - 192);
}

/* OpenMP parallel memory operations */
static void parallel_mem_ops(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        size_t size = g_mem_size * (tid + 1);
        
        buffers[tid] = (char*)malloc(size);
        if (!buffers[tid]) {
            #pragma omp critical
            printf("Thread %d: Allocation failed\n", tid);
            return;
        }
        
        /* Each thread uses different builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(buffers[tid], tid, size);
                break;
            case 1:
                if (tid > 0) {
                    __builtin_memcpy(buffers[tid], buffers[tid-1], 
                                   size < g_mem_size * tid ? size : g_mem_size * tid);
                }
                break;
            case 2:
                __builtin_memmove(buffers[tid], buffers[tid] + size/2, size/2);
                break;
        }
        
        /* Verify with regular memcpy to ensure ASAN intercepts */
        char verify[256];
        memcpy(verify, buffers[tid], size < 256 ? size : 256);
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        if (buffers[i]) free(buffers[i]);
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize data */
    const size_t total_size = 4096;
    char* main_buffer = (char*)malloc(total_size);
    if (!main_buffer) return 1;
    
    /* Force initialization of asan_memfn_rtls cache */
    __builtin_memset(main_buffer, 0, total_size);
    __builtin_memcpy(main_buffer, tokens, sizeof(tokens));
    
    /* Create recursive structure */
    ASTNode* ast_root = create_ast(4, 1);
    
    /* Process with memory operations */
    process_ast(ast_root, main_buffer, total_size);
    
    /* Execute parallel operations */
    parallel_mem_ops();
    
    /* Additional builtin usage with volatile control */
    volatile size_t dynamic_size = g_mem_size;
    char* dynamic_buf = (char*)malloc(dynamic_size);
    if (dynamic_buf) {
        __builtin_memset(dynamic_buf, 0xFF, dynamic_size);
        
        /* Test memmove with overlapping regions */
        for (size_t i = 0; i < dynamic_size/2; i += 64) {
            __builtin_memmove(dynamic_buf + i, dynamic_buf + dynamic_size/2 + i, 32);
        }
        
        free(dynamic_buf);
    }
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < total_size && i < 1024; i++) {
        hash = hash * 31 + main_buffer[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    
    /* Cleanup */
    free(main_buffer);
    
    /* Free AST recursively */
    void free_ast(ASTNode* node) {
        if (!node) return;
        free_ast(node->left);
        free_ast(node->right);
        free(node);
    }
    free_ast(ast_root);
    
    printf("Test completed successfully.\n");
    return 0;
}
