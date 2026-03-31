/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _OPENMP
#include <omp.h>
#endif

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[32];
    int id;
    uint8_t padding[8]; /* For alignment testing */
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
static const int g_token_count = 6;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    volatile char buffer[16];
    /* Force early built-in usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 8, "INIT", 5);
    
    printf("[Constructor] Initialized ASAN environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[32];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    printf("[Destructor] Cleaning up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with built-ins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data */
    const char* token = g_tokens[id % g_token_count];
    size_t len = strlen(token);
    __builtin_memcpy(node->data, token, len < 31 ? len : 31);
    node->data[31] = '\0';
    node->id = id;
    
    /* Create children with goto for flow control */
    int left_id = id * 2;
    int right_id = id * 2 + 1;
    
    /* Use goto to create interesting control flow */
    if (depth > 1) {
        goto create_left;
        
    create_left:
        node->left = create_ast(depth - 1, left_id);
        goto create_right;
        
    create_right:
        node->right = create_ast(depth - 1, right_id);
        goto done;
        
    done:
        ; /* Empty statement for label */
    }
    
    return node;
}

/* Function with complex memory operations and goto */
static void process_ast_with_goto(ASTNode* root) {
    if (!root) return;
    
    ASTNode local_copy;
    volatile int use_memmove = 1;
    
    /* Jump into memory operation block */
    if (root->left) {
        goto copy_left;
    } else {
        goto skip_copy;
    }
    
copy_left:
    {
        /* This block tests memmove with goto entry */
        char temp[32];
        __builtin_memcpy(temp, root->left->data, 32);
        
        if (use_memmove) {
            /* Force memmove usage */
            __builtin_memmove(root->left->data + 8, root->left->data, 16);
        }
        
        /* Copy back with overlapping regions */
        __builtin_memmove(root->left->data, temp, 32);
    }
    goto after_copy;
    
skip_copy:
    /* Alternative path */
    __builtin_memset(&local_copy, 0xCC, sizeof(local_copy));
    
after_copy:
    /* Process current node */
    __builtin_memcpy(&local_copy, root, sizeof(ASTNode));
    
    /* Recursive processing */
    process_ast_with_goto(root->left);
    process_ast_with_goto(root->right);
}

/* OpenMP parallel memory operations */
static uint64_t parallel_memory_operations(void) {
    uint64_t hash = 0;
    const int buffer_count = 16;
    char* buffers[buffer_count];
    
    /* Allocate buffers */
    for (int i = 0; i < buffer_count; i++) {
        buffers[i] = (char*)malloc(g_mem_size);
        if (buffers[i]) {
            __builtin_memset(buffers[i], i, g_mem_size);
        }
    }
    
    #pragma omp parallel reduction(+:hash)
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        #pragma omp for
        for (int i = 0; i < buffer_count; i++) {
            if (buffers[i]) {
                /* Mix of memory operations in parallel region */
                char temp[64];
                
                /* Use all three built-ins */
                __builtin_memcpy(temp, buffers[i], g_mem_size < 64 ? g_mem_size : 64);
                __builtin_memset(buffers[i] + 16, thread_id, 8);
                __builtin_memmove(buffers[i] + 8, buffers[i], 32);
                
                /* Compute simple hash */
                for (size_t j = 0; j < g_mem_size && j < 64; j++) {
                    hash += (uint64_t)buffers[i][j] * (j + 1);
                }
            }
        }
        
        /* Additional memory operation outside parallel for */
        char thread_local_buf[128];
        __builtin_memset(thread_local_buf, thread_id, sizeof(thread_local_buf));
        __builtin_memcpy(thread_local_buf + 64, "THREAD", 7);
    }
    
    /* Cleanup */
    for (int i = 0; i < buffer_count; i++) {
        free(buffers[i]);
    }
    
    return hash;
}

/* Function with variable-length memory operations */
static void variable_length_operations(void) {
    volatile size_t sizes[] = {1, 2, 4, 8, 16, 32, 64, 128};
    char src[256], dst[256];
    
    /* Initialize source with pattern */
    for (size_t i = 0; i < sizeof(src); i++) {
        src[i] = (char)(i % 256);
    }
    
    /* Test different sizes with all built-ins */
    for (int i = 0; i < 8; i++) {
        size_t size = sizes[i];
        
        __builtin_memset(dst, 0xFF, sizeof(dst));
        __builtin_memcpy(dst, src, size);
        
        /* Create overlapping regions for memmove */
        if (size > 8) {
            __builtin_memmove(dst + 4, dst, size - 4);
        }
        
        /* Verify with regular memcmp */
        if (__builtin_memcmp(dst + 4, src, size - 4) != 0) {
            printf("Mismatch at size %zu\n", size);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    printf("\nPhase 1: Creating and processing AST\n");
    ASTNode* root = create_ast(4, 1);
    process_ast_with_goto(root);
    
    /* Phase 2: OpenMP parallel operations */
    printf("\nPhase 2: Parallel memory operations\n");
    uint64_t parallel_hash = parallel_memory_operations();
    printf("Parallel hash result: %llu\n", (unsigned long long)parallel_hash);
    
    /* Phase 3: Variable length operations */
    printf("\nPhase 3: Variable length memory operations\n");
    variable_length_operations();
    
    /* Phase 4: Direct built-in calls in different contexts */
    printf("\nPhase 4: Direct built-in calls\n");
    {
        char final_buffer[256];
        char source_buffer[256];
        
        /* Chain of memory operations */
        __builtin_memset(final_buffer, 0, sizeof(final_buffer));
        __builtin_memset(source_buffer, 0xAA, sizeof(source_buffer));
        
        for (int i = 0; i < 10; i++) {
            size_t offset = i * 16;
            size_t length = 32 - i * 2;
            
            __builtin_memcpy(final_buffer + offset, source_buffer, length);
            __builtin_memmove(final_buffer + offset + 8, final_buffer + offset, length - 8);
        }
        
        /* Compute final checksum */
        uint64_t checksum = 0;
        for (size_t i = 0; i < sizeof(final_buffer); i++) {
            checksum += (uint64_t)final_buffer[i];
        }
        printf("Final checksum: %llu\n", (unsigned long long)checksum);
    }
    
    /* Cleanup AST */
    /* Note: Proper AST cleanup would require recursive free */
    
    printf("\nTest completed successfully\n");
    return 0;
}
