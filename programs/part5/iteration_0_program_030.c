/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    size_t hash;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force early initialization of ASAN runtime */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    volatile char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_tree(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Create pattern in data using memcpy */
    char pattern[64];
    for (int i = 0; i < 64; i++) {
        pattern[i] = (char)(depth + i);
    }
    __builtin_memcpy(node->data, pattern, 64);
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int use_goto = (depth % 2 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_tree(depth - 1);
        node->right = create_tree(depth - 1);
        
        if (use_goto) {
            skip_children:
            /* Copy data between nodes using memmove */
            if (node->left && node->right) {
                __builtin_memmove(node->left->data + 32, 
                                 node->right->data, 32);
            }
        }
        return node;
        
        create_children:
        node->left = create_tree(depth - 2);
        node->right = create_tree(depth - 2);
        goto skip_children;
    }
    
    return node;
}

/* Calculate hash using memory operations */
static size_t compute_tree_hash(ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 0;
    char temp_buf[128];
    
    /* Use volatile to prevent folding */
    volatile size_t* hash_ptr = &hash;
    
    /* Copy node data with builtin memcpy */
    __builtin_memcpy(temp_buf, node->data, 64);
    
    /* Process data with memmove for overlap testing */
    __builtin_memmove(temp_buf + 32, temp_buf, 32);
    
    /* XOR all bytes */
    for (int i = 0; i < 64; i++) {
        *hash_ptr ^= (size_t)temp_buf[i] << ((i % 8) * 8);
    }
    
    /* Recursive hash computation */
    *hash_ptr ^= compute_tree_hash(node->left);
    *hash_ptr ^= compute_tree_hash(node->right);
    
    return hash;
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_ops(void) {
    const int num_blocks = 8;
    char* blocks[num_blocks];
    size_t block_size = g_mem_size;
    
    /* Allocate memory blocks */
    for (int i = 0; i < num_blocks; i++) {
        blocks[i] = (char*)malloc(block_size);
        if (!blocks[i]) return;
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0: {
                /* memset pattern */
                char fill = (char)(thread_id + 65);
                __builtin_memset(blocks[thread_id], fill, block_size);
                break;
            }
            case 1: {
                /* memcpy between blocks */
                int src_idx = (thread_id + 1) % num_blocks;
                int dst_idx = thread_id;
                __builtin_memcpy(blocks[dst_idx], blocks[src_idx], 
                                block_size / 2);
                break;
            }
            case 2: {
                /* memmove with overlap */
                char* mid = blocks[thread_id] + block_size / 2;
                __builtin_memmove(mid, blocks[thread_id], block_size / 4);
                break;
            }
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* Verify operations */
        #pragma omp for
        for (int i = 0; i < num_blocks; i++) {
            volatile char check = blocks[i][0];
            (void)check; /* Use volatile to prevent removal */
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_blocks; i++) {
        free(blocks[i]);
    }
}

/* Complex token processing with goto jumps */
static size_t process_tokens(const char** tokens, int count) {
    size_t result = 0;
    char buffer[256];
    int i = 0;
    
    /* Start with goto to test control flow */
    goto start_processing;
    
    loop_body:
    {
        /* Copy token with builtin */
        size_t len = __builtin_strlen(tokens[i]);
        if (len > 255) len = 255;
        
        __builtin_memcpy(buffer, tokens[i], len);
        buffer[len] = '\0';
        
        /* Move data around */
        __builtin_memmove(buffer + 128, buffer, len);
        
        /* XOR into result */
        for (int j = 0; j < len; j++) {
            result ^= (size_t)buffer[j] << ((j % 8) * 8);
        }
        
        i++;
    }
    
    start_processing:
    while (i < count) {
        /* Jump into different processing modes */
        if (i % 3 == 0) {
            goto loop_body;
        } else if (i % 3 == 1) {
            /* Alternative path with memset */
            __builtin_memset(buffer, tokens[i][0], 128);
            goto loop_body;
        } else {
            /* Direct path */
            goto loop_body;
        }
    }
    
    return result;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Tree operations */
    ASTNode* root = create_tree(4);
    if (!root) {
        fprintf(stderr, "Failed to create tree\n");
        return 1;
    }
    
    size_t tree_hash = compute_tree_hash(root);
    printf("Tree hash: 0x%016zx\n", tree_hash);
    
    /* Phase 2: Token processing */
    const char* tokens[] = {
        "ASAN_TEST_STRING_1",
        "BUILTIN_MEMCPY_REDIRECT",
        "HWASAN_KERNEL_PATH",
        "GOTO_CONTROL_FLOW",
        "VOLATILE_OPTIMIZATION_BARRIER",
        "OPENMP_PARALLEL_REGION"
    };
    
    size_t token_hash = process_tokens(tokens, 
                                      sizeof(tokens)/sizeof(tokens[0]));
    printf("Token hash: 0x%016zx\n", token_hash);
    
    /* Phase 3: Parallel memory operations */
    printf("Running parallel memory operations...\n");
    parallel_memory_ops();
    
    /* Phase 4: Mixed operations */
    char final_buffer[512];
    volatile size_t final_size = 256;
    
    /* Chain of memory operations */
    __builtin_memset(final_buffer, 0xAA, final_size);
    __builtin_memcpy(final_buffer + 128, final_buffer, 128);
    __builtin_memmove(final_buffer + 64, final_buffer + 192, 64);
    
    /* Final hash calculation */
    size_t final_hash = 0;
    for (int i = 0; i < 256; i++) {
        final_hash ^= (size_t)final_buffer[i] << ((i % 8) * 8);
    }
    printf("Final hash: 0x%016zx\n", final_hash);
    
    /* Cleanup */
    /* Note: In real code, would need proper tree freeing */
    
    printf("Test completed successfully.\n");
    return 0;
}
