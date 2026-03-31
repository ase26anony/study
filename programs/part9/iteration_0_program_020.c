/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Global token array */
static char global_tokens[1024];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    
    /* Force symbol initialization */
    char local_buf[128];
    __builtin_memset(local_buf, 0, sizeof(local_buf));
    __builtin_memcpy(local_buf, "constructor_init", 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use all three builtins in destructor */
    char cleanup_buf[256];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    __builtin_memcpy(cleanup_buf, global_tokens, 128);
    __builtin_memmove(cleanup_buf + 64, cleanup_buf, 64);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with pattern */
    for (int i = 0; i < (int)sizeof(node->data) - 1; i++) {
        node->data[i] = 'A' + (i % 26);
    }
    node->size = sizeof(node->data);
    
    /* Recursive creation with goto for flow control */
    int create_children = volatile_flag;
    
    if (create_children) {
        node->left = create_ast(depth - 1);
        
        /* Jump label for goto testing */
        create_right:
        node->right = create_ast(depth - 1);
        
        /* Copy data between nodes if both exist */
        if (node->left && node->right) {
            size_t copy_len = volatile_len % 128;
            __builtin_memcpy(node->right->data, node->left->data, copy_len);
            
            /* Use memmove with overlapping regions */
            __builtin_memmove(node->left->data + 32, node->left->data, 96);
        }
    } else {
        /* Alternative path with goto */
        goto skip_creation;
    }
    
    return node;
    
skip_creation:
    /* Different memory operation sequence */
    char temp[128];
    __builtin_memset(temp, depth, sizeof(temp));
    __builtin_memcpy(node->data, temp, 64);
    return node;
}

/* Function with goto jumping into memory block */
static void goto_memory_operations(void) {
    char buffer1[256];
    char buffer2[256];
    
    /* Initial setup */
    __builtin_memset(buffer1, 'X', sizeof(buffer1));
    
    /* Jump into the middle of operations */
    goto middle_block;
    
start_block:
    __builtin_memcpy(buffer2, buffer1, 128);
    return;
    
middle_block:
    /* This tests flow sensitivity */
    __builtin_memmove(buffer1 + 64, buffer1, 128);
    goto start_block;
}

/* Parallel memory dispatch logic */
static void parallel_memory_dispatch(void) {
    const int num_workers = 4;
    char worker_buffers[num_workers][256];
    volatile int results[num_workers];
    
    #pragma omp parallel num_threads(num_workers)
    {
        int tid = omp_get_thread_num();
        
        /* Each thread uses different builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(worker_buffers[tid], tid, sizeof(worker_buffers[tid]));
                break;
            case 1:
                __builtin_memcpy(worker_buffers[tid], global_tokens, 
                                volatile_len % sizeof(worker_buffers[tid]));
                break;
            case 2:
                __builtin_memmove(worker_buffers[tid] + 32, 
                                 worker_buffers[tid], 128);
                break;
        }
        
        /* Compute simple hash */
        int hash = 0;
        for (int i = 0; i < 256; i++) {
            hash += worker_buffers[tid][i];
        }
        results[tid] = hash;
    }
    
    /* Verify parallel execution */
    int total = 0;
    for (int i = 0; i < num_workers; i++) {
        total += results[i];
    }
    printf("Parallel hash sum: %d\n", total);
}

/* Multi-stage initialization */
static void multi_stage_init(void) {
    /* Stage 1: Direct builtin calls */
    char stage1[512];
    __builtin_memset(stage1, 0, sizeof(stage1));
    
    /* Stage 2: Indirect through volatile */
    volatile char* volatile_ptr = stage1;
    size_t len = volatile_len % 256;
    __builtin_memcpy(stage1 + 128, volatile_ptr, len);
    
    /* Stage 3: Overlapping regions */
    __builtin_memmove(stage1 + 64, stage1 + 32, 192);
    
    /* Stage 4: Nested operations */
    for (int i = 0; i < 4; i++) {
        char nested[64];
        __builtin_memset(nested, i, sizeof(nested));
        __builtin_memcpy(stage1 + (i * 64), nested, sizeof(nested));
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN builtin redirection test\n");
    
    /* 1. Initialize complex token array */
    for (int i = 0; i < (int)sizeof(global_tokens); i++) {
        global_tokens[i] = (char)((i * 31) & 0xFF);
    }
    
    /* 2. Invoke recursive parser */
    ASTNode* root = create_ast(3);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* 3. Execute goto-based memory operations */
    goto_memory_operations();
    
    /* 4. Execute parallelized memory dispatch */
    parallel_memory_dispatch();
    
    /* 5. Multi-stage interaction */
    multi_stage_init();
    
    /* 6. Compute verification result */
    unsigned long long total_hash = 0;
    
    /* Hash global tokens */
    for (int i = 0; i < 1024; i++) {
        total_hash += (unsigned long long)global_tokens[i] * (i + 1);
    }
    
    /* Hash AST data recursively */
    ASTNode* stack[16];
    int stack_ptr = 0;
    stack[stack_ptr++] = root;
    
    while (stack_ptr > 0) {
        ASTNode* node = stack[--stack_ptr];
        
        for (int i = 0; i < 256; i++) {
            total_hash += (unsigned long long)node->data[i] * (i + 257);
        }
        
        if (node->right) stack[stack_ptr++] = node->right;
        if (node->left) stack[stack_ptr++] = node->left;
        
        /* Cleanup with builtin */
        __builtin_memset(node->data, 0, sizeof(node->data));
    }
    
    /* Free AST memory */
    free(root);
    
    printf("Final verification hash: %llu\n", total_hash);
    printf("Test completed successfully\n");
    
    return 0;
}
