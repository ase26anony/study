/* ISO C99-compliant test program for ASAN built-in redirection */
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
    size_t size;
} ASTNode;

/* Global token array */
char global_tokens[1024];
volatile int token_index = 0;

/* Constructor function */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    printf("Constructor: Initialized global tokens\n");
}

/* Destructor function */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    /* Use builtin memcpy in destructor */
    char temp[16];
    __builtin_memcpy(temp, global_tokens, 16);
    printf("Destructor: Cleaned up (first 16 chars: %.16s)\n", temp);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset for node initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with pattern using builtin memcpy */
    char pattern[32];
    __builtin_memset(pattern, '0' + (depth % 10), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(pattern));
    
    node->size = sizeof(ASTNode);
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int state = 0;
    
    if (!src || !dst) return;
    
    /* Jump into memory operation block */
    goto start_copy;
    
copy_block:
    /* This block contains builtin memmove with goto */
    __builtin_memmove(dst->data, src->data, 
                     volatile_len < sizeof(src->data) ? volatile_len : sizeof(src->data));
    state = 1;
    goto after_copy;
    
start_copy:
    if (volatile_flag) {
        goto copy_block;
    }
    
after_copy:
    /* Jump out of scope */
    if (state) {
        goto finalize;
    }
    
    /* Alternative path with builtin memcpy */
    __builtin_memcpy(dst->data + 128, src->data, 32);
    
finalize:
    return;
}

/* OpenMP parallel section */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[256];
        char src_buf[256];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, thread_id + 'A', sizeof(local_buf));
        __builtin_memset(src_buf, thread_id + 'a', sizeof(src_buf));
        
        #pragma omp barrier
        
        /* Conditional memcpy based on thread ID */
        if (thread_id % 2 == 0) {
            __builtin_memcpy(local_buf + 64, src_buf, 128);
        } else {
            __builtin_memmove(local_buf, src_buf, 192);
        }
        
        /* Store result in global array */
        #pragma omp critical
        {
            size_t offset = (thread_id * 32) % sizeof(global_tokens);
            __builtin_memcpy(global_tokens + offset, local_buf, 32);
        }
    }
}

/* Multi-stage processing */
static unsigned long compute_hash(const char* data, size_t len) {
    unsigned long hash = 5381;
    volatile size_t i = 0;
    
    /* Use goto for flow control around builtins */
    if (len == 0) goto empty_buffer;
    
process_loop:
    while (i < len) {
        hash = ((hash << 5) + hash) + data[i];
        i++;
        
        /* Every 16 bytes, use builtin memcpy on hash buffer */
        if (i % 16 == 0) {
            char temp[8];
            __builtin_memcpy(temp, &hash, sizeof(hash));
            /* memmove within temp buffer */
            __builtin_memmove(temp + 2, temp, 6);
        }
    }
    
empty_buffer:
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Create recursive structures */
    ASTNode* ast1 = create_ast(3);
    ASTNode* ast2 = create_ast(3);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Phase 2: Test goto edge cases */
    process_with_goto(ast1, ast2);
    
    /* Phase 3: OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_ops();
    #else
    printf("OpenMP not available, skipping parallel section\n");
    #endif
    
    /* Phase 4: Compute verification hash */
    unsigned long hash1 = compute_hash(ast1->data, sizeof(ast1->data));
    unsigned long hash2 = compute_hash(ast2->data, sizeof(ast2->data));
    unsigned long hash3 = compute_hash(global_tokens, sizeof(global_tokens));
    
    /* Use builtin memset for final buffer */
    char result_buf[64];
    __builtin_memset(result_buf, 0, sizeof(result_buf));
    
    /* Combine hashes with builtin memcpy */
    __builtin_memcpy(result_buf, &hash1, sizeof(hash1));
    __builtin_memcpy(result_buf + 16, &hash2, sizeof(hash2));
    __builtin_memcpy(result_buf + 32, &hash3, sizeof(hash3));
    
    /* Final builtin memmove */
    __builtin_memmove(result_buf + 8, result_buf, 56);
    
    /* Print verification result */
    unsigned long final_hash = compute_hash(result_buf, sizeof(result_buf));
    printf("Verification hash: %lu\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    return 0;
}
