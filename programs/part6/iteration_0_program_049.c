/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_init_flag = 0;

/* Recursive AST-like structure */
struct ast_node {
    int type;
    char data[64];
    struct ast_node *left;
    struct ast_node *right;
    struct ast_node *next;
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force early initialization of memory functions */
    char buffer[32];
    volatile char *volatile_ptr = buffer;
    
    /* Use all three builtins in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 8);
    __builtin_memmove(buffer + 8, buffer, 12);
    
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    volatile int cleanup = 1;
    char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    struct ast_node *node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(struct ast_node));
    
    /* Copy data with builtin memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) 
        copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->type = depth;
    
    /* Build children recursively */
    char child_data[64];
    volatile int offset = 5;
    
    /* Use goto for control flow edge case */
    if (depth > 1) {
        goto build_children;
    }
    
    skip_children:
    node->left = NULL;
    node->right = NULL;
    return node;
    
    build_children:
    /* Complex memmove with goto jumping into block */
    __builtin_memmove(child_data, base_data, copy_len);
    child_data[offset] = 'L';
    node->left = build_ast(depth - 1, child_data);
    
    /* Another memmove */
    __builtin_memmove(child_data, base_data, copy_len);
    child_data[offset] = 'R';
    node->right = build_ast(depth - 1, child_data);
    
    goto skip_children;
}

/* Process AST with memory operations between nodes */
static int process_ast(struct ast_node *node, int *sum) {
    if (!node) return 0;
    
    volatile int local_sum = 0;
    
    /* Copy data between nodes if siblings exist */
    if (node->left && node->right) {
        char temp[64];
        
        /* Use all three builtins in sequence */
        __builtin_memset(temp, 0, sizeof(temp));
        __builtin_memcpy(temp, node->left->data, sizeof(temp));
        __builtin_memmove(node->right->data, temp, sizeof(temp));
        
        /* Verify the copy */
        if (__builtin_memcmp(node->left->data, node->right->data, 32) == 0) {
            local_sum += 1;
        }
    }
    
    /* Process children */
    local_sum += process_ast(node->left, sum);
    local_sum += process_ast(node->right, sum);
    
    /* Add node type to sum */
    *sum += node->type + local_sum;
    
    return local_sum;
}

/* Free AST recursively */
static void free_ast(struct ast_node *node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    volatile char *data = node->data;
    __builtin_memset(data, 0, sizeof(node->data));
    free(node);
}

/* Main test function with OpenMP parallel section */
int main(void) {
    int total_sum = 0;
    volatile size_t array_size = g_mem_size;
    
    /* Create token array */
    char *token_array = malloc(array_size);
    if (!token_array) return 1;
    
    /* Initialize with builtin memset */
    __builtin_memset(token_array, 0xCC, array_size);
    
    /* Build AST */
    struct ast_node *root = build_ast(4, "TestASTData");
    if (!root) {
        free(token_array);
        return 1;
    }
    
    /* Process AST */
    process_ast(root, &total_sum);
    
    /* OpenMP parallel section with memory operations */
    #pragma omp parallel
    {
        int thread_sum = 0;
        char local_buffer[128];
        volatile int i;
        
        /* Each thread uses builtins */
        #pragma omp for
        for (i = 0; i < 16; i++) {
            /* Pattern: memset -> memcpy -> memmove */
            __builtin_memset(local_buffer, i, sizeof(local_buffer));
            
            /* Copy to token array with offset */
            size_t offset = (i * 8) % array_size;
            if (offset + 8 <= array_size) {
                __builtin_memcpy(token_array + offset, local_buffer, 8);
                __builtin_memmove(local_buffer, token_array + offset, 8);
            }
            
            thread_sum += i;
        }
        
        #pragma omp atomic
        total_sum += thread_sum;
    }
    
    /* Additional memory operations in main */
    char final_buffer[256];
    volatile int use_memmove = 1;
    
    if (use_memmove) {
        /* Complex memmove scenario */
        __builtin_memmove(final_buffer, token_array, 128);
        __builtin_memmove(final_buffer + 128, token_array + 128, 128);
        __builtin_memcpy(token_array, final_buffer, 64);
    } else {
        __builtin_memcpy(final_buffer, token_array, sizeof(final_buffer));
    }
    
    /* Verify operations with final memset */
    __builtin_memset(final_buffer + 192, 0xFF, 64);
    
    /* Calculate final hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        hash = hash * 31 + final_buffer[i];
    }
    
    total_sum += (int)(hash % 1000);
    
    /* Cleanup */
    free_ast(root);
    free(token_array);
    
    /* Print verification result */
    printf("ASAN test completed. Result: %d (init flag: %d)\n", 
           total_sum, g_init_flag);
    
    return 0;
}
