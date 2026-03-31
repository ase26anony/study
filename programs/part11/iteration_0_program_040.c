/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 16;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan",
    "instrument", "redzone", "builtin", "coverage", "test"
};
static const int token_count = 10;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < 256; i++) {
        volatile_src[i] = (char)(i % 256);
    }
    
    /* Force early built-in usage in constructor */
    __builtin_memset(volatile_dest, 0xAA, sizeof(volatile_dest));
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operations in destructor */
    char final_buf[128];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
    __builtin_memmove(final_buf + 32, final_buf, 64);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with built-ins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy token into node data */
    const char* token = tokens[id % token_count];
    size_t token_len = __builtin_strlen(token);
    if (token_len < sizeof(node->data)) {
        __builtin_memcpy(node->data, token, token_len);
    }
    
    node->id = id;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    /* Jump into block with memmove */
    {
        char temp_buf[64];
        __builtin_memcpy(temp_buf, node->data, sizeof(node->data));
        __builtin_memmove(node->data + 16, node->data, 32);
        __builtin_memcpy(node->data, temp_buf, 16);
    }
    
    node->left = create_ast_node(depth - 1, id * 2);
    node->right = create_ast_node(depth - 1, id * 2 + 1);
    
done:
    return node;
}

/* Complex memory operation with goto jumps */
static void complex_memory_operations(void) {
    char buffer1[256];
    char buffer2[256];
    char buffer3[256];
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 0x11, sizeof(buffer1));
    __builtin_memset(buffer2, 0x22, sizeof(buffer2));
    __builtin_memset(buffer3, 0x33, sizeof(buffer3));
    
    int operation = 0;
    
operation_loop:
    if (operation >= 3) goto operations_done;
    
    switch (operation) {
        case 0:
            /* Jump into memcpy block */
            goto do_memcpy;
        case 1:
            /* Jump into memset block */
            goto do_memset;
        case 2:
            /* Jump into memmove block */
            goto do_memmove;
    }
    
do_memcpy:
    {
        /* Use volatile length */
        int len = volatile_len + operation * 8;
        __builtin_memcpy(buffer2, buffer1, len);
        operation++;
        goto operation_loop;
    }
    
do_memset:
    {
        /* Pattern based on operation */
        char pattern = (char)(0x40 + operation * 16);
        __builtin_memset(buffer3, pattern, 128);
        operation++;
        goto operation_loop;
    }
    
do_memmove:
    {
        /* Overlapping memory move */
        __builtin_memmove(buffer1 + 64, buffer1, 128);
        operation++;
        goto operation_loop;
    }
    
operations_done:
    /* Final consolidation */
    __builtin_memcpy(buffer1, buffer2, 64);
    __builtin_memcpy(buffer1 + 64, buffer3, 64);
}

/* Parallel memory dispatch with OpenMP */
static void parallel_memory_dispatch(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buf[128];
        char shared_buf[256];
        
        /* Thread-specific initialization */
        __builtin_memset(local_buf, (char)thread_id, sizeof(local_buf));
        
        #pragma omp critical
        {
            /* Critical section with memory operations */
            __builtin_memcpy(shared_buf + thread_id * 32, local_buf, 32);
            __builtin_memset(local_buf, 0, 16);
            __builtin_memmove(local_buf + 16, local_buf, 48);
        }
        
        #pragma omp barrier
        
        #pragma omp single
        {
            /* Single thread consolidates results */
            __builtin_memset(shared_buf + 224, 0xFF, 32);
        }
    }
}

/* Calculate hash from AST */
static unsigned long calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    
    /* Hash node data */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + (unsigned long)node->data[i];
    }
    
    /* Recursive hash calculation */
    hash += calculate_ast_hash(node->left);
    hash += calculate_ast_hash(node->right);
    
    return hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Recursive AST creation with memory ops */
    ASTNode* root = create_ast_node(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Complex memory operations with goto */
    complex_memory_operations();
    
    /* Phase 3: OpenMP parallel memory dispatch */
    #ifdef _OPENMP
    parallel_memory_dispatch();
    #endif
    
    /* Phase 4: Memory operations between AST nodes */
    if (root->left && root->right) {
        /* Copy data between child nodes */
        __builtin_memcpy(root->left->data + 32, root->right->data, 32);
        __builtin_memmove(root->right->data, root->left->data, 32);
        
        /* Set pattern in parent */
        __builtin_memset(root->data, 0xCC, 16);
    }
    
    /* Phase 5: Additional built-in stress tests */
    {
        char final_buffer[512];
        char* dynamic_buffer = (char*)malloc(256);
        
        if (dynamic_buffer) {
            /* Mix of static and dynamic memory operations */
            __builtin_memset(dynamic_buffer, 0x55, 256);
            __builtin_memcpy(final_buffer, dynamic_buffer, 128);
            __builtin_memmove(final_buffer + 128, final_buffer, 256);
            __builtin_memset(final_buffer + 384, 0x66, 128);
            
            free(dynamic_buffer);
        }
    }
    
    /* Calculate and print verification result */
    unsigned long hash = calculate_ast_hash(root);
    printf("AST Hash: %lu\n", hash);
    
    /* Cleanup */
    free_ast(root);
    
    /* Final memory operation in main */
    char exit_buf[64];
    __builtin_memset(exit_buf, 0, sizeof(exit_buf));
    __builtin_memcpy(exit_buf, "TestComplete", 13);
    
    printf("ASAN test completed successfully\n");
    return 0;
}
