/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor))
static void init_constructor(void) {
    g_init_flag = 1;
    printf("Constructor: Initializing ASAN environment\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->size = sizeof(ASTNode);
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) pattern[i] = (char)((i + depth) & 0xFF);
    __builtin_memcpy(node->data, pattern, 64);
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_left = depth % 2;
        
        if (use_left) {
            node->left = create_ast(depth - 1);
            goto skip_right;
        }
        
        node->right = create_ast(depth - 1);
        skip_right:
        
        /* Copy between nodes if both exist */
        if (node->left && node->right) {
            __builtin_memmove(node->left->data, node->right->data, 32);
        }
    }
    
    return node;
}

/* Parallel memory operations with OpenMP */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        size_t local_size = g_mem_size + thread_id;
        
        /* Thread-local buffers */
        char* src = (char*)malloc(local_size);
        char* dst = (char*)malloc(local_size);
        
        if (src && dst) {
            /* Initialize with __builtin_memset */
            __builtin_memset(src, thread_id, local_size);
            
            /* Copy with __builtin_memcpy */
            __builtin_memcpy(dst, src, local_size);
            
            /* Move overlapping regions with __builtin_memmove */
            if (local_size > 16) {
                __builtin_memmove(dst + 8, dst, local_size - 8);
            }
            
            /* Verify copy */
            int valid = 1;
            for (size_t i = 0; i < local_size; i++) {
                if (dst[i] != (char)thread_id) valid = 0;
            }
            
            #pragma omp critical
            {
                printf("Thread %d: %s (size=%zu)\n", 
                       thread_id, valid ? "PASS" : "FAIL", local_size);
            }
        }
        
        free(src);
        free(dst);
    }
}

/* Complex token processing with goto jumps */
static size_t process_tokens(const char** tokens, int count) {
    size_t hash = 0;
    char buffer[512];
    int i = 0;
    
    start_loop:
    if (i >= count) goto end_processing;
    
    /* Jump into memory operation block */
    if (tokens[i][0] == 'C') {
        goto copy_block;
    }
    
    normal_processing:
    hash += tokens[i][0];
    i++;
    goto start_loop;
    
    copy_block:
    {
        /* Use __builtin_memmove with goto control flow */
        __builtin_memset(buffer, 0, sizeof(buffer));
        size_t len = strlen(tokens[i]);
        
        if (len < sizeof(buffer)) {
            __builtin_memmove(buffer, tokens[i], len);
            hash += buffer[len/2];
        }
        
        i++;
        goto normal_processing;
    }
    
    end_processing:
    return hash;
}

int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Recursive AST operations */
    printf("\nPhase 1: Creating AST structure\n");
    ASTNode* root = create_ast(4);
    
    if (root) {
        /* Copy between tree levels */
        if (root->left && root->right) {
            __builtin_memcpy(root->data, root->left->data, 32);
            __builtin_memmove(root->right->data, root->data, 32);
        }
    }
    
    /* Phase 2: Parallel memory operations */
    printf("\nPhase 2: Parallel memory operations\n");
    parallel_mem_ops();
    
    /* Phase 3: Token processing with control flow */
    printf("\nPhase 3: Token processing\n");
    const char* tokens[] = {
        "CopyThis", "Data123", "MoveThat", "Test456",
        "Another", "Buffer", "Memory", "Operation"
    };
    
    size_t token_hash = process_tokens(tokens, 8);
    printf("Token hash: %zu\n", token_hash);
    
    /* Phase 4: Variable-sized operations */
    printf("\nPhase 4: Variable-sized memory operations\n");
    volatile size_t dynamic_size = g_mem_size;
    char* large_src = (char*)malloc(dynamic_size * 2);
    char* large_dst = (char*)malloc(dynamic_size * 2);
    
    if (large_src && large_dst) {
        /* Chain of built-in calls */
        __builtin_memset(large_src, 0xAA, dynamic_size);
        __builtin_memcpy(large_dst, large_src, dynamic_size);
        __builtin_memmove(large_dst + dynamic_size/2, large_dst, dynamic_size/2);
        __builtin_memset(large_src + dynamic_size, 0xBB, dynamic_size);
        
        /* Verify final pattern */
        int valid = (large_dst[dynamic_size/4] == 0xAA);
        printf("Dynamic operations: %s\n", valid ? "VALID" : "INVALID");
    }
    
    /* Cleanup */
    free(large_src);
    free(large_dst);
    
    /* Free AST recursively */
    void free_ast(ASTNode* node) {
        if (!node) return;
        free_ast(node->left);
        free_ast(node->right);
        free(node);
    }
    free_ast(root);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
