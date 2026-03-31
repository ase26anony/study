/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize token pool with pattern */
    __builtin_memset(token_pool, 0xAA, sizeof(token_pool));
    
    /* Force early built-in usage in constructor */
    char local_buf[128];
    __builtin_memset(local_buf, 0xCC, sizeof(local_buf));
    __builtin_memcpy(&token_pool[512], local_buf, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Verify memory operations in destructor */
    char verify_buf[64];
    __builtin_memset(verify_buf, 0, sizeof(verify_buf));
    __builtin_memcpy(verify_buf, &token_pool[1024], 64);
    
    /* Use memmove in destructor */
    __builtin_memmove(&token_pool[2048], &token_pool[1024], 128);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_recursive(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with built-ins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy data with varying sizes */
    size_t copy_len = (size_t)(volatile_len % 128) + 1;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Use memmove for overlapping regions */
    if (depth % 3 == 0) {
        __builtin_memmove(&node->data[32], &node->data[16], 48);
    }
    
    node->size = copy_len;
    node->left = create_ast_recursive(depth - 1, base_data);
    node->right = create_ast_recursive(depth - 2, base_data);
    
    return node;
}

/* Function with goto control flow */
static void process_with_goto(ASTNode* node1, ASTNode* node2) {
    int state = 0;
    
    if (!node1 || !node2) return;
    
start_block:
    /* Jump into memory operation block */
    if (state == 0) {
        state = 1;
        goto mem_op_block;
    }
    
    goto end_processing;
    
mem_op_block:
    {
        /* Complex memory operation with goto */
        char temp[256];
        __builtin_memset(temp, 0, sizeof(temp));
        
        /* Copy between AST nodes */
        size_t len = node1->size < node2->size ? node1->size : node2->size;
        __builtin_memcpy(temp, node1->data, len);
        
        if (volatile_flag) {
            /* Use memmove with goto */
            __builtin_memmove(node2->data, temp, len);
            goto update_state;
        }
        
        __builtin_memcpy(node2->data, temp, len);
        
update_state:
        state = 2;
        goto start_block;
    }
    
end_processing:
    /* Final memory operation */
    __builtin_memset(node1->data + 128, 0xFF, 64);
}

/* Parallel memory dispatch function */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char thread_buf[512];
        char shared_buf[1024];
        
        /* Initialize with built-ins */
        __builtin_memset(thread_buf, thread_id, sizeof(thread_buf));
        
        #pragma omp critical
        {
            /* Copy to shared buffer */
            size_t offset = (thread_id * 64) % 768;
            __builtin_memcpy(&shared_buf[offset], thread_buf, 64);
            
            /* Use memmove for overlapping in parallel region */
            if (thread_id % 2 == 0) {
                __builtin_memmove(&shared_buf[offset + 32], 
                                 &shared_buf[offset], 32);
            }
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        #pragma omp single
        {
            /* Single thread processes final copy */
            __builtin_memcpy(&token_pool[thread_id * 128], 
                           shared_buf, 512);
        }
    }
}

/* Multi-stage processing */
static unsigned long process_tokens(void) {
    unsigned long hash = 0;
    char processing_buf[2048];
    
    /* Stage 1: Initial copy */
    __builtin_memcpy(processing_buf, token_pool, sizeof(processing_buf));
    
    /* Stage 2: Overlapping move */
    __builtin_memmove(&processing_buf[1024], &processing_buf[512], 1024);
    
    /* Stage 3: Final initialization */
    __builtin_memset(&processing_buf[1536], 0x5A, 512);
    
    /* Compute simple hash */
    for (size_t i = 0; i < sizeof(processing_buf); i++) {
        hash = (hash * 31) + (unsigned char)processing_buf[i];
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive AST creation */
    ASTNode* ast1 = create_ast_recursive(5, "AST_Base_Data_String_1");
    ASTNode* ast2 = create_ast_recursive(4, "AST_Base_Data_String_2");
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Phase 2: Goto-based processing */
    process_with_goto(ast1, ast2);
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 4: Multi-stage token processing */
    unsigned long final_hash = process_tokens();
    
    /* Phase 5: Additional built-in usage patterns */
    {
        char pattern_buf[256];
        char dest_buf[256];
        
        /* memset with volatile length */
        __builtin_memset(pattern_buf, 0x33, (size_t)volatile_len);
        
        /* memcpy with conditional length */
        size_t copy_len = volatile_flag ? 128 : 64;
        __builtin_memcpy(dest_buf, pattern_buf, copy_len);
        
        /* memmove with overlapping */
        __builtin_memmove(&dest_buf[64], &dest_buf[32], 96);
        
        /* Copy to token pool */
        __builtin_memcpy(&token_pool[3072], dest_buf, 128);
    }
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    printf("Test completed. Final hash: %lu\n", final_hash);
    printf("Token pool[1000] = 0x%02X\n", (unsigned char)token_pool[1000]);
    
    return 0;
}
