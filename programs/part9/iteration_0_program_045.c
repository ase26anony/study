/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[32];
    uint32_t hash;
} ASTNode;

/* Global token array */
static char g_token_pool[1024];
static volatile size_t g_token_idx = 0;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_token_pool(void) {
    /* Use memset to trigger ASAN initialization */
    __builtin_memset(g_token_pool, 'A', sizeof(g_token_pool));
    g_init_flag = 1;
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_verify(void) {
    /* Final memory operation in destructor */
    char verify_buf[16];
    __builtin_memset(verify_buf, 0xFF, sizeof(verify_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to control copy size */
    volatile size_t copy_len = 16;
    
    /* Built-in memcpy with goto for flow control */
    copy_start:
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Conditional goto to test flow sensitivity */
    if (depth % 2 == 0) {
        goto skip_memmove;
    }
    
    /* Built-in memmove with overlapping regions */
    char temp[32];
    __builtin_memmove(temp, node->data, copy_len);
    __builtin_memmove(node->data + 8, temp, copy_len - 8);
    
    skip_memmove:
    
    /* Built-in memset for padding */
    __builtin_memset(node->data + copy_len, 0, sizeof(node->data) - copy_len);
    
    /* Recursive creation */
    node->left = create_ast(depth - 1, node->data);
    node->right = create_ast(depth - 1, node->data + 8);
    
    return node;
}

/* Compute hash with memory operations */
static uint32_t compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    uint32_t hash = 0;
    volatile size_t op_size = g_mem_size % 32;
    
    /* Memory operations in loop */
    for (int i = 0; i < 3; i++) {
        char buffer[64];
        
        /* Mix of built-ins */
        __builtin_memset(buffer, i, op_size);
        __builtin_memcpy(buffer + op_size, node->data, 16);
        __builtin_memmove(node->data, buffer, 16);
        
        /* XOR hash computation */
        for (size_t j = 0; j < 16; j++) {
            hash ^= (node->data[j] << ((j % 4) * 8));
        }
    }
    
    uint32_t left_hash = compute_ast_hash(node->left);
    uint32_t right_hash = compute_ast_hash(node->right);
    
    /* Final memory operation */
    char final_buf[8];
    __builtin_memset(final_buf, hash & 0xFF, sizeof(final_buf));
    
    return hash ^ left_hash ^ right_hash;
}

/* Parallel memory dispatcher */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[128];
        char src_buf[128];
        
        /* Initialize source with thread-specific pattern */
        __builtin_memset(src_buf, thread_id + '0', sizeof(src_buf));
        
        /* OpenMP sections with different memory operations */
        #pragma omp sections
        {
            #pragma omp section
            {
                /* Section 1: memcpy operations */
                for (int i = 0; i < 4; i++) {
                    volatile size_t len = (i * 16 + 8) % 64;
                    __builtin_memcpy(local_buf + i * 16, src_buf, len);
                }
            }
            
            #pragma omp section
            {
                /* Section 2: memset operations */
                for (int i = 0; i < 4; i++) {
                    volatile char fill = 'A' + i;
                    volatile size_t len = (i * 12 + 4) % 64;
                    __builtin_memset(local_buf + 32 + i * 12, fill, len);
                }
            }
            
            #pragma omp section
            {
                /* Section 3: memmove with overlap */
                char overlap_buf[96];
                __builtin_memset(overlap_buf, 'X', sizeof(overlap_buf));
                
                /* Forward overlap */
                __builtin_memmove(overlap_buf + 16, overlap_buf, 64);
                
                /* Backward overlap with goto */
                if (thread_id % 2 == 0) {
                    goto do_backward_move;
                }
                
                __builtin_memmove(overlap_buf, overlap_buf + 32, 32);
                goto move_complete;
                
                do_backward_move:
                __builtin_memmove(overlap_buf + 48, overlap_buf + 32, 32);
                
                move_complete:
                /* Copy result back */
                __builtin_memcpy(local_buf + 96, overlap_buf, 32);
            }
        }
        
        /* Verify thread operations */
        char verify[8];
        __builtin_memset(verify, 0, sizeof(verify));
        __builtin_memcpy(verify, local_buf, sizeof(verify) < 8 ? sizeof(verify) : 8);
    }
}

/* Main test driver */
int main(void) {
    /* Wait for constructor initialization */
    while (!g_init_flag) {}
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast(4, "AST_BASE_DATA_0123456789");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Compute hash with memory operations */
    uint32_t ast_hash = compute_ast_hash(root);
    printf("AST Hash: 0x%08X\n", ast_hash);
    
    /* Phase 3: Parallel memory operations */
    printf("Running parallel memory operations...\n");
    parallel_memory_ops();
    
    /* Phase 4: Token pool operations */
    volatile size_t token_ops = 8;
    for (size_t i = 0; i < token_ops; i++) {
        char temp[128];
        size_t offset = (i * 37) % sizeof(g_token_pool);
        size_t len = (i * 13 + 7) % 64;
        
        /* Mix of all three built-ins */
        __builtin_memcpy(temp, g_token_pool + offset, len);
        __builtin_memset(g_token_pool + offset, 'Z' - i, len);
        __builtin_memmove(g_token_pool + offset + 32, temp, len > 32 ? 32 : len);
        
        /* Goto to skip every 3rd iteration */
        if (i % 3 == 0) {
            goto skip_token_hash;
        }
        
        /* Additional memory operation */
        __builtin_memset(temp + len, i, sizeof(temp) - len);
        
        skip_token_hash:
        g_token_idx = (g_token_idx + 1) % sizeof(g_token_pool);
    }
    
    /* Final verification */
    char final_check[256];
    __builtin_memset(final_check, 0, sizeof(final_check));
    __builtin_memcpy(final_check, g_token_pool, 
                    sizeof(final_check) < sizeof(g_token_pool) ? 
                    sizeof(final_check) : sizeof(g_token_pool));
    
    /* Compute final checksum */
    uint32_t final_sum = 0;
    for (size_t i = 0; i < sizeof(final_check); i++) {
        final_sum += final_check[i];
    }
    
    printf("Final checksum: %u\n", final_sum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    /* Note: In real code, would need to free AST recursively */
    
    return 0;
}
