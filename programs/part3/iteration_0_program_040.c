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
    int value;
} ASTNode;

/* Global token array */
static char g_tokens[256];
static ASTNode* g_ast_root = NULL;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_constructor(void) {
    /* Force initialization of ASAN memfn cache early */
    char local_buf[32];
    __builtin_memset(local_buf, 0xA5, sizeof(local_buf));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_destructor(void) {
    if (g_ast_root) {
        free(g_ast_root);
        g_ast_root = NULL;
    }
}

/* Recursive parser with memory operations */
static ASTNode* parse_recursive(int depth, const char* tokens, size_t len) {
    if (depth <= 0 || len == 0) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data with builtin memcpy */
    size_t copy_len = len < 31 ? len : 31;
    __builtin_memcpy(node->data, tokens, copy_len);
    node->data[copy_len] = '\0';
    
    /* Use goto for control flow edge cases */
    if (depth > 3) {
        goto skip_left;
    }
    
    node->left = parse_recursive(depth - 1, tokens + 1, len - 1);
    
skip_left:
    /* Jump back into block with memmove */
    char temp_buf[64];
    __builtin_memcpy(temp_buf, node->data, 32);
    
    if (depth % 2 == 0) {
        goto process_right;
    } else {
        /* Use builtin memmove within goto block */
        __builtin_memmove(node->data + 8, node->data, 16);
        goto process_right;
    }
    
process_right:
    node->right = parse_recursive(depth - 2, tokens + 2, len - 2);
    
    /* Complex memory operation with volatile size */
    volatile size_t local_size = g_mem_size;
    if (local_size > sizeof(node->data)) {
        local_size = sizeof(node->data);
    }
    
    /* Force all three builtins to be called */
    __builtin_memset(node->data + 16, 0xCC, local_size - 16);
    __builtin_memcpy(temp_buf + 32, node->data, 16);
    __builtin_memmove(node->data, temp_buf, 32);
    
    return node;
}

/* Parallel memory dispatch logic */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char thread_buf[128];
        char src_buf[128];
        
        /* Initialize source with pattern */
        for (int i = 0; i < 128; i++) {
            src_buf[i] = (char)((i + thread_id) & 0xFF);
        }
        
        /* Use all three builtins in parallel region */
        __builtin_memset(thread_buf, thread_id, sizeof(thread_buf));
        
        /* Volatile-controlled copy size */
        volatile size_t copy_size = g_mem_size;
        if (copy_size > 64) copy_size = 64;
        
        __builtin_memcpy(thread_buf + 32, src_buf, copy_size);
        
        /* Overlapping memmove */
        __builtin_memmove(thread_buf + 16, thread_buf + 8, 48);
        
        /* Verify with simple checksum */
        unsigned int checksum = 0;
        for (int i = 0; i < 64; i++) {
            checksum += (unsigned char)thread_buf[i];
        }
        
        #pragma omp critical
        {
            printf("Thread %d checksum: %u\n", thread_id, checksum);
        }
    }
}

/* Multi-stage initialization */
static void initialize_tokens(void) {
    /* Fill tokens with pattern */
    for (int i = 0; i < 256; i++) {
        g_tokens[i] = (char)(i ^ 0x55);
    }
    
    /* Use builtins on global array */
    __builtin_memset(g_tokens + 128, 0xAA, 64);
    __builtin_memcpy(g_tokens + 192, g_tokens, 32);
    __builtin_memmove(g_tokens + 64, g_tokens + 32, 96);
}

/* Main execution flow */
int main(void) {
    uint64_t final_hash = 0;
    
    /* Stage 1: Initialize token array */
    initialize_tokens();
    
    /* Stage 2: Build recursive AST */
    g_ast_root = parse_recursive(5, g_tokens, 256);
    
    /* Stage 3: Parallel memory operations */
    parallel_memory_ops();
    
    /* Stage 4: Process AST and compute hash */
    if (g_ast_root) {
        /* Traverse AST and hash values */
        ASTNode* stack[32];
        int stack_ptr = 0;
        stack[stack_ptr++] = g_ast_root;
        
        while (stack_ptr > 0) {
            ASTNode* current = stack[--stack_ptr];
            
            /* Add to hash */
            for (int i = 0; i < 32; i++) {
                final_hash = (final_hash << 5) ^ (final_hash >> 59) ^ current->data[i];
            }
            
            /* Push children */
            if (current->right) {
                stack[stack_ptr++] = current->right;
            }
            if (current->left) {
                stack[stack_ptr++] = current->left;
            }
            
            /* Memory operation between nodes */
            if (current->left && current->right) {
                __builtin_memcpy(current->data, current->left->data, 16);
                __builtin_memmove(current->right->data + 8, current->data, 16);
            }
        }
    }
    
    /* Final memory operations with volatile control */
    volatile size_t final_size = g_mem_size % 128;
    char final_buf[128];
    
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf, g_tokens, final_size);
    __builtin_memmove(final_buf + 64, final_buf, 32);
    
    /* Add final buffer to hash */
    for (int i = 0; i < 128; i++) {
        final_hash ^= ((uint64_t)final_buf[i] << ((i % 8) * 8));
    }
    
    printf("Final hash: 0x%016llx\n", (unsigned long long)final_hash);
    
    return 0;
}
