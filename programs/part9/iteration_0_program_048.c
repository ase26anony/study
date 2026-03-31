/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int g_volatile_size = 64;
static volatile char g_volatile_char = 'A';

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_token_array[256];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(g_token_array, 0, sizeof(g_token_array));
    
    /* Fill with pattern using builtin memcpy */
    char pattern[] = "TEST_PATTERN_1234567890";
    __builtin_memcpy(g_token_array, pattern, sizeof(pattern) - 1);
    
    printf("Constructor: Initialized token array\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Clear sensitive data with builtin memset */
    __builtin_memset(g_token_array, 0, sizeof(g_token_array));
    printf("Destructor: Cleaned up token array\n");
}

/* Recursive parser with control flow jumps */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->id = (*counter)++;
    
    /* Use goto for control flow edge case */
    if (depth % 2 == 0) {
        goto even_depth;
    }
    
    /* Fill data with builtin memcpy */
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Node_%d_Depth_%d", node->id, depth);
    __builtin_memcpy(node->data, buffer, strlen(buffer) + 1);
    
    goto create_children;
    
even_depth:
    /* Alternative initialization path */
    __builtin_memset(node->data, g_volatile_char, sizeof(node->data) - 1);
    node->data[sizeof(node->data) - 1] = '\0';
    
create_children:
    /* Recursive creation with goto jumping back */
    int child_counter = *counter;
    
    if (depth > 1) {
        node->left = create_ast(depth - 1, &child_counter);
        node->right = create_ast(depth - 1, &child_counter);
        
        /* Copy between nodes using builtin memmove */
        if (node->left && node->right) {
            __builtin_memmove(node->left->data + 16, 
                            node->right->data, 
                            g_volatile_size % 16);
        }
    }
    
    *counter = child_counter;
    return node;
}

/* Complex memory operation with goto jumps */
static void process_with_goto_jumps(char* dest, const char* src, size_t size) {
    volatile int use_memmove = 1;
    
    if (size < 32) {
        goto small_copy;
    }
    
    /* Jump into memory operation block */
    if (use_memmove) {
        goto use_memmove_block;
    }
    
small_copy:
    __builtin_memcpy(dest, src, size);
    goto done;
    
use_memmove_block:
    /* This tests the memmove redirection */
    __builtin_memmove(dest, src, size);
    
    /* Jump out and do additional processing */
    if (size > 64) {
        goto large_processing;
    }
    
done:
    return;
    
large_processing:
    /* Additional processing with memset */
    __builtin_memset(dest + size/2, 0xFF, size/4);
    goto done;
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[128];
        char shared_buf[256];
        
        /* Initialize with builtin memset */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        /* Copy to shared buffer using builtin memcpy */
        #pragma omp critical
        {
            __builtin_memcpy(shared_buf + thread_id * 64, 
                           local_buf, 
                           sizeof(local_buf));
        }
        
        /* Move data around with builtin memmove */
        if (thread_id % 2 == 0) {
            __builtin_memmove(local_buf + 32, local_buf, 64);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    int counter = 1;
    ASTNode* root = create_ast(4, &counter);
    
    if (root) {
        /* Process AST with memory operations */
        char result_buffer[256] = {0};
        
        /* Copy AST data using builtin functions */
        __builtin_memcpy(result_buffer, root->data, sizeof(root->data));
        
        if (root->left && root->right) {
            /* Move data between children */
            __builtin_memmove(root->left->data, 
                            root->right->data, 
                            g_volatile_size % 32);
        }
        
        /* Free AST (simplified - real code would need recursive free) */
        free(root);
    }
    
    /* Phase 2: Goto-based control flow */
    char src_data[128];
    char dest_data[128];
    
    /* Initialize source with pattern */
    for (int i = 0; i < sizeof(src_data); i++) {
        src_data[i] = (char)(i % 256);
    }
    
    /* Test various memory operations with goto jumps */
    process_with_goto_jumps(dest_data, src_data, 32);
    process_with_goto_jumps(dest_data + 32, src_data + 32, 96);
    
    /* Phase 3: OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_operations();
    #endif
    
    /* Phase 4: Direct built-in calls with volatile sizes */
    volatile size_t dynamic_size = 48;
    char final_buffer[256];
    
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, dest_data, dynamic_size);
    __builtin_memmove(final_buffer + 64, final_buffer, 32);
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        hash = hash * 31 + (unsigned char)final_buffer[i];
    }
    
    printf("Test completed. Verification hash: 0x%08lx\n", hash);
    printf("Expected: All built-in memory functions redirected via ASAN\n");
    
    return 0;
}
