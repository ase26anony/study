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
static char g_tokens[][32] = {
    "token1", "token2", "token3", "token4",
    "token5", "token6", "token7", "token8"
};

/* Constructor attribute - forces early initialization */
__attribute__((constructor))
static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force __builtin_memset initialization */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    printf("[Constructor] Initialized buffer\n");
}

/* Destructor attribute - tests cleanup paths */
__attribute__((destructor))
static void cleanup_asan_late(void) {
    printf("[Destructor] Cleaning up\n");
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
    char pattern[32];
    __builtin_memset(pattern, 'A' + (id % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(pattern));
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = NULL;
        
        create_children:
        /* Jump target with __builtin_memmove */
        if (g_use_memmove) {
            ASTNode temp;
            __builtin_memcpy(&temp, node, sizeof(ASTNode));
            __builtin_memmove(node->data, temp.data + 8, 32);
        }
        
        if (!use_goto) {
            node->right = create_ast(depth - 1, id * 2 + 1);
        }
    }
    
    return node;
}

/* Function with OpenMP parallel section */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        volatile char local_buf[256];
        
        /* Each thread uses different builtins */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(local_buf, thread_id, g_mem_size);
                break;
            case 1:
                __builtin_memcpy(local_buf, g_tokens[thread_id % 8], 32);
                break;
            case 2:
                __builtin_memmove(local_buf + 32, local_buf, 64);
                break;
        }
        
        #pragma omp barrier
        
        /* Cross-thread memory operation */
        #pragma omp single
        {
            volatile char shared_buf[512];
            __builtin_memset(shared_buf, 0xCC, sizeof(shared_buf));
        }
    }
}

/* Complex function with goto jumping around mem operations */
static int process_tokens_with_goto(void) {
    char buffer[1024];
    int sum = 0;
    int i = 0;
    
    start_loop:
    if (i >= 8) goto end_processing;
    
    /* Copy token to buffer */
    __builtin_memcpy(buffer + i * 32, g_tokens[i], 32);
    
    if (i == 3) {
        /* Jump over memset */
        goto skip_memset;
    }
    
    /* Clear part of buffer */
    __builtin_memset(buffer + i * 32 + 16, 0, 16);
    
    skip_memset:
    if (i == 5) {
        /* Use memmove with overlap */
        __builtin_memmove(buffer + 200, buffer + 100, 100);
        goto fast_forward;
    }
    
    i++;
    goto start_loop;
    
    fast_forward:
    i = 7;
    goto start_loop;
    
    end_processing:
    
    /* Calculate hash/sum */
    for (int j = 0; j < 256; j++) {
        sum += buffer[j];
    }
    
    return sum;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create recursive structure */
    ASTNode* root = create_ast(4, 1);
    
    /* Perform memory operations between nodes */
    if (root && root->left && root->right) {
        /* Test __builtin_memcpy between structures */
        __builtin_memcpy(root->right->data, root->left->data, 128);
        
        /* Test __builtin_memmove with overlap */
        __builtin_memmove(root->data + 64, root->data, 128);
    }
    
    /* Execute parallel section */
    parallel_memory_operations();
    
    /* Process tokens with goto flow */
    int result = process_tokens_with_goto();
    printf("Token processing result: %d\n", result);
    
    /* Additional builtin calls in different contexts */
    volatile char final_buf[1024];
    
    /* Chain of memory operations */
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf + 256, g_tokens[0], 256);
    __builtin_memmove(final_buf + 512, final_buf + 256, 256);
    
    /* Verify by calculating checksum */
    unsigned long checksum = 0;
    for (size_t i = 0; i < sizeof(final_buf); i++) {
        checksum += final_buf[i];
    }
    printf("Final checksum: %lu\n", checksum);
    
    /* Cleanup */
    free(root);
    
    printf("Test completed successfully\n");
    return 0;
}
