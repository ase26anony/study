/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
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
static char global_tokens[4096];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    printf("Constructor: Initialized global tokens\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtin memcpy in destructor */
    char local_buf[64];
    __builtin_memcpy(local_buf, global_tokens, 64);
    printf("Destructor: Cleaned up resources\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memcpy for node initialization */
    __builtin_memcpy(node->data, base_data, strlen(base_data) + 1);
    node->id = depth;
    
    /* Create children with goto-controlled flow */
    int use_goto = (depth % 2 == 0);
    
    if (use_goto) {
        goto create_left;
    }
    
    node->left = create_ast(depth - 1, "LeftChild");
    
create_left:
    if (use_goto) {
        node->left = create_ast(depth - 1, "LeftGoto");
        goto create_right;
    }
    
    node->right = create_ast(depth - 1, "RightChild");
    return node;

create_right:
    if (use_goto) {
        node->right = create_ast(depth - 1, "RightGoto");
    }
    
    return node;
}

/* Memory operation between AST nodes */
static void copy_ast_data(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use builtin memmove with volatile length */
    size_t copy_len = volatile_len % 256;
    
    /* Jump into memory operation block */
    if (volatile_flag) {
        goto do_memmove;
    }
    
    __builtin_memcpy(dest->data, src->data, copy_len);
    return;
    
do_memmove:
    /* This tests flow-sensitivity of asan_memfn_rtls */
    __builtin_memmove(dest->data, src->data, copy_len);
    
    /* Jump out of block */
    if (copy_len > 128) {
        goto finish_copy;
    }
    
    /* Additional memset */
    __builtin_memset(dest->data + copy_len, 0, 256 - copy_len);
    
finish_copy:
    return;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf1[1024];
        char local_buf2[1024];
        
        /* Initialize with builtin memset */
        __builtin_memset(local_buf1, thread_id, sizeof(local_buf1));
        
        /* Copy between buffers */
        __builtin_memcpy(local_buf2, local_buf1, sizeof(local_buf1));
        
        /* Overlapping memory move */
        size_t offset = volatile_len % 512;
        __builtin_memmove(local_buf1 + offset, local_buf1, 256);
        
        /* Update global tokens (potential race condition for ASAN to detect) */
        #pragma omp critical
        {
            size_t copy_size = 32 + (thread_id * 16);
            if (copy_size < sizeof(global_tokens) - token_index) {
                __builtin_memcpy(global_tokens + token_index, 
                               local_buf1, 
                               copy_size);
                token_index += copy_size;
            }
        }
    }
}

/* Calculate hash of AST tree */
static unsigned long long hash_ast(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long long hash = 5381;
    char* ptr = node->data;
    
    /* Simple hash calculation */
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    /* Recursive hash combination */
    hash += hash_ast(node->left);
    hash += hash_ast(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Create and manipulate AST */
    ASTNode* root = create_ast(4, "RootNode");
    ASTNode* copy = create_ast(3, "CopyNode");
    
    if (root && copy) {
        /* Test memory operations between nodes */
        copy_ast_data(copy, root);
        
        /* Create overlapping copy */
        __builtin_memmove(root->data + 128, root->data, 128);
        
        /* Calculate verification hash */
        unsigned long long hash1 = hash_ast(root);
        unsigned long long hash2 = hash_ast(copy);
        
        printf("AST Hash Results:\n");
        printf("  Root hash: %llu\n", hash1);
        printf("  Copy hash: %llu\n", hash2);
        printf("  Hash sum: %llu\n", hash1 + hash2);
    }
    
    /* Phase 2: Parallel memory operations */
    printf("\nExecuting parallel memory operations...\n");
    parallel_memory_operations();
    
    /* Phase 3: Verify global token array */
    printf("\nVerifying global tokens...\n");
    size_t total_set = 0;
    for (int i = 0; i < token_index; i++) {
        if (global_tokens[i] != 0) {
            total_set++;
        }
    }
    
    /* Final builtin memset for verification */
    char verify_buf[256];
    __builtin_memset(verify_buf, 0, sizeof(verify_buf));
    __builtin_memcpy(verify_buf, global_tokens, 
                    (token_index < 256) ? token_index : 256);
    
    /* Calculate final checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += (unsigned char)verify_buf[i];
    }
    
    printf("Verification complete:\n");
    printf("  Tokens set: %zu/%d\n", total_set, token_index);
    printf("  Final checksum: %u\n", checksum);
    
    /* Cleanup */
    /* Note: In real code, you'd need to free the AST properly */
    
    return 0;
}
