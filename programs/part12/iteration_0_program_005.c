/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 256;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 192;

/* Recursive AST-like structure */
struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_late(void) {
    volatile char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static struct ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    struct ASTNode* node = malloc(sizeof(*node));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    node->left = node->right = NULL;
    
    /* Use builtins on node data */
    __builtin_memset(node->data, node->id % 256, sizeof(node->data));
    
    if (depth > 1) {
        node->left = create_ast(depth - 1, counter);
        node->right = create_ast(depth - 1, counter);
        
        /* Copy between child nodes if both exist */
        if (node->left && node->right) {
            __builtin_memcpy(node->right->data, node->left->data, 
                           sizeof(node->data) / 2);
        }
    }
    
    return node;
}

/* Function with goto edge cases */
static void test_goto_memmove(void* dst, const void* src, size_t n) {
    int use_memmove = 1;
    
    if (n == 0) goto skip_op;
    
    /* Jump into block with builtin */
    if (use_memmove) {
        goto do_memmove;
    } else {
        goto skip_op;
    }
    
do_memmove:
    __builtin_memmove(dst, src, n);
    goto after_op;
    
skip_op:
    /* Empty path */
    ;
    
after_op:
    /* Control flow continues */
    return;
}

/* OpenMP parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[256];
        char shared_buf[256];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp master
        {
            __builtin_memset(shared_buf, 0xCC, sizeof(shared_buf));
        }
        
        #pragma omp barrier
        
        /* Memcpy with volatile length */
        size_t copy_len = g_memcpy_len;
        if (copy_len > sizeof(local_buf)) copy_len = sizeof(local_buf);
        __builtin_memcpy(local_buf, shared_buf, copy_len);
        
        #pragma omp barrier
        
        /* Memmove with overlapping regions */
        if (tid % 2 == 0) {
            __builtin_memmove(local_buf + 32, local_buf, 64);
        }
    }
}

/* Main execution flow */
int main(void) {
    int counter = 1;
    unsigned long hash = 0;
    
    /* Phase 1: Create and manipulate AST */
    struct ASTNode* root = create_ast(4, &counter);
    
    if (root) {
        /* Traverse and compute hash */
        struct ASTNode* stack[32];
        int top = 0;
        stack[top++] = root;
        
        while (top > 0) {
            struct ASTNode* node = stack[--top];
            hash = hash * 31 + node->id;
            
            /* Use builtins on node data */
            char temp[64];
            __builtin_memcpy(temp, node->data, sizeof(temp));
            
            if (node->right) stack[top++] = node->right;
            if (node->left) stack[top++] = node->left;
        }
    }
    
    /* Phase 2: Test goto edge cases */
    char src_buf[512];
    char dst_buf[512];
    
    __builtin_memset(src_buf, 0x55, sizeof(src_buf));
    
    for (int i = 0; i < 3; i++) {
        test_goto_memmove(dst_buf + i * 64, src_buf + i * 32, 
                         g_memmove_len % 128);
    }
    
    /* Phase 3: Parallel operations */
    parallel_mem_ops();
    
    /* Phase 4: Direct builtin calls with volatile lengths */
    volatile char final_buffer[1024];
    
    __builtin_memset(final_buffer, 0, 
                    g_memset_len > sizeof(final_buffer) ? 
                    sizeof(final_buffer) : g_memset_len);
    
    __builtin_memcpy(final_buffer + 256, final_buffer, 
                    g_memcpy_len % 512);
    
    __builtin_memmove(final_buffer + 512, final_buffer + 256, 
                     g_memmove_len % 256);
    
    /* Verify by computing final hash */
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        hash = hash * 17 + final_buffer[i];
    }
    
    printf("Result hash: %lu\n", hash);
    
    /* Cleanup */
    /* ... AST cleanup code would go here ... */
    
    return 0;
}
