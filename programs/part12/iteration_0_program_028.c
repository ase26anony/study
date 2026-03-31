#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    volatile int depth;
} ASTNode;

/* Global volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_init_flag = 1;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_hooks(void) {
    volatile char buffer[128];
    volatile char* dest = buffer;
    volatile const char* src = "Constructor init";
    
    /* Force builtin calls in constructor */
    __builtin_memcpy((void*)dest, (void*)src, 16);
    __builtin_memset((void*)(dest + 16), 'X', 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_hooks(void) {
    volatile char cleanup_buf[256];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile control */
    volatile size_t copy_len = g_mem_size;
    if (copy_len > 63) copy_len = 63;
    
    /* Complex control flow with goto */
    int use_memmove = 0;
    if (depth % 3 == 0) {
        use_memmove = 1;
        goto memmove_block;
    }
    
    /* Regular memcpy path */
    __builtin_memset(node->data, 0, sizeof(node->data));
    __builtin_memcpy(node->data, base_data, copy_len);
    goto after_copy;
    
memmove_block:
    /* Memmove with overlapping regions */
    char temp[128];
    __builtin_memcpy(temp, base_data, copy_len);
    __builtin_memmove(node->data, temp, copy_len);
    
after_copy:
    node->depth = depth;
    
    /* Recursive creation with different patterns */
    if (depth > 1) {
        char child_data[64];
        volatile int offset = depth * 4;
        __builtin_memcpy(child_data, node->data, copy_len);
        child_data[copy_len - 1] += offset;
        
        node->left = create_ast(depth - 1, child_data);
        node->right = create_ast(depth - 2, child_data);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with goto jumping in/out of memory blocks */
static void process_with_goto(ASTNode* node) {
    if (!node) return;
    
    volatile int mode = node->depth % 4;
    char buffer[128];
    
    switch (mode) {
        case 0:
            __builtin_memset(buffer, 'A', 64);
            goto process_block;
            
        case 1:
            __builtin_memcpy(buffer, node->data, 32);
            goto skip_memmove;
            
        case 2:
            /* Jump into memmove block */
            goto memmove_jump;
            
        default:
            break;
    }
    
    return;
    
process_block:
    __builtin_memmove(buffer + 16, buffer, 32);
    return;
    
memmove_jump:
    /* This tests flow sensitivity */
    __builtin_memmove(node->data, buffer, 16);
    return;
    
skip_memmove:
    /* Skip memmove but use memset */
    __builtin_memset(buffer + 32, 'Z', 16);
    return;
}

/* Calculate hash of AST */
static uint32_t hash_ast(const ASTNode* node) {
    if (!node) return 0;
    
    uint32_t hash = 5381;
    volatile size_t len = g_mem_size;
    if (len > 63) len = 63;
    
    /* Hash node data */
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    /* Recursive hash */
    hash += hash_ast(node->left);
    hash ^= hash_ast(node->right);
    
    return hash;
}

/* Free AST */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    volatile char* data = node->data;
    __builtin_memset(data, 0, sizeof(node->data));
    free(node);
}

/* Main execution with OpenMP */
int main(void) {
    const char* base_data = "AST Node Base Data for ASAN Testing 0123456789";
    volatile int ast_depth = 5;
    
    /* Create AST structure */
    ASTNode* root = create_ast(ast_depth, base_data);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    uint32_t final_hash = 0;
    
    /* OpenMP parallel region with memory operations */
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[256];
        char src_buf[256];
        
        /* Initialize with builtins */
        volatile size_t init_size = g_mem_size + thread_id * 16;
        if (init_size > 255) init_size = 255;
        
        __builtin_memset(local_buf, thread_id + '0', init_size);
        __builtin_memcpy(src_buf, base_data, init_size % 64);
        
        /* Complex memory operations */
        for (int i = 0; i < 3; i++) {
            volatile size_t op_size = (init_size * (i + 1)) % 128;
            
            switch (i) {
                case 0:
                    __builtin_memcpy(local_buf + 64, src_buf, op_size);
                    break;
                case 1:
                    __builtin_memset(local_buf + 128, 'X', op_size);
                    break;
                case 2:
                    /* Overlapping memmove */
                    __builtin_memmove(local_buf + 32, local_buf + 16, op_size);
                    break;
            }
        }
        
        /* Process AST with goto patterns */
        #pragma omp critical
        {
            process_with_goto(root);
        }
        
        /* Update hash */
        #pragma omp atomic
        final_hash += hash_ast(root) + thread_id;
    }
    
    /* Additional serial memory operations */
    char final_buffer[512];
    volatile size_t final_size = g_mem_size * 2;
    
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, root->data, 64);
    __builtin_memmove(final_buffer + 128, final_buffer, 64);
    
    /* Print results */
    printf("AST Hash: %u\n", final_hash);
    printf("Buffer[0]: %c\n", final_buffer[0]);
    printf("Buffer[128]: %c\n", final_buffer[128]);
    
    /* Cleanup */
    free_ast(root);
    
    /* Final builtin calls */
    volatile char exit_buf[64];
    __builtin_memset(exit_buf, 0, sizeof(exit_buf));
    __builtin_memcpy(exit_buf, "Exit", 4);
    
    return 0;
}
