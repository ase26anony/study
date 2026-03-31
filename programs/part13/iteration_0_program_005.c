/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char buffer[128];
    /* Force early builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 64, buffer, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    volatile char final_check[32];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Complex token array initialization */
static void initialize_token_array(char* tokens, size_t count) {
    volatile size_t i = 0;
    
    /* Use goto for control flow complexity */
    if (count == 0) goto empty_array;
    
    init_loop:
    for (; i < count; i++) {
        tokens[i] = (char)(i % 256);
    }
    
    /* Jump back with goto */
    if (i < count * 2) {
        i++;
        goto init_loop;
    }
    
    empty_array:
    return;
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast_recursive(int depth, size_t base_size) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Volatile size to prevent folding */
    volatile size_t copy_size = base_size + depth * 16;
    if (copy_size > 256) copy_size = 256;
    
    /* Initialize node data with builtins */
    __builtin_memset(node->data, depth, sizeof(node->data));
    
    /* Create pattern in left half */
    char pattern[128];
    for (int i = 0; i < 128; i++) {
        pattern[i] = (char)((i + depth) % 128);
    }
    
    /* Copy with goto-controlled flow */
    int copy_done = 0;
    copy_start:
    if (!copy_done) {
        __builtin_memcpy(node->data, pattern, copy_size);
        copy_done = 1;
        goto copy_end;
    }
    
    copy_end:
    /* Move data around within node */
    __builtin_memmove(node->data + 64, node->data, 64);
    
    node->size = copy_size;
    node->left = create_ast_recursive(depth - 1, base_size);
    node->right = create_ast_recursive(depth - 1, base_size);
    
    return node;
}

/* AST copy operation testing memcpy between structures */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    volatile size_t copy_len = dest->size;
    if (src->size < copy_len) copy_len = src->size;
    
    /* Direct builtin usage */
    __builtin_memcpy(dest->data, src->data, copy_len);
    
    /* Recursive copy */
    if (dest->left && src->left) {
        copy_ast_data(dest->left, src->left);
    }
    if (dest->right && src->right) {
        copy_ast_data(dest->right, src->right);
    }
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(char* buffer, size_t size) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread works on different segments */
        size_t segment = size / 4;
        char* thread_buf = buffer + thread_id * segment;
        
        if (thread_id < 4) {
            /* Pattern initialization */
            __builtin_memset(thread_buf, thread_id, segment);
            
            /* Intra-thread copying */
            __builtin_memcpy(thread_buf + segment/2, thread_buf, segment/2);
            
            /* Cross-thread simulation with memmove */
            if (thread_id > 0) {
                __builtin_memmove(thread_buf, buffer + (thread_id-1)*segment, segment/4);
            }
        }
    }
}

/* Main test driver */
int main(void) {
    const size_t BUFFER_SIZE = 4096;
    char* main_buffer = (char*)malloc(BUFFER_SIZE);
    char* shadow_buffer = (char*)malloc(BUFFER_SIZE);
    
    if (!main_buffer || !shadow_buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return EXIT_FAILURE;
    }
    
    /* Stage 1: Initialize with builtins */
    __builtin_memset(main_buffer, 0, BUFFER_SIZE);
    initialize_token_array(main_buffer, 1024);
    
    /* Stage 2: Create and manipulate AST */
    ASTNode* ast1 = create_ast_recursive(4, g_mem_size);
    ASTNode* ast2 = create_ast_recursive(3, g_mem_size + 16);
    
    if (ast1 && ast2) {
        /* Test memcpy between AST nodes */
        copy_ast_data(ast2, ast1);
        
        /* Test memmove within AST node with goto */
        int moved = 0;
        move_again:
        if (!moved) {
            __builtin_memmove(ast1->data + 128, ast1->data, 128);
            moved = 1;
            goto move_complete;
        }
        move_complete:
        
        /* Additional memcpy */
        __builtin_memcpy(shadow_buffer, ast1->data, 256);
    }
    
    /* Stage 3: Parallel operations */
    parallel_memory_operations(main_buffer, BUFFER_SIZE);
    
    /* Stage 4: Final verification with all builtins */
    __builtin_memcpy(shadow_buffer + 2048, main_buffer, 2048);
    __builtin_memset(main_buffer + 3072, 0xCC, 1024);
    __builtin_memmove(main_buffer, shadow_buffer, 1024);
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        hash = (hash * 31) + (unsigned char)main_buffer[i];
    }
    
    printf("Verification hash: 0x%08lx\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(main_buffer);
    free(shadow_buffer);
    
    /* Free AST recursively */
    void free_ast(ASTNode* node) {
        if (!node) return;
        free_ast(node->left);
        free_ast(node->right);
        free(node);
    }
    free_ast(ast1);
    free_ast(ast2);
    
    return EXIT_SUCCESS;
}
