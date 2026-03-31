/* ISO C99-compliant test program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t hash;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force early initialization of ASAN runtime */
    volatile char buffer[32];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_late(void) {
    /* Final memory operation to ensure coverage */
    volatile char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy with volatile size */
    size_t copy_len = g_mem_size % 64;
    if (copy_len > 63) copy_len = 63;
    
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Create children with goto-based control flow */
    node->left = NULL;
    node->right = NULL;
    
    if (depth > 1) {
        /* Jump label for goto */
        create_left:
        node->left = create_ast(depth - 1, "LEFT");
        
        /* Conditional goto to test flow sensitivity */
        if (g_use_memmove) {
            goto use_memmove_block;
        }
        
        create_right:
        node->right = create_ast(depth - 1, "RIGHT");
    }
    
    /* Compute hash using memory operations */
    node->hash = 0;
    for (size_t i = 0; i < sizeof(node->data); i++) {
        node->hash = (node->hash * 31) + node->data[i];
    }
    
    return node;
    
    /* Target label for goto into memmove block */
    use_memmove_block: {
        volatile char temp[64];
        
        /* __builtin_memmove with overlapping regions */
        __builtin_memcpy(temp, node->data, sizeof(node->data));
        __builtin_memmove(node->data + 10, node->data, 32);
        __builtin_memmove(node->data, temp, sizeof(node->data));
        
        goto create_right;
    }
}

/* Function with OpenMP parallel section */
static uint32_t process_ast_parallel(ASTNode* root) {
    uint32_t total_hash = 0;
    
    #pragma omp parallel reduction(+:total_hash)
    {
        /* Each thread performs memory operations */
        volatile char thread_buf[128];
        size_t local_size = g_mem_size % 128;
        
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            /* Mix of memory built-ins */
            __builtin_memset(thread_buf, i, local_size);
            
            if (i % 2 == 0) {
                __builtin_memcpy(thread_buf + 64, thread_buf, 32);
            } else {
                __builtin_memmove(thread_buf + 32, thread_buf, 48);
            }
            
            /* Simulate work */
            uint32_t local_hash = 0;
            for (size_t j = 0; j < local_size; j++) {
                local_hash ^= (thread_buf[j] << (j % 32));
            }
            total_hash += local_hash;
        }
    }
    
    return total_hash;
}

/* Multi-stage initialization function */
static void complex_initialization(void) {
    /* Stage 1: Direct built-in calls */
    volatile char stage1[256];
    __builtin_memset(stage1, 0xCC, sizeof(stage1));
    __builtin_memcpy(stage1 + 128, stage1, 64);
    
    /* Stage 2: Nested memory operations */
    volatile char stage2[512];
    for (int i = 0; i < 8; i++) {
        size_t offset = (i * 64) % 448;
        __builtin_memcpy(stage2 + offset, stage1 + (i * 32), 32);
    }
    
    /* Stage 3: Overlapping memmove */
    __builtin_memmove(stage2 + 256, stage2, 256);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Force initialization of asan_memfn_rtls cache */
    complex_initialization();
    
    /* Create recursive structure */
    ASTNode* root = create_ast(3, "ROOT_NODE");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with OpenMP */
    uint32_t parallel_hash = process_ast_parallel(root);
    
    /* Additional memory operations in main */
    volatile char main_buf[1024];
    size_t dynamic_size = g_mem_size % 1024;
    
    __builtin_memset(main_buf, 0x55, dynamic_size);
    __builtin_memcpy(main_buf + 512, main_buf, 256);
    
    /* Conditional memmove with goto */
    if (g_use_memmove) {
        goto do_memmove;
    }
    
    after_memmove:
    __builtin_memcpy(main_buf + 768, main_buf + 256, 128);
    
    /* Compute final result */
    uint32_t final_hash = root->hash ^ parallel_hash;
    for (size_t i = 0; i < dynamic_size; i++) {
        final_hash = (final_hash * 17) + main_buf[i];
    }
    
    printf("Result hash: 0x%08X\n", final_hash);
    
    /* Cleanup */
    free(root);
    
    return 0;
    
    /* Label for goto-based memmove */
    do_memmove:
    __builtin_memmove(main_buf + 256, main_buf, 384);
    goto after_memmove;
}
