#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Complex AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* volatile to prevent optimization */
    struct ASTNode* left;
    struct ASTNode* right;
    char buffer[256];
} ASTNode;

/* Global token array */
volatile char global_tokens[4096];
volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset((void*)global_tokens, 'A', sizeof(global_tokens));
    
    /* Force symbol initialization early */
    volatile char temp[64];
    __builtin_memcpy((void*)temp, (void*)global_tokens, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Use builtin memmove in destructor */
    volatile char cleanup_buf[128];
    __builtin_memmove((void*)cleanup_buf, (void*)global_tokens, 128);
}

/* Recursive AST parser with goto control flow */
static ASTNode* parse_expression(int depth, volatile int* counter) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = depth;
    node->value = (*counter)++;
    node->size = sizeof(ASTNode) - 256 + (depth * 16);  /* volatile size */
    node->left = NULL;
    node->right = NULL;
    
    /* Initialize buffer with builtin memset */
    __builtin_memset(node->buffer, 0, sizeof(node->buffer));
    
    if (depth > 0) {
        /* Complex goto-based control flow around memmove */
        volatile int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto memmove_block;
        }
        
        node->left = parse_expression(depth - 1, counter);
        
        memmove_block:
        /* This label forces flow sensitivity */
        if (node->left) {
            /* Copy data between nodes using builtin memmove */
            __builtin_memmove(node->buffer, node->left->buffer, 
                            node->left->size % sizeof(node->buffer));
            goto after_memmove;
        }
        
        after_memmove:
        node->right = parse_expression(depth - 1, counter);
        
        if (node->right && node->left) {
            /* Copy between children using builtin memcpy */
            volatile size_t copy_size = node->left->size % 128;
            __builtin_memcpy(node->right->buffer, node->left->buffer, copy_size);
        }
    }
    
    return node;
}

/* Parallel memory dispatch logic */
static long long parallel_memory_ops(ASTNode* root, int iterations) {
    volatile long long total_hash = 0;
    volatile char local_buf[512];
    
    #pragma omp parallel reduction(+:total_hash)
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < iterations; i++) {
            /* Each thread uses builtin memory functions */
            volatile size_t op_size = (i * 17 + thread_id * 23) % 256 + 1;
            
            /* Pattern 1: builtin memset */
            __builtin_memset((void*)local_buf, i + thread_id, op_size);
            
            /* Pattern 2: builtin memcpy from global tokens */
            __builtin_memcpy((void*)local_buf, 
                           (void*)&global_tokens[thread_id * 64], 
                           op_size % 64);
            
            /* Pattern 3: builtin memmove within local buffer */
            volatile size_t move_offset = (i * 7) % 128;
            __builtin_memmove((void*)&local_buf[move_offset], 
                            (void*)local_buf, 
                            op_size % 64);
            
            /* Compute hash from buffer contents */
            volatile long long local_hash = 0;
            for (size_t j = 0; j < op_size && j < sizeof(local_buf); j++) {
                local_hash = local_hash * 31 + local_buf[j];
            }
            total_hash += local_hash;
            
            /* Additional goto-based control flow in parallel region */
            if (i % 13 == 0) {
                goto parallel_mem_op;
            }
            
            continue;
            
            parallel_mem_op:
            /* Extra memmove operation triggered by goto */
            volatile char temp[64];
            __builtin_memmove((void*)temp, (void*)local_buf, 64);
        }
    }
    
    return total_hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    /* Use goto for control flow in cleanup */
    if (node->left) {
        goto free_left;
    }
    
    goto check_right;
    
    free_left:
    free_ast(node->left);
    
    check_right:
    if (node->right) {
        free_ast(node->right);
    }
    
    /* Clear node buffer before free */
    __builtin_memset(node->buffer, 0, sizeof(node->buffer));
    free(node);
}

int main(void) {
    volatile int counter = 0;
    long long final_hash = 0;
    
    printf("Starting ASAN/HWASAN test program...\n");
    
    /* Phase 1: Build recursive AST */
    ASTNode* ast_root = parse_expression(4, &counter);
    
    if (!ast_root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Update global tokens using builtin functions */
    for (volatile int i = 0; i < 32; i++) {
        volatile size_t copy_len = (i * 19) % 256 + 1;
        
        /* Mix of all three builtins */
        if (i % 3 == 0) {
            __builtin_memset((void*)&global_tokens[i * 32], i, copy_len);
        } else if (i % 3 == 1) {
            __builtin_memcpy((void*)&global_tokens[i * 32], 
                           ast_root->buffer, 
                           copy_len % sizeof(ast_root->buffer));
        } else {
            __builtin_memmove((void*)&global_tokens[i * 32], 
                            (void*)&global_tokens[(i-1) * 32], 
                            copy_len);
        }
    }
    
    /* Phase 3: Execute parallel memory operations */
    final_hash = parallel_memory_ops(ast_root, 1000);
    
    /* Phase 4: Final verification with goto control flow */
    volatile char verify_buf[1024];
    volatile int verify_step = 0;
    
    verify_start:
    if (verify_step == 0) {
        __builtin_memset(verify_buf, 0xCC, sizeof(verify_buf));
        verify_step = 1;
        goto verify_start;
    } else if (verify_step == 1) {
        __builtin_memcpy(verify_buf, ast_root->buffer, 
                        sizeof(ast_root->buffer));
        verify_step = 2;
        goto verify_start;
    } else {
        __builtin_memmove(&verify_buf[512], verify_buf, 512);
    }
    
    /* Compute final verification hash */
    volatile long long verify_hash = 0;
    for (size_t i = 0; i < sizeof(verify_buf); i++) {
        verify_hash = verify_hash * 37 + verify_buf[i];
    }
    final_hash ^= verify_hash;
    
    printf("Final hash: %lld\n", final_hash);
    
    /* Cleanup */
    free_ast(ast_root);
    
    /* Final builtin operations in main */
    volatile char exit_buf[256];
    __builtin_memset(exit_buf, 0xFF, sizeof(exit_buf));
    __builtin_memcpy(exit_buf, (void*)global_tokens, 128);
    __builtin_memmove(&exit_buf[128], exit_buf, 128);
    
    return 0;
}
