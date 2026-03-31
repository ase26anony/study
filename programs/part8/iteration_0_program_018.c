/* ISO C99-compliant program targeting ASAN built-in redirection logic */
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
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Force early initialization of ASAN runtime */
    volatile char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operation to ensure cleanup paths are taken */
    volatile char final_buf[32];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with __builtin_memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) 
        copy_len = sizeof(node->data);
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->size = copy_len;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    } else {
        node->left = NULL;
        node->right = NULL;
        goto done;
    }
    
create_children:
    /* Complex goto pattern around memory operations */
    char child_data[256];
    __builtin_memset(child_data, 'A' + depth, sizeof(child_data));
    
    node->left = create_ast(depth - 1, child_data);
    
    /* Jump back and forth */
    if (node->left) {
        goto copy_right_data;
    } else {
        goto skip_right;
    }
    
copy_right_data:
    /* Use __builtin_memmove for overlapping regions */
    __builtin_memmove(child_data + 128, child_data, 128);
    node->right = create_ast(depth - 2, child_data + 128);
    goto done;
    
skip_right:
    node->right = NULL;
    
done:
    return node;
}

/* Function with OpenMP parallel memory operations */
static void parallel_memory_operations(ASTNode* nodes[], int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i] && nodes[i]->left && nodes[i]->right) {
            /* Copy between AST nodes using builtins */
            size_t copy_size = nodes[i]->left->size;
            if (copy_size > sizeof(nodes[i]->right->data))
                copy_size = sizeof(nodes[i]->right->data);
            
            /* Force all three builtins in parallel region */
            __builtin_memcpy(nodes[i]->right->data, 
                           nodes[i]->left->data, 
                           copy_size);
            
            /* Clear with memset */
            __builtin_memset(nodes[i]->left->data + copy_size/2, 
                           0, 
                           copy_size/4);
            
            /* Move with memmove (potential overlap) */
            __builtin_memmove(nodes[i]->left->data,
                            nodes[i]->right->data,
                            copy_size/2);
        }
    }
}

/* Complex control flow with goto around memory operations */
static void goto_memory_patterns(char* buffer, size_t size) {
    volatile int pattern = 1;
    
    if (pattern == 1) {
        goto pattern_a;
    } else {
        goto pattern_b;
    }
    
pattern_a:
    __builtin_memset(buffer, 'A', size/2);
    goto check_overlap;
    
pattern_b:
    __builtin_memset(buffer, 'B', size);
    goto done;
    
check_overlap:
    /* Create overlapping regions for memmove */
    __builtin_memmove(buffer + size/4, buffer, size/4);
    
    volatile int use_memcpy = 1;
    if (use_memcpy) {
        goto do_memcpy;
    } else {
        goto done;
    }
    
do_memcpy:
    __builtin_memcpy(buffer + size/2, buffer, size/4);
    
done:
    return;
}

/* Main execution flow */
int main(void) {
    const int NUM_NODES = 8;
    ASTNode* nodes[NUM_NODES];
    char token_array[1024];
    size_t hash = 0;
    int i;
    
    /* Initialize token array with volatile size */
    size_t init_size = g_mem_size;
    if (init_size > sizeof(token_array))
        init_size = sizeof(token_array);
    
    __builtin_memset(token_array, 0, sizeof(token_array));
    
    /* Create recursive AST structures */
    for (i = 0; i < NUM_NODES; i++) {
        char base_data[64];
        __builtin_memset(base_data, '0' + i, sizeof(base_data));
        nodes[i] = create_ast(4 + (i % 3), base_data);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations(nodes, NUM_NODES);
    
    /* Apply goto patterns to token array */
    goto_memory_patterns(token_array, sizeof(token_array));
    
    /* Compute verification hash */
    for (i = 0; i < (int)sizeof(token_array); i++) {
        hash = (hash * 31) + (unsigned char)token_array[i];
    }
    
    /* Additional memory operations in main */
    volatile char temp_buf[256];
    __builtin_memcpy(temp_buf, token_array, 256);
    __builtin_memset(temp_buf + 128, 0xCC, 64);
    __builtin_memmove(token_array, temp_buf, 256);
    
    /* Update hash with final state */
    for (i = 0; i < 256; i++) {
        hash = (hash * 17) + (unsigned char)token_array[i];
    }
    
    printf("Verification hash: %zu\n", hash);
    
    /* Cleanup */
    for (i = 0; i < NUM_NODES; i++) {
        if (nodes[i]) {
            free(nodes[i]);
        }
    }
    
    return 0;
}
