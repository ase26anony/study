/* asan_coverage.c - Comprehensive test for ASAN memory built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_token_array[4096];
static volatile size_t g_token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Force initialization of ASAN runtime */
    g_init_flag = 1;
    
    /* Initialize token array with pattern */
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)(i % 256);
    }
    
    /* Use builtins in constructor */
    __builtin_memset(g_token_array, 0xAA, 256);
    __builtin_memcpy(g_token_array + 256, g_token_array, 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    /* Final memory operations */
    volatile char cleanup_buf[128];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_recursive(int depth, size_t* node_count) {
    if (depth <= 0 || *node_count > 100) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    (*node_count)++;
    
    /* Use volatile to control memory operation size */
    volatile size_t copy_size = g_mem_size % 128 + 16;
    
    /* Builtin memory operations with goto for flow control */
    int use_memmove = 0;
    
    if (depth % 3 == 0) {
        use_memmove = 1;
        goto memmove_block;
    }
    
    /* Regular memset/memcpy path */
    __builtin_memset(node->data, depth, sizeof(node->data));
    
    /* Copy from token array with variable offset */
    size_t offset = (depth * 17) % (sizeof(g_token_array) - copy_size);
    __builtin_memcpy(node->data + 64, 
                    g_token_array + offset, 
                    copy_size);
    
    goto skip_memmove;
    
memmove_block:
    /* Goto target with memmove */
    char temp_buf[256];
    __builtin_memcpy(temp_buf, g_token_array, 128);
    
    /* Force memmove with overlapping regions */
    __builtin_memmove(node->data, 
                     node->data + 32, 
                     copy_size);
    
    /* Also use memcpy here */
    __builtin_memcpy(node->data + 128, temp_buf, 64);
    
skip_memmove:
    node->size = copy_size;
    
    /* Recursive creation */
    node->left = create_ast_recursive(depth - 1, node_count);
    node->right = create_ast_recursive(depth - 2, node_count);
    
    return node;
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
        char local_buf[512];
        char src_buf[512];
        
        /* Initialize source buffer */
        for (int i = 0; i < 512; i++) {
            src_buf[i] = (char)((i + thread_id) % 256);
        }
        
        /* Use all three builtins in parallel region */
        __builtin_memset(local_buf, thread_id, 128);
        
        #pragma omp barrier
        
        __builtin_memcpy(local_buf + 128, src_buf, 256);
        
        #pragma omp barrier
        
        /* Conditional memmove with goto */
        if (thread_id % 2 == 0) {
            goto do_memmove;
        }
        
        __builtin_memcpy(local_buf + 384, src_buf + 128, 128);
        goto skip_parallel_memmove;
        
    do_memmove:
        __builtin_memmove(local_buf, local_buf + 64, 192);
        
    skip_parallel_memmove:
        
        /* Use result to prevent optimization */
        volatile char sum = 0;
        for (int i = 0; i < 128; i++) {
            sum += local_buf[i];
        }
    }
}

/* Complex memory operation sequence */
static size_t execute_memory_sequence(ASTNode* root) {
    if (!root) return 0;
    
    size_t total_hash = 0;
    ASTNode* nodes[100];
    int node_count = 0;
    
    /* Collect nodes */
    ASTNode* stack[100];
    int stack_ptr = 0;
    stack[stack_ptr++] = root;
    
    while (stack_ptr > 0 && node_count < 100) {
        ASTNode* current = stack[--stack_ptr];
        nodes[node_count++] = current;
        
        if (current->right) stack[stack_ptr++] = current->right;
        if (current->left) stack[stack_ptr++] = current->left;
    }
    
    /* Perform memory operations between nodes */
    for (int i = 0; i < node_count - 1; i++) {
        volatile size_t op_size = (nodes[i]->size + nodes[i+1]->size) / 2;
        
        if (op_size > 256) op_size = 256;
        
        /* Alternate between memcpy and memmove */
        if (i % 3 == 0) {
            __builtin_memmove(nodes[i]->data + 32,
                            nodes[i]->data,
                            op_size);
        } else if (i % 3 == 1) {
            __builtin_memcpy(nodes[i]->data,
                           nodes[i+1]->data,
                           op_size);
        } else {
            __builtin_memset(nodes[i]->data + 128,
                           i % 256,
                           op_size % 128);
        }
        
        /* Compute hash */
        for (size_t j = 0; j < op_size && j < 128; j++) {
            total_hash += (size_t)nodes[i]->data[j];
        }
    }
    
    return total_hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Phase 1: Create recursive AST */
    size_t node_count = 0;
    ASTNode* ast_root = create_ast_recursive(5, &node_count);
    
    printf("Created AST with %zu nodes\n", node_count);
    
    /* Phase 2: Execute parallel memory operations */
    #ifdef _OPENMP
    printf("Executing parallel memory operations...\n");
    #endif
    
    parallel_memory_operations();
    
    /* Phase 3: Complex memory sequence */
    printf("Executing complex memory sequence...\n");
    size_t final_hash = execute_memory_sequence(ast_root);
    
    /* Phase 4: Additional builtin calls in main */
    char final_buffer[1024];
    volatile size_t final_size = g_mem_size % 512 + 64;
    
    /* Use all three builtins in sequence */
    __builtin_memset(final_buffer, 0xCC, final_size);
    
    /* Goto with memcpy */
    if (final_hash % 2 == 0) {
        goto copy_block;
    }
    
    __builtin_memmove(final_buffer + 128, final_buffer, 256);
    goto skip_copy;
    
copy_block:
    __builtin_memcpy(final_buffer + 256, g_token_array, 384);
    
skip_copy:
    /* Final memset */
    __builtin_memset(final_buffer + 512, final_hash % 256, 128);
    
    /* Compute verification result */
    size_t verification = 0;
    for (int i = 0; i < 256; i++) {
        verification += (size_t)final_buffer[i];
    }
    verification += final_hash;
    
    printf("Verification result: %zu\n", verification);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    /* Note: In real code, you'd want to properly free the AST */
    
    return 0;
}
