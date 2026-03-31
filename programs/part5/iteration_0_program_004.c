/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    char data[256];
    struct ast_node *left;
    struct ast_node *right;
    int depth;
};

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[128];
    /* Force __builtin_memset redirection early */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    g_use_hwasan = (rand() % 2);
}

/* Destructor to test cleanup paths */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for initialization */
    __builtin_memcpy(node->data, base_data, strlen(base_data) + 1);
    node->depth = depth;
    
    /* Create child nodes with goto for flow control */
    if (depth > 1) {
        char child_data[256];
        volatile size_t copy_len = g_mem_size % 128;
        
        /* Jump label for goto testing */
        build_left:
        __builtin_memcpy(child_data, node->data, copy_len);
        node->left = build_ast(depth - 1, child_data);
        
        /* Conditional goto to test flow sensitivity */
        if (depth % 3 == 0) {
            goto skip_right;
        }
        
        /* Use __builtin_memmove for overlapping regions */
        __builtin_memmove(child_data + 32, child_data, copy_len);
        node->right = build_ast(depth - 1, child_data + 32);
        goto done;
        
        skip_right:
        node->right = NULL;
        
        done:;
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with OpenMP parallel memory operations */
static void parallel_memory_ops(struct ast_node** nodes, int count) {
    volatile int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            char temp[256];
            volatile size_t op_size = (g_mem_size + i) % 128;
            
            /* Mix of memory built-ins in parallel region */
            __builtin_memset(temp, i, sizeof(temp));
            __builtin_memcpy(nodes[i]->data, temp, op_size);
            
            /* Conditional __builtin_memmove with goto */
            if (i % 4 == 0) {
                goto do_memmove;
            }
            continue;
            
            do_memmove:
            __builtin_memmove(temp + 16, temp, op_size);
            __builtin_memcpy(nodes[i]->data + 16, temp + 16, op_size);
        }
    }
}

/* Complex token processing with varied memory operations */
static unsigned long process_tokens(const char** tokens, int token_count) {
    unsigned long hash = 0xDEADBEEF;
    char buffer[512];
    volatile size_t buf_idx = 0;
    
    for (int i = 0; i < token_count; i++) {
        size_t len = strlen(tokens[i]);
        volatile size_t copy_len = len % 256;
        
        /* Force __builtin_memcpy redirection */
        __builtin_memcpy(buffer + buf_idx, tokens[i], copy_len);
        buf_idx += copy_len;
        
        /* Intermittent __builtin_memset */
        if (i % 5 == 0) {
            __builtin_memset(buffer + buf_idx, 0xCC, 32);
            buf_idx += 32;
        }
        
        /* Goto-based __builtin_memmove */
        if (i == token_count / 2) {
            goto mid_point;
        }
        continue;
        
        mid_point:
        __builtin_memmove(buffer, buffer + 128, 256);
    }
    
    /* Compute hash from buffer */
    for (size_t j = 0; j < buf_idx; j++) {
        hash = (hash * 31) + buffer[j];
    }
    
    return hash;
}

int main(void) {
    /* Initialize complex token array */
    const char* tokens[] = {
        "ASAN_TEST_STRING_1",
        "HWASAN_REDIRECTION_2",
        "MEMCPY_BUILTIN_3",
        "MEMSET_FLOW_4",
        "MEMMOVE_GOTO_5",
        "PARALLEL_OMP_6",
        "RECURSIVE_AST_7",
        "VOLATILE_VARS_8"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Build recursive AST structures */
    struct ast_node* nodes[8];
    for (int i = 0; i < 8; i++) {
        nodes[i] = build_ast(3 + (i % 3), tokens[i % token_count]);
    }
    
    /* Execute parallelized memory operations */
    parallel_memory_ops(nodes, 8);
    
    /* Process tokens with memory built-ins */
    unsigned long result = process_tokens(tokens, token_count);
    
    /* Print verification result */
    printf("Memory operations completed. Hash: 0x%08lX\n", result);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            /* Final __builtin_memset before free */
            __builtin_memset(nodes[i]->data, 0, sizeof(nodes[i]->data));
            free(nodes[i]);
        }
    }
    
    return 0;
}
