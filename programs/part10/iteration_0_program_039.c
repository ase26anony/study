/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    size_t data_len;
} ASTNode;

/* Global token array */
static char g_token_array[1024];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token array with pattern */
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)((i * 31) & 0xFF);
    }
    
    /* Force early initialization of memory builtins */
    char temp[16];
    __builtin_memset(temp, 0xAA, sizeof(temp));
    __builtin_memcpy(temp + 8, temp, 8);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_test(void) {
    /* Final memory operation in destructor */
    volatile char final_check[32];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using builtin memcpy with volatile size */
    size_t copy_len = g_mem_size % 64;
    if (copy_len > 0) {
        __builtin_memcpy(node->data, 
                        &g_token_array[g_token_idx], 
                        copy_len);
        node->data_len = copy_len;
        g_token_idx = (g_token_idx + copy_len) % sizeof(g_token_array);
    }
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_left = (depth % 2 == 0);
        
        if (use_left) {
            /* Normal path */
            node->left = create_ast_node(depth - 1);
            node->right = create_ast_node(depth - 2);
        } else {
            /* Goto path - testing flow sensitivity */
            goto create_right;
        }
        
        return node;
        
    create_right:
        /* Jump target for memmove testing */
        ASTNode* temp_right = create_ast_node(depth - 1);
        
        /* Use builtin memmove with overlapping regions */
        if (temp_right && node) {
            __builtin_memmove(node->data + 16, 
                             node->data, 
                             32);
            __builtin_memmove(temp_right->data, 
                             node->data + 8, 
                             24);
        }
        
        node->right = temp_right;
        node->left = create_ast_node(depth - 2);
    }
    
    return node;
}

/* Calculate hash of AST structure */
static uint64_t hash_ast(const ASTNode* node, int depth) {
    if (!node || depth <= 0) return 0;
    
    uint64_t hash = 5381;
    
    /* Process data with memory operations */
    for (size_t i = 0; i < node->data_len; i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Recursive hashing */
    uint64_t left_hash = hash_ast(node->left, depth - 1);
    uint64_t right_hash = hash_ast(node->right, depth - 1);
    
    /* Combine hashes */
    char combine_buf[16];
    __builtin_memcpy(combine_buf, &left_hash, sizeof(left_hash));
    __builtin_memcpy(combine_buf + 8, &right_hash, sizeof(right_hash));
    
    for (int i = 0; i < 16; i++) {
        hash = ((hash << 5) + hash) + combine_buf[i];
    }
    
    return hash;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear sensitive data before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    node->data_len = 0;
    free(node);
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[128];
        char src_buf[128];
        
        /* Initialize source with pattern */
        for (int i = 0; i < 128; i++) {
            src_buf[i] = (char)((i + thread_id * 17) & 0xFF);
        }
        
        /* Memory operations in parallel region */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        /* Conditional memcpy based on thread ID */
        if (thread_id % 2 == 0) {
            __builtin_memcpy(local_buf + 32, src_buf, 64);
        } else {
            __builtin_memmove(local_buf, src_buf + 16, 48);
        }
        
        /* Overlapping copy within same buffer */
        __builtin_memmove(local_buf + 16, local_buf, 32);
        
        /* Use result to prevent optimization */
        volatile char sum = 0;
        for (int i = 0; i < 128; i++) {
            sum += local_buf[i];
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Create and process AST */
    ASTNode* root = create_ast_node(4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    uint64_t ast_hash = hash_ast(root, 4);
    printf("AST hash: 0x%016llx\n", (unsigned long long)ast_hash);
    
    /* Phase 2: Parallel memory operations */
    printf("Running parallel memory operations...\n");
    parallel_memory_ops();
    
    /* Phase 3: Direct builtin calls with volatile control */
    volatile char* dynamic_buf = (char*)malloc(512);
    if (dynamic_buf) {
        /* Chain of memory operations */
        __builtin_memset(dynamic_buf, 0xCC, 512);
        
        volatile size_t offset = g_mem_size % 256;
        __builtin_memcpy(dynamic_buf + 128, dynamic_buf, offset);
        
        /* Overlapping move */
        __builtin_memmove(dynamic_buf + 64, dynamic_buf + 32, 128);
        
        /* Verify by calculating checksum */
        volatile uint32_t checksum = 0;
        for (int i = 0; i < 512; i++) {
            checksum += dynamic_buf[i];
        }
        printf("Dynamic buffer checksum: %u\n", checksum);
        
        /* Clear and free */
        __builtin_memset(dynamic_buf, 0, 512);
        free((void*)dynamic_buf);
    }
    
    /* Phase 4: Array operations with gotos */
    char array_a[256];
    char array_b[256];
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        array_a[i] = (char)i;
        array_b[i] = (char)(255 - i);
    }
    
    /* Complex flow with goto around memmove */
    int use_special_path = (ast_hash & 1);
    
    if (use_special_path) {
        goto special_copy;
    }
    
    /* Normal copy path */
    __builtin_memcpy(array_a, array_b, 128);
    goto after_copy;
    
special_copy:
    /* Special path with overlapping move */
    __builtin_memmove(array_a + 64, array_a, 192);
    __builtin_memcpy(array_b, array_a + 32, 96);
    
after_copy:
    /* Final verification sum */
    volatile int final_sum = 0;
    for (int i = 0; i < 256; i++) {
        final_sum += array_a[i] + array_b[i];
    }
    printf("Final array sum: %d\n", final_sum);
    
    /* Cleanup */
    free_ast(root);
    
    printf("ASAN test completed successfully\n");
    return 0;
}
