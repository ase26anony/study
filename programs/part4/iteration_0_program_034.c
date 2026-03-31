/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_trigger = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
char global_tokens[1024];
int token_hash = 0;

/* Constructor/destructor for initialization coordination */
void __attribute__((constructor)) init_global_data(void) {
    /* Initialize with pattern */
    for (int i = 0; i < 1024; i++) {
        global_tokens[i] = (i % 256);
    }
}

void __attribute__((destructor)) cleanup_data(void) {
    /* Verify operations by computing hash */
    printf("Token hash: %d\n", token_hash);
}

/* Recursive parser with memory operations */
ASTNode* create_node(int id) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = id;
    node->left = node->right = NULL;
    
    /* Use __builtin_memset with volatile length */
    size_t len = volatile_len % 256;
    __builtin_memset(node->data, id, len);
    
    return node;
}

void copy_node_data(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Force __builtin_memcpy with non-constant size */
    size_t copy_len = volatile_len % sizeof(dest->data);
    if (copy_len > 0) {
        __builtin_memcpy(dest->data, src->data, copy_len);
    }
}

/* Function with goto jumps around memory operations */
void goto_memmove_test(char* buf1, char* buf2) {
    int use_memmove = volatile_trigger;
    
    if (use_memmove) {
        goto do_memmove;
    } else {
        goto skip_memmove;
    }
    
do_memmove:
    /* This should trigger __builtin_memmove redirection */
    __builtin_memmove(buf1, buf2, 128);
    goto after_memmove;
    
skip_memmove:
    __builtin_memset(buf1, 0, 128);
    
after_memmove:
    /* Compute partial hash */
    for (int i = 0; i < 128; i++) {
        token_hash += buf1[i];
    }
}

/* OpenMP parallel section with memory operations */
void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[256];
        char shared_buf[256];
        
        /* Initialize with __builtin_memset */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Copy between buffers using __builtin_memcpy */
        if (thread_id % 2 == 0) {
            __builtin_memcpy(shared_buf, local_buf, 128);
        } else {
            __builtin_memcpy(local_buf, shared_buf, 128);
        }
        
        #pragma omp atomic
        token_hash += local_buf[0];
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Basic built-in calls */
    char buffer1[512];
    char buffer2[512];
    
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 100, buffer1, 200);
    
    /* Phase 2: Recursive structure operations */
    ASTNode* root = create_node(1);
    ASTNode* child1 = create_node(2);
    ASTNode* child2 = create_node(3);
    
    if (root && child1 && child2) {
        root->left = child1;
        root->right = child2;
        
        copy_node_data(child2, child1);
        copy_node_data(root, child2);
        
        /* Complex memory rearrangement */
        __builtin_memmove(root->data + 50, child1->data, 100);
    }
    
    /* Phase 3: Goto flow control */
    goto_memmove_test(buffer1, buffer2);
    
    /* Phase 4: OpenMP parallel section */
    parallel_memory_ops();
    
    /* Phase 5: Mixed operations in loop */
    for (int i = 0; i < 10; i++) {
        volatile_len = (i * 32) + 16;
        
        if (i % 3 == 0) {
            __builtin_memset(global_tokens + i * 64, i, 64);
        } else if (i % 3 == 1) {
            __builtin_memcpy(global_tokens + i * 64, 
                           global_tokens + (i-1) * 64, 64);
        } else {
            __builtin_memmove(global_tokens + i * 64 + 16,
                            global_tokens + i * 64, 48);
        }
    }
    
    /* Final hash computation */
    for (int i = 0; i < 1024; i++) {
        token_hash += global_tokens[i];
    }
    
    /* Cleanup */
    free(root);
    free(child1);
    free(child2);
    
    printf("Test completed. Final hash: %d\n", token_hash);
    return 0;
}
