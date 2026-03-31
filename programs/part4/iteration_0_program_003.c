#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Complex AST-like structure for data access patterns */
typedef struct ASTNode {
    int type;
    int value;
    volatile int flags;
    struct ASTNode *left;
    struct ASTNode *right;
    char padding[32];  /* Ensure size for memcpy operations */
} ASTNode;

/* Global volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_init_value = 0x42;
volatile int g_use_hwasan = 0;

/* Token array for parser simulation */
typedef struct {
    int token_type;
    char data[64];
    volatile int processed;
} Token;

Token g_tokens[100];
volatile int g_token_count = 50;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Initializing ASAN environment...\n");
    
    /* Force initialization of memory builtins in constructor context */
    char buffer1[128];
    char buffer2[128];
    
    /* Use all three builtins in constructor */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    
    volatile int use_memmove = 1;
    if (use_memmove) {
        __builtin_memmove(buffer1 + 10, buffer1, 50);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Cleaning up ASAN environment...\n");
    
    /* Additional builtin usage in destructor */
    char final_buffer[64];
    __builtin_memset(final_buffer, 0xFF, sizeof(final_buffer));
}

/* Recursive AST manipulation with memory operations */
ASTNode* create_ast_node(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = depth * 10;
    node->flags = g_init_value;
    
    /* Recursive creation with goto for flow control */
    if (depth < max_depth - 1) {
        node->left = create_ast_node(depth + 1, max_depth);
        
        /* Jump label for goto testing */
        create_right:
        node->right = create_ast_node(depth + 2, max_depth);
    }
    
    return node;
}

/* Function with goto jumping into memmove block */
void process_ast_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_memmove = 0;
    
    /* Goto jumping into memmove block */
    if (src->flags & 0x1) {
        goto do_memmove;
    }
    
    /* Normal memcpy path */
    __builtin_memcpy(dst, src, sizeof(ASTNode));
    return;
    
do_memmove:
    /* This block is entered via goto */
    volatile size_t move_size = sizeof(ASTNode) - 8;
    __builtin_memmove((char*)dst + 4, (char*)src + 4, move_size);
    
    /* Jump back out */
    goto finish;
    
skip_operation:
    /* Alternative path */
    dst->value = src->value;
    
finish:
    return;
}

/* OpenMP parallel memory operations */
void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[256];
        char shared_buf[256];
        
        /* Each thread uses memory builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Conditional memcpy based on thread ID */
        if (thread_id % 2 == 0) {
            volatile size_t copy_len = g_mem_size / 2;
            __builtin_memcpy(shared_buf, local_buf, copy_len);
        } else {
            /* Use memmove for overlapping regions */
            __builtin_memmove(local_buf + 32, local_buf, 64);
        }
        
        #pragma omp barrier
        
        /* Final memset in parallel region */
        __builtin_memset(local_buf + 128, 0xCC, 64);
    }
}

/* Complex parser with varied memory operations */
int parse_tokens_with_memory_ops(void) {
    int result_hash = 0;
    Token local_tokens[20];
    
    /* Initialize with volatile-controlled size */
    volatile int init_size = 20 * sizeof(Token);
    __builtin_memset(local_tokens, 0, init_size);
    
    for (volatile int i = 0; i < g_token_count && i < 20; i++) {
        /* Copy from global to local with builtin */
        __builtin_memcpy(&local_tokens[i], &g_tokens[i], sizeof(Token));
        
        /* Process token with goto for flow control */
        if (local_tokens[i].token_type == 1) {
            goto special_process;
        }
        
        normal_process:
        result_hash += local_tokens[i].data[0];
        continue;
        
        special_process:
        /* Use memmove for overlapping copy */
        __builtin_memmove(
            local_tokens[i].data + 10,
            local_tokens[i].data,
            30
        );
        result_hash += 0x100;
        goto normal_process;
    }
    
    /* Final memory operation on aggregated data */
    char final_buffer[256];
    __builtin_memcpy(final_buffer, local_tokens, sizeof(local_tokens));
    __builtin_memset(final_buffer + 200, 0xEE, 56);
    
    return result_hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Initialize token array with varied patterns */
    for (int i = 0; i < 100; i++) {
        g_tokens[i].token_type = i % 3;
        g_tokens[i].processed = 0;
        __builtin_memset(g_tokens[i].data, i, sizeof(g_tokens[i].data));
    }
    
    /* Create AST structures */
    ASTNode* ast1 = create_ast_node(0, 4);
    ASTNode* ast2 = create_ast_node(0, 3);
    
    if (!ast1 || !ast2) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Process AST with goto-based control flow */
    process_ast_with_goto(ast1, ast2);
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Parse tokens with complex memory patterns */
    int final_hash = parse_tokens_with_memory_ops();
    
    /* Additional direct builtin calls in main */
    char verification_buf[512];
    volatile size_t verify_size = 256;
    
    __builtin_memset(verification_buf, 0x55, verify_size);
    __builtin_memcpy(verification_buf + 128, verification_buf, 128);
    __builtin_memmove(verification_buf + 64, verification_buf + 32, 160);
    
    /* Calculate verification sum */
    int verify_sum = 0;
    for (int i = 0; i < 256; i++) {
        verify_sum += verification_buf[i];
    }
    
    final_hash += verify_sum;
    
    printf("Final result hash: 0x%08X\n", final_hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    return 0;
}
