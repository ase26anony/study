/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[128];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_tokens[][32] = {
    "memcpy_test", "memset_test", "memmove_test",
    "asan_check", "hwasan_check", "parallel_exec"
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char buffer[256];
    /* Force __builtin_memset in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[128];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* src, size_t depth) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for initialization */
    __builtin_memcpy(node->data, src, strlen(src) + 1);
    node->size = strlen(src) + 1;
    node->left = node->right = NULL;
    
    if (depth > 0) {
        char left_data[64], right_data[64];
        volatile size_t copy_len = g_mem_size % 32 + 16;
        
        /* Complex memcpy pattern */
        __builtin_memcpy(left_data, "left_branch_", 12);
        __builtin_memcpy(left_data + 12, src, copy_len);
        
        __builtin_memcpy(right_data, "right_branch_", 13);
        __builtin_memcpy(right_data + 13, src + (depth % 3), copy_len);
        
        node->left = create_ast_node(left_data, depth - 1);
        node->right = create_ast_node(right_data, depth - 1);
        
        /* Memmove between nodes if both exist */
        if (node->left && node->right) {
            volatile char temp[128];
            __builtin_memcpy(temp, node->left->data, sizeof(temp));
            __builtin_memmove(node->left->data, node->right->data, 
                            node->right->size);
            __builtin_memmove(node->right->data, temp, node->left->size);
        }
    }
    
    return node;
}

/* Function with goto jumps around memmove */
static void test_goto_memmove(void) {
    volatile char buffer1[256], buffer2[256];
    volatile int use_buffer1 = 1;
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 0x11, sizeof(buffer1));
    __builtin_memset(buffer2, 0x22, sizeof(buffer2));
    
    goto jump_point;
    
memmove_block:
    /* This block contains __builtin_memmove */
    if (use_buffer1) {
        __builtin_memmove(buffer1 + 64, buffer1, 128);
    } else {
        __builtin_memmove(buffer2 + 32, buffer2, 96);
    }
    goto after_memmove;
    
jump_point:
    /* Jump into memmove block */
    use_buffer1 = 0;
    goto memmove_block;
    
after_memmove:
    /* Verify the move worked */
    __builtin_memcpy(buffer1, buffer2, 64);
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    volatile char parallel_buf[1024];
    volatile size_t chunk_size = g_mem_size;
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        size_t offset = tid * 128;
        
        /* Each thread uses different builtins */
        if (tid % 3 == 0) {
            __builtin_memset(parallel_buf + offset, tid, chunk_size);
        } else if (tid % 3 == 1) {
            char pattern[64];
            __builtin_memset(pattern, 0xCC, sizeof(pattern));
            __builtin_memcpy(parallel_buf + offset, pattern, 
                           chunk_size < 64 ? chunk_size : 64);
        } else {
            /* Memmove within thread's region */
            __builtin_memmove(parallel_buf + offset + 32, 
                            parallel_buf + offset, 64);
        }
    }
    
    /* Final consolidation memcpy */
    __builtin_memcpy(parallel_buf + 512, parallel_buf, 512);
}

/* Calculate hash from AST */
static size_t compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 5381;
    volatile char* ptr = node->data;
    
    /* Process data with memcpy to temp buffer */
    char temp[128];
    volatile size_t copy_size = node->size < 128 ? node->size : 128;
    __builtin_memcpy(temp, node->data, copy_size);
    
    for (size_t i = 0; i < copy_size && temp[i]; i++) {
        hash = ((hash << 5) + hash) + temp[i];
    }
    
    /* Recursive hash combination */
    size_t left_hash = compute_ast_hash(node->left);
    size_t right_hash = compute_ast_hash(node->right);
    
    char combine_buf[32];
    __builtin_memset(combine_buf, 0, sizeof(combine_buf));
    __builtin_memcpy(combine_buf, &left_hash, sizeof(left_hash));
    __builtin_memcpy(combine_buf + 16, &right_hash, sizeof(right_hash));
    
    for (size_t i = 0; i < 32; i++) {
        hash ^= (combine_buf[i] << (i % 8));
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize token array with memset */
    for (size_t i = 0; i < sizeof(g_tokens)/sizeof(g_tokens[0]); i++) {
        volatile size_t len = strlen(g_tokens[i]) + 1;
        __builtin_memset(g_tokens[i] + len, 0xFF, 
                        sizeof(g_tokens[0]) - len);
    }
    
    /* Create recursive AST */
    ASTNode* root = create_ast_node("root_node", 3);
    
    /* Test goto with memmove */
    test_goto_memmove();
    
    /* Execute parallel operations */
    parallel_memory_ops();
    
    /* Compute verification hash */
    size_t final_hash = compute_ast_hash(root);
    
    /* Additional built-in calls in main */
    volatile char final_buffer[512];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    
    /* Chain of memory operations */
    for (int i = 0; i < 4; i++) {
        size_t offset = i * 64;
        __builtin_memcpy(final_buffer + offset, 
                        g_tokens[i % 6], 32);
        if (i % 2 == 0) {
            __builtin_memmove(final_buffer + offset + 16,
                            final_buffer + offset, 48);
        }
    }
    
    /* Final verification step */
    char verify_buf[64];
    __builtin_memcpy(verify_buf, final_buffer, 64);
    __builtin_memset(verify_buf + 32, 0xAA, 32);
    
    printf("Test completed. Final hash: %zu\n", final_hash);
    printf("Verification buffer[0]: 0x%02x\n", (unsigned char)verify_buf[0]);
    
    /* Cleanup */
    free(root);
    
    return (final_hash != 0) ? 0 : 1;
}
