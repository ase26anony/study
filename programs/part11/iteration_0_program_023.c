/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char g_token_pool[1024];
static volatile int g_token_idx = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(g_token_pool); i++) {
        g_token_pool[i] = (char)((i * 31) & 0xFF);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Test completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with volatile size */
    size_t copy_size = (g_mem_size % 64) + 1;
    __builtin_memcpy(node->data, base_data, copy_size);
    node->size = copy_size;
    
    /* Create children with goto for control flow */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, node->data);
        node->right = NULL;
        
        create_children:
        /* Jump target with __builtin_memmove */
        char temp[64];
        __builtin_memmove(temp, node->data, node->size);
        __builtin_memmove(node->data + 10, temp, node->size - 10);
        
        if (!use_goto) {
            node->right = create_ast(depth - 2, node->data);
        }
    }
    
    return node;
}

/* Function with complex memory operations and goto */
static void process_ast(ASTNode* root, char* output) {
    if (!root) return;
    
    volatile int stage = 0;
    
    stage_1:
    /* First memory operation */
    __builtin_memcpy(output, root->data, root->size);
    output += root->size;
    stage++;
    
    if (root->left) {
        goto stage_2;
    }
    
    stage = 3;
    goto stage_end;
    
    stage_2:
    /* Second memory operation with memmove */
    char buffer[128];
    __builtin_memmove(buffer, root->left->data, root->left->size);
    __builtin_memcpy(output, buffer, root->left->size);
    output += root->left->size;
    stage++;
    
    if (root->right && stage < 3) {
        /* Third memory operation */
        __builtin_memset(output, 0xAA, 32);
        output += 32;
        stage++;
    }
    
    stage_end:
    /* Process children */
    process_ast(root->left, output);
    process_ast(root->right, output);
}

/* Parallel memory dispatch function */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[256];
        char src_buf[256];
        
        /* Initialize source with pattern */
        for (int i = 0; i < 256; i++) {
            src_buf[i] = (char)((i + thread_id * 17) & 0xFF);
        }
        
        /* Force built-in calls in parallel region */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            volatile size_t len = (i % 64) + 32;
            
            /* Mix different memory operations */
            if (i % 3 == 0) {
                __builtin_memcpy(local_buf + i, src_buf, len);
            } else if (i % 3 == 1) {
                __builtin_memset(local_buf + i, i, len);
            } else {
                __builtin_memmove(local_buf + i, local_buf + i - 16, len);
            }
        }
        
        /* Verify with checksum */
        unsigned char checksum = 0;
        for (int i = 0; i < 256; i++) {
            checksum ^= local_buf[i];
        }
        
        #pragma omp critical
        {
            printf("Thread %d checksum: %02X\n", thread_id, checksum);
        }
    }
}

/* Token parser with memory operations */
static int parse_tokens(void) {
    char parse_buffer[512];
    volatile int pos = g_token_idx;
    int token_count = 0;
    
    parse_loop:
    if (pos >= sizeof(g_token_pool) - 64) {
        goto parse_done;
    }
    
    /* Copy token using builtin */
    __builtin_memcpy(parse_buffer + token_count * 16, 
                    g_token_pool + pos, 16);
    
    /* Move position */
    __builtin_memmove(g_token_pool + pos, 
                     g_token_pool + pos + 16, 32);
    
    token_count++;
    pos += 16;
    
    if (token_count < 10) {
        goto parse_loop;
    }
    
    parse_done:
    /* Clear remaining buffer */
    __builtin_memset(parse_buffer + token_count * 16, 0, 
                     sizeof(parse_buffer) - token_count * 16);
    
    return token_count;
}

int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* ast = create_ast(4, "BaseDataForAST");
    if (!ast) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    char ast_output[1024];
    __builtin_memset(ast_output, 0, sizeof(ast_output));
    process_ast(ast, ast_output);
    
    /* Calculate hash of AST output */
    unsigned long ast_hash = 0;
    for (int i = 0; i < 256; i++) {
        ast_hash = (ast_hash * 31) + ast_output[i];
    }
    printf("AST hash: %lu\n", ast_hash);
    
    /* Phase 2: Parallel memory operations */
    printf("\nParallel memory operations:\n");
    parallel_memory_ops();
    
    /* Phase 3: Token parsing */
    printf("\nToken parsing:\n");
    int tokens = parse_tokens();
    printf("Parsed %d tokens\n", tokens);
    
    /* Phase 4: Direct built-in calls with volatile control */
    volatile char* dynamic_buf = (char*)malloc(512);
    if (dynamic_buf) {
        volatile size_t op_size = g_mem_size % 128;
        
        /* Exercise all three builtins */
        __builtin_memset(dynamic_buf, 0xCC, op_size);
        __builtin_memcpy(dynamic_buf + 128, dynamic_buf, op_size / 2);
        __builtin_memmove(dynamic_buf + 256, dynamic_buf + 64, op_size / 4);
        
        /* Verify with sum */
        int sum = 0;
        for (size_t i = 0; i < op_size; i++) {
            sum += dynamic_buf[i];
        }
        printf("Dynamic buffer sum: %d\n", sum);
        
        free(dynamic_buf);
    }
    
    /* Cleanup */
    /* Note: In real code, would need proper AST freeing */
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
