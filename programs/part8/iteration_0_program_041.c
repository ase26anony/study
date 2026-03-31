/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    uint64_t hash;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "asan", "hwasan"};
static const int token_count = 5;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Force early initialization of ASAN runtime */
    volatile char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Destructor: ASAN environment cleanup\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Build node data using __builtin_memcpy */
    const char* token = tokens[depth % token_count];
    size_t len = strlen(token);
    __builtin_memcpy(node->data, token, len);
    node->data[len] = '\0';
    
    /* Recursive creation with goto for control flow testing */
    if (depth % 2 == 0) {
        goto create_left;
    } else {
        goto create_right;
    }
    
create_left:
    node->left = create_ast(depth + 1, max_depth);
    if (node->left && g_use_memmove) {
        /* Use __builtin_memmove between nodes */
        __builtin_memmove(&node->hash, &node->left->hash, sizeof(uint64_t));
    }
    goto after_branch;
    
create_right:
    node->right = create_ast(depth + 1, max_depth);
    if (node->right) {
        /* Use __builtin_memcpy for hash propagation */
        __builtin_memcpy(&node->hash, &node->right->hash, sizeof(uint64_t));
    }
    goto after_branch;
    
after_branch:
    /* Compute hash using memory operations */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        node->hash ^= (uint64_t)node->data[i] << ((i % 8) * 8);
    }
    
    return node;
}

/* Function with complex memory operations and goto */
static void process_memory_blocks(void) {
    volatile char src[256];
    volatile char dst[256];
    volatile char temp[256];
    
    /* Initialize with __builtin_memset */
    __builtin_memset(src, 'A', sizeof(src));
    __builtin_memset(dst, 'B', sizeof(dst));
    
    /* Label for goto testing */
    start_copy:
    
    /* Use volatile size to prevent constant folding */
    size_t copy_size = g_mem_size % 128;
    
    /* Force all three builtins to be called */
    if (copy_size > 64) {
        __builtin_memcpy(dst, src, copy_size);
        goto after_memcpy;
    } else if (copy_size > 32) {
        __builtin_memset(dst, 'C', copy_size);
        goto after_memset;
    } else {
        __builtin_memmove(temp, src, copy_size);
        __builtin_memcpy(dst, temp, copy_size);
        goto after_memmove;
    }
    
after_memcpy:
    /* Verify copy with another memcpy */
    __builtin_memcpy(temp, dst, copy_size);
    goto end_processing;
    
after_memset:
    /* Verify memset with memcpy */
    __builtin_memcpy(temp, dst, copy_size);
    goto end_processing;
    
after_memmove:
    /* Chain operations */
    __builtin_memset(temp + copy_size, 'D', 16);
    goto end_processing;
    
end_processing:
    /* Prevent dead code elimination */
    if (dst[0] != temp[0]) {
        __builtin_memcpy(dst, temp, copy_size);
    }
}

/* OpenMP parallel section */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_src[128];
        char local_dst[128];
        
        /* Each thread uses different builtins */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(local_src, thread_id, sizeof(local_src));
                __builtin_memcpy(local_dst, local_src, sizeof(local_src));
                break;
            case 1:
                __builtin_memset(local_dst, 0xFF, sizeof(local_dst));
                __builtin_memmove(local_src, local_dst, sizeof(local_dst) / 2);
                break;
            case 2:
                __builtin_memcpy(local_src, tokens[thread_id % token_count], 
                                strlen(tokens[thread_id % token_count]));
                __builtin_memmove(local_dst, local_src, 64);
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Final memory operation */
        #pragma omp for
        for (int i = 0; i < 10; i++) {
            char tmp[64];
            __builtin_memset(tmp, i, sizeof(tmp));
            __builtin_memcpy(local_dst + i * 4, tmp, 4);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast(0, 4);
    
    /* Phase 2: Complex memory operations with goto */
    process_memory_blocks();
    
    /* Phase 3: OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_operations();
    #endif
    
    /* Phase 4: Verify results */
    uint64_t total_hash = 0;
    ASTNode* nodes[4];
    nodes[0] = root;
    
    /* Use memory operations to traverse and compute */
    for (int i = 0; i < 4 && nodes[i]; i++) {
        char buffer[256];
        __builtin_memcpy(buffer, nodes[i]->data, sizeof(buffer));
        
        /* Mix in volatile variable */
        buffer[g_mem_size % 256] ^= 0x55;
        
        /* Compute final hash */
        for (int j = 0; j < 64; j++) {
            total_hash ^= (uint64_t)buffer[j] << ((j % 8) * 8);
        }
        
        /* Queue next nodes */
        if (nodes[i]->left) {
            if (i + 1 < 4) {
                __builtin_memcpy(&nodes[i + 1], &nodes[i]->left, sizeof(ASTNode*));
            }
        }
    }
    
    /* Cleanup */
    free(root);
    
    printf("Test completed. Final hash: 0x%016llx\n", 
           (unsigned long long)total_hash);
    
    return total_hash != 0 ? 0 : 1;
}
