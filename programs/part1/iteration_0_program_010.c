/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_switch = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
char global_tokens[1024];
int token_hash = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force early initialization of memory builtins */
    char local_buf[32];
    __builtin_memset(local_buf, 0xA5, sizeof(local_buf));
    __builtin_memcpy(global_tokens, local_buf, 32);
    
    printf("Constructor: Initialized global tokens\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Final memory operation in destructor */
    char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    printf("Destructor: Cleanup completed\n");
}

/* Recursive function with memory operations */
ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with builtins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data */
    char pattern[32];
    __builtin_memset(pattern, id % 256, sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(pattern));
    
    node->id = id;
    
    /* Recursive creation with goto for control flow */
    int use_goto = (id % 3 == 0);
    
    if (use_goto) {
        goto create_children;
    }
    
    node->left = create_ast(depth - 1, id * 2);
    
create_children:
    node->right = create_ast(depth - 1, id * 2 + 1);
    
    if (use_goto) {
        node->left = create_ast(depth - 1, id * 2 + 2);
        goto skip_adjust;
    }
    
    /* Additional memory operation after goto */
    char temp[64];
    __builtin_memmove(temp, node->data, volatile_len % 64);
    
skip_adjust:
    return node;
}

/* Copy between AST nodes with volatile control */
void copy_ast_data(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    size_t copy_len = volatile_len % 256;
    
    /* Use all three builtins in different scenarios */
    if (volatile_switch & 1) {
        __builtin_memcpy(dest->data, src->data, copy_len);
    } else if (volatile_switch & 2) {
        __builtin_memmove(dest->data, src->data, copy_len);
    } else {
        __builtin_memset(dest->data, 0xCC, copy_len);
    }
    
    /* Recursive copy */
    if (dest->left && src->left) {
        copy_ast_data(dest->left, src->left);
    }
}

/* OpenMP parallel memory operations */
void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char thread_buf[128];
        
        /* Each thread uses memory builtins */
        __builtin_memset(thread_buf, thread_id, sizeof(thread_buf));
        
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            char local_buf[64];
            
            /* Mix of memory operations */
            if (i % 3 == 0) {
                __builtin_memcpy(local_buf, thread_buf + (i % 64), 32);
            } else if (i % 3 == 1) {
                __builtin_memmove(local_buf, thread_buf + (i % 64), 32);
            } else {
                __builtin_memset(local_buf, i, 32);
            }
            
            #pragma omp atomic
            token_hash += local_buf[0];
        }
        
        /* Barrier with memory operation */
        #pragma omp barrier
        
        /* Final memory operation per thread */
        char final_buf[32];
        __builtin_memcpy(final_buf, thread_buf, 32);
    }
}

/* Function with complex control flow and gotos */
void complex_control_flow(void) {
    char buffer_a[256];
    char buffer_b[256];
    
    /* Initialize buffers */
    __builtin_memset(buffer_a, 0xAA, sizeof(buffer_a));
    __builtin_memset(buffer_b, 0xBB, sizeof(buffer_b));
    
    int counter = 0;
    
start:
    if (counter >= 3) goto finish;
    
    /* Jump into block with memmove */
    if (counter == 1) {
        goto memmove_block;
    }
    
    /* Normal memcpy */
    __builtin_memcpy(buffer_a + 64, buffer_b + 32, 64);
    counter++;
    goto start;
    
memmove_block:
    /* This tests goto into memmove block */
    __builtin_memmove(buffer_b, buffer_a, 128);
    counter++;
    
    /* Jump out of block */
    if (counter == 2) {
        goto memset_block;
    }
    
memset_block:
    __builtin_memset(buffer_a, counter, 96);
    counter++;
    goto start;
    
finish:
    /* Final consolidation */
    __builtin_memmove(global_tokens, buffer_a, 128);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST creation and manipulation */
    ASTNode* root = create_ast(4, 1);
    ASTNode* copy = create_ast(4, 100);
    
    if (root && copy) {
        copy_ast_data(copy, root);
        
        /* Additional memory operations on AST */
        __builtin_memcpy(root->data + 128, copy->data, 64);
        __builtin_memmove(copy->data, root->data, 128);
    }
    
    /* Phase 2: Complex control flow with gotos */
    complex_control_flow();
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Phase 4: Direct builtin calls with volatile lengths */
    char final_buffer[512];
    size_t dynamic_len = volatile_len % 256;
    
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, global_tokens, dynamic_len);
    __builtin_memmove(final_buffer + 256, final_buffer, dynamic_len);
    
    /* Calculate verification hash */
    unsigned int hash = 0;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        hash = (hash * 31) + final_buffer[i];
    }
    
    printf("Result hash: %u\n", hash);
    printf("Token hash from parallel ops: %d\n", token_hash);
    
    /* Cleanup */
    /* Note: In real code, would need to free AST nodes recursively */
    
    return 0;
}
