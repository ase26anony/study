/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
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
    size_t len;
    uint32_t hash;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 256 + (rand() % 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with volatile-controlled memset */
    volatile size_t init_size = sizeof(ASTNode);
    __builtin_memset(node, 0, init_size);
    
    /* Copy data with builtin memcpy */
    volatile size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) 
        copy_len = sizeof(node->data);
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->len = copy_len;
    
    /* Create children with goto for flow control */
    if (depth > 1) {
        char child_data[32];
        volatile int use_left = depth % 2;
        
        if (use_left) {
            /* Jump into block with memmove */
            goto create_left;
        } else {
            goto create_right;
        }
        
    create_left:
        __builtin_snprintf(child_data, sizeof(child_data), 
                          "%s-L%d", base_data, depth);
        node->left = create_ast(depth - 1, child_data);
        if (!use_left) goto skip_right;
        
    create_right:
        __builtin_snprintf(child_data, sizeof(child_data), 
                          "%s-R%d", base_data, depth);
        node->right = create_ast(depth - 1, child_data);
        
    skip_right:
        ; /* Empty statement for label */
    }
    
    return node;
}

/* Compute hash with memory operations */
static uint32_t compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    uint32_t hash = 5381;
    char buffer[128];
    
    /* Use memmove with overlapping regions */
    volatile size_t move_len = node->len;
    if (move_len > sizeof(buffer)) 
        move_len = sizeof(buffer);
    
    /* Test overlapping memmove */
    __builtin_memcpy(buffer, node->data, move_len);
    __builtin_memmove(buffer + 10, buffer, move_len - 10);
    
    /* Hash computation */
    for (size_t i = 0; i < move_len; i++) {
        hash = ((hash << 5) + hash) + buffer[i];
    }
    
    /* Recursive hash combination */
    uint32_t left_hash = compute_ast_hash(node->left);
    uint32_t right_hash = compute_ast_hash(node->right);
    
    /* Combine with memcpy */
    uint32_t child_hashes[2];
    volatile size_t hash_copy_size = sizeof(child_hashes);
    __builtin_memset(child_hashes, 0, hash_copy_size);
    __builtin_memcpy(&child_hashes[0], &left_hash, sizeof(left_hash));
    __builtin_memcpy(&child_hashes[1], &right_hash, sizeof(right_hash));
    
    return hash ^ child_hashes[0] ^ child_hashes[1];
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
        char src_buffer[512];
        char dst_buffer[512];
        
        /* Initialize with memset */
        volatile size_t init_size = sizeof(src_buffer);
        __builtin_memset(src_buffer, thread_id + 'A', init_size);
        
        /* Copy with memcpy */
        volatile size_t copy_size = g_mem_size;
        if (copy_size > sizeof(src_buffer)) 
            copy_size = sizeof(src_buffer);
        
        __builtin_memcpy(dst_buffer, src_buffer, copy_size);
        
        /* Move with memmove (potential overlap) */
        volatile int overlap_offset = thread_id * 16;
        if (overlap_offset + copy_size > sizeof(dst_buffer))
            overlap_offset = 0;
        
        __builtin_memmove(dst_buffer + overlap_offset, 
                         dst_buffer, 
                         copy_size);
        
        #pragma omp critical
        {
            printf("Thread %d: Memory ops completed (size=%zu)\n", 
                   thread_id, copy_size);
        }
    }
}

/* Complex token processing */
static void process_token_array(const char** tokens, size_t count) {
    char combined[1024];
    size_t offset = 0;
    
    for (size_t i = 0; i < count; i++) {
        volatile size_t token_len = strlen(tokens[i]);
        if (offset + token_len >= sizeof(combined)) 
            break;
        
        /* Use all three builtins */
        if (i % 3 == 0) {
            __builtin_memcpy(combined + offset, tokens[i], token_len);
        } else if (i % 3 == 1) {
            __builtin_memmove(combined + offset, tokens[i], token_len);
        } else {
            /* Clear then copy */
            __builtin_memset(combined + offset, 0, token_len);
            __builtin_memcpy(combined + offset, tokens[i], token_len);
        }
        
        offset += token_len;
        if (i < count - 1) {
            combined[offset++] = ' ';
        }
    }
    
    combined[offset] = '\0';
    printf("Combined tokens: %s\n", combined);
}

int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Recursive AST operations */
    printf("\nPhase 1: Creating AST structure\n");
    ASTNode* root = create_ast(4, "ROOT");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    uint32_t ast_hash = compute_ast_hash(root);
    printf("AST hash: 0x%08X\n", ast_hash);
    
    /* Phase 2: Parallel memory operations */
    printf("\nPhase 2: Parallel memory operations\n");
    parallel_memory_ops();
    
    /* Phase 3: Token processing */
    printf("\nPhase 3: Token array processing\n");
    const char* tokens[] = {
        "memcpy", "memset", "memmove", 
        "asan", "hwasan", "instrumentation",
        "redzone", "builtin", "coverage"
    };
    process_token_array(tokens, sizeof(tokens)/sizeof(tokens[0]));
    
    /* Phase 4: Edge case with goto and volatile */
    printf("\nPhase 4: Goto flow control test\n");
    {
        char buffer1[256];
        char buffer2[256];
        volatile int use_goto = 1;
        
        __builtin_memset(buffer1, 'X', sizeof(buffer1));
        
        if (use_goto) {
            goto perform_memmove;
        }
        
        __builtin_memcpy(buffer2, buffer1, 128);
        goto skip_memmove;
        
    perform_memmove:
        __builtin_memmove(buffer2, buffer1, 128);
        
    skip_memmove:
        /* Verify copy */
        int match = 1;
        for (int i = 0; i < 128; i++) {
            if (buffer2[i] != 'X') {
                match = 0;
                break;
            }
        }
        printf("Goto test: %s\n", match ? "PASS" : "FAIL");
    }
    
    /* Cleanup */
    /* Note: In real code, would need proper AST freeing */
    free(root);
    
    printf("\n=== Test completed ===\n");
    return 0;
}
