/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 256;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 192;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

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
    volatile char final_check[16];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile lengths */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    
    /* Fill data with pattern */
    for (int i = 0; i < 63; i++) {
        node->data[i] = 'A' + (depth + i) % 26;
    }
    node->data[63] = '\0';
    
    /* Recursive creation with goto for flow control */
    int create_children = 1;
    
    if (depth > 3) {
        create_children = 0;
    }
    
    if (create_children) {
        node->left = create_ast(depth - 1);
        node->right = create_ast(depth - 2);
        
        /* Copy between nodes if both exist */
        if (node->left && node->right) {
            volatile size_t copy_len = 32;
            if (copy_len > sizeof(node->left->data)) 
                copy_len = sizeof(node->left->data);
            
            __builtin_memcpy(node->right->data, node->left->data, copy_len);
            
            /* Goto jumping into memory operation block */
            goto memmove_block;
        }
    } else {
        node->left = node->right = NULL;
    }
    
    goto skip_memmove;
    
memmove_block:
    /* This tests flow sensitivity */
    if (node->left && node->right) {
        volatile char temp[64];
        __builtin_memmove(temp, node->left->data, 32);
        __builtin_memmove(node->right->data, temp, 32);
    }
    
skip_memmove:
    return node;
}

/* Function with complex memory operations */
static void process_ast(ASTNode* node, int* sum) {
    if (!node) return;
    
    /* Process data with builtins */
    volatile char buffer[128];
    size_t len = strlen(node->data);
    
    if (len > 0) {
        /* Force all three builtins */
        __builtin_memset(buffer, 0, sizeof(buffer));
        __builtin_memcpy(buffer, node->data, len);
        
        /* Conditional memmove with goto */
        if (len > 32) {
            goto do_memmove;
        }
        
        __builtin_memcpy(buffer + 64, buffer, len);
        goto skip_conditional;
        
    do_memmove:
        __builtin_memmove(buffer + 64, buffer, 32);
        
    skip_conditional:
        /* Add to sum */
        for (size_t i = 0; i < len && i < sizeof(buffer); i++) {
            *sum += buffer[i];
        }
    }
    
    process_ast(node->left, sum);
    process_ast(node->right, sum);
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
        char buffer1[256];
        char buffer2[256];
        
        /* Use volatile lengths */
        volatile size_t len = g_memcpy_len;
        if (len > sizeof(buffer1)) len = sizeof(buffer1);
        
        /* Initialize with builtin */
        __builtin_memset(buffer1, thread_id + 'A', len);
        
        /* Copy between buffers */
        __builtin_memcpy(buffer2, buffer1, len);
        
        /* Move within buffer */
        if (len > 64) {
            __builtin_memmove(buffer1 + 128, buffer1, 64);
        }
        
        /* Verify copy */
        for (size_t i = 0; i < len; i++) {
            if (buffer1[i] != buffer2[i]) {
                #pragma omp critical
                printf("Thread %d: mismatch at %zu\n", thread_id, i);
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    int total_sum = 0;
    
    printf("Starting ASAN/HWASAN builtin test...\n");
    
    /* Phase 1: Create and process AST */
    ASTNode* root = create_ast(5);
    if (root) {
        process_ast(root, &total_sum);
        
        /* Additional memory operations on AST */
        if (root->left && root->right) {
            volatile size_t copy_size = 48;
            __builtin_memcpy(root->right->data, root->left->data, 
                           copy_size > 63 ? 63 : copy_size);
        }
    }
    
    /* Phase 2: Parallel operations */
    parallel_memory_ops();
    
    /* Phase 3: Direct builtin calls with volatile control */
    {
        volatile char src[512];
        volatile char dst[512];
        volatile char mid[512];
        
        /* Initialize with pattern */
        for (size_t i = 0; i < sizeof(src); i++) {
            src[i] = (char)(i % 256);
        }
        
        /* Chain of builtin operations */
        __builtin_memset(dst, 0, sizeof(dst));
        __builtin_memcpy(dst, src, g_memcpy_len);
        __builtin_memmove(mid, dst, g_memmove_len);
        __builtin_memset(mid + 100, 0xCC, g_memset_len);
        
        /* Verify results */
        for (size_t i = 0; i < 256; i++) {
            total_sum += dst[i] + mid[i];
        }
    }
    
    /* Phase 4: Nested goto with memory ops */
    {
        volatile int counter = 0;
        volatile char data[4][64];
        
    restart:
        __builtin_memset(data[counter % 4], counter, 64);
        
        if (counter < 3) {
            counter++;
            goto restart;
        }
        
        /* Copy between all buffers */
        for (int i = 0; i < 3; i++) {
            __builtin_memcpy(data[i+1], data[i], 32);
        }
        
        total_sum += counter;
    }
    
    printf("Final checksum: %d\n", total_sum);
    
    /* Cleanup */
    /* Note: In real code, would need to free AST recursively */
    
    return total_sum != 0 ? 0 : 1;
}
