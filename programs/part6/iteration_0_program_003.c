/* asan_coverage.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    uint32_t hash;
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
static const int g_token_count = sizeof(g_tokens) / sizeof(g_tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_sanitizer_env(void) {
    volatile char buffer[32];
    /* Force early builtin usage in constructor */
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, "init", 5);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_sanitizer_env(void) {
    volatile char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* token) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data with control flow jumps */
    volatile int copy_mode = 1;
    
    copy_start:
    if (copy_mode) {
        __builtin_memcpy(node->data, token, strlen(token) + 1);
        copy_mode = 0;
        goto copy_start; /* Jump back to test flow sensitivity */
    }
    
    /* Create children with goto jumping around memmove */
    if (depth > 1) {
        goto create_left;
        
        memmove_jump:
        /* This memmove should be visible to ASAN */
        volatile char tmp[64];
        __builtin_memmove(tmp, node->data, sizeof(tmp));
        __builtin_memcpy(node->data, tmp, sizeof(tmp));
        goto create_right;
        
        create_left:
        node->left = create_ast(depth - 1, g_tokens[(depth * 3) % g_token_count]);
        goto memmove_jump;
        
        create_right:
        node->right = create_ast(depth - 1, g_tokens[(depth * 7) % g_token_count]);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Calculate hash with memory operations */
static uint32_t compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    uint32_t hash = 5381;
    volatile size_t len = strlen(node->data);
    
    /* Use builtins in loop */
    for (volatile size_t i = 0; i < len; i++) {
        char buffer[4];
        __builtin_memset(buffer, node->data[i], 1);
        __builtin_memcpy(&hash, buffer, 1); /* Partial copy */
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Recursive hash computation */
    uint32_t left_hash = compute_ast_hash(node->left);
    uint32_t right_hash = compute_ast_hash(node->right);
    
    /* Memmove between hash values */
    volatile uint32_t hashes[3] = {hash, left_hash, right_hash};
    __builtin_memmove(&hash, hashes, sizeof(uint32_t));
    
    return hash ^ left_hash ^ right_hash;
}

/* Parallel memory operations */
static void parallel_mem_operations(void) {
    volatile size_t block_size = g_mem_size;
    char* buffers[4];
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp critical
        {
            buffers[tid] = (char*)malloc(block_size);
            if (buffers[tid]) {
                /* Force all three builtins in parallel region */
                __builtin_memset(buffers[tid], tid, block_size);
                
                if (tid > 0) {
                    __builtin_memcpy(buffers[tid], buffers[tid-1], block_size / 2);
                }
                
                /* Circular memmove */
                volatile size_t move_size = block_size / 4;
                __builtin_memmove(buffers[tid] + move_size, 
                                 buffers[tid], 
                                 block_size - move_size);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        if (buffers[i]) {
            free(buffers[i]);
        }
    }
}

/* Complex memory dispatch with goto patterns */
static void memory_dispatch_logic(void) {
    volatile char src[512];
    volatile char dst[512];
    
    /* Initialize source with pattern */
    for (volatile int i = 0; i < sizeof(src); i++) {
        src[i] = (char)(i % 256);
    }
    
    int operation = 0;
    
    operation_loop:
    switch (operation) {
        case 0:
            __builtin_memcpy(dst, src, g_mem_size);
            operation++;
            goto operation_loop;
            
        case 1:
            __builtin_memset(dst + 128, 0xAA, g_mem_size / 2);
            operation++;
            goto operation_loop;
            
        case 2:
            /* Jump into memmove block */
            goto do_memmove;
            
            do_memmove:
            __builtin_memmove(dst + 64, dst, g_mem_size - 64);
            operation++;
            /* Fall through */
            
        default:
            /* Final overlapping operation */
            __builtin_memmove(dst + 256, dst, 128);
            break;
    }
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast(4, g_tokens[0]);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    uint32_t ast_hash = compute_ast_hash(root);
    printf("AST hash: %u\n", ast_hash);
    
    /* Phase 2: Parallel memory operations */
    #ifdef _OPENMP
    parallel_mem_operations();
    printf("Parallel operations completed\n");
    #endif
    
    /* Phase 3: Dispatch logic with control flow */
    memory_dispatch_logic();
    printf("Memory dispatch completed\n");
    
    /* Phase 4: Variable-sized operations */
    volatile size_t sizes[] = {16, 32, 64, 128, 256};
    char final_buffer[512];
    
    for (volatile int i = 0; i < 5; i++) {
        volatile size_t size = sizes[i];
        char temp[256];
        
        __builtin_memset(temp, i, size);
        __builtin_memcpy(final_buffer + (i * 64), temp, size);
        
        if (i % 2 == 0) {
            __builtin_memmove(final_buffer + (i * 64) + 32, 
                            final_buffer + (i * 64), 
                            size / 2);
        }
    }
    
    /* Calculate final checksum */
    uint32_t checksum = 0;
    for (volatile int i = 0; i < sizeof(final_buffer); i++) {
        checksum += (uint8_t)final_buffer[i];
    }
    
    printf("Final checksum: %u\n", checksum);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(root);
    
    return 0;
}
