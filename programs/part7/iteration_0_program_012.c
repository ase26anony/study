/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    char data[256];
    struct ast_node* left;
    struct ast_node* right;
    size_t size;
} ast_node_t;

/* Global token array */
static char g_tokens[][32] = {
    "TOKEN_A", "TOKEN_B", "TOKEN_C", "TOKEN_D",
    "TOKEN_E", "TOKEN_F", "TOKEN_G", "TOKEN_H"
};

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    volatile char buffer[128];
    
    /* Force __builtin_memset initialization */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    
    /* Initialize global memory size */
    g_mem_size = 128;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[64];
    
    /* Final memory operation in destructor */
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive parser with memory operations */
static ast_node_t* create_ast_node(const char* data, size_t depth) {
    if (depth == 0) return NULL;
    
    ast_node_t* node = malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for node initialization */
    __builtin_memcpy(node->data, data, strlen(data) + 1);
    node->size = strlen(data) + 1;
    
    /* Recursive creation with goto for control flow */
    int create_left = 1;
    
    if (depth > 2) {
        goto skip_left;
    }
    
    create_left = 0;
    
skip_left:
    if (create_left) {
        char left_data[256];
        __builtin_snprintf(left_data, sizeof(left_data), "%s_L%d", data, (int)depth);
        node->left = create_ast_node(left_data, depth - 1);
    } else {
        node->left = NULL;
    }
    
    /* Jump back into block with memmove */
    if (depth % 2 == 0) {
        goto memmove_block;
    }
    
    char right_data[256];
    __builtin_snprintf(right_data, sizeof(right_data), "%s_R%d", data, (int)depth);
    node->right = create_ast_node(right_data, depth - 1);
    
    return node;
    
memmove_block:
    /* This block tests goto into memmove operations */
    volatile char temp_buf[128];
    __builtin_memset(temp_buf, 0xCC, sizeof(temp_buf));
    
    /* Use __builtin_memmove with overlapping regions */
    __builtin_memmove(temp_buf + 32, temp_buf, 64);
    
    char right_data2[256];
    __builtin_snprintf(right_data2, sizeof(right_data2), "%s_R%d_MOVED", data, (int)depth);
    node->right = create_ast_node(right_data2, depth - 1);
    
    return node;
}

/* Parallel memory dispatch logic */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        volatile char thread_buf[256];
        volatile size_t local_size = g_mem_size;
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                /* Test __builtin_memcpy */
                __builtin_memcpy((void*)thread_buf, 
                               g_tokens[thread_id % 8],
                               local_size < 32 ? local_size : 32);
                break;
                
            case 1:
                /* Test __builtin_memset */
                __builtin_memset((void*)thread_buf, 
                               thread_id + 0x30,
                               local_size < 128 ? local_size : 128);
                break;
                
            case 2:
                /* Test __builtin_memmove with overlap */
                __builtin_memset((void*)thread_buf, 0xAA, sizeof(thread_buf));
                __builtin_memmove((void*)(thread_buf + 64),
                                thread_buf,
                                128);
                break;
        }
        
        /* Barrier to ensure all memory ops complete */
        #pragma omp barrier
        
        /* Additional memcpy after barrier */
        if (thread_id == 0) {
            volatile char master_buf[512];
            __builtin_memcpy((void*)master_buf,
                           thread_buf,
                           sizeof(thread_buf));
        }
    }
}

/* Complex memory operation with goto edge cases */
static size_t complex_memory_pattern(char* buffer, size_t size) {
    volatile char local_buf[1024];
    size_t hash = 0;
    int i = 0;
    
    /* Initialize with memset */
    __builtin_memset(local_buf, 0, sizeof(local_buf));
    
    /* Jump table simulation with goto */
    if (size < 256) {
        goto small_buffer;
    }
    
    /* Large buffer path */
    __builtin_memcpy(local_buf, buffer, size > 1024 ? 1024 : size);
    
    /* Overlapping memmove */
    __builtin_memmove(local_buf + 512, local_buf, 256);
    goto compute_hash;
    
small_buffer:
    /* Small buffer path with different pattern */
    __builtin_memset(local_buf, 0x55, 128);
    __builtin_memcpy(local_buf + 64, buffer, size);
    
    /* Another goto into memmove */
    if (size > 64) {
        goto overlapping_move;
    }
    
    goto compute_hash;
    
overlapping_move:
    __builtin_memmove(local_buf + 32, local_buf, 96);
    
compute_hash:
    /* Compute simple hash from buffer */
    for (i = 0; i < (int)(size < 1024 ? size : 1024); i++) {
        hash = (hash * 31) + local_buf[i];
    }
    
    return hash;
}

/* Main execution flow */
int main(void) {
    size_t final_hash = 0;
    ast_node_t* ast_root = NULL;
    volatile char main_buffer[2048];
    
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Initialize main buffer with pattern */
    for (size_t i = 0; i < sizeof(main_buffer); i++) {
        main_buffer[i] = (char)(i % 256);
    }
    
    /* Create recursive AST structure */
    ast_root = create_ast_node("ROOT", 4);
    
    if (ast_root) {
        /* Copy between AST nodes */
        if (ast_root->left && ast_root->right) {
            __builtin_memcpy(ast_root->right->data,
                           ast_root->left->data,
                           ast_root->left->size);
        }
        
        /* Complex memory pattern on AST data */
        final_hash ^= complex_memory_pattern(ast_root->data, ast_root->size);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Final memory operations in main */
    volatile char final_buf[1024];
    
    /* Test all three builtins in sequence */
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf + 256, main_buffer, 512);
    __builtin_memmove(final_buf, final_buf + 128, 384);
    
    /* Compute final verification hash */
    for (int i = 0; i < 1024; i++) {
        final_hash = (final_hash * 31) + final_buf[i];
    }
    
    printf("Final hash: 0x%08zx\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real code, you'd need to free the AST recursively */
    
    return 0;
}
