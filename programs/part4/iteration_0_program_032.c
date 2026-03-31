/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[128];
static volatile char volatile_src[128];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {
    "memcpy_test", "memset_test", "memmove_test",
    "recursive", "parallel", "asan"
};
static const int token_count = sizeof(tokens) / sizeof(tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < 128; i++) {
        volatile_src[i] = (char)(i % 26 + 'A');
    }
    
    /* Force early built-in usage in constructor */
    __builtin_memset(volatile_dest, 0xCC, volatile_len);
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operations in destructor */
    char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
    __builtin_memmove(final_buf + 16, final_buf, 32);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with built-ins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy token based on ID */
    const char* token = tokens[id % token_count];
    size_t len = strlen(token);
    if (len > sizeof(node->data) - 1) len = sizeof(node->data) - 1;
    
    __builtin_memcpy(node->data, token, len);
    node->data[len] = '\0';
    node->id = id;
    
    /* Recursive creation with goto for flow control */
    int use_goto = (id % 3 == 0);
    
    if (use_goto) {
        goto create_children;
    }
    
    node->left = create_ast_node(depth - 1, id * 2);
    node->right = NULL;
    
create_children:
    if (!use_goto) {
        node->right = create_ast_node(depth - 1, id * 2 + 1);
    } else {
        node->left = create_ast_node(depth - 1, id * 2);
        node->right = create_ast_node(depth - 1, id * 2 + 1);
    }
    
    /* Memory move between nodes if both children exist */
    if (node->left && node->right) {
        __builtin_memmove(node->left->data + 16, node->right->data, 8);
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void complex_memory_operations(void) {
    char buffer1[256];
    char buffer2[256];
    char buffer3[256];
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 'A', sizeof(buffer1));
    __builtin_memset(buffer2, 'B', sizeof(buffer2));
    __builtin_memset(buffer3, 'C', sizeof(buffer3));
    
    /* Goto jumping into memory operation block */
    int jump_target = 1;
    
    if (jump_target) {
        goto mem_operation_block;
    }
    
    /* This should be skipped */
    __builtin_memset(buffer1, 'X', 32);
    
mem_operation_block:
    /* Memory operations after goto */
    __builtin_memcpy(buffer2, buffer1, volatile_len);
    
    /* Another goto out of block */
    if (buffer2[0] == 'A') {
        goto after_block;
    }
    
    __builtin_memmove(buffer3, buffer2, 128);
    
after_block:
    /* Final operation after goto */
    __builtin_memcpy(buffer1, buffer3, 64);
}

/* OpenMP parallel memory dispatch */
static void parallel_memory_dispatch(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char local_buf[128];
        char shared_buf[128];
        
        /* Thread-specific initialization */
        __builtin_memset(local_buf, thread_id + '0', sizeof(local_buf));
        
        #pragma omp critical
        {
            /* Critical section with memory operations */
            static char critical_buffer[256];
            __builtin_memcpy(shared_buf, local_buf, 64);
            __builtin_memmove(critical_buffer + thread_id * 32, shared_buf, 32);
        }
        
        /* More operations after critical section */
        __builtin_memset(local_buf + 64, 0, 64);
    }
}

/* Calculate hash from AST tree */
static unsigned long calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* ptr = node->data;
    
    /* Simple hash calculation */
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    hash += calculate_ast_hash(node->left);
    hash += calculate_ast_hash(node->right);
    hash += node->id;
    
    return hash;
}

/* Free AST tree */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create recursive AST structure */
    ASTNode* root = create_ast_node(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Perform complex memory operations with goto */
    complex_memory_operations();
    
    /* Execute parallel memory operations */
    #ifdef _OPENMP
    printf("Running with OpenMP parallelization\n");
    #endif
    parallel_memory_dispatch();
    
    /* Additional memory operations in main */
    char main_buffer[512];
    __builtin_memset(main_buffer, 0, sizeof(main_buffer));
    
    /* Copy from volatile source */
    __builtin_memcpy(main_buffer, (void*)volatile_src, 
                     volatile_len > sizeof(main_buffer) ? 
                     sizeof(main_buffer) : volatile_len);
    
    /* Overlapping memory move */
    __builtin_memmove(main_buffer + 128, main_buffer, 256);
    
    /* Calculate and print result */
    unsigned long hash = calculate_ast_hash(root);
    printf("AST hash result: %lu\n", hash);
    
    /* Verify some operations */
    int verification_sum = 0;
    for (int i = 0; i < 64; i++) {
        verification_sum += main_buffer[i];
    }
    printf("Buffer verification sum: %d\n", verification_sum);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully.\n");
    return 0;
}
