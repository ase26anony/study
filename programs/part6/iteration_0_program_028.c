/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token pool with pattern */
    for (size_t i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)((i * 13) & 0xFF);
    }
    
    /* Use __builtin_memset in constructor */
    __builtin_memset(token_pool + 1024, 0xAA, 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use __builtin_memmove in destructor */
    char temp[256];
    __builtin_memmove(temp, token_pool, 256);
    __builtin_memset(token_pool, 0, 256);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = depth;
    node->left = NULL;
    node->right = NULL;
    
    /* Use __builtin_memcpy with volatile length */
    size_t copy_len = (volatile_len % 128) + 64;
    __builtin_memcpy(node->data, token_pool + (depth * 64), copy_len);
    
    /* Null-terminate */
    node->data[copy_len < 255 ? copy_len : 255] = '\0';
    
    /* Recursive creation with goto for control flow */
    if (depth < max_depth - 1) {
        int branch = depth % 3;
        
        switch (branch) {
            case 0:
                node->left = create_ast(depth + 1, max_depth);
                break;
            case 1:
                node->right = create_ast(depth + 1, max_depth);
                break;
            case 2: {
                /* Complex goto pattern around memmove */
                volatile int use_goto = volatile_flag;
                
                if (use_goto) {
                    goto memmove_block;
                } else {
                    /* Fall through */
                }
                
                /* This label creates interesting control flow */
                memmove_block: {
                    char buffer[128];
                    __builtin_memmove(buffer, node->data, 64);
                    __builtin_memcpy(node->data + 64, buffer, 64);
                }
                
                /* Jump out of the block */
                if (depth % 2) {
                    goto skip_right;
                }
                
                node->right = create_ast(depth + 1, max_depth);
                skip_right:
                break;
            }
        }
    }
    
    return node;
}

/* Calculate hash of AST */
static unsigned long hash_ast(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Simple DJB2 hash */
    while (*ptr) {
        hash = ((hash << 5) + hash) + (unsigned long)(*ptr++);
    }
    
    return hash + hash_ast(node->left) + hash_ast(node->right);
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[512];
        char shared_buf[512];
        
        /* Initialize with pattern */
        for (int i = 0; i < 512; i++) {
            local_buf[i] = (char)((i + thread_id * 17) & 0xFF);
        }
        
        /* Use all three builtins in parallel region */
        #pragma omp barrier
        
        /* memcpy between buffers */
        __builtin_memcpy(shared_buf, local_buf, 256);
        
        /* memset part of buffer */
        __builtin_memset(shared_buf + 128, thread_id, 64);
        
        /* memmove overlapping regions */
        __builtin_memmove(shared_buf + 64, shared_buf, 192);
        
        /* Copy back with volatile length */
        size_t copy_size = (volatile_len % 256) + 64;
        __builtin_memcpy(local_buf + 128, shared_buf, copy_size);
        
        #pragma omp barrier
        
        /* Verify pattern */
        int errors = 0;
        for (int i = 0; i < 64; i++) {
            if (shared_buf[128 + i] != (char)thread_id) {
                errors++;
            }
        }
        
        #pragma omp critical
        {
            if (errors > 0) {
                volatile_flag = 0; /* Signal error */
            }
        }
    }
}

/* Multi-stage initialization */
static void initialize_stages(void) {
    /* Stage 1: Direct builtin calls */
    char stage1_buf[1024];
    __builtin_memset(stage1_buf, 0xCC, 512);
    __builtin_memcpy(stage1_buf + 512, token_pool, 512);
    __builtin_memmove(stage1_buf, stage1_buf + 256, 256);
    
    /* Stage 2: Indirect through function pointer */
    void* (*mem_ops[3])(void*, const void*, size_t) = {
        (void* (*)(void*, const void*, size_t))__builtin_memcpy,
        (void* (*)(void*, const void*, size_t))__builtin_memset,
        (void* (*)(void*, const void*, size_t))__builtin_memmove
    };
    
    char stage2_buf[512];
    for (int i = 0; i < 3; i++) {
        size_t len = (volatile_len % 128) + 64;
        mem_ops[i](stage2_buf + i * 64, stage1_buf, len);
    }
    
    /* Stage 3: Nested loops with builtins */
    for (int outer = 0; outer < 4; outer++) {
        char nested_buf[256];
        for (int inner = 0; inner < 8; inner++) {
            size_t offset = (outer * 64 + inner * 8) % 192;
            __builtin_memcpy(nested_buf + offset, 
                           stage2_buf + inner * 32, 
                           32);
            
            if (inner % 3 == 0) {
                __builtin_memset(nested_buf + offset + 16, 
                               outer + inner, 
                               16);
            }
        }
        
        /* Conditional memmove with goto */
        if (outer % 2 == 0) {
            goto do_memmove;
        } else {
            /* Skip */
            continue;
        }
        
        do_memmove:
        __builtin_memmove(stage2_buf, nested_buf, 128);
    }
}

int main(void) {
    unsigned long total_hash = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize and parse AST */
    ASTNode* root = create_ast(0, 5);
    if (root) {
        total_hash += hash_ast(root);
        printf("AST hash: %lu\n", hash_ast(root));
    }
    
    /* Phase 2: Parallel memory operations */
    printf("Running parallel memory operations...\n");
    parallel_memory_ops();
    
    /* Phase 3: Multi-stage initialization */
    printf("Running multi-stage initialization...\n");
    initialize_stages();
    
    /* Phase 4: Final verification with all builtins */
    char final_buf[2048];
    size_t final_len = volatile_len % 1024;
    
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf, token_pool, final_len);
    __builtin_memmove(final_buf + 512, final_buf, 512);
    
    /* Calculate final checksum */
    unsigned long checksum = 0;
    for (size_t i = 0; i < sizeof(final_buf); i++) {
        checksum += (unsigned long)final_buf[i];
    }
    
    total_hash += checksum;
    
    printf("Total hash/checksum: %lu\n", total_hash);
    printf("Volatile flag: %d\n", volatile_flag);
    
    /* Cleanup */
    /* Note: In real code, you'd want to free the AST properly */
    
    return (volatile_flag == 1 && total_hash > 0) ? 0 : 1;
}
