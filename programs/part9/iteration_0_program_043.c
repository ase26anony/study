/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_tokens[][32] = {
    "memcpy_test", "memset_test", "memmove_test",
    "recursive", "parallel", "asan"
};

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_sanitizer_hook(void) {
    volatile char buffer[128];
    /* Force early builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_printf("Constructor initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_sanitizer_hook(void) {
    volatile char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
    __builtin_printf("Destructor cleaning up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)__builtin_malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with memcpy from tokens */
    int token_idx = id % (sizeof(g_tokens)/sizeof(g_tokens[0]));
    __builtin_memcpy(node->data, g_tokens[token_idx], 
                     __builtin_strlen(g_tokens[token_idx]));
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    if (depth > 2) {
        create_left = 0;
        goto skip_left;
    }
    
    node->left = create_ast(depth - 1, id * 2);
    
skip_left:
    /* Jump back into normal flow */
    if (create_left) {
        node->right = create_ast(depth - 1, id * 2 + 1);
    } else {
        /* Use memmove to shift data when skipping left */
        char temp[256];
        __builtin_memcpy(temp, node->data, sizeof(temp));
        __builtin_memmove(node->data + 32, node->data, 128);
        __builtin_memcpy(node->data, temp, 32);
        node->right = NULL;
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void memory_operation_sequence(char* dest, const char* src, size_t len) {
    volatile int use_memmove = 0;
    
    /* First operation - memcpy */
    __builtin_memcpy(dest, src, len);
    
    /* Jump to different operation based on condition */
    if (len > 32) {
        use_memmove = 1;
        goto do_memmove;
    }
    
    /* Normal memset path */
    __builtin_memset(dest + 16, 0xCC, len / 2);
    goto finish;
    
do_memmove:
    /* Overlapping memory operation */
    __builtin_memmove(dest + 8, dest, len - 8);
    __builtin_memset(dest, 0xDD, 16);
    
finish:
    /* Final touch */
    if (use_memmove) {
        __builtin_memcpy(dest + len - 8, "END", 4);
    }
}

/* OpenMP parallel section */
static void parallel_memory_operations(void) {
    const int num_threads = 4;
    char thread_buffers[num_threads][256];
    int results[num_threads];
    
    #pragma omp parallel for
    for (int i = 0; i < num_threads; i++) {
        /* Each thread uses different builtins */
        switch (i % 3) {
            case 0:
                __builtin_memset(thread_buffers[i], i, g_mem_size);
                break;
            case 1:
                __builtin_memcpy(thread_buffers[i], g_tokens[i], 32);
                break;
            case 2:
                __builtin_memmove(thread_buffers[i], thread_buffers[(i+1)%num_threads], 64);
                break;
        }
        
        /* Compute simple hash */
        int hash = 0;
        for (int j = 0; j < 64; j++) {
            hash += thread_buffers[i][j];
        }
        results[i] = hash;
    }
    
    /* Verify results */
    int total = 0;
    for (int i = 0; i < num_threads; i++) {
        total += results[i];
    }
    __builtin_printf("Parallel hash total: %d\n", total);
}

/* Multi-stage initialization */
static void initialize_system(void) {
    volatile char init_buffer[512];
    volatile char* volatile_ptr = init_buffer;
    
    /* Stage 1: Clear everything */
    __builtin_memset(init_buffer, 0, sizeof(init_buffer));
    
    /* Stage 2: Fill with pattern using memcpy */
    char pattern[64];
    __builtin_memset(pattern, 0xAB, sizeof(pattern));
    
    for (int i = 0; i < 8; i++) {
        __builtin_memcpy(init_buffer + i * 64, pattern, 64);
    }
    
    /* Stage 3: Shift data around with memmove */
    __builtin_memmove(init_buffer + 128, init_buffer, 256);
    
    /* Use volatile pointer to force memory access */
    volatile_ptr[255] = 0xFF;
}

int main(void) {
    __builtin_printf("Starting ASAN builtin redirection test\n");
    
    /* Phase 1: System initialization */
    initialize_system();
    
    /* Phase 2: Create and manipulate AST */
    ASTNode* root = create_ast(4, 1);
    if (root) {
        char node_copy[256];
        
        /* Copy between nodes */
        __builtin_memcpy(node_copy, root->data, sizeof(node_copy));
        
        if (root->left) {
            __builtin_memmove(root->left->data, node_copy, 128);
        }
        
        /* Recursive traversal with memory ops */
        ASTNode* current = root;
        while (current) {
            char temp[256];
            __builtin_memcpy(temp, current->data, sizeof(temp));
            __builtin_memset(current->data + 128, 0, 64);
            __builtin_memcpy(current->data, temp, 128);
            
            current = current->right;
        }
        
        /* Cleanup */
        __builtin_free(root);
    }
    
    /* Phase 3: Complex sequence with goto */
    char seq_buffer[1024];
    char seq_source[1024];
    __builtin_memset(seq_source, 0x55, sizeof(seq_source));
    
    memory_operation_sequence(seq_buffer, seq_source, 512);
    
    /* Phase 4: Parallel operations */
    parallel_memory_operations();
    
    /* Phase 5: Final verification */
    volatile char final_check[256];
    __builtin_memset(final_check, 0, sizeof(final_check));
    __builtin_memcpy(final_check, "ASAN_TEST_COMPLETE", 19);
    __builtin_memmove(final_check + 32, final_check, 19);
    
    /* Compute final hash */
    unsigned long final_hash = 0;
    for (int i = 0; i < 256; i++) {
        final_hash = final_hash * 31 + final_check[i];
    }
    
    __builtin_printf("Test completed. Final hash: %lu\n", final_hash);
    
    return (final_hash != 0) ? 0 : 1;
}
