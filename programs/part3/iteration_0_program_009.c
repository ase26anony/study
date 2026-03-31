/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
static const int g_num_tokens = 6;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_env(void) {
    volatile char buffer[32];
    /* Force initialization of memcpy redirection */
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_env(void) {
    volatile char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    printf("Destructor: ASAN environment cleaned up\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int* node_id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*node_id)++;
    node->left = NULL;
    node->right = NULL;
    
    /* Use memset to initialize node data */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Copy token into node data with control flow jumps */
    int token_idx = node->id % g_num_tokens;
    
    /* Jump into memcpy block */
    if (token_idx < 3) {
        goto do_memcpy;
    } else {
        /* Use memmove for overlapping regions */
        char temp[64];
        __builtin_memcpy(temp, g_tokens[token_idx], 
                        strlen(g_tokens[token_idx]) + 1);
        goto do_memmove;
    }
    
do_memcpy:
    __builtin_memcpy(node->data, g_tokens[token_idx], 
                    strlen(g_tokens[token_idx]) + 1);
    goto after_copy;
    
do_memmove:
    /* Create overlapping memory scenario */
    __builtin_memmove(node->data + 10, node->data, 20);
    __builtin_memmove(node->data, temp, strlen(temp) + 1);
    
after_copy:
    /* Recursive creation with varied memory sizes */
    volatile size_t left_size = g_mem_size / 2;
    volatile size_t right_size = g_mem_size - left_size;
    
    node->left = create_ast(depth - 1, node_id);
    node->right = create_ast(depth - 1, node_id);
    
    return node;
}

/* Process AST with parallel memory operations */
static void process_ast_parallel(ASTNode* root) {
    if (!root) return;
    
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            /* Parallel memory operations on different nodes */
            #pragma omp task
            {
                volatile char task_buf[128];
                __builtin_memset(task_buf, root->id, sizeof(task_buf));
                
                if (root->left) {
                    __builtin_memcpy(task_buf + 64, root->left->data, 
                                    sizeof(root->left->data));
                }
            }
            
            #pragma omp task
            {
                volatile char task_buf2[128];
                if (root->right) {
                    __builtin_memmove(task_buf2, root->right->data,
                                     sizeof(root->right->data));
                    __builtin_memset(task_buf2 + 32, 0xAA, 64);
                }
            }
        }
        
        #pragma omp taskwait
        
        /* Nested parallel region for additional coverage */
        #pragma omp parallel for
        for (int i = 0; i < 4; i++) {
            volatile char loop_buf[256];
            size_t offset = i * 64;
            
            /* Mix different built-ins in loop */
            switch (i % 3) {
                case 0:
                    __builtin_memset(loop_buf + offset, i, 64);
                    break;
                case 1:
                    if (offset > 0) {
                        __builtin_memmove(loop_buf + offset - 32, 
                                         loop_buf + offset, 32);
                    }
                    break;
                case 2:
                    __builtin_memcpy(loop_buf + offset, root->data, 
                                    sizeof(root->data));
                    break;
            }
        }
    }
    
    /* Recursive processing */
    process_ast_parallel(root->left);
    process_ast_parallel(root->right);
}

/* Calculate hash from AST */
static unsigned long calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    volatile char* data = node->data;
    
    /* Process data with memory operations */
    for (size_t i = 0; i < sizeof(node->data); i++) {
        hash = ((hash << 5) + hash) + data[i];
        
        /* Occasional memcpy within hash calculation */
        if (i % 16 == 0 && node->left) {
            volatile char temp[16];
            __builtin_memcpy(temp, node->left->data + i, 16);
            for (int j = 0; j < 16; j++) {
                hash ^= temp[j];
            }
        }
    }
    
    unsigned long left_hash = calculate_ast_hash(node->left);
    unsigned long right_hash = calculate_ast_hash(node->right);
    
    /* Use memmove for hash combination */
    volatile unsigned long hashes[3] = {hash, left_hash, right_hash};
    __builtin_memmove(hashes, hashes + 1, 2 * sizeof(unsigned long));
    
    return hashes[0] ^ hashes[1] ^ node->id;
}

/* Free AST with memory sanitization */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Sanitize memory before free */
    volatile char* data = node->data;
    __builtin_memset(data, 0xDE, sizeof(node->data));
    
    /* Use memcpy to preserve ID for debugging */
    volatile int saved_id = node->id;
    __builtin_memcpy(&saved_id, &node->id, sizeof(int));
    
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    int node_id = 1;
    ASTNode* root = create_ast(4, &node_id);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    printf("Created AST with %d nodes\n", node_id - 1);
    
    /* Process AST with OpenMP parallelism */
    process_ast_parallel(root);
    
    /* Calculate verification hash */
    unsigned long final_hash = calculate_ast_hash(root);
    printf("AST hash: 0x%08lx\n", final_hash);
    
    /* Additional memory operation stress test */
    volatile char stress_buf[1024];
    volatile char src_buf[1024];
    
    /* Initialize source buffer */
    for (size_t i = 0; i < sizeof(src_buf); i++) {
        src_buf[i] = (char)(i % 256);
    }
    
    /* Chain memory operations with goto */
    size_t offset = 0;
    
copy_chain:
    __builtin_memcpy(stress_buf + offset, src_buf + offset, 128);
    offset += 128;
    
    if (offset < 512) {
        goto copy_chain;
    }
    
move_chain:
    __builtin_memmove(stress_buf + 256, stress_buf, 256);
    
    if (g_use_hwasan) {
        __builtin_memset(stress_buf + 512, 0xFF, 512);
    }
    
    /* Verify operations */
    volatile int sum = 0;
    for (size_t i = 0; i < sizeof(stress_buf); i++) {
        sum += stress_buf[i];
    }
    
    printf("Stress test sum: %d\n", sum);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
