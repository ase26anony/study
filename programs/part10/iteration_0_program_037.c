/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Forward declarations for attribute functions */
void __attribute__((constructor)) init_asan_test(void);
void __attribute__((destructor)) cleanup_asan_test(void);

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    volatile int depth;  /* Prevent optimization */
} ASTNode;

/* Global volatile variables to prevent constant folding */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Token array for parser simulation */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
#define TOKEN_COUNT (sizeof(tokens)/sizeof(tokens[0]))

/* Recursive parser with memory operations */
ASTNode* parse_expression(int depth, const char** token_ptr) {
    if (depth <= 0 || **token_ptr == '\0') {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with volatile memset */
    volatile char* dest = node->data;
    volatile size_t n = sizeof(node->data);
    __builtin_memset((void*)dest, 0, n);
    
    /* Copy token using builtin memcpy */
    const char* current_token = *token_ptr;
    size_t token_len = strlen(current_token);
    if (token_len > sizeof(node->data) - 1) {
        token_len = sizeof(node->data) - 1;
    }
    
    __builtin_memcpy(node->data, current_token, token_len);
    node->data[token_len] = '\0';
    
    node->depth = depth;
    
    /* Move to next token with goto for flow control */
    int use_goto = (depth % 2 == 0);
    
    if (use_goto) {
        goto skip_left;
    }
    
    /* Recursive left child */
    (*token_ptr)++;
    if (**token_ptr) {
        node->left = parse_expression(depth - 1, token_ptr);
    } else {
        node->left = NULL;
    }
    
skip_left:
    /* Right child with memmove between nodes */
    if (node->left) {
        ASTNode temp;
        __builtin_memcpy(&temp, node->left, sizeof(ASTNode));
        __builtin_memmove(node->data + 32, temp.data, 32);
    }
    
    (*token_ptr)++;
    if (**token_ptr && depth > 1) {
        node->right = parse_expression(depth - 2, token_ptr);
        
        /* Complex memmove with overlapping regions */
        if (node->right) {
            char buffer[128];
            __builtin_memcpy(buffer, node->data, sizeof(node->data));
            __builtin_memmove(node->data, node->right->data, 32);
            __builtin_memmove(node->right->data, buffer, 32);
        }
    } else {
        node->right = NULL;
    }
    
    return node;
}

/* Calculate hash of AST tree */
uint64_t hash_ast(ASTNode* node) {
    if (!node) return 0;
    
    uint64_t hash = 5381;
    char* p = node->data;
    
    /* DJB2 hash algorithm */
    while (*p) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    /* Recursive hash combination */
    uint64_t left_hash = hash_ast(node->left);
    uint64_t right_hash = hash_ast(node->right);
    
    /* Mix hashes with memory operations */
    uint64_t hashes[3] = {hash, left_hash, right_hash};
    uint64_t result;
    
    __builtin_memcpy(&result, hashes, sizeof(uint64_t));
    __builtin_memmove(hashes + 1, hashes, 2 * sizeof(uint64_t));
    __builtin_memset(hashes, 0, sizeof(uint64_t));
    
    return result ^ left_hash ^ right_hash;
}

/* OpenMP parallel memory operations */
void parallel_memory_ops(void) {
    const int array_size = 1024;
    char* src = (char*)malloc(array_size);
    char* dst = (char*)malloc(array_size);
    
    if (!src || !dst) {
        free(src);
        free(dst);
        return;
    }
    
    /* Initialize source with pattern */
    for (int i = 0; i < array_size; i++) {
        src[i] = (char)(i % 256);
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int chunk_size = array_size / omp_get_num_threads();
        int start = thread_id * chunk_size;
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:  /* memcpy */
                __builtin_memcpy(dst + start, src + start, chunk_size);
                break;
            case 1:  /* memset */
                __builtin_memset(dst + start, thread_id, chunk_size);
                break;
            case 2:  /* memmove with overlap */
                if (start + chunk_size + 16 < array_size) {
                    __builtin_memmove(dst + start + 8, src + start, chunk_size);
                }
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Verify with another memcpy */
        if (thread_id == 0) {
            char verify[64];
            __builtin_memcpy(verify, dst, 64);
            __builtin_memset(verify + 32, 0xFF, 32);
        }
    }
    
    /* Cleanup with volatile size */
    volatile size_t cleanup_size = array_size;
    __builtin_memset(src, 0, cleanup_size);
    __builtin_memset(dst, 0, cleanup_size);
    
    free(src);
    free(dst);
}

/* Constructor - runs before main */
void __attribute__((constructor)) init_asan_test(void) {
    printf("ASAN Test Initializer\n");
    
    /* Force early initialization of memory functions */
    char init_buf[16];
    volatile int init_flag = 1;
    
    if (init_flag) {
        __builtin_memset(init_buf, 0xAA, sizeof(init_buf));
        __builtin_memcpy(init_buf + 8, init_buf, 8);
        __builtin_memmove(init_buf, init_buf + 4, 12);
    }
}

/* Destructor - runs after main */
void __attribute__((destructor)) cleanup_asan_test(void) {
    printf("ASAN Test Cleanup\n");
    
    /* Final memory operations */
    char final_buf[32];
    volatile size_t final_size = sizeof(final_buf);
    
    __builtin_memset(final_buf, 0, final_size);
    __builtin_memcpy(final_buf, "CLEANUP", 8);
    __builtin_memmove(final_buf + 16, final_buf, 16);
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN Built-in Redirection Test\n");
    
    /* Phase 1: Recursive AST parsing with memory ops */
    const char* token_ptr = tokens[0];
    ASTNode* root = parse_expression(4, &token_ptr);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Calculate and print hash */
    uint64_t hash = hash_ast(root);
    printf("AST Hash: 0x%016llx\n", (unsigned long long)hash);
    
    /* Phase 3: OpenMP parallel operations */
    printf("Starting parallel memory operations...\n");
    parallel_memory_ops();
    printf("Parallel operations completed\n");
    
    /* Phase 4: Complex memory operation sequence */
    char buffer1[256];
    char buffer2[256];
    volatile size_t op_size = g_mem_size;
    
    /* Sequence testing all three builtins */
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, op_size);
    
    for (int i = 0; i < 3; i++) {
        /* Overlapping memmove */
        __builtin_memmove(buffer1 + 64, buffer1 + 32, 128);
        
        /* Partial memset */
        __builtin_memset(buffer2 + i * 32, i * 16, 32);
        
        /* Cross-copy */
        __builtin_memcpy(buffer1 + i * 16, buffer2 + (2 - i) * 16, 32);
    }
    
    /* Phase 5: Cleanup AST */
    /* Recursive free function */
    void free_ast(ASTNode* node) {
        if (!node) return;
        
        /* Clear node data before free */
        volatile char* data = node->data;
        __builtin_memset((void*)data, 0, sizeof(node->data));
        
        free_ast(node->left);
        free_ast(node->right);
        
        /* Final memset before free */
        __builtin_memset(node, 0, sizeof(ASTNode));
        free(node);
    }
    
    free_ast(root);
    
    printf("ASAN Test Completed Successfully\n");
    return 0;
}
