/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
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
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(token_array, 0xAA, sizeof(token_array));
    printf("Constructor: Token array initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtin memcpy in destructor */
    char temp[16];
    __builtin_memcpy(temp, token_array, sizeof(temp));
    printf("Destructor: Cleanup completed\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtin memset */
    __builtin_memset(node->data, id, sizeof(node->data));
    node->id = id;
    
    /* Create children with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 2 == 0);
        
        if (use_goto) {
            goto create_left;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        
        create_left:
        node->left = create_ast(depth - 1, id * 2 + 1);
        
        node->right = create_ast(depth - 1, id * 3);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with goto jumping into memmove block */
static void goto_memmove_test(char* dest, char* src, int len) {
    int condition = len > 32;
    
    if (condition) {
        goto perform_copy;
    }
    
    /* This block will be jumped into */
    perform_copy:
    /* Force memmove with overlapping regions */
    __builtin_memmove(dest + 10, dest, len - 10);
    
    /* Jump out to different context */
    if (len < 100) {
        goto finish;
    }
    
    /* Another memmove in different context */
    __builtin_memmove(src, dest, 20);
    
    finish:
    return;
}

/* Parallel memory dispatch logic */
static void parallel_memory_ops(void) {
    int i;
    char buffer1[128];
    char buffer2[128];
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memset(buffer2, 0xDD, sizeof(buffer2));
    
    #pragma omp parallel for private(i)
    for (i = 0; i < 8; i++) {
        char local_buf[64];
        int offset = i * 8;
        
        /* Mix of memory operations in parallel region */
        __builtin_memcpy(local_buf, buffer1 + offset, 32);
        __builtin_memset(local_buf + 32, i, 32);
        
        /* Conditional memmove with goto */
        if (i % 3 == 0) {
            goto_memmove_test(buffer2 + offset, local_buf, 40);
        }
        
        /* Copy back with builtin */
        __builtin_memcpy(buffer1 + offset, local_buf, 32);
    }
}

/* Complex initialization with volatile control */
static void init_with_volatile(void) {
    int len = volatile_len;
    
    /* Use volatile variables to control memory ops */
    __builtin_memset((void*)volatile_dest, 0x55, len);
    
    /* Copy between volatile and non-volatile */
    char temp[128];
    __builtin_memcpy(temp, (void*)volatile_dest, len / 2);
    
    /* Move with overlapping */
    __builtin_memmove((void*)(volatile_dest + 16), volatile_dest, len - 16);
    
    /* Update volatile source */
    __builtin_memcpy((void*)volatile_src, temp, sizeof(temp));
}

/* AST manipulation with memory operations */
static void process_ast(ASTNode* root, int* sum) {
    if (!root) return;
    
    char temp[32];
    
    /* Copy node data */
    __builtin_memcpy(temp, root->data, sizeof(temp));
    
    /* Process data with builtin */
    for (int i = 0; i < 32; i++) {
        temp[i] ^= 0xFF;
    }
    
    /* Move processed data back */
    __builtin_memmove(root->data, temp, sizeof(temp));
    
    *sum += root->id;
    
    /* Recursive processing */
    process_ast(root->left, sum);
    process_ast(root->right, sum);
}

/* Main execution flow */
int main(void) {
    int total_sum = 0;
    ASTNode* ast_root = NULL;
    
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Stage 1: Initialize with volatile control */
    printf("Stage 1: Volatile initialization\n");
    init_with_volatile();
    
    /* Stage 2: Create and process recursive AST */
    printf("Stage 2: AST creation and processing\n");
    ast_root = create_ast(4, 1);
    process_ast(ast_root, &total_sum);
    
    /* Stage 3: Parallel memory operations */
    printf("Stage 3: Parallel memory dispatch\n");
    parallel_memory_ops();
    
    /* Stage 4: Complex goto flow with memmove */
    printf("Stage 4: Goto flow control test\n");
    char flow_buffer[256];
    __builtin_memset(flow_buffer, 0, sizeof(flow_buffer));
    goto_memmove_test(flow_buffer, flow_buffer + 128, 150);
    
    /* Stage 5: Token array manipulation */
    printf("Stage 5: Token array operations\n");
    for (int i = 0; i < 16; i++) {
        int offset = i * 64;
        
        /* Mix of all three builtins */
        __builtin_memcpy(token_array + offset, flow_buffer, 32);
        __builtin_memset(token_array + offset + 32, i, 32);
        
        if (i % 2 == 0) {
            __builtin_memmove(token_array + offset + 16, 
                            token_array + offset, 48);
        }
    }
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < sizeof(token_array); i++) {
        hash = (hash * 31) + token_array[i];
    }
    
    printf("\n=== Results ===\n");
    printf("AST Node Sum: %d\n", total_sum);
    printf("Token Hash: 0x%08lx\n", hash);
    printf("Volatile Dest[0]: 0x%02x\n", (unsigned char)volatile_dest[0]);
    printf("Volatile Src[0]: 0x%02x\n", (unsigned char)volatile_src[0]);
    
    /* Cleanup */
    /* Note: Proper AST cleanup omitted for brevity */
    
    return 0;
}
