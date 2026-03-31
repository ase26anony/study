/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    char data[64];
    struct ast_node* left;
    struct ast_node* right;
    int depth;
};

/* Attribute constructors/destructors for initialization coordination */
__attribute__((constructor)) static void init_asan_hooks(void) {
    printf("Constructor: Initializing ASAN/HWASAN hooks\n");
}

__attribute__((destructor)) static void cleanup_asan_hooks(void) {
    printf("Destructor: Cleaning up ASAN/HWASAN hooks\n");
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(struct ast_node));
    
    /* Copy data using __builtin_memcpy with volatile size */
    size_t copy_len = g_mem_size < 64 ? g_mem_size : 64;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->depth = depth;
    
    /* Recursive construction with goto for flow control */
    int build_left = 1;
    
    if (depth > 2) {
        goto build_children;
    } else {
        node->left = NULL;
        node->right = NULL;
        goto done;
    }
    
build_children:
    node->left = build_ast(depth - 1, base_data);
    
    /* Jump back and forth with goto */
    if (depth % 2 == 0) {
        goto build_right;
    } else {
        node->right = build_ast(depth - 2, base_data);
        goto done;
    }
    
build_right:
    node->right = build_ast(depth - 1, base_data);
    
done:
    return node;
}

/* Function with complex memory operations and goto */
static void process_ast(struct ast_node* node, char* buffer) {
    if (!node) return;
    
    volatile int use_memmove = 1;
    
    /* Label for goto into memory operation block */
process_node:
    if (use_memmove) {
        /* Use __builtin_memmove with goto control flow */
        char temp[64];
        __builtin_memcpy(temp, node->data, 64);
        
        if (node->depth > 1) {
            goto do_memmove;
        }
        
        __builtin_memcpy(buffer, temp, 64);
        goto next;
        
    do_memmove:
        /* This tests the memmove redirection */
        __builtin_memmove(buffer, temp, 64);
        goto next;
    }
    
next:
    /* Process children */
    process_ast(node->left, buffer + 64);
    process_ast(node->right, buffer + 128);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread allocates and operates on memory */
        buffers[tid] = malloc(g_mem_size);
        if (!buffers[tid]) {
            #pragma omp critical
            printf("Thread %d: Allocation failed\n", tid);
            return;
        }
        
        /* Use all three builtins in parallel */
        __builtin_memset(buffers[tid], tid, g_mem_size);
        
        #pragma omp barrier
        
        /* Circular shift using memcpy/memmove */
        int src_tid = (tid + 1) % num_threads;
        
        if (tid % 2 == 0) {
            __builtin_memcpy(buffers[tid], buffers[src_tid], g_mem_size / 2);
        } else {
            __builtin_memmove(buffers[tid], buffers[src_tid], g_mem_size / 2);
        }
        
        #pragma omp barrier
        
        /* Verify and compute hash */
        uint32_t hash = 0;
        for (size_t i = 0; i < g_mem_size; i++) {
            hash = (hash * 31) + buffers[tid][i];
        }
        
        #pragma omp critical
        {
            printf("Thread %d: Hash = 0x%08x\n", tid, hash);
        }
        
        free(buffers[tid]);
    }
}

/* Multi-stage initialization with memory builtins */
static char* init_token_array(void) {
    static const char* tokens[] = {
        "memcpy_test", "memset_test", "memmove_test",
        "asan_redirect", "hwasan_check", "builtin_flow"
    };
    
    size_t total_len = 0;
    for (int i = 0; i < 6; i++) {
        total_len += strlen(tokens[i]) + 1;
    }
    
    char* array = malloc(total_len);
    if (!array) return NULL;
    
    char* ptr = array;
    for (int i = 0; i < 6; i++) {
        size_t len = strlen(tokens[i]);
        __builtin_memcpy(ptr, tokens[i], len);
        ptr[len] = (i < 5) ? ':' : '\0';
        ptr += len + 1;
    }
    
    return array;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Stage 1: Initialize token array */
    char* tokens = init_token_array();
    if (!tokens) {
        fprintf(stderr, "Failed to initialize tokens\n");
        return 1;
    }
    
    /* Stage 2: Build and process recursive AST */
    struct ast_node* root = build_ast(4, tokens);
    if (!root) {
        fprintf(stderr, "Failed to build AST\n");
        free(tokens);
        return 1;
    }
    
    char ast_buffer[256];
    __builtin_memset(ast_buffer, 0, sizeof(ast_buffer));
    process_ast(root, ast_buffer);
    
    /* Stage 3: Execute parallel memory operations */
    printf("\nParallel memory operations:\n");
    parallel_memory_ops();
    
    /* Stage 4: Verify results and compute final hash */
    uint64_t final_hash = 0;
    for (int i = 0; i < 256; i++) {
        final_hash = final_hash * 31 + ast_buffer[i];
    }
    
    for (size_t i = 0; tokens[i] != '\0'; i++) {
        final_hash = final_hash * 31 + tokens[i];
    }
    
    printf("\nFinal verification hash: 0x%016llx\n", 
           (unsigned long long)final_hash);
    
    /* Cleanup */
    /* Recursive free would be needed for full AST cleanup */
    free(tokens);
    
    printf("Test completed successfully\n");
    return 0;
}
