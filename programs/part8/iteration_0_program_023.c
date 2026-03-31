/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_tokens[][32] = {
    "memcpy_test", "memset_test", "memmove_test",
    "recursive", "parallel", "asan"
};

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    volatile char buf[16];
    /* Force early builtin usage in constructor */
    __builtin_memset(buf, 0xAA, sizeof(buf));
    printf("Constructor: Initialized ASAN environment\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    volatile char buf[16];
    __builtin_memset(buf, 0xFF, sizeof(buf));
    printf("Destructor: Cleaning up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to control sizes */
    volatile size_t copy_size = g_mem_size / (depth + 1);
    if (copy_size > sizeof(node->data)) copy_size = sizeof(node->data);
    
    /* Initialize with builtin memset */
    __builtin_memset(node->data, depth, copy_size);
    
    /* Copy token based on depth */
    int token_idx = depth % (sizeof(g_tokens)/sizeof(g_tokens[0]));
    __builtin_memcpy(node->data + 10, g_tokens[token_idx], 
                     strlen(g_tokens[token_idx]));
    
    node->id = (*counter)++;
    node->left = create_ast(depth - 1, counter);
    node->right = create_ast(depth - 2, counter);
    
    return node;
}

/* Function with goto jumps around memmove */
static void test_goto_memmove(char* dest, char* src, size_t size) {
    int use_memmove = 1;
    
    if (size < 8) goto skip_memmove;
    
    /* Jump into block with memmove */
    goto do_memmove;
    
memmove_block:
    /* This label is jumped into */
    __builtin_memmove(dest + 8, src + 8, size - 8);
    goto after_memmove;
    
do_memmove:
    if (use_memmove) {
        /* Jump to the middle of the block */
        goto memmove_block;
    }
    
skip_memmove:
    __builtin_memcpy(dest, src, size < 8 ? size : 8);
    
after_memmove:
    /* Jump out of scope */
    if (size > 16) goto finalize;
    
    __builtin_memset(dest + size/2, 0xCC, size/4);
    goto end;
    
finalize:
    __builtin_memset(dest, 0xDD, 4);
    
end:
    return;
}

/* Parallel memory operations */
static unsigned long parallel_memory_ops(void) {
    unsigned long hash = 0;
    const int num_buffers = 8;
    char* buffers[num_buffers];
    volatile size_t sizes[num_buffers];
    
    /* Initialize with varying sizes */
    for (int i = 0; i < num_buffers; i++) {
        sizes[i] = g_mem_size * (i + 1);
        buffers[i] = (char*)malloc(sizes[i]);
        if (buffers[i]) {
            __builtin_memset(buffers[i], i, sizes[i]);
        }
    }
    
    #pragma omp parallel reduction(+:hash)
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        for (int i = thread_id; i < num_buffers; i += 4) {
            if (buffers[i]) {
                volatile size_t op_size = sizes[i];
                
                /* Mix of memory operations */
                if (i % 3 == 0) {
                    __builtin_memcpy(buffers[i] + 10, 
                                    g_tokens[i % 6], 16);
                } else if (i % 3 == 1) {
                    __builtin_memset(buffers[i] + 20, 
                                    thread_id, op_size / 4);
                } else {
                    /* Create overlapping regions for memmove */
                    size_t move_size = op_size / 2;
                    if (move_size > 32) move_size = 32;
                    __builtin_memmove(buffers[i] + 16, 
                                     buffers[i] + 8, move_size);
                }
                
                /* Compute simple hash */
                for (size_t j = 0; j < (op_size < 64 ? op_size : 64); j++) {
                    hash += (unsigned long)buffers[i][j];
                }
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_buffers; i++) {
        if (buffers[i]) {
            free(buffers[i]);
        }
    }
    
    return hash;
}

/* Recursive tree traversal with memory operations */
static void process_ast(ASTNode* node, char* output, size_t* offset) {
    if (!node) return;
    
    volatile size_t copy_len = g_mem_size / (node->id + 2);
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    
    /* Copy node data with builtin */
    __builtin_memcpy(output + *offset, node->data, copy_len);
    *offset += copy_len;
    
    /* Process children recursively */
    process_ast(node->left, output, offset);
    process_ast(node->right, output, offset);
    
    /* Move data around within node */
    if (node->left && node->right) {
        size_t move_size = copy_len / 2;
        if (move_size > 0) {
            __builtin_memmove(node->data + move_size, 
                             node->data, move_size);
        }
    }
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    volatile size_t clear_size = sizeof(node->data);
    __builtin_memset(node->data, 0, clear_size);
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    int counter = 1;
    ASTNode* root = create_ast(5, &counter);
    
    char output_buffer[4096];
    size_t output_offset = 0;
    __builtin_memset(output_buffer, 0, sizeof(output_buffer));
    
    process_ast(root, output_buffer, &output_offset);
    
    /* Phase 2: Goto-based memmove test */
    char src_data[128];
    char dest_data[128];
    
    for (int i = 0; i < (int)sizeof(src_data); i++) {
        src_data[i] = (char)(i % 256);
    }
    
    test_goto_memmove(dest_data, src_data, sizeof(dest_data));
    
    /* Phase 3: Parallel memory operations */
    unsigned long parallel_hash = parallel_memory_ops();
    
    /* Phase 4: Direct builtin calls with volatile control */
    volatile char final_buffer[256];
    volatile size_t final_size = g_mem_size * 2;
    if (final_size > sizeof(final_buffer)) {
        final_size = sizeof(final_buffer);
    }
    
    /* Exercise all three builtins in sequence */
    __builtin_memset(final_buffer, 0x55, final_size);
    __builtin_memcpy((char*)final_buffer + 32, src_data, 64);
    __builtin_memmove((char*)final_buffer + 16, 
                     (char*)final_buffer + 32, 48);
    
    /* Compute final verification hash */
    unsigned long final_hash = parallel_hash;
    for (size_t i = 0; i < output_offset && i < sizeof(output_buffer); i++) {
        final_hash += (unsigned long)output_buffer[i];
    }
    for (size_t i = 0; i < sizeof(dest_data); i += 4) {
        final_hash += (unsigned long)dest_data[i];
    }
    for (size_t i = 0; i < final_size && i < sizeof(final_buffer); i += 8) {
        final_hash += (unsigned long)final_buffer[i];
    }
    
    printf("Verification hash: %lu\n", final_hash);
    printf("AST nodes created: %d\n", counter - 1);
    printf("Output bytes processed: %zu\n", output_offset);
    
    /* Cleanup */
    free_ast(root);
    
    return (final_hash != 0) ? 0 : 1;
}
