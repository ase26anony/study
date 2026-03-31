/* ISO C99-compliant program targeting ASAN built-in redirection logic */
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
static char global_tokens[4096];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    
    /* Force redirection path with volatile control */
    if (volatile_flag) {
        char local_buf[128];
        __builtin_memset(local_buf, 0x42, volatile_len % 128);
        __builtin_memcpy(global_tokens + 512, local_buf, 64);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Final memory operation in destructor */
    char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_recursive(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for initialization */
    size_t copy_len = (volatile_len % 128) + 64;
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, copy_len < sizeof(node->data) ? copy_len : sizeof(node->data) - 1);
    node->size = copy_len;
    
    /* Recursive creation with goto for flow control */
    int use_goto = volatile_flag;
    
    if (use_goto) {
        goto recursive_call;
    }
    
    node->left = create_ast_recursive(depth - 1, node->data);
    
recursive_call:
    /* Jump target for goto */
    node->right = create_ast_recursive(depth - 1, node->data + 128);
    
    /* Complex goto pattern around memmove */
    if (depth % 2 == 0) {
        goto memmove_block;
    }
    
    return node;
    
memmove_block:
    /* This block tests goto into memory operations */
    char temp[256];
    __builtin_memcpy(temp, node->data, node->size);
    
    /* Critical: goto jumps into memmove context */
    if (depth > 1) {
        goto perform_memmove;
    }
    
    __builtin_memmove(node->data, temp, node->size);
    goto finish_node;
    
perform_memmove:
    /* Target of goto - tests flow sensitivity */
    __builtin_memmove(node->data + 64, temp + 32, node->size - 32);
    
finish_node:
    return node;
}

/* Parallel memory dispatch with OpenMP */
static void parallel_memory_operations(ASTNode* root) {
    int i;
    char parallel_buf[1024];
    
    #pragma omp parallel private(i)
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (i = 0; i < 16; i++) {
            /* Each thread uses builtins */
            char local_buf[256];
            size_t len = (volatile_len + thread_id * 16) % 128 + 64;
            
            __builtin_memset(local_buf, thread_id, len);
            
            /* Copy to shared buffer with offset */
            size_t offset = (thread_id * 64) % 768;
            __builtin_memcpy(parallel_buf + offset, local_buf, len);
            
            /* Memmove within buffer */
            if (len > 32) {
                __builtin_memmove(parallel_buf + offset + 16, 
                                 parallel_buf + offset, 
                                 len - 16);
            }
        }
        
        /* Additional memory ops outside parallel for */
        if (thread_id == 0 && root) {
            char verify_buf[512];
            __builtin_memcpy(verify_buf, root->data, 
                           root->size < 512 ? root->size : 512);
            __builtin_memset(verify_buf + 256, 0xAA, 128);
        }
    }
    
    /* Post-parallel builtin usage */
    __builtin_memcpy(global_tokens + 1024, parallel_buf, 512);
}

/* Complex control flow with nested memory operations */
static size_t compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 0;
    char hash_buf[512];
    volatile int use_memmove = volatile_flag;
    
    /* Initialize hash buffer */
    __builtin_memset(hash_buf, 0, sizeof(hash_buf));
    
    /* Copy node data with goto pattern */
    if (node->size > 0) {
        goto copy_data;
    }
    
    hash = 1;
    goto compute_children;
    
copy_data:
    __builtin_memcpy(hash_buf, node->data, 
                    node->size < sizeof(hash_buf) ? node->size : sizeof(hash_buf));
    
    /* Conditional memmove */
    if (use_memmove && node->size > 128) {
        __builtin_memmove(hash_buf + 64, hash_buf, 64);
    }
    
compute_children:
    /* Process children */
    size_t left_hash = compute_ast_hash(node->left);
    size_t right_hash = compute_ast_hash(node->right);
    
    /* Combine hashes with memory operation */
    char combine_buf[128];
    __builtin_memset(combine_buf, 0, sizeof(combine_buf));
    __builtin_memcpy(combine_buf, &left_hash, sizeof(left_hash));
    __builtin_memcpy(combine_buf + 64, &right_hash, sizeof(right_hash));
    
    for (size_t i = 0; i < sizeof(combine_buf); i++) {
        hash = hash * 31 + combine_buf[i];
    }
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, node->size);
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Initialize and create AST */
    ASTNode* root = create_ast_recursive(4, "BaseASTData");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Parallel memory operations */
    parallel_memory_operations(root);
    
    /* Phase 3: Compute verification hash */
    size_t final_hash = compute_ast_hash(root);
    
    /* Phase 4: Additional builtin usage in main */
    char result_buf[256];
    __builtin_memset(result_buf, 0, sizeof(result_buf));
    __builtin_memcpy(result_buf, &final_hash, sizeof(final_hash));
    
    /* Final memmove for overlap test */
    if (sizeof(final_hash) * 2 < sizeof(result_buf)) {
        __builtin_memmove(result_buf + 128, result_buf, sizeof(final_hash));
    }
    
    /* Compute printable result */
    unsigned long long display_hash = 0;
    for (size_t i = 0; i < sizeof(result_buf); i++) {
        display_hash = display_hash * 65599 + (unsigned char)result_buf[i];
    }
    
    printf("Result hash: 0x%016llx\n", display_hash);
    printf("Verification: %s\n", 
           (display_hash != 0) ? "PASS" : "FAIL");
    
    /* Cleanup */
    free_ast(root);
    
    /* Final builtin in main */
    __builtin_memset(result_buf, 0, sizeof(result_buf));
    
    return 0;
}
