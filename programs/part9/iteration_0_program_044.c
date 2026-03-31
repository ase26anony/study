/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    struct ast_node* left;
    struct ast_node* right;
    char data[32];
    uint32_t hash;
} ast_node_t;

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force early initialization of ASAN runtime */
    char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_destructor(void) {
    volatile char dummy[8];
    __builtin_memset(dummy, 0xFF, sizeof(dummy));
}

/* Recursive function with memory operations */
static ast_node_t* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    
    /* Copy data with builtin memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) 
        copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Create children recursively */
    node->left = create_ast(depth - 1, "left_child");
    node->right = create_ast(depth - 1, "right_child");
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ast_node_t* src, ast_node_t* dst) {
    volatile int use_memmove = 1;
    
    if (!src || !dst) goto cleanup;
    
    /* Jump into memory operation block */
    if (use_memmove) {
        goto do_memmove;
    }
    
skip_memmove:
    /* Normal processing */
    __builtin_memcpy(dst->data, "modified", 9);
    goto cleanup;
    
do_memmove:
    /* Use builtin memmove with overlap */
    __builtin_memmove(src->data + 8, src->data, 16);
    goto skip_memmove;
    
cleanup:
    /* Final cleanup with memset */
    volatile char temp[16];
    __builtin_memset(temp, 0, sizeof(temp));
}

/* Compute hash of AST structure */
static uint32_t compute_ast_hash(ast_node_t* node) {
    if (!node) return 0;
    
    uint32_t hash = 5381;
    char* ptr = node->data;
    
    /* Process string with volatile control */
    volatile size_t i = 0;
    while (i < sizeof(node->data) && ptr[i]) {
        hash = ((hash << 5) + hash) + ptr[i];
        i++;
    }
    
    /* Combine with children hashes */
    hash ^= compute_ast_hash(node->left);
    hash ^= compute_ast_hash(node->right);
    
    return hash;
}

/* Main parallel processing function */
static void parallel_memory_operations(void) {
    const int num_ops = 100;
    char* buffers[10];
    
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < num_ops; i++) {
            /* Each thread allocates and operates on memory */
            size_t size = g_mem_size + (i % 32);
            char* buf1 = (char*)malloc(size);
            char* buf2 = (char*)malloc(size);
            
            if (buf1 && buf2) {
                /* Use all three builtins in parallel */
                __builtin_memset(buf1, i % 256, size);
                __builtin_memcpy(buf2, buf1, size);
                
                /* Create overlap for memmove */
                if (size > 16) {
                    __builtin_memmove(buf1 + 8, buf1, size - 8);
                }
                
                /* Store some buffers for later verification */
                if (i < 10) {
                    buffers[i % 10] = buf2;
                } else {
                    free(buf2);
                }
            }
            
            free(buf1);
        }
        
        /* Verify buffers in critical section */
        #pragma omp critical
        {
            for (int i = 0; i < 10; i++) {
                if (buffers[i]) {
                    volatile char check = buffers[i][0];
                    (void)check; /* Use variable */
                    free(buffers[i]);
                    buffers[i] = NULL;
                }
            }
        }
    }
}

/* Multi-stage initialization */
static void initialize_system(void) {
    /* Stage 1: Basic memory setup */
    volatile char stage1_buf[128];
    __builtin_memset(stage1_buf, 0xAA, sizeof(stage1_buf));
    
    /* Stage 2: Copy between volatiles */
    volatile char stage2_buf[128];
    __builtin_memcpy((void*)stage2_buf, (void*)stage1_buf, sizeof(stage1_buf));
    
    /* Stage 3: Overlapping move */
    __builtin_memmove((void*)(stage1_buf + 32), (void*)stage1_buf, 64);
}

int main(void) {
    uint32_t final_hash = 0;
    
    /* Phase 1: System initialization */
    initialize_system();
    
    /* Phase 2: Create and process AST */
    ast_node_t* root = create_ast(3, "root_node");
    if (root) {
        /* Process with goto jumps */
        process_with_goto(root, root);
        
        /* Compute verification hash */
        final_hash = compute_ast_hash(root);
        
        /* Cleanup AST */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    /* Phase 3: Parallel operations */
    parallel_memory_operations();
    
    /* Phase 4: Final verification */
    volatile char final_buf[256];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf + 128, final_buf, 128);
    __builtin_memmove(final_buf, final_buf + 64, 192);
    
    /* Print result for verification */
    printf("AST Hash: 0x%08X\n", final_hash);
    printf("Memory operations completed successfully.\n");
    
    return 0;
}
