/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "instrument"
};
static const size_t g_token_count = sizeof(g_tokens) / sizeof(g_tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char buffer[64];
    /* Force initialization with builtins */
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, "constructor_init", 16);
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[32];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    printf("Destructor: Cleanup completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* data, size_t depth) {
    if (depth == 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile control */
    volatile size_t copy_size = (depth < 256) ? depth : 256;
    __builtin_memset(node, 0, sizeof(ASTNode));
    __builtin_memcpy(node->data, data, copy_size);
    node->size = copy_size;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_left = (depth % 2 == 0);
        
        if (use_left) {
            goto create_left;
        } else {
            goto create_right;
        }
        
    create_left:
        node->left = create_ast_node("left_branch", depth - 1);
        goto skip_right;
        
    create_right:
        node->right = create_ast_node("right_branch", depth - 1);
        goto skip_left;
        
    skip_right:
        node->right = NULL;
        goto done;
        
    skip_left:
        node->left = NULL;
        goto done;
    }
    
done:
    return node;
}

/* Memory operation between AST nodes */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    volatile size_t copy_len = dest->size;
    if (src->size < copy_len) copy_len = src->size;
    
    /* Critical builtin call with goto */
    if (copy_len > 128) {
        goto large_copy;
    } else {
        goto small_copy;
    }
    
large_copy:
    __builtin_memmove(dest->data, src->data, copy_len);
    goto copy_done;
    
small_copy:
    __builtin_memcpy(dest->data, src->data, copy_len);
    goto copy_done;
    
copy_done:
    /* Verify with memset pattern */
    char verify_buf[256];
    __builtin_memset(verify_buf, 0xAA, copy_len);
    __builtin_memcpy(verify_buf, dest->data, copy_len);
}

/* Parallel memory dispatch logic */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        volatile char local_buf[512];
        volatile int thread_id = omp_get_thread_num();
        
        /* Each thread uses different builtins */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(local_buf, thread_id, sizeof(local_buf));
                break;
            case 1:
                __builtin_memcpy(local_buf, g_tokens[thread_id % g_token_count], 
                                sizeof(local_buf) < 64 ? sizeof(local_buf) : 64);
                break;
            case 2:
                __builtin_memmove(local_buf + 128, local_buf, 256);
                break;
        }
        
        /* Nested parallel region for complex flow */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            volatile char nested_buf[128];
            __builtin_memset(nested_buf, i, sizeof(nested_buf));
            
            if (i % 10 == 0) {
                goto skip_memmove;
            }
            
            __builtin_memmove(nested_buf + 32, nested_buf, 64);
            
        skip_memmove:
            __builtin_memcpy(nested_buf + 96, nested_buf, 32);
        }
    }
}

/* Compute hash from AST */
static size_t compute_ast_hash(const ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 5381;
    volatile size_t i = 0;
    
    /* Process data with builtin assistance */
    char temp_buf[256];
    __builtin_memcpy(temp_buf, node->data, node->size);
    
    for (i = 0; i < node->size; i++) {
        hash = ((hash << 5) + hash) + temp_buf[i];
    }
    
    /* Recursive hash computation */
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear sensitive data before free */
    volatile char* data_ptr = node->data;
    volatile size_t clear_size = node->size;
    __builtin_memset(data_ptr, 0, clear_size);
    
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Initialize and create AST */
    ASTNode* root = create_ast_node("root_data", 4);
    ASTNode* copy = create_ast_node("copy_target", 4);
    
    if (!root || !copy) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Phase 2: Perform memory operations between nodes */
    copy_ast_data(copy, root);
    
    /* Phase 3: Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 4: Complex memory pattern with gotos */
    volatile char pattern_buf[1024];
    volatile size_t offset = 0;
    
    for (int i = 0; i < 3; i++) {
        if (i == 0) goto pattern_memset;
        if (i == 1) goto pattern_memcpy;
        if (i == 2) goto pattern_memmove;
        
    pattern_memset:
        __builtin_memset(pattern_buf + offset, 0xCC, 256);
        offset += 256;
        continue;
        
    pattern_memcpy:
        __builtin_memcpy(pattern_buf + offset, g_tokens[i % g_token_count], 128);
        offset += 128;
        continue;
        
    pattern_memmove:
        __builtin_memmove(pattern_buf + 512, pattern_buf, 256);
        break;
    }
    
    /* Phase 5: Compute and verify result */
    size_t root_hash = compute_ast_hash(root);
    size_t copy_hash = compute_ast_hash(copy);
    
    printf("Root AST hash: %zu\n", root_hash);
    printf("Copy AST hash: %zu\n", copy_hash);
    printf("Hash difference: %zu\n", 
           root_hash > copy_hash ? root_hash - copy_hash : copy_hash - root_hash);
    
    /* Phase 6: Cleanup */
    free_ast(root);
    free_ast(copy);
    
    /* Final builtin calls */
    volatile char final_buf[64];
    __builtin_memset(final_buf, 0xFE, sizeof(final_buf));
    __builtin_memcpy(final_buf + 32, final_buf, 32);
    
    printf("ASAN test completed successfully\n");
    return 0;
}
