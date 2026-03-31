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
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    volatile char buffer[128];
    /* Force __builtin_memset redirection in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    printf("Constructor: Initialized buffer with memset\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    volatile char cleanup_buf[64];
    /* Force __builtin_memcpy redirection in destructor */
    __builtin_memcpy(cleanup_buf, "DESTRUCTOR", 10);
    printf("Destructor: Cleanup operations completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy with volatile length */
    size_t copy_len = volatile_len % 128;
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data[copy_len] = '\0';
    
    /* Use __builtin_memset for padding */
    __builtin_memset(node->data + copy_len, 0xCC, sizeof(node->data) - copy_len);
    node->size = sizeof(node->data);
    
    /* Recursive creation with goto for control flow */
    int use_left = 1;
    
    if (depth > 2) {
        use_left = 0;
        goto skip_left;
    }
    
    node->left = create_ast(depth - 1, "LEFT_CHILD");
    
skip_left:
    if (use_left) {
        /* This path uses __builtin_memmove with goto */
        char temp[256];
        __builtin_memcpy(temp, node->data, node->size);
        goto move_data;
    }
    
move_data:
    if (!use_left) {
        __builtin_memmove(node->data + 32, node->data, 64);
    }
    
    node->right = create_ast(depth - 1, "RIGHT_CHILD");
    
    return node;
}

/* Function with complex control flow and memory operations */
static void process_ast_with_goto(ASTNode* node) {
    if (!node) return;
    
    volatile int jump_flag = volatile_selector;
    
    if (jump_flag == 0) {
        goto block1;
    } else if (jump_flag == 1) {
        goto block2;
    } else {
        goto block3;
    }
    
block1:
    {
        char local_buf[512];
        /* Force __builtin_memcpy redirection */
        __builtin_memcpy(local_buf, node->data, node->size);
        goto process;
    }
    
block2:
    {
        char local_buf[512];
        /* Force __builtin_memset redirection */
        __builtin_memset(local_buf, 0xDD, sizeof(local_buf));
        goto process;
    }
    
block3:
    {
        char local_buf[512];
        /* Force __builtin_memmove redirection with overlapping regions */
        __builtin_memcpy(local_buf, node->data, node->size);
        __builtin_memmove(local_buf + 128, local_buf, 256);
        goto process;
    }
    
process:
    /* Process children */
    process_ast_with_goto(node->left);
    process_ast_with_goto(node->right);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        volatile char thread_buf[1024];
        
        /* Each thread uses different builtins */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(thread_buf, thread_id, sizeof(thread_buf));
                break;
            case 1:
                __builtin_memcpy(thread_buf, "THREAD_DATA", 11);
                break;
            case 2:
                __builtin_memmove(thread_buf + 512, thread_buf, 512);
                break;
        }
        
        #pragma omp barrier
        
        /* All threads use memmove after barrier */
        __builtin_memmove(thread_buf, thread_buf + 256, 256);
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create complex AST structure */
    ASTNode* root = create_ast(4, "ROOT_NODE_DATA");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto control flow */
    process_ast_with_goto(root);
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Additional direct builtin calls */
    volatile char final_buf[2048];
    volatile size_t len = volatile_len;
    
    /* Force all three builtins in sequence */
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf + 512, root->data, len % 256);
    __builtin_memmove(final_buf, final_buf + 1024, 512);
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(final_buf); i++) {
        hash = hash * 31 + final_buf[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Recursive free omitted for brevity */
    
    return 0;
}
