/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char* g_tokens[] = {
    "token1", "token2", "token3", "token4", "token5",
    "token6", "token7", "token8", "token9", "token10"
};
static const int NUM_TOKENS = 10;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Initializing ASAN environment...\n");
    
    /* Force early initialization of memory functions */
    volatile char buffer1[128];
    volatile char buffer2[128];
    
    /* Use builtins in constructor to trigger redirection */
    __builtin_memset((void*)buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy((void*)buffer2, (void*)buffer1, sizeof(buffer1));
    
    printf("Constructor completed\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Cleaning up ASAN environment...\n");
    
    /* Final memory operations in destructor */
    volatile char final_buf[64];
    __builtin_memset((void*)final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* parse_tokens(int start, int end, int depth) {
    if (start >= end || depth > 3) {
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = start * 100 + end;
    
    /* Copy token data using builtin memcpy */
    int token_len = strlen(g_tokens[start]);
    __builtin_memcpy(node->data, g_tokens[start], 
                    token_len < 255 ? token_len : 255);
    
    /* Recursive calls */
    int mid = (start + end) / 2;
    node->left = parse_tokens(start, mid, depth + 1);
    node->right = parse_tokens(mid, end, depth + 1);
    
    return node;
}

/* Function with goto statements for flow control */
static void process_with_goto(ASTNode* node1, ASTNode* node2) {
    if (!node1 || !node2) return;
    
    volatile int use_copy = 1;
    
    /* Jump into memory operation block */
    if (use_copy) {
        goto copy_block;
    }
    
    skip_copy:
    /* Normal processing continues */
    node1->id += 1000;
    return;
    
    copy_block:
    {
        /* Memory operation inside goto block */
        volatile char temp[256];
        
        /* Use builtin memcpy with volatile size */
        __builtin_memcpy((void*)temp, node1->data, g_mem_size % 256);
        
        /* Conditional memmove with goto */
        if (g_use_memmove) {
            goto move_block;
        }
        
        __builtin_memcpy(node2->data, temp, g_mem_size % 256);
        goto skip_copy;
        
        move_block:
        __builtin_memmove(node2->data, temp, g_mem_size % 256);
        goto skip_copy;
    }
}

/* Parallel memory dispatch logic */
static void parallel_memory_operations(ASTNode** nodes, int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < count; i++) {
        if (nodes[i] && nodes[(i + 1) % count]) {
            volatile size_t op_size = (g_mem_size + i) % 128;
            
            /* Mix of memory operations in parallel region */
            if (i % 3 == 0) {
                __builtin_memset(nodes[i]->data, i, op_size);
            } else if (i % 3 == 1) {
                __builtin_memcpy(nodes[i]->data, 
                               nodes[(i + 1) % count]->data, 
                               op_size);
            } else {
                __builtin_memmove(nodes[i]->data, 
                                nodes[(i + 1) % count]->data, 
                                op_size);
            }
        }
    }
}

/* Calculate hash from AST structure */
static unsigned long calculate_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    int i;
    
    /* Simple DJB2 hash over node data */
    for (i = 0; i < 256 && node->data[i]; i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    hash += node->id;
    hash += calculate_ast_hash(node->left);
    hash += calculate_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create AST structures */
    ASTNode* ast1 = parse_tokens(0, NUM_TOKENS / 2, 0);
    ASTNode* ast2 = parse_tokens(NUM_TOKENS / 2, NUM_TOKENS, 0);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST structures\n");
        return 1;
    }
    
    /* Test goto-based memory operations */
    process_with_goto(ast1, ast2);
    
    /* Create array for parallel operations */
    ASTNode* node_array[6];
    node_array[0] = ast1;
    node_array[1] = ast2;
    node_array[2] = parse_tokens(0, 3, 0);
    node_array[3] = parse_tokens(3, 6, 0);
    node_array[4] = parse_tokens(6, 9, 0);
    node_array[5] = parse_tokens(9, NUM_TOKENS, 0);
    
    /* Execute parallel memory operations */
    parallel_memory_operations(node_array, 6);
    
    /* Additional direct builtin calls */
    volatile char final_buffer[512];
    volatile char source_buffer[512];
    
    /* Chain of memory operations */
    __builtin_memset(source_buffer, 0x55, sizeof(source_buffer));
    __builtin_memcpy(final_buffer, source_buffer, sizeof(source_buffer));
    __builtin_memmove(source_buffer, final_buffer, sizeof(final_buffer) / 2);
    
    /* Calculate and print verification result */
    unsigned long total_hash = 0;
    int i;
    for (i = 0; i < 6; i++) {
        if (node_array[i]) {
            total_hash ^= calculate_ast_hash(node_array[i]);
        }
    }
    
    /* Mix in buffer hash */
    for (i = 0; i < 512; i++) {
        total_hash = ((total_hash << 3) ^ final_buffer[i]) + source_buffer[i];
    }
    
    printf("Verification hash: 0x%08lx\n", total_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    for (i = 2; i < 6; i++) {
        free(node_array[i]);
    }
    free(ast1);
    free(ast2);
    
    return 0;
}
