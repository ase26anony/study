/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "asan", "hwasan"};
static const int token_count = 5;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char init_buf[128];
    /* Force builtin usage in constructor */
    __builtin_memset(init_buf, 0xAA, sizeof(init_buf));
    __builtin_memcpy(init_buf + 64, init_buf, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern */
    for (int i = 0; i < 255; i++) {
        node->data[i] = (char)((id + i) % 256);
    }
    node->data[255] = '\0';
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = NULL;
        
        create_children:
        /* Jump target with memmove */
        volatile char temp[256];
        size_t copy_size = (size_t)(g_mem_size % 128) + 64;
        
        if (g_use_memmove) {
            __builtin_memmove(temp, node->data, copy_size);
            __builtin_memmove(node->data, temp, copy_size);
        }
        
        node->right = create_ast(depth - 1, id * 2 + 1);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with complex control flow and builtins */
static void process_ast(ASTNode* root, int* result) {
    if (!root) return;
    
    volatile int local_result = 0;
    volatile char buffer[512];
    
    /* Label for goto jumps */
    process_node:
    
    /* Use all three builtins */
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, root->data, strlen(root->data) + 1);
    
    if (root->left && root->right) {
        /* Overlapping memory with memmove */
        size_t overlap_size = (size_t)(root->id % 128) + 64;
        __builtin_memmove(root->left->data + 32, root->right->data, overlap_size);
        
        /* Jump back based on condition */
        if (root->id % 7 == 0) {
            root = root->left;
            goto process_node;
        }
    }
    
    /* Compute hash */
    for (int i = 0; i < 256 && root->data[i]; i++) {
        local_result ^= root->data[i] << ((i % 4) * 8);
    }
    
    *result ^= local_result;
    
    process_ast(root->left, result);
    process_ast(root->right, result);
}

/* Parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        volatile char thread_buf[1024];
        volatile size_t sizes[3] = {128, 256, 512};
        
        /* Each thread uses different builtins */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(thread_buf, thread_id, sizes[thread_id % 3]);
                break;
            case 1:
                __builtin_memcpy(thread_buf + 128, thread_buf, sizes[thread_id % 3]);
                break;
            case 2:
                __builtin_memmove(thread_buf + 256, thread_buf, sizes[thread_id % 3]);
                break;
        }
        
        /* Barrier with memory operation */
        #pragma omp barrier
        
        /* All threads do memmove after barrier */
        if (thread_id % 2 == 0) {
            __builtin_memmove(thread_buf, thread_buf + 384, 128);
        }
    }
}

/* Main execution flow */
int main(void) {
    int final_result = 0xDEADBEEF;
    
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast(4, 1);
    if (root) {
        process_ast(root, &final_result);
    }
    
    /* Phase 2: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 3: Token processing with builtins */
    volatile char token_buffer[1024];
    volatile char* current = token_buffer;
    
    for (int i = 0; i < token_count; i++) {
        size_t len = strlen(tokens[i]) + 1;
        __builtin_memcpy(current, tokens[i], len);
        current += len;
        
        /* Every third token triggers memmove */
        if (i % 3 == 2) {
            __builtin_memmove(token_buffer + 128, token_buffer, 256);
        }
    }
    
    /* Final memset to ensure all builtins used */
    __builtin_memset(token_buffer + 768, 0xCC, 256);
    
    /* Cleanup */
    /* Note: In real code, would need proper AST freeing */
    
    printf("Final result: 0x%08X\n", final_result);
    printf("Test completed - check ASAN/HWASAN instrumentation\n");
    
    return final_result == 0xDEADBEEF ? 0 : 1;
}
