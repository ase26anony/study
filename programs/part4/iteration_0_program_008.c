/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ast_node {
    char data[256];
    struct ast_node* left;
    struct ast_node* right;
    size_t size;
} ast_node_t;

/* Global token array */
static char g_token_pool[4096];
static volatile size_t g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Force initialization of memory functions */
    char buffer[128];
    volatile char* volatile_ptr = buffer;
    
    /* Use all three builtins in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 64, buffer, 32);
    __builtin_memmove(buffer + 32, buffer, 16);
    
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operations */
    char final_buf[64];
    volatile int* vptr = (volatile int*)final_buf;
    
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
    *vptr = 0xDEADBEEF;
}

/* Recursive parser with memory operations */
static ast_node_t* parse_expression(size_t depth) {
    if (depth == 0) {
        ast_node_t* leaf = malloc(sizeof(ast_node_t));
        if (!leaf) return NULL;
        
        /* Use builtins in allocation context */
        __builtin_memset(leaf, 0, sizeof(*leaf));
        leaf->size = g_mem_size;
        __builtin_memcpy(leaf->data, g_token_pool + g_token_idx, 32);
        g_token_idx = (g_token_idx + 32) % sizeof(g_token_pool);
        
        return leaf;
    }
    
    ast_node_t* node = malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(*node));
    node->size = depth * 16;
    
    /* Recursive parsing with goto for flow control */
    int state = 0;
    
    goto parse_left;
    
parse_left:
    node->left = parse_expression(depth - 1);
    state = 1;
    goto check_state;
    
parse_right:
    node->right = parse_expression(depth - 1);
    state = 2;
    goto check_state;
    
check_state:
    if (state == 1) {
        /* Copy data between nodes using memcpy */
        if (node->left) {
            __builtin_memcpy(node->data, node->left->data, 64);
            goto parse_right;
        }
    }
    
    /* Use memmove for overlapping regions */
    if (state == 2 && node->right) {
        __builtin_memmove(node->data + 32, node->data, 32);
        __builtin_memcpy(node->data + 64, node->right->data, 64);
    }
    
    return node;
}

/* Complex memory dispatch with OpenMP */
static size_t dispatch_memory_operations(void) {
    size_t total_hash = 0;
    char* buffers[8];
    volatile size_t sizes[8];
    
    /* Initialize with volatile sizes */
    for (int i = 0; i < 8; i++) {
        sizes[i] = (g_mem_size * (i + 1)) % 256 + 16;
        buffers[i] = malloc(sizes[i]);
        if (!buffers[i]) continue;
        
        /* Force builtin usage before parallel region */
        __builtin_memset(buffers[i], i, sizes[i]);
    }
    
    #pragma omp parallel reduction(+:total_hash)
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread uses different builtins */
        switch (thread_id % 3) {
            case 0: {
                /* memcpy pattern */
                char local_buf[128];
                __builtin_memcpy(local_buf, buffers[thread_id % 8], 
                                sizes[thread_id % 8] % 128);
                
                /* Compute hash */
                for (size_t j = 0; j < sizeof(local_buf); j++) {
                    total_hash += (size_t)local_buf[j];
                }
                break;
            }
            case 1: {
                /* memset pattern with goto */
                volatile char* target = buffers[(thread_id + 1) % 8];
                volatile size_t len = sizes[(thread_id + 1) % 8];
                
                if (len > 0) {
                    goto do_memset;
                }
                goto skip;
                
            do_memset:
                __builtin_memset((void*)target, thread_id, len);
                goto skip;
                
            skip:
                /* memmove after memset */
                if (len > 32) {
                    __builtin_memmove((void*)(target + 16), target, len - 16);
                }
                break;
            }
            case 2: {
                /* memmove with overlapping regions */
                char overlap_buf[256];
                volatile int use_builtin = 1;
                
                if (use_builtin) {
                    __builtin_memset(overlap_buf, 0xCC, sizeof(overlap_buf));
                    __builtin_memmove(overlap_buf + 64, overlap_buf, 128);
                    __builtin_memcpy(buffers[thread_id % 8], overlap_buf, 64);
                }
                break;
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        if (buffers[i]) {
            free(buffers[i]);
        }
    }
    
    return total_hash;
}

/* Main execution flow */
int main(void) {
    /* Initialize token pool */
    for (size_t i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)(i % 256);
    }
    
    /* Create recursive structure */
    ast_node_t* root = parse_expression(3);
    
    /* Process structure with builtins */
    if (root) {
        /* Copy entire structure */
        ast_node_t copy;
        __builtin_memcpy(&copy, root, sizeof(ast_node_t));
        
        /* Move data within structure */
        __builtin_memmove(root->data + 128, root->data, 128);
        
        /* Clear part of structure */
        __builtin_memset(root->data + 192, 0, 64);
    }
    
    /* Execute parallel memory operations */
    size_t final_hash = dispatch_memory_operations();
    
    /* Additional builtin usage in main */
    char final_buffer[512];
    volatile char* vdest = final_buffer;
    volatile const char* vsrc = g_token_pool;
    volatile size_t vlen = g_mem_size;
    
    /* Chain of builtins with volatile control */
    __builtin_memset(vdest, 0, sizeof(final_buffer));
    __builtin_memcpy(vdest, vsrc, vlen);
    __builtin_memmove(vdest + 256, vdest, 128);
    
    /* Compute verification result */
    size_t verification = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        verification += (size_t)final_buffer[i];
    }
    
    verification += final_hash;
    
    /* Print result for verification */
    printf("ASAN coverage test result: %zu (init flag: %d)\n", 
           verification, (int)g_init_flag);
    
    /* Cleanup */
    if (root) {
        free(root);
    }
    
    return 0;
}
