/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_array[1024];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(token_array, 0xAA, sizeof(token_array));
    printf("Constructor: Token array initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_destructor(void) {
    /* Verify with builtin memcmp */
    char check[1024];
    __builtin_memset(check, 0xAA, sizeof(check));
    int result = __builtin_memcmp(token_array, check, sizeof(token_array));
    printf("Destructor: Memory verification %s\n", 
           result == 0 ? "PASSED" : "FAILED");
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Fill data with pattern using builtin memcpy */
    char pattern[32];
    for (int i = 0; i < 32; i++) pattern[i] = (char)(id + i);
    __builtin_memcpy(node->data, pattern, 32);
    
    /* Recursive creation with goto for flow control */
    int create_right = 0;
    
create_left:
    node->left = create_ast(depth - 1, id * 2);
    
    if (!create_right) {
        create_right = 1;
        goto create_right;  /* Jump to force flow sensitivity */
    }
    
create_right:
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    return node;
}

/* Copy AST data with builtin memmove (handles overlap) */
static void copy_ast_data(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use volatile length to prevent folding */
    int len = volatile_len % 32;
    
    /* Direct builtin memcpy */
    __builtin_memcpy(dest->data, src->data, len);
    
    /* Overlapping copy with builtin memmove */
    if (len > 16) {
        __builtin_memmove(dest->data + 8, dest->data, 16);
    }
    
    /* Recursive copy */
    copy_ast_data(dest->left, src->left);
    copy_ast_data(dest->right, src->right);
}

/* Process tokens with memory operations */
static void process_tokens(void) {
    char buffer[512];
    char temp[512];
    
    /* Initialize with builtin memset */
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memset(temp, 0, sizeof(temp));
    
    /* Copy tokens with builtin memcpy */
    int copy_len = volatile_len % 256;
    __builtin_memcpy(buffer, token_array + token_index, copy_len);
    
    /* Process with goto jumps around memory operations */
    int stage = 0;
    
stage1:
    /* Builtin memmove with overlap */
    __builtin_memmove(buffer + 128, buffer + 64, 64);
    if (stage == 0) {
        stage = 1;
        goto stage2;  /* Jump over next operation */
    }
    
    __builtin_memset(buffer + 192, 0xFF, 32);  /* This gets skipped first time */
    
stage2:
    /* Reverse copy with builtin memcpy */
    __builtin_memcpy(temp, buffer, 256);
    
    if (stage == 1) {
        stage = 2;
        goto stage1;  /* Jump back */
    }
    
    /* Final builtin memmove */
    __builtin_memmove(token_array, temp, 256);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    int i;
    char local_buf[4][128];
    
    #pragma omp parallel private(i)
    {
        int thread_id = omp_get_thread_num();
        
        /* Each thread uses builtin memory functions */
        #pragma omp for
        for (i = 0; i < 4; i++) {
            /* Initialize with builtin memset */
            __builtin_memset(local_buf[i], thread_id + i, 128);
            
            /* Copy with builtin memcpy */
            __builtin_memcpy(local_buf[i] + 64, token_array + i * 64, 64);
            
            /* Move with builtin memmove (potential overlap) */
            __builtin_memmove(local_buf[i] + 32, local_buf[i], 64);
        }
        
        /* Critical section for final builtin memset */
        #pragma omp critical
        {
            __builtin_memset(volatile_dest, thread_id, volatile_len % 128);
        }
    }
    
    /* Combine results with builtin memcpy */
    for (i = 0; i < 4; i++) {
        __builtin_memcpy(token_array + i * 128, local_buf[i], 128);
    }
}

/* Main execution flow */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: AST operations */
    printf("Phase 1: Creating AST structures...\n");
    ASTNode* ast1 = create_ast(3, 1);
    ASTNode* ast2 = create_ast(3, 100);
    
    if (ast1 && ast2) {
        /* Copy between ASTs using builtin functions */
        copy_ast_data(ast2, ast1);
        
        /* Verify copy with builtin memcmp */
        int cmp = __builtin_memcmp(ast1->data, ast2->data, 32);
        printf("AST data comparison: %s\n", cmp == 0 ? "MATCH" : "DIFFERENT");
        
        free(ast1);
        free(ast2);
    }
    
    /* Phase 2: Token processing with goto flow */
    printf("Phase 2: Processing tokens with goto flow...\n");
    process_tokens();
    
    /* Phase 3: OpenMP parallel operations */
    printf("Phase 3: Parallel memory operations...\n");
    parallel_memory_ops();
    
    /* Phase 4: Volatile memory operations */
    printf("Phase 4: Volatile memory operations...\n");
    
    /* Initialize volatile source */
    for (int i = 0; i < 256; i++) {
        volatile_src[i] = (char)(i % 256);
    }
    
    /* Chain of builtin operations with volatile control */
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, volatile_len);
    
    /* Overwrite middle section */
    __builtin_memset((void*)(volatile_dest + 64), 0xCC, volatile_len / 2);
    
    /* Move overlapping regions */
    __builtin_memmove((void*)(volatile_dest + 32), (void*)volatile_dest, 96);
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < 256; i++) {
        hash = (hash * 31) + (unsigned char)volatile_dest[i];
    }
    
    printf("Final hash: 0x%08lx\n", hash);
    printf("=== Test Complete ===\n");
    
    return 0;
}
