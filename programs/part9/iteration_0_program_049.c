/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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
static char g_token_pool[4096];
static volatile int g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i * 13) & 0xFF);
    }
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    printf("Destructor: ASAN environment cleaned up\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    
    /* Use __builtin_memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Fill with pattern using volatile length */
    volatile size_t fill_len = sizeof(node->data) / 2;
    __builtin_memset(node->data, node->id, fill_len);
    
    /* Create left subtree */
    node->left = create_ast(depth - 1, counter);
    
    /* Copy data between nodes if both children exist */
    if (node->left && depth > 2) {
        ASTNode* temp = malloc(sizeof(ASTNode));
        if (temp) {
            /* Use __builtin_memcpy with goto for flow control */
            goto copy_block;
copy_block:
            __builtin_memcpy(temp->data, node->left->data, 
                           sizeof(node->left->data));
            __builtin_memcpy(node->left->data, node->data,
                           sizeof(node->data));
            __builtin_memcpy(node->data, temp->data,
                           sizeof(temp->data));
            free(temp);
        }
    }
    
    /* Create right subtree */
    node->right = create_ast(depth - 1, counter);
    
    /* Use __builtin_memmove for overlapping regions */
    if (node->right) {
        char buffer[128];
        volatile size_t move_len = sizeof(buffer) / 4;
        
        __builtin_memcpy(buffer, node->right->data, move_len);
        __builtin_memmove(node->right->data, 
                         node->right->data + 16,
                         move_len - 16);
        __builtin_memcpy(node->right->data + move_len - 16,
                        buffer, 16);
    }
    
    return node;
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[512];
        char src_buf[512];
        
        /* Initialize source with pattern */
        for (int i = 0; i < sizeof(src_buf); i++) {
            src_buf[i] = (char)((i + thread_id * 31) & 0xFF);
        }
        
        /* Force built-in calls with volatile control */
        volatile size_t copy_size = g_mem_size + thread_id * 64;
        if (copy_size > sizeof(local_buf)) 
            copy_size = sizeof(local_buf);
        
        /* Critical built-in calls that should trigger ASAN redirection */
        #pragma omp critical
        {
            __builtin_memset(local_buf, thread_id, copy_size);
            __builtin_memcpy(local_buf + 128, src_buf, copy_size / 2);
            
            /* Overlapping memmove */
            volatile int offset = 64 + thread_id * 8;
            __builtin_memmove(local_buf + offset, 
                            local_buf + offset / 2,
                            copy_size / 4);
        }
        
        /* Verify operations */
        int sum = 0;
        for (size_t i = 0; i < copy_size; i++) {
            sum += local_buf[i];
        }
        
        #pragma omp atomic
        g_token_idx += sum & 0xFF;
    }
}

/* Complex control flow with goto around memory operations */
static void control_flow_test(void) {
    char buffer_a[256];
    char buffer_b[256];
    volatile int use_memmove = 1;
    
    /* Initialize buffers */
    for (int i = 0; i < sizeof(buffer_a); i++) {
        buffer_a[i] = (char)i;
        buffer_b[i] = (char)(255 - i);
    }
    
    /* Goto-based control flow */
    if (use_memmove) {
        goto memmove_block;
    } else {
        goto memcpy_block;
    }
    
memmove_block:
    {
        volatile size_t len = 128;
        __builtin_memmove(buffer_a + 64, buffer_a + 32, len);
        goto after_ops;
    }
    
memcpy_block:
    {
        volatile size_t len = 192;
        __builtin_memcpy(buffer_b, buffer_a, len);
        goto after_ops;
    }
    
after_ops:
    /* Mixed operations */
    __builtin_memset(buffer_a + 192, 0xFF, 32);
    __builtin_memcpy(buffer_b + 64, buffer_a + 128, 96);
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    int counter = 1;
    ASTNode* root = create_ast(4, &counter);
    
    if (root) {
        /* Traverse and compute hash */
        int ast_hash = 0;
        ASTNode* stack[32];
        int top = 0;
        stack[top++] = root;
        
        while (top > 0) {
            ASTNode* node = stack[--top];
            ast_hash ^= node->id;
            
            for (int i = 0; i < sizeof(node->data); i++) {
                ast_hash += node->data[i];
            }
            
            if (node->right) stack[top++] = node->right;
            if (node->left) stack[top++] = node->left;
        }
        
        printf("AST hash: %d\n", ast_hash);
        
        /* Cleanup */
        /* Recursive free omitted for brevity - would need implementation */
    }
    
    /* Phase 2: Control flow tests */
    control_flow_test();
    
    /* Phase 3: Parallel operations */
    parallel_memory_operations();
    
    /* Phase 4: Direct built-in calls with volatile parameters */
    {
        volatile char dest[1024];
        volatile char src[1024];
        volatile size_t op_size = g_mem_size;
        
        /* All three built-ins in sequence */
        __builtin_memset(dest, 0xAA, op_size);
        __builtin_memcpy(src, dest, op_size / 2);
        __builtin_memmove(dest + op_size / 4, dest, op_size / 4);
        
        /* Compute verification sum */
        int final_sum = 0;
        for (size_t i = 0; i < op_size && i < sizeof(dest); i++) {
            final_sum += dest[i];
        }
        printf("Final memory sum: %d\n", final_sum);
    }
    
    printf("Token pool index: %d\n", g_token_idx);
    printf("Test completed successfully\n");
    
    return 0;
}
