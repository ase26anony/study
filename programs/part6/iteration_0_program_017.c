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

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Force early initialization of ASAN runtime */
    volatile char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    volatile char buffer[16];
    __builtin_memset(buffer, 0xFF, sizeof(buffer));
    printf("Destructor: ASAN environment cleaned up\n");
}

/* Function with goto statements for flow control */
static void memcpy_with_goto(char* dest, const char* src, size_t n) {
    int use_builtin = 1;
    
    if (n == 0) goto skip_copy;
    
    /* Jump into memory operation block */
    goto do_copy;
    
copy_block:
    /* Force __builtin_memcpy redirection */
    __builtin_memcpy(dest, src, n);
    goto after_copy;
    
do_copy:
    if (use_builtin) goto copy_block;
    
    /* Alternative path */
    for (size_t i = 0; i < n; i++) dest[i] = src[i];
    
after_copy:
    /* Verify copy */
    for (size_t i = 0; i < n; i++) {
        if (dest[i] != src[i]) {
            printf("Copy verification failed at index %zu\n", i);
            break;
        }
    }
    
skip_copy:
    return;
}

/* Recursive AST manipulation with memory operations */
static struct ast_node* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, 0, sizeof(struct ast_node));
    node->depth = depth;
    
    /* Fill data with pattern */
    for (int i = 0; i < 63; i++) {
        node->data[i] = 'A' + (depth + i) % 26;
    }
    node->data[63] = '\0';
    
    /* Recursive creation */
    node->left = create_ast(depth - 1);
    node->right = create_ast(depth - 1);
    
    return node;
}

/* Copy AST nodes using __builtin_memmove (handles overlapping) */
static void copy_ast_node(struct ast_node* dest, const struct ast_node* src) {
    if (!dest || !src) return;
    
    /* Use volatile to prevent optimization */
    volatile size_t copy_size = sizeof(struct ast_node);
    
    /* __builtin_memmove for potentially overlapping regions */
    __builtin_memmove(dest, src, copy_size);
    
    /* Verify with regular memcpy for comparison */
    struct ast_node verify;
    __builtin_memcpy(&verify, src, sizeof(struct ast_node));
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread allocates and operates on its own buffer */
        buffers[tid] = malloc(g_mem_size);
        if (buffers[tid]) {
            /* Use all three builtins in different contexts */
            __builtin_memset(buffers[tid], tid, g_mem_size);
            
            if (tid > 0) {
                /* Copy from previous thread's buffer */
                __builtin_memcpy(buffers[tid], buffers[tid-1], g_mem_size);
            }
            
            /* Circular shift within buffer using memmove */
            size_t shift = g_mem_size / 4;
            __builtin_memmove(buffers[tid], buffers[tid] + shift, 
                            g_mem_size - shift);
            
            /* Verify pattern */
            int errors = 0;
            for (size_t i = 0; i < g_mem_size - shift; i++) {
                if (buffers[tid][i] != (char)((tid > 0) ? tid-1 : tid)) {
                    errors++;
                }
            }
            
            #pragma omp critical
            {
                printf("Thread %d: %d verification errors\n", tid, errors);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        if (buffers[i]) free(buffers[i]);
    }
}

/* Complex token array initialization */
static void initialize_token_array(char tokens[][32], int count) {
    for (int i = 0; i < count; i++) {
        /* Use __builtin_memset for initialization */
        __builtin_memset(tokens[i], 0, 32);
        
        /* Fill with pattern */
        for (int j = 0; j < 31; j++) {
            tokens[i][j] = 'a' + (i + j) % 26;
        }
        tokens[i][31] = '\0';
        
        /* Occasionally use memcpy between tokens */
        if (i > 0 && (i % 3 == 0)) {
            __builtin_memcpy(tokens[i], tokens[i-1], 32);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Basic built-in calls with volatile control */
    volatile size_t test_size = 128;
    char src[256], dest[256];
    
    /* Force all three builtins to be called */
    __builtin_memset(src, 0xAA, test_size);
    __builtin_memcpy(dest, src, test_size);
    __builtin_memmove(src + 64, src, 64);  /* Overlapping region */
    
    /* Phase 2: Goto-based memory operations */
    memcpy_with_goto(dest + 128, src, 64);
    
    /* Phase 3: Recursive AST operations */
    struct ast_node* ast = create_ast(3);
    if (ast) {
        struct ast_node ast_copy;
        copy_ast_node(&ast_copy, ast);
        
        /* Recursive traversal and memory operations */
        struct ast_node* stack[10];
        int top = 0;
        stack[top++] = ast;
        
        while (top > 0) {
            struct ast_node* current = stack[--top];
            if (current->left) {
                __builtin_memcpy(stack[top++], current->left, 
                               sizeof(struct ast_node*));
            }
            if (current->right) {
                __builtin_memcpy(stack + top, &current->right, 
                               sizeof(struct ast_node*));
                top++;
            }
        }
        
        /* Free AST */
        free(ast);
    }
    
    /* Phase 4: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 5: Token array processing */
    char tokens[10][32];
    initialize_token_array(tokens, 10);
    
    /* Compute verification hash */
    uint32_t hash = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 32; j++) {
            hash = (hash * 31) + tokens[i][j];
        }
    }
    
    printf("Final verification hash: 0x%08X\n", hash);
    printf("Test completed successfully\n");
    
    return 0;
}
