#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* volatile to prevent optimization */
    struct ASTNode* left;
    struct ASTNode* right;
    char padding[32];      /* Ensure size for memcpy operations */
} ASTNode;

/* Global volatile variables to prevent constant folding */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Force initialization of ASAN runtime */
    volatile char buffer[128];
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("Constructor: ASAN initialization triggered\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    volatile char cleanup_buf[64];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    printf("Destructor: ASAN cleanup triggered\n");
}

/* Recursive function that builds and manipulates AST nodes */
static ASTNode* build_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = (*counter)++;
    node->size = g_mem_size;  /* volatile size */
    
    /* Recursive construction */
    node->left = build_ast(depth - 1, counter);
    node->right = build_ast(depth - 1, counter);
    
    return node;
}

/* Function with goto statements for control flow testing */
static void test_goto_memcpy(ASTNode* src, ASTNode* dst) {
    volatile int use_copy = 1;
    
    if (use_copy) {
        goto copy_block;
    } else {
        goto skip_copy;
    }
    
copy_block:
    /* This block contains __builtin_memcpy with goto entry */
    __builtin_memcpy(dst, src, sizeof(ASTNode));
    goto after_copy;
    
skip_copy:
    /* Alternative path */
    __builtin_memset(dst, 0xFF, sizeof(ASTNode));
    
after_copy:
    /* Jump back into block with memory operation */
    if (g_use_memmove) {
        goto move_block;
    }
    
    return;
    
move_block:
    /* Test __builtin_memmove with goto */
    ASTNode temp;
    __builtin_memmove(&temp, dst, sizeof(ASTNode));
    __builtin_memmove(dst, &temp, sizeof(ASTNode));
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_operations(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i)
    for (i = 0; i < count; i++) {
        volatile char local_buf[256];
        
        /* Force all three builtins in parallel region */
        __builtin_memset(local_buf, i, sizeof(local_buf));
        
        if (nodes[i] && nodes[(i + 1) % count]) {
            __builtin_memcpy(nodes[i], nodes[(i + 1) % count], sizeof(ASTNode));
        }
        
        /* Conditional memmove based on volatile */
        if (g_use_memmove && i % 2 == 0) {
            ASTNode temp_node;
            __builtin_memmove(&temp_node, nodes[i], sizeof(ASTNode));
            __builtin_memmove(nodes[i], &temp_node, sizeof(ASTNode));
        }
    }
}

/* Complex token processing with memory operations */
static uint32_t process_tokens(const char* tokens, size_t token_count) {
    uint32_t hash = 0xDEADBEEF;
    volatile size_t chunk_size = 16;  /* Prevent constant folding */
    
    for (size_t i = 0; i < token_count; i += chunk_size) {
        char buffer[64];
        size_t remaining = token_count - i;
        size_t copy_size = (remaining < chunk_size) ? remaining : chunk_size;
        
        /* Use all three builtins in sequence */
        __builtin_memset(buffer, 0, sizeof(buffer));
        __builtin_memcpy(buffer, tokens + i, copy_size);
        
        if (i > 0 && copy_size > 8) {
            __builtin_memmove(buffer + 8, buffer, copy_size - 8);
        }
        
        /* Compute hash from processed buffer */
        for (size_t j = 0; j < copy_size; j++) {
            hash = (hash << 5) + hash + buffer[j];
        }
    }
    
    return hash;
}

int main(void) {
    const int NUM_NODES = 8;
    const int AST_DEPTH = 3;
    int counter = 0;
    
    printf("Starting ASAN builtin redirection test...\n");
    
    /* 1. Build recursive AST structures */
    ASTNode* nodes[NUM_NODES];
    for (int i = 0; i < NUM_NODES; i++) {
        nodes[i] = build_ast(AST_DEPTH, &counter);
        if (!nodes[i]) {
            fprintf(stderr, "Failed to allocate node %d\n", i);
            return 1;
        }
    }
    
    /* 2. Test goto control flow with memory operations */
    for (int i = 0; i < NUM_NODES - 1; i++) {
        test_goto_memcpy(nodes[i], nodes[i + 1]);
    }
    
    /* 3. Execute parallel memory operations */
    parallel_memory_operations(nodes, NUM_NODES);
    
    /* 4. Process complex token array */
    const char* token_array = "MEMCPY_MEMSET_MEMMOVE_ASAN_HWASAN_BUILTIN_REDIRECTION";
    size_t token_len = strlen(token_array);
    uint32_t final_hash = process_tokens(token_array, token_len);
    
    /* 5. Additional direct builtin calls in main */
    volatile char final_buffer[128];
    __builtin_memset(final_buffer, 0xAA, sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 32, token_array, token_len);
    __builtin_memmove(final_buffer, final_buffer + 16, 64);
    
    /* Verify and print results */
    printf("AST nodes created: %d\n", counter);
    printf("Token processing hash: 0x%08X\n", final_hash);
    printf("Final buffer first byte: 0x%02X\n", (unsigned char)final_buffer[0]);
    
    /* Cleanup */
    for (int i = 0; i < NUM_NODES; i++) {
        free(nodes[i]);
    }
    
    printf("Test completed successfully.\n");
    return 0;
}
