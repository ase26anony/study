/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    struct ASTNode* left;
    struct ASTNode* right;
    char padding[32];  /* Ensure size for meaningful copies */
} ASTNode;

/* Global token array */
static char g_token_array[1024];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token array with pattern */
    for (int i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)(i % 256);
    }
    printf("Constructor: Token array initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    printf("Destructor: Program cleanup complete\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = depth;
    node->value = depth * 10;
    
    /* Use __builtin_memset to initialize padding */
    __builtin_memset(node->padding, depth, sizeof(node->padding));
    
    /* Recursive creation with goto for control flow */
    if (depth % 2 == 0) {
        goto create_left;
    } else {
        goto create_right;
    }
    
create_left:
    node->left = create_ast(depth + 1, max_depth);
    goto after_left;
    
create_right:
    node->right = create_ast(depth + 1, max_depth);
    goto after_right;
    
after_left:
after_right:
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int use_memmove = 1;
    
    if (src && dst) {
        if (use_memmove) {
            goto do_memmove;
        } else {
            goto skip_memmove;
        }
    }
    
do_memmove:
    /* This goto jumps into the memmove operation block */
    {
        size_t len = g_memmove_len;
        if (len > sizeof(ASTNode)) len = sizeof(ASTNode);
        
        /* Force builtin memmove call */
        __builtin_memmove(dst, src, len);
        
        /* Jump out of the block */
        goto after_operation;
    }
    
skip_memmove:
    /* Alternative path */
    dst->type = -1;
    
after_operation:
    return;
}

/* Parallel memory dispatch logic */
static unsigned long parallel_memory_ops(void) {
    unsigned long hash = 0;
    char buffer1[256];
    char buffer2[256];
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memset(buffer2, 0x55, sizeof(buffer2));
    
    #pragma omp parallel reduction(+:hash)
    {
        int thread_id = omp_get_thread_num();
        char local_buf[128];
        
        /* Each thread performs memory operations */
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            /* Mix of memory operations */
            if (i % 3 == 0) {
                /* Use volatile length to prevent optimization */
                size_t len = g_memcpy_len;
                if (len > sizeof(local_buf)) len = sizeof(local_buf);
                
                __builtin_memcpy(local_buf, 
                               &g_token_array[i % sizeof(g_token_array)], 
                               len);
            } else if (i % 3 == 1) {
                size_t len = g_memset_len;
                if (len > sizeof(local_buf)) len = sizeof(local_buf);
                
                __builtin_memset(local_buf, thread_id + i, len);
            } else {
                size_t len = g_memmove_len;
                if (len > sizeof(local_buf)) len = sizeof(local_buf);
                
                __builtin_memmove(local_buf, buffer1, len);
            }
            
            /* Compute hash from buffer contents */
            for (size_t j = 0; j < sizeof(local_buf); j++) {
                hash += (unsigned long)local_buf[j];
            }
        }
        
        /* Additional memory operation in critical section */
        #pragma omp critical
        {
            char crit_buf[64];
            __builtin_memcpy(crit_buf, buffer2, sizeof(crit_buf));
            __builtin_memset(&buffer2[32], thread_id, 32);
        }
    }
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create AST structures */
    ASTNode* ast1 = create_ast(0, 3);
    ASTNode* ast2 = create_ast(0, 3);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Test goto control flow with memmove */
    process_with_goto(ast1, ast2);
    
    /* Perform memory copy between AST nodes */
    size_t copy_len = g_memcpy_len;
    if (copy_len > sizeof(ASTNode)) copy_len = sizeof(ASTNode);
    
    __builtin_memcpy(ast1, ast2, copy_len);
    
    /* Execute parallel memory operations */
    unsigned long result_hash = parallel_memory_ops();
    
    /* Additional builtin usage in main */
    char final_buffer[512];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, g_token_array, 
                    sizeof(g_token_array) < sizeof(final_buffer) ? 
                    sizeof(g_token_array) : sizeof(final_buffer));
    
    /* Use memmove with overlapping regions */
    __builtin_memmove(&final_buffer[128], &final_buffer[64], 256);
    
    /* Compute final verification value */
    unsigned long final_sum = result_hash;
    for (size_t i = 0; i < sizeof(final_buffer); i += 64) {
        final_sum += (unsigned long)final_buffer[i];
    }
    
    printf("Result verification hash: %lu\n", final_sum);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    return 0;
}
