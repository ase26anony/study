/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_memory_pool(void) {
    /* Force initialization of ASAN structures */
    char buffer[32];
    volatile char* volatile_ptr = buffer;
    
    /* Use all three builtins in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 8);
    __builtin_memmove(buffer + 8, buffer, 8);
}

/* Destructor to test cleanup paths */
__attribute__((destructor))
static void cleanup_memory_pool(void) {
    /* Additional builtin usage in destructor */
    char local_buf[16];
    __builtin_memset(local_buf, 0xFF, sizeof(local_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Complex memory initialization with volatile control */
    size_t copy_size = g_mem_size % 128;
    if (copy_size > 256) copy_size = 256;
    
    /* Use all three builtins with goto for flow control */
    volatile int use_memmove = (depth % 3 == 0);
    
    if (use_memmove) {
        /* Jump into memory operation block */
        goto memmove_block;
    } else {
        /* Direct path */
        __builtin_memset(node->data, depth, sizeof(node->data));
    }
    
    /* Copy base data with memcpy */
    size_t base_len = strlen(base_data);
    if (base_len > 0) {
        __builtin_memcpy(node->data, base_data, 
                        base_len < sizeof(node->data) ? base_len : sizeof(node->data));
    }
    
    node->size = copy_size;
    
memmove_block:
    /* Label for goto target with memmove operation */
    if (use_memmove) {
        char temp[256];
        __builtin_memcpy(temp, node->data, sizeof(temp));
        __builtin_memmove(node->data + 32, temp, 64);
        /* Jump out of the block */
        goto after_memmove;
    }
    
after_memmove:
    /* Recursive creation with different memory patterns */
    node->left = create_ast(depth - 1, "left_branch");
    node->right = create_ast(depth - 1, "right_branch");
    
    return node;
}

/* Function with OpenMP parallel section */
static void parallel_memory_operations(ASTNode* nodes[], int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            /* Mixed builtin usage in parallel region */
            char buffer[512];
            volatile size_t op_size = g_mem_size;
            
            /* Pattern 1: memset then memcpy */
            __builtin_memset(buffer, i, op_size % sizeof(buffer));
            __builtin_memcpy(nodes[i]->data, buffer, 
                           op_size % sizeof(nodes[i]->data));
            
            /* Pattern 2: memmove within same buffer */
            if (i % 2 == 0) {
                __builtin_memmove(buffer + 128, buffer, 64);
            }
            
            /* Pattern 3: Chain operations */
            char temp[128];
            __builtin_memset(temp, 0xCC, sizeof(temp));
            __builtin_memcpy(buffer, temp, 64);
            __builtin_memmove(nodes[i]->data + 64, buffer, 32);
        }
    }
}

/* Complex token processing with goto */
static size_t process_tokens(char tokens[][64], int token_count) {
    size_t hash = 0;
    int i = 0;
    
process_loop:
    if (i >= token_count) goto done;
    
    volatile int operation = i % 4;
    
    switch (operation) {
        case 0:
            /* memset pattern */
            __builtin_memset(tokens[i], 'A' + i, 32);
            goto next_token;
            
        case 1:
            /* memcpy pattern with goto into block */
            if (i > 0) {
                goto copy_block;
            }
            break;
            
        case 2:
            /* memmove pattern */
            __builtin_memmove(tokens[i] + 16, tokens[i], 16);
            goto next_token;
            
        case 3:
            /* Combined operations */
            __builtin_memset(tokens[i], 0, 64);
            __builtin_memcpy(tokens[i], "DEFAULT", 8);
            goto next_token;
    }
    
    goto next_token;
    
copy_block:
    /* Target for goto from case 1 */
    __builtin_memcpy(tokens[i], tokens[i-1], 32);
    /* Fall through */
    
next_token:
    /* Calculate hash from token data */
    for (int j = 0; j < 32; j++) {
        hash += (size_t)tokens[i][j];
    }
    i++;
    goto process_loop;
    
done:
    return hash;
}

int main(void) {
    const int NUM_NODES = 8;
    const int TOKEN_COUNT = 16;
    const int AST_DEPTH = 4;
    
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Phase 1: Initialize complex token array */
    char tokens[TOKEN_COUNT][64];
    
    /* Fill tokens with pattern using builtins */
    for (int i = 0; i < TOKEN_COUNT; i++) {
        __builtin_memset(tokens[i], '0' + i, sizeof(tokens[i]));
        if (i > 0) {
            __builtin_memcpy(tokens[i] + 32, tokens[i-1], 16);
        }
    }
    
    /* Phase 2: Process tokens with goto flow */
    size_t token_hash = process_tokens(tokens, TOKEN_COUNT);
    printf("Token hash: %zu\n", token_hash);
    
    /* Phase 3: Create recursive AST structures */
    ASTNode* nodes[NUM_NODES];
    for (int i = 0; i < NUM_NODES; i++) {
        char base[32];
        __builtin_memset(base, 'A' + i, sizeof(base));
        base[31] = '\0';
        
        nodes[i] = create_ast(AST_DEPTH, base);
        
        /* Additional memory operations on nodes */
        if (nodes[i] && i > 0) {
            __builtin_memcpy(nodes[i]->data, nodes[i-1]->data, 128);
            __builtin_memmove(nodes[i]->data + 64, nodes[i]->data, 32);
        }
    }
    
    /* Phase 4: Execute parallelized memory operations */
    #ifdef _OPENMP
    printf("Running OpenMP parallel section...\n");
    #endif
    
    parallel_memory_operations(nodes, NUM_NODES);
    
    /* Phase 5: Verify and cleanup */
    size_t final_sum = token_hash;
    
    for (int i = 0; i < NUM_NODES; i++) {
        if (nodes[i]) {
            /* Final memory operation before free */
            char verify_buf[256];
            __builtin_memset(verify_buf, 0, sizeof(verify_buf));
            __builtin_memcpy(verify_buf, nodes[i]->data, 
                           sizeof(verify_buf) < sizeof(nodes[i]->data) ? 
                           sizeof(verify_buf) : sizeof(nodes[i]->data));
            
            /* Add to verification sum */
            for (int j = 0; j < 64; j++) {
                final_sum += (size_t)verify_buf[j];
            }
            
            /* Recursive free */
            free(nodes[i]);
        }
    }
    
    printf("Final verification sum: %zu\n", final_sum);
    printf("Test completed successfully.\n");
    
    return 0;
}
