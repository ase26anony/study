/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    uint32_t hash;
} ASTNode;

/* Global token array */
static char g_token_pool[4096];
volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i * 31) & 0xFF);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    /* Clear sensitive data */
    __builtin_memset(g_token_pool, 0, sizeof(g_token_pool));
    printf("Destructor: Token pool cleared\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_recursive(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using builtin memcpy with goto for flow control */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    
    /* Goto-based control flow around memcpy */
    if (depth % 2 == 0) {
        goto even_depth_copy;
    } else {
        goto odd_depth_copy;
    }
    
even_depth_copy:
    __builtin_memcpy(node->data, base_data, copy_len);
    goto after_copy;
    
odd_depth_copy:
    {
        char temp[64];
        __builtin_memcpy(temp, base_data, copy_len);
        __builtin_memmove(node->data, temp, copy_len);
    }
    goto after_copy;
    
after_copy:
    /* Compute hash using volatile size */
    uint32_t hash = 0;
    for (volatile size_t i = 0; i < copy_len; i++) {
        hash = hash * 31 + node->data[i];
    }
    node->hash = hash;
    
    /* Recursive creation with different memory operations */
    char child_data[64];
    __builtin_snprintf(child_data, sizeof(child_data), "%s-%d", base_data, depth);
    
    node->left = create_ast_recursive(depth - 1, child_data);
    node->right = create_ast_recursive(depth - 2, child_data);
    
    return node;
}

/* Parallel memory dispatch function */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char src_buf[512];
        char dst_buf[512];
        
        /* Initialize with pattern */
        for (volatile size_t i = 0; i < sizeof(src_buf); i++) {
            src_buf[i] = (char)((i + thread_id * 73) & 0xFF);
        }
        
        /* Force builtin calls with volatile size */
        volatile size_t op_size = g_mem_size + thread_id;
        if (op_size > sizeof(src_buf)) op_size = sizeof(src_buf);
        
        /* Different memory operations based on thread */
        switch (thread_id % 3) {
            case 0:
                __builtin_memcpy(dst_buf, src_buf, op_size);
                break;
            case 1:
                __builtin_memset(dst_buf, thread_id, op_size);
                break;
            case 2:
                __builtin_memmove(dst_buf, src_buf, op_size);
                break;
        }
        
        /* Verify with another memcpy */
        char verify_buf[512];
        __builtin_memcpy(verify_buf, dst_buf, op_size);
        
        /* Compute checksum */
        uint32_t checksum = 0;
        for (volatile size_t i = 0; i < op_size; i++) {
            checksum += verify_buf[i];
        }
        
        #pragma omp critical
        {
            printf("Thread %d: checksum = %u (size=%zu)\n", 
                   thread_id, checksum, (size_t)op_size);
        }
    }
}

/* Multi-stage interaction function */
static uint64_t process_ast_with_memory_ops(ASTNode* root) {
    if (!root) return 0;
    
    uint64_t total_hash = 0;
    ASTNode* stack[64];
    int stack_ptr = 0;
    
    stack[stack_ptr++] = root;
    
    while (stack_ptr > 0) {
        ASTNode* current = stack[--stack_ptr];
        
        total_hash += current->hash;
        
        /* Process node data with memory operations */
        char processed[128];
        volatile size_t process_len = strlen(current->data) + 8;
        
        /* Use goto for control flow variation */
        if (current->hash % 3 == 0) {
            goto use_memcpy;
        } else if (current->hash % 3 == 1) {
            goto use_memset;
        } else {
            goto use_memmove;
        }
        
    use_memcpy:
        __builtin_memcpy(processed, "CPY:", 4);
        __builtin_memcpy(processed + 4, current->data, 
                        process_len > sizeof(processed) - 4 ? 
                        sizeof(processed) - 4 : process_len);
        goto after_process;
        
    use_memset:
        __builtin_memset(processed, 'M', 
                        process_len > sizeof(processed) ? 
                        sizeof(processed) : process_len);
        goto after_process;
        
    use_memmove:
        {
            char temp[128];
            __builtin_memcpy(temp, current->data, 
                           process_len > sizeof(temp) ? 
                           sizeof(temp) : process_len);
            __builtin_memmove(processed, temp, 
                            process_len > sizeof(processed) ? 
                            sizeof(processed) : process_len);
        }
        goto after_process;
        
    after_process:
        /* Push children */
        if (current->right) {
            if (stack_ptr < 64) stack[stack_ptr++] = current->right;
        }
        if (current->left) {
            if (stack_ptr < 64) stack[stack_ptr++] = current->left;
        }
    }
    
    return total_hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Stage 1: Create recursive AST */
    printf("Creating recursive AST structure...\n");
    ASTNode* root = create_ast_recursive(5, "ROOT");
    
    /* Stage 2: Process with memory operations */
    printf("Processing AST with memory operations...\n");
    uint64_t ast_hash = process_ast_with_memory_ops(root);
    printf("AST total hash: %llu\n", (unsigned long long)ast_hash);
    
    /* Stage 3: Parallel memory operations */
    printf("\nExecuting parallel memory operations...\n");
    parallel_memory_operations();
    
    /* Stage 4: Token pool operations */
    printf("\nPerforming token pool operations...\n");
    char token_copy[1024];
    volatile size_t copy_size = g_mem_size * 2;
    if (copy_size > sizeof(token_copy)) copy_size = sizeof(token_copy);
    
    /* Mix of memory operations on global pool */
    __builtin_memcpy(token_copy, g_token_pool, copy_size);
    __builtin_memset(g_token_pool + 512, 0xAA, 256);
    __builtin_memmove(g_token_pool, g_token_pool + 256, 512);
    
    /* Compute final verification hash */
    uint32_t final_hash = 0;
    for (volatile size_t i = 0; i < copy_size; i++) {
        final_hash = final_hash * 31 + token_copy[i];
    }
    printf("Final verification hash: %u\n", final_hash);
    
    /* Cleanup */
    free_ast(root);
    
    printf("\n=== Test completed successfully ===\n");
    return 0;
}
