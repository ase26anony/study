/* asan_coverage.c - Comprehensive test for ASAN memory builtin redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* volatile to prevent optimization */
    struct ASTNode *left;
    struct ASTNode *right;
    char padding[32];      /* Ensure size for memcpy operations */
} ASTNode;

/* Global volatile variables to prevent constant folding */
volatile size_t g_mem_size = 64;
volatile int g_init_value = 0xAA;
volatile int g_use_hwasan = 0;

/* Token array for parser simulation */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "alloc", "free", "process"
};
#define TOKEN_COUNT (sizeof(tokens)/sizeof(tokens[0]))

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    /* Force early initialization of memory functions */
    char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive parser with goto control flow */
static ASTNode* parse_expression(int depth, int* token_idx) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with volatile memset */
    __builtin_memset(node, g_init_value, sizeof(ASTNode));
    
    node->type = (*token_idx) % 3;
    node->value = depth * 10 + (*token_idx);
    node->size = g_mem_size;
    node->left = NULL;
    node->right = NULL;
    
    /* Control flow with goto to test flow sensitivity */
    if (depth > 0) {
        int use_goto = (depth % 2 == 0);
        
        if (use_goto) {
            goto build_left;
        } else {
            /* Normal path */
            if ((*token_idx + 1) < TOKEN_COUNT) {
                (*token_idx)++;
                node->left = parse_expression(depth - 1, token_idx);
            }
            goto build_right;
        }
        
    build_left:
        /* Jump target for goto */
        if ((*token_idx + 2) < TOKEN_COUNT) {
            (*token_idx) += 2;
            node->left = parse_expression(depth - 1, token_idx);
        }
        
    build_right:
        /* Another jump target */
        if ((*token_idx + 1) < TOKEN_COUNT) {
            (*token_idx)++;
            node->right = parse_expression(depth - 1, token_idx);
        }
        
        /* Use memmove with goto jumping around it */
        if (node->left && node->right) {
            char temp[sizeof(ASTNode)];
            __builtin_memcpy(temp, node->left, sizeof(ASTNode));
            
            if (node->left->value > node->right->value) {
                goto do_memmove;
            } else {
                /* Skip memmove */
                goto skip_memmove;
            }
            
        do_memmove:
            /* This should trigger ASAN memmove redirection */
            __builtin_memmove(node->right, temp, sizeof(ASTNode));
            
        skip_memmove:
            /* Continue execution */
            node->value += node->left->value + node->right->value;
        }
    }
    
    return node;
}

/* Process AST with memory operations */
static int process_ast(ASTNode* root, int* sum) {
    if (!root) return 0;
    
    int local_sum = root->value;
    
    /* Create copies using memcpy */
    ASTNode copy;
    __builtin_memcpy(&copy, root, sizeof(ASTNode));
    
    /* Process children */
    local_sum += process_ast(root->left, sum);
    local_sum += process_ast(root->right, sum);
    
    /* Use memset on the copy */
    __builtin_memset(&copy, 0, sizeof(copy.type) + sizeof(copy.value));
    
    *sum += local_sum;
    return local_sum;
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    const int num_blocks = 8;
    char* blocks[num_blocks];
    
    #pragma omp parallel for
    for (int i = 0; i < num_blocks; i++) {
        blocks[i] = malloc(g_mem_size);
        if (blocks[i]) {
            /* Each thread uses builtin memory functions */
            __builtin_memset(blocks[i], i + 1, g_mem_size);
            
            if (i > 0) {
                /* Inter-block copying */
                __builtin_memcpy(blocks[i], blocks[i-1], g_mem_size / 2);
                
                /* Overlapping memmove */
                __builtin_memmove(blocks[i] + 10, blocks[i], g_mem_size - 20);
            }
        }
    }
    
    /* Cleanup */
    #pragma omp parallel for
    for (int i = 0; i < num_blocks; i++) {
        if (blocks[i]) {
            free(blocks[i]);
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN memory builtin redirection test\n");
    
    /* Phase 1: Recursive AST parsing with control flow */
    int token_idx = 0;
    ASTNode* ast = parse_expression(3, &token_idx);
    
    if (!ast) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Process AST with memory operations */
    int ast_sum = 0;
    process_ast(ast, &ast_sum);
    printf("AST processing sum: %d\n", ast_sum);
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_operations();
    
    /* Phase 4: Direct builtin calls with volatile parameters */
    volatile char src_buffer[128];
    volatile char dst_buffer[128];
    
    for (volatile int i = 0; i < 128; i++) {
        src_buffer[i] = (char)(i % 256);
    }
    
    /* Force all three builtins to be called */
    __builtin_memset(dst_buffer, 0xFF, sizeof(dst_buffer));
    __builtin_memcpy(dst_buffer, src_buffer, g_mem_size);
    __builtin_memmove(dst_buffer + 32, dst_buffer, 64);
    
    /* Verify results */
    int verify_sum = 0;
    for (volatile int i = 0; i < 64; i++) {
        verify_sum += dst_buffer[i];
    }
    printf("Verification sum: %d\n", verify_sum);
    
    /* Cleanup */
    free(ast);
    
    printf("Test completed successfully\n");
    return 0;
}
