/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 128;
volatile size_t g_memset_len = 256;
volatile size_t g_memmove_len = 64;

/* Recursive AST-like structure */
struct ast_node {
    int type;
    char *data;
    size_t data_len;
    struct ast_node *left;
    struct ast_node *right;
};

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor for cleanup coordination */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char dummy[8];
    __builtin_memset(dummy, 0xFF, sizeof(dummy));
}

/* Recursive function with memory operations */
static struct ast_node* create_ast(int depth, const char *seed) {
    if (depth <= 0) return NULL;
    
    struct ast_node *node = malloc(sizeof(*node));
    if (!node) return NULL;
    
    node->type = depth;
    node->data_len = (size_t)(depth * 16);
    node->data = malloc(node->data_len);
    
    /* Use builtins with volatile lengths */
    __builtin_memset(node->data, depth, node->data_len);
    
    /* Copy seed data with goto for flow control */
    if (seed) {
        size_t copy_len = node->data_len < strlen(seed) ? node->data_len : strlen(seed);
        
        /* Goto block for testing flow sensitivity */
        goto copy_block;
        
        copy_block:
        __builtin_memcpy(node->data, seed, copy_len);
    }
    
    /* Recursive creation */
    node->left = create_ast(depth - 1, seed);
    node->right = create_ast(depth - 2, seed);
    
    return node;
}

/* Function with goto jumping in/out of memmove blocks */
static void goto_memmove_test(char *dest, char *src, size_t len) {
    int use_memmove = 1;
    
    if (len > 100) {
        goto no_move;
    }
    
    /* Jump into memmove block */
    goto do_move;
    
    do_move:
    __builtin_memmove(dest, src, len);
    goto end;
    
    no_move:
    __builtin_memcpy(dest, src, len);
    
    end:
    return;
}

/* Parallel memory dispatch */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char buffer1[512];
        char buffer2[512];
        
        /* Each thread uses different builtins */
        switch (tid % 3) {
            case 0:
                __builtin_memset(buffer1, tid, g_memset_len);
                break;
            case 1:
                __builtin_memcpy(buffer2, buffer1, g_memcpy_len);
                break;
            case 2:
                __builtin_memmove(buffer1 + 100, buffer1, g_memmove_len);
                break;
        }
        
        /* Barrier to ensure all threads reach here */
        #pragma omp barrier
        
        /* All threads do memmove with goto */
        if (tid % 2) {
            goto_memmove_test(buffer1, buffer2, 128);
        }
    }
}

/* AST traversal with memory operations between nodes */
static int traverse_ast(struct ast_node *node1, struct ast_node *node2) {
    if (!node1 || !node2) return 0;
    
    int sum = node1->type + node2->type;
    
    /* Copy data between AST nodes */
    if (node1->data && node2->data) {
        size_t copy_len = node1->data_len < node2->data_len ? 
                         node1->data_len : node2->data_len;
        
        /* Force builtin usage with volatile length */
        volatile size_t vlen = copy_len;
        __builtin_memcpy(node2->data, node1->data, vlen);
    }
    
    /* Recursive traversal */
    sum += traverse_ast(node1->left, node2->right);
    sum += traverse_ast(node1->right, node2->left);
    
    return sum;
}

int main(void) {
    /* Initialize complex token array */
    char tokens[4][128];
    for (int i = 0; i < 4; i++) {
        __builtin_memset(tokens[i], 'A' + i, sizeof(tokens[i]));
    }
    
    /* Create recursive AST structures */
    struct ast_node *ast1 = create_ast(5, "AST_SEED_1");
    struct ast_node *ast2 = create_ast(4, "AST_SEED_2");
    
    /* Execute parallelized memory dispatch */
    parallel_memory_ops();
    
    /* Perform AST operations */
    int ast_sum = traverse_ast(ast1, ast2);
    
    /* Additional builtin calls in main */
    char final_buffer[1024];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    
    /* Chain of memory operations */
    __builtin_memcpy(final_buffer, tokens[0], 64);
    __builtin_memmove(final_buffer + 128, final_buffer, 64);
    __builtin_memset(final_buffer + 256, 0xCC, 128);
    
    /* Compute verification hash */
    unsigned long hash = 5381;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        hash = ((hash << 5) + hash) + final_buffer[i];
    }
    
    printf("AST traversal sum: %d\n", ast_sum);
    printf("Buffer hash: %lu\n", hash);
    printf("Verification: %s\n", (ast_sum > 0 && hash != 5381) ? "PASS" : "FAIL");
    
    /* Cleanup */
    free(ast1->data); free(ast1);
    free(ast2->data); free(ast2);
    
    return 0;
}
