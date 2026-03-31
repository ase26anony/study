/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_token_array[1024];
static volatile size_t g_token_pos = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize token array with pattern */
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 13) % 256);
    }
    
    /* Force early initialization of memory functions */
    char buffer[32];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    /* Final memory operations */
    char final_buffer[64];
    __builtin_memset(final_buffer, 0xFF, sizeof(final_buffer));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* data, size_t depth) {
    if (depth == 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy with volatile size */
    size_t copy_size = g_mem_size % 64;
    if (copy_size > 63) copy_size = 63;
    
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, data, copy_size);
    node->size = copy_size;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        char child_data[64];
        __builtin_memcpy(child_data, data, copy_size);
        
        /* Goto-based control flow */
        if (depth % 2 == 0) {
            goto create_left;
        } else {
            goto create_right;
        }
        
    create_left:
        node->left = create_ast_node(child_data, depth - 1);
        goto skip_right;
        
    create_right:
        node->right = create_ast_node(child_data, depth - 1);
        goto skip_left;
        
    skip_right:
        node->right = NULL;
        goto done;
        
    skip_left:
        node->left = NULL;
        goto done;
        
    done:;
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Copy between AST nodes with builtin memmove */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use volatile to prevent folding */
    volatile size_t move_size = src->size;
    if (move_size > sizeof(dest->data)) {
        move_size = sizeof(dest->data);
    }
    
    /* Builtin memmove with overlapping regions */
    __builtin_memmove(dest->data, src->data, move_size);
    
    /* Also copy to overlapping region */
    if (move_size > 16) {
        __builtin_memmove(dest->data + 8, src->data, move_size - 8);
    }
}

/* Parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        char local_buffer[128];
        volatile int thread_id = omp_get_thread_num();
        
        /* Each thread uses builtin memory functions */
        __builtin_memset(local_buffer, thread_id, sizeof(local_buffer));
        
        /* Copy between buffers */
        char temp_buffer[128];
        __builtin_memcpy(temp_buffer, local_buffer, sizeof(local_buffer));
        
        /* Move with overlap */
        __builtin_memmove(local_buffer + 32, local_buffer, 64);
        
        /* Update global token array */
        #pragma omp critical
        {
            size_t pos = g_token_pos;
            if (pos < sizeof(g_token_array) - 128) {
                __builtin_memcpy(&g_token_array[pos], local_buffer, 64);
                g_token_pos += 64;
            }
        }
    }
}

/* Complex initialization with goto jumps */
static void initialize_with_goto(void) {
    char buffer_a[256];
    char buffer_b[256];
    
    /* Initial memset */
    __builtin_memset(buffer_a, 0xCC, sizeof(buffer_a));
    
    /* Goto into block with memmove */
    if (g_use_hwasan) {
        goto hwasan_block;
    } else {
        goto asan_block;
    }
    
hwasan_block:
    /* This should trigger HWASAN path if compiled with -fsanitize=kernel-hwaddress */
    __builtin_memmove(buffer_b, buffer_a, 128);
    goto continue_main;
    
asan_block:
    /* Standard ASAN path */
    __builtin_memmove(buffer_b, buffer_a, 128);
    goto continue_main;
    
continue_main:
    /* Copy back with overlap */
    __builtin_memcpy(buffer_a + 64, buffer_b, 128);
}

/* Calculate hash of token array */
static unsigned long calculate_token_hash(void) {
    unsigned long hash = 5381;
    
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        hash = ((hash << 5) + hash) + g_token_array[i];
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize AST */
    ASTNode* root = create_ast_node("Test AST Node Data", 4);
    
    /* Phase 2: Perform AST operations */
    if (root && root->left) {
        copy_ast_data(root, root->left);
    }
    
    /* Phase 3: Parallel operations */
    parallel_memory_operations();
    
    /* Phase 4: Goto-based initialization */
    initialize_with_goto();
    
    /* Phase 5: Additional builtin calls */
    char final_buffer[512];
    volatile size_t final_size = g_mem_size % 512;
    
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, g_token_array, 
                    final_size < sizeof(g_token_array) ? final_size : sizeof(g_token_array));
    __builtin_memmove(final_buffer + 256, final_buffer, 256);
    
    /* Calculate and print verification result */
    unsigned long hash = calculate_token_hash();
    printf("Token array hash: 0x%08lx\n", hash);
    printf("Memory operations completed.\n");
    
    /* Cleanup */
    if (root) {
        free(root->left);
        free(root->right);
        free(root);
    }
    
    return 0;
}
