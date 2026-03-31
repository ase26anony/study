/* ISO C99-compliant test program for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token pool with pattern */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)((i * 13) & 0xFF);
    }
    printf("Constructor: Token pool initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using __builtin_memcpy with volatile length */
    int copy_len = (volatile_len % 128) + 64;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->id = depth;
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        char new_data[256];
        __builtin_memset(new_data, 'A' + depth, sizeof(new_data));
        
        /* Jump label for goto */
        create_left:
        node->left = create_ast(depth - 1, new_data);
        
        /* Use goto to jump around */
        if (volatile_flag) {
            goto skip_right;
        }
        
        node->right = create_ast(depth - 1, new_data);
        goto done;
        
        skip_right:
        __builtin_memset(new_data, 'Z' - depth, sizeof(new_data));
        node->right = create_ast(depth - 2, new_data);
        
        done:;
    }
    
    return node;
}

/* Function with __builtin_memmove and goto */
static void manipulate_ast(ASTNode* node) {
    if (!node) return;
    
    char buffer[512];
    
    /* Label for goto into memory operation block */
    start_copy:
    
    /* Use __builtin_memmove with overlapping regions */
    __builtin_memmove(buffer, node->data, 128);
    __builtin_memmove(node->data + 64, buffer, 64);
    
    /* Jump out of block */
    if (node->id % 2 == 0) {
        goto skip_operation;
    }
    
    /* Another memory operation after goto */
    __builtin_memcpy(buffer + 128, node->data, 64);
    
    skip_operation:
    
    /* Process children */
    if (node->left) {
        /* Jump back into operation block */
        if (node->id % 3 == 0) {
            goto start_copy;
        }
        manipulate_ast(node->left);
    }
    
    if (node->right) {
        manipulate_ast(node->right);
    }
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_ops(void) {
    const int num_blocks = 16;
    char* blocks[num_blocks];
    
    /* Allocate memory blocks */
    for (int i = 0; i < num_blocks; i++) {
        blocks[i] = (char*)malloc(1024);
        if (!blocks[i]) continue;
        
        /* Initialize with pattern */
        __builtin_memset(blocks[i], i * 16, 1024);
    }
    
    /* OpenMP parallel region */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < num_blocks; i++) {
            if (blocks[i]) {
                /* Use all three builtins in parallel */
                char temp[256];
                
                /* __builtin_memcpy */
                __builtin_memcpy(temp, blocks[i], 256);
                
                /* __builtin_memset with volatile length */
                int len = (volatile_len + thread_id) % 256;
                __builtin_memset(blocks[i] + 256, thread_id, len);
                
                /* __builtin_memmove with overlap */
                __builtin_memmove(blocks[i] + 128, blocks[i], 128);
                
                /* Copy back */
                __builtin_memcpy(blocks[i], temp, 256);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_blocks; i++) {
        if (blocks[i]) {
            free(blocks[i]);
        }
    }
}

/* Multi-stage initialization */
static void init_stage_1(void) {
    char stage_buffer[1024];
    
    /* Force multiple builtin calls */
    for (int i = 0; i < 10; i++) {
        __builtin_memset(stage_buffer, i, sizeof(stage_buffer));
        __builtin_memcpy(token_pool + i * 128, stage_buffer, 128);
    }
}

static void init_stage_2(void) {
    char temp[512];
    volatile int offset = 0;
    
    /* Complex memory pattern */
    while (offset < 2048) {
        int len = (volatile_len + offset) % 256;
        __builtin_memcpy(temp, token_pool + offset, len);
        __builtin_memset(token_pool + offset + 512, temp[0], len);
        __builtin_memmove(token_pool + offset + 256, token_pool + offset, len);
        offset += len + 1;
    }
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Multi-stage initialization */
    init_stage_1();
    init_stage_2();
    
    /* Create recursive AST */
    ASTNode* root = create_ast(4, "BaseDataForAST");
    
    /* Manipulate AST with goto and memory ops */
    if (root) {
        manipulate_ast(root);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < sizeof(token_pool); i++) {
        hash = (hash * 31 + token_pool[i]) & 0xFFFFFFFF;
    }
    
    printf("Verification hash: 0x%08lX\n", hash);
    
    /* Cleanup */
    /* Note: In real code, you'd want to free the AST recursively */
    
    return 0;
}
