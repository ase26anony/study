/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    char data[64];
    struct ast_node *left;
    struct ast_node *right;
    int id;
};

/* Constructor attribute for early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    printf("Constructor: Initializing ASAN/HWASAN environment\n");
    /* Force initialization of runtime */
    volatile char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    printf("Destructor: ASAN/HWASAN cleanup complete\n");
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node, 0, sizeof(*node));
    node->id = id;
    
    /* Create pattern in data */
    snprintf(node->data, sizeof(node->data), "AST_%d_%d", depth, id);
    
    /* Recursive construction */
    node->left = build_ast(depth - 1, id * 2);
    node->right = build_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Function with goto and memory operations */
static void process_with_goto(struct ast_node* src, struct ast_node* dst) {
    int use_memmove = 0;
    
    /* Jump into memory operation block */
    if (src && dst) {
        goto copy_block;
    }
    
    return;
    
copy_block:
    /* This goto tests flow sensitivity */
    if (use_memmove) {
        /* Use builtin memmove with overlap */
        __builtin_memmove(dst->data, src->data, 32);
    } else {
        /* Use builtin memcpy */
        __builtin_memcpy(dst->data, src->data, 32);
    }
    
    /* Jump out */
    goto finish;
    
finish:
    /* Final builtin memset */
    __builtin_memset(src->data + 32, 0xAA, 16);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(struct ast_node** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        volatile size_t local_size = g_mem_size;
        char temp_buffer[512];
        
        /* Mix of memory operations */
        __builtin_memset(temp_buffer, i, local_size % 512);
        
        if (nodes[i]) {
            __builtin_memcpy(nodes[i]->data, temp_buffer, 64);
            
            /* Conditional memmove */
            if (i > 0 && nodes[i-1]) {
                __builtin_memmove(nodes[i-1]->data + 16, 
                                 nodes[i]->data, 32);
            }
        }
        
        /* Use all three builtins in one block */
        __builtin_memset(temp_buffer + 128, 0xFF, 64);
        __builtin_memcpy(temp_buffer + 192, temp_buffer, 64);
        __builtin_memmove(temp_buffer + 256, temp_buffer + 128, 64);
    }
}

/* Complex token processing */
static unsigned long process_tokens(const char** tokens, int token_count) {
    unsigned long hash = 5381;
    char buffer[256];
    int i;
    
    /* Initialize buffer with builtin */
    __builtin_memset(buffer, 0, sizeof(buffer));
    
    for (i = 0; i < token_count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Copy token with builtin */
        __builtin_memcpy(buffer, tokens[i], len > 255 ? 255 : len);
        
        /* Process token */
        for (int j = 0; buffer[j] && j < 255; j++) {
            hash = ((hash << 5) + hash) + buffer[j];
        }
        
        /* Clear with builtin */
        __builtin_memset(buffer, 0, 256);
    }
    
    return hash;
}

int main(void) {
    const char* tokens[] = {
        "ASAN", "HWASAN", "MEMCPY", "MEMSET", "MEMMOVE",
        "BUILTIN", "REDIRECT", "COVERAGE", "TEST"
    };
    const int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Build AST structures */
    struct ast_node* ast1 = build_ast(3, 1);
    struct ast_node* ast2 = build_ast(3, 100);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to allocate AST nodes\n");
        return 1;
    }
    
    /* Test goto flow with memory operations */
    process_with_goto(ast1, ast2);
    
    /* Create array for parallel processing */
    struct ast_node* node_array[8];
    node_array[0] = ast1;
    node_array[1] = ast2;
    
    for (int i = 2; i < 8; i++) {
        node_array[i] = build_ast(2, i * 10);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops(node_array, 8);
    
    /* Process tokens with memory operations */
    unsigned long final_hash = process_tokens(tokens, token_count);
    
    /* Additional memory operation sequences */
    char final_buffer[1024];
    volatile size_t dynamic_size = g_mem_size * 2;
    
    /* Chain of memory operations */
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, ast1->data, 64);
    __builtin_memmove(final_buffer + 128, final_buffer, 64);
    __builtin_memset(final_buffer + 192, final_hash & 0xFF, 128);
    
    /* One more memcpy with volatile size */
    __builtin_memcpy(final_buffer + 384, final_buffer, 
                    dynamic_size > 384 ? 384 : dynamic_size);
    
    printf("Final hash: %lu\n", final_hash);
    printf("Buffer[0] = 0x%02x\n", (unsigned char)final_buffer[0]);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        if (node_array[i]) {
            free(node_array[i]);
        }
    }
    
    printf("Test completed successfully\n");
    return 0;
}
