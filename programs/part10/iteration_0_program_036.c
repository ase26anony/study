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
    size_t size;
} ASTNode;

/* Global token array */
static char g_tokens[8][32] = {
    "memcpy_test_token_1",
    "memset_test_token_2",
    "memmove_test_token_3",
    "asan_coverage_4",
    "hwasan_branch_5",
    "builtin_redirect_6",
    "flow_sensitive_7",
    "omp_parallel_8"
};

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    /* Force initialization of memory functions */
    char buf[32];
    __builtin_memset(buf, 0, sizeof(buf));
    __builtin_memcpy(buf, "constructor_init", 16);
    
    /* Set volatile flag based on environment */
    const char* env = getenv("USE_HWASAN");
    if (env && atoi(env) > 0) {
        g_use_hwasan = 1;
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operations */
    char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* src, size_t depth) {
    if (depth > 3) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for node initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    __builtin_memcpy(node->data, src, strlen(src) + 1);
    node->size = strlen(src) + 1;
    
    /* Recursive creation with goto for flow control */
    if (depth < 3) {
        int use_left = (depth % 2 == 0);
        
        if (use_left) {
            goto create_left;
        } else {
            goto create_right;
        }
        
    create_left:
        node->left = create_ast_node(g_tokens[depth * 2], depth + 1);
        goto skip_right;
        
    create_right:
        node->right = create_ast_node(g_tokens[depth * 2 + 1], depth + 1);
        
    skip_right:
        /* memmove between nodes if both exist */
        if (node->left && node->right) {
            char temp[64];
            __builtin_memcpy(temp, node->left->data, node->left->size);
            __builtin_memmove(node->left->data, node->right->data, 
                            node->right->size < 64 ? node->right->size : 64);
            __builtin_memmove(node->right->data, temp, 
                            node->left->size < 64 ? node->left->size : 64);
        }
    }
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_operations(void) {
    size_t local_size = g_mem_size;
    char* buffers[4];
    
    /* Allocate buffers */
    for (int i = 0; i < 4; i++) {
        buffers[i] = malloc(local_size);
        if (!buffers[i]) return;
        __builtin_memset(buffers[i], i, local_size);
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0: /* memcpy operations */
                __builtin_memcpy(buffers[thread_id % 4], 
                               g_tokens[thread_id % 8],
                               strlen(g_tokens[thread_id % 8]) + 1);
                break;
                
            case 1: /* memset operations */
                __builtin_memset(buffers[thread_id % 4], 
                               thread_id + 'A',
                               local_size / 2);
                break;
                
            case 2: /* memmove operations with overlap */
                if (thread_id < 3) {
                    size_t move_size = local_size / 4;
                    __builtin_memmove(buffers[thread_id] + move_size,
                                    buffers[thread_id],
                                    move_size * 3);
                }
                break;
        }
        
        /* Barrier to ensure all operations complete */
        #pragma omp barrier
        
        /* Verify operations with another memory function */
        if (thread_id == 0) {
            char verify_buf[128];
            __builtin_memset(verify_buf, 0, sizeof(verify_buf));
            for (int i = 0; i < 4; i++) {
                __builtin_memcpy(verify_buf + i * 16, 
                               buffers[i], 
                               16);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(buffers[i]);
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast_node("root_node", 0);
    
    /* Phase 2: Parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 3: Direct built-in calls with volatile control */
    volatile char* dynamic_buf = malloc(g_mem_size);
    if (dynamic_buf) {
        /* Force all three built-ins in sequence */
        __builtin_memset(dynamic_buf, 0x42, g_mem_size);
        
        char pattern[32];
        __builtin_memset(pattern, 0xAA, sizeof(pattern));
        __builtin_memcpy(dynamic_buf + 64, pattern, sizeof(pattern));
        
        /* Overlapping memmove */
        __builtin_memmove(dynamic_buf + 32, dynamic_buf + 16, 48);
        
        /* Compute verification hash */
        unsigned long hash = 0;
        for (size_t i = 0; i < g_mem_size && i < 128; i++) {
            hash = hash * 31 + dynamic_buf[i];
        }
        printf("Memory operation hash: %lu\n", hash);
        
        free(dynamic_buf);
    }
    
    /* Phase 4: Edge case with goto jumping into memory block */
    {
        char edge_buf[3][64];
        int use_memmove = 1;
        
        __builtin_memset(edge_buf[0], 'X', 64);
        __builtin_memset(edge_buf[1], 'Y', 64);
        
        if (use_memmove) {
            goto perform_memmove;
        }
        
        __builtin_memcpy(edge_buf[2], edge_buf[0], 64);
        goto skip_memmove;
        
    perform_memmove:
        /* This tests flow sensitivity */
        __builtin_memmove(edge_buf[2], edge_buf[1], 64);
        
    skip_memmove:
        /* Verify the result */
        printf("Edge case buffer[2][0] = %c\n", edge_buf[2][0]);
    }
    
    /* Cleanup AST */
    /* Note: In a full implementation, you'd want to free the AST recursively */
    
    printf("ASAN test completed successfully\n");
    return 0;
}
