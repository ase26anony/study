/* ISO C99-compliant test program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_tokens[][32] = {
    "memcpy_test", "memset_test", "memmove_test",
    "asan_check", "hwasan_check", "parallel_exec"
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_env(void) {
    /* Force initialization of sanitizer runtime */
    volatile char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_sanitizer_env(void) {
    /* Final memory operation to ensure cleanup paths are taken */
    volatile char final_buf[8];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* token) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token using __builtin_memcpy */
    size_t len = strlen(token);
    if (len > sizeof(node->data) - 1)
        len = sizeof(node->data) - 1;
    __builtin_memcpy(node->data, token, len);
    node->data[len] = '\0';
    
    node->id = depth;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_left = (depth % 2 == 0);
        
        if (use_left) {
            node->left = create_ast(depth - 1, g_tokens[depth % 6]);
            goto skip_right;  /* Jump over right creation */
        }
        
        node->right = create_ast(depth - 1, g_tokens[depth % 6]);
        goto done;
        
    skip_right:
        node->right = NULL;
    done:
        ;
    }
    
    return node;
}

/* Function with goto jumping into memmove block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int condition = src->id % 3;
    
    if (condition == 0) {
        goto direct_copy;
    } else if (condition == 1) {
        goto skip_operation;
    }
    
    /* This block contains __builtin_memmove */
    {
        char temp[64];
        __builtin_memcpy(temp, src->data, sizeof(temp));
        __builtin_memmove(dst->data, temp, sizeof(dst->data));
        goto finish;
    }
    
direct_copy:
    __builtin_memcpy(dst->data, src->data, sizeof(dst->data));
    goto finish;
    
skip_operation:
    /* No operation */
    ;
    
finish:
    /* Additional memmove with overlapping regions */
    char overlap_buf[128];
    volatile size_t overlap_size = g_mem_size % 64;
    
    __builtin_memset(overlap_buf, 'A', sizeof(overlap_buf));
    __builtin_memmove(overlap_buf + 32, overlap_buf, overlap_size);
}

/* Parallel memory dispatch logic */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[256];
        char shared_buf[256];
        
        /* Use volatile to prevent optimization */
        volatile size_t op_size = g_mem_size;
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(local_buf, thread_id, op_size % sizeof(local_buf));
                __builtin_memcpy(shared_buf, local_buf, op_size % sizeof(shared_buf));
                break;
            case 1:
                __builtin_memcpy(local_buf, g_tokens[thread_id % 6], 
                                strlen(g_tokens[thread_id % 6]) + 1);
                __builtin_memmove(shared_buf, local_buf, op_size % sizeof(shared_buf));
                break;
            case 2:
                __builtin_memset(shared_buf, 0xFF, op_size % sizeof(shared_buf));
                __builtin_memmove(local_buf, shared_buf, op_size % sizeof(local_buf));
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Final memory operation in parallel region */
        #pragma omp for
        for (int i = 0; i < 16; i++) {
            char loop_buf[64];
            __builtin_memset(loop_buf, i, sizeof(loop_buf));
            __builtin_memcpy(&shared_buf[i * 4], loop_buf, 4);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create recursive AST structure */
    ASTNode* root = create_ast(4, g_tokens[0]);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Create destination node */
    ASTNode* dest = (ASTNode*)malloc(sizeof(ASTNode));
    if (!dest) {
        free(root);
        return 1;
    }
    
    __builtin_memset(dest, 0, sizeof(ASTNode));
    
    /* Test goto flow with memory operations */
    process_with_goto(root, dest);
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Additional memory operations with volatile control */
    volatile char* dynamic_buf = (char*)malloc(g_mem_size);
    if (dynamic_buf) {
        __builtin_memset(dynamic_buf, 0x42, g_mem_size);
        
        char verify_buf[256];
        size_t copy_size = g_mem_size > sizeof(verify_buf) ? 
                          sizeof(verify_buf) : g_mem_size;
        
        __builtin_memcpy(verify_buf, dynamic_buf, copy_size);
        __builtin_memmove(dynamic_buf + 128, dynamic_buf, 
                         g_mem_size > 128 ? 128 : g_mem_size);
        
        free(dynamic_buf);
    }
    
    /* Calculate and print verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 6; i++) {
        for (size_t j = 0; j < strlen(g_tokens[i]); j++) {
            hash = hash * 31 + g_tokens[i][j];
        }
    }
    
    hash += root->id + dest->id;
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(dest);
    
    /* Recursive free (simplified for example) */
    free(root->left);
    free(root->right);
    free(root);
    
    return 0;
}
