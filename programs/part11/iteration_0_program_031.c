/* asan_coverage.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    char data[256];
    struct ast_node* left;
    struct ast_node* right;
    size_t size;
};

/* Global token array */
static const char* tokens[] = {
    "memcpy", "memset", "memmove",
    "asan", "hwasan", "instrument",
    "redzone", "shadow", "poison"
};
static const size_t num_tokens = sizeof(tokens) / sizeof(tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_sanitizer_hook(void) {
    volatile char buffer[64];
    /* Force initialization of memcpy redirection */
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("[constructor] Initialized sanitizer hooks\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_sanitizer_hook(void) {
    volatile char cleanup_buf[32];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    printf("[destructor] Cleaned up sanitizer state\n");
}

/* Recursive parser with memory operations */
static struct ast_node* parse_expression(int depth, const char** token_ptr) {
    if (depth <= 0 || *token_ptr == NULL) {
        return NULL;
    }
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(*node));
    
    /* Copy token data with memcpy */
    const char* current_token = *token_ptr;
    size_t token_len = strlen(current_token);
    size_t copy_len = token_len < sizeof(node->data) - 1 ? token_len : sizeof(node->data) - 1;
    
    __builtin_memcpy(node->data, current_token, copy_len);
    node->data[copy_len] = '\0';
    node->size = copy_len;
    
    /* Move to next token with goto for flow control */
    token_ptr++;
    if (*token_ptr == NULL) {
        token_ptr = tokens;
    }
    
    /* Recursive calls with goto jumps */
    if (depth > 1) {
        int use_left = (depth % 2) == 0;
        
        if (use_left) {
            goto build_left;
        } else {
            goto build_right;
        }
        
    build_left:
        node->left = parse_expression(depth - 1, token_ptr);
        goto skip_right;
        
    build_right:
        node->right = parse_expression(depth - 1, token_ptr);
        goto skip_left;
        
    skip_right:
        node->right = parse_expression(depth - 2, token_ptr);
        goto done;
        
    skip_left:
        node->left = parse_expression(depth - 2, token_ptr);
        goto done;
    }
    
done:
    return node;
}

/* Compute hash of AST */
static size_t compute_ast_hash(struct ast_node* root) {
    if (!root) return 0;
    
    size_t hash = 5381;
    char* ptr = root->data;
    
    /* Use builtin memcpy in loop */
    char buffer[256];
    __builtin_memcpy(buffer, root->data, root->size);
    buffer[root->size] = '\0';
    
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    /* Move hash data */
    size_t hash_copy;
    __builtin_memmove(&hash_copy, &hash, sizeof(hash));
    
    return hash_copy + compute_ast_hash(root->left) + compute_ast_hash(root->right);
}

/* Free AST with memory clearing */
static void free_ast(struct ast_node* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear sensitive data before free */
    volatile char* data_ptr = (volatile char*)node;
    for (size_t i = 0; i < sizeof(*node); i++) {
        data_ptr[i] = 0;
    }
    
    free(node);
}

/* Parallel memory operations */
static void parallel_memory_operations(void) {
    const size_t num_workers = 4;
    size_t results[num_workers];
    
    #pragma omp parallel num_threads(num_workers)
    {
        int thread_id = omp_get_thread_num();
        size_t local_size = g_mem_size / num_workers;
        
        /* Thread-local buffers */
        char src_buffer[512];
        char dst_buffer[512];
        
        /* Initialize with memset */
        __builtin_memset(src_buffer, thread_id, sizeof(src_buffer));
        __builtin_memset(dst_buffer, 0, sizeof(dst_buffer));
        
        /* Copy with memcpy */
        __builtin_memcpy(dst_buffer, src_buffer, local_size < sizeof(src_buffer) ? local_size : sizeof(src_buffer));
        
        /* Move data around with memmove (overlapping regions) */
        if (thread_id % 2 == 0) {
            __builtin_memmove(dst_buffer + 128, dst_buffer, 256);
        }
        
        /* Compute thread result */
        size_t sum = 0;
        for (size_t i = 0; i < sizeof(dst_buffer); i++) {
            sum += dst_buffer[i];
        }
        
        results[thread_id] = sum;
        
        #pragma omp barrier
        
        /* Master thread consolidates with more memory ops */
        #pragma omp master
        {
            char master_buffer[1024];
            __builtin_memset(master_buffer, 0, sizeof(master_buffer));
            
            for (int i = 0; i < num_workers; i++) {
                __builtin_memcpy(master_buffer + i * 64, &results[i], sizeof(size_t));
            }
            
            /* Final memmove for consolidation */
            __builtin_memmove(master_buffer, master_buffer + 128, 256);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST parsing with memory ops */
    const char* token_ptr = tokens;
    struct ast_node* ast_root = parse_expression(5, &token_ptr);
    
    if (!ast_root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Compute and verify AST hash */
    size_t ast_hash = compute_ast_hash(ast_root);
    printf("AST hash: %zu\n", ast_hash);
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_operations();
    printf("Parallel operations completed\n");
    
    /* Phase 4: Direct built-in calls with volatile control */
    volatile char test_buffer[1024];
    volatile char src_buffer[1024];
    
    /* Initialize source with pattern */
    for (size_t i = 0; i < sizeof(src_buffer); i++) {
        src_buffer[i] = (char)(i % 256);
    }
    
    /* Test all three builtins with volatile sizes */
    volatile size_t copy_size = g_mem_size % 512;
    
    __builtin_memset(test_buffer, 0xAA, copy_size);
    __builtin_memcpy(test_buffer, src_buffer, copy_size);
    __builtin_memmove(test_buffer + 256, test_buffer, copy_size / 2);
    
    /* Phase 5: Nested memory operations in complex control flow */
    {
        char* dynamic_buf = malloc(2048);
        if (dynamic_buf) {
            /* Chain of memory operations */
            __builtin_memset(dynamic_buf, 0, 2048);
            
            for (int i = 0; i < 10; i++) {
                size_t offset = (i * 137) % 2048; /* Non-aligned offsets */
                size_t len = (i * 89) % 256;
                
                if (i % 3 == 0) {
                    __builtin_memcpy(dynamic_buf + offset, src_buffer, len);
                } else if (i % 3 == 1) {
                    __builtin_memset(dynamic_buf + offset, i, len);
                } else {
                    __builtin_memmove(dynamic_buf + offset + 64, dynamic_buf + offset, len);
                }
            }
            
            free(dynamic_buf);
        }
    }
    
    /* Cleanup */
    free_ast(ast_root);
    
    printf("Test completed successfully\n");
    return 0;
}
