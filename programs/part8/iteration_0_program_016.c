/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
    unsigned char padding[16]; /* Ensure redzone creation */
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force early initialization of ASAN runtime */
    volatile char buffer[64];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    /* Final memory operation to ensure coverage */
    volatile char final_buf[32];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Non-foldable size using volatile */
    size_t data_len = g_mem_size / (depth * 4);
    node->len = data_len;
    node->data = malloc(data_len + 1);
    
    if (node->data) {
        /* Use all three builtins in one function */
        __builtin_memset(node->data, 0, data_len);
        __builtin_memcpy(node->data, base_data, 
                        data_len < strlen(base_data) ? data_len : strlen(base_data));
        
        /* Conditional memmove with goto */
        if (g_use_memmove && depth > 2) {
            char *temp = malloc(data_len);
            if (temp) {
                __builtin_memcpy(temp, node->data, data_len);
                goto do_memmove;
do_memmove:
                __builtin_memmove(node->data + 8, temp, data_len - 8);
                free(temp);
            }
        }
    }
    
    node->left = create_ast(depth - 1, base_data);
    node->right = create_ast(depth - 2, base_data);
    
    /* Copy between sibling nodes */
    if (node->left && node->right && node->left->data && node->right->data) {
        size_t copy_len = node->left->len < node->right->len ? 
                         node->left->len : node->right->len;
        __builtin_memcpy(node->right->data, node->left->data, copy_len);
    }
    
    return node;
}

static void free_ast(ASTNode *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node->data);
    free(node);
}

/* Function with OpenMP parallel section */
static unsigned long process_ast_parallel(ASTNode *root) {
    unsigned long total_hash = 0;
    
    #pragma omp parallel reduction(+:total_hash)
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < 100; ++i) {
            /* Each thread creates its own buffer */
            char local_buf[128];
            volatile size_t local_size = 64 + (i % 64);
            
            /* Use all three builtins in parallel region */
            __builtin_memset(local_buf, thread_id, local_size);
            
            /* Create pattern with memcpy */
            char pattern[32];
            __builtin_memset(pattern, i, sizeof(pattern));
            __builtin_memcpy(local_buf + 16, pattern, 
                           sizeof(pattern) < local_size - 16 ? sizeof(pattern) : local_size - 16);
            
            /* Conditional memmove with goto */
            if (i % 3 == 0) {
                goto parallel_memmove;
parallel_memmove:
                __builtin_memmove(local_buf + 8, local_buf + 24, 16);
            }
            
            /* Compute simple hash */
            for (size_t j = 0; j < local_size; ++j) {
                total_hash += (unsigned long)local_buf[j] * (j + 1);
            }
        }
    }
    
    return total_hash;
}

/* Multi-stage initialization */
static void stage1_init(void) {
    volatile char init_buf[256];
    __builtin_memset(init_buf, 0xCC, sizeof(init_buf));
    
    /* Chain of memory operations */
    volatile char chain_buf[128];
    __builtin_memcpy(chain_buf, init_buf + 64, 64);
    __builtin_memmove(chain_buf + 32, chain_buf, 32);
}

static void stage2_process(void) {
    /* Different memory operation pattern */
    volatile int numbers[100];
    for (int i = 0; i < 100; ++i) {
        numbers[i] = i * i;
    }
    
    volatile int copy[100];
    __builtin_memcpy(copy, numbers, sizeof(numbers));
    
    /* Partial overlap memmove */
    __builtin_memmove((void*)(numbers + 10), numbers, 90 * sizeof(int));
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Stage 1: Early initialization */
    stage1_init();
    
    /* Stage 2: Create complex AST */
    const char *base_data = "TEST_DATA_FOR_AST_NODES_1234567890";
    ASTNode *root = create_ast(5, base_data);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Stage 3: Parallel processing */
    unsigned long hash = process_ast_parallel(root);
    printf("Parallel hash result: %lu\n", hash);
    
    /* Stage 4: Additional processing */
    stage2_process();
    
    /* Final verification */
    volatile char verify_buf[512];
    __builtin_memset(verify_buf, 0x55, sizeof(verify_buf));
    
    /* Use all three builtins in main */
    volatile char final_copy[512];
    __builtin_memcpy(final_copy, verify_buf, sizeof(verify_buf));
    __builtin_memmove(final_copy + 128, final_copy, 256);
    __builtin_memset(final_copy + 384, 0xAA, 128);
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
