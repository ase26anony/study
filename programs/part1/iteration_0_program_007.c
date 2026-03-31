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
struct ast_node {
    int type;
    char data[256];
    struct ast_node *left;
    struct ast_node *right;
};

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force builtin usage before main */
    char buf1[256], buf2[256];
    volatile int trigger = 1;
    
    if (trigger) {
        __builtin_memset(buf1, 0xAA, sizeof(buf1));
        __builtin_memcpy(buf2, buf1, sizeof(buf1));
        __builtin_memmove(buf1, buf2, sizeof(buf1));
    }
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    /* Final memory operations */
    char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    struct ast_node *node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Use builtins with volatile lengths */
    __builtin_memset(node, 0, sizeof(*node));
    node->type = depth;
    
    /* Fill data with pattern */
    __builtin_memset(node->data, 'A' + depth, sizeof(node->data) - 1);
    node->data[sizeof(node->data) - 1] = '\0';
    
    /* Recursive construction */
    node->left = build_ast(depth + 1, max_depth);
    node->right = build_ast(depth + 2, max_depth);
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(struct ast_node *src, struct ast_node *dst) {
    volatile int use_goto = 1;
    
    if (use_goto) {
        goto copy_block;
    }
    
    skip_copy:
    return;
    
    copy_block:
    {
        /* This block should be reached via goto */
        size_t copy_len = g_memcpy_len % 256;
        __builtin_memcpy(dst->data, src->data, copy_len);
        
        /* Jump out */
        goto skip_copy;
    }
}

/* OpenMP parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[512];
        char shared_buf[512];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        /* Collective memory operation */
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            char thread_buf[256];
            __builtin_memset(thread_buf, i, sizeof(thread_buf));
            __builtin_memcpy(&shared_buf[i * 128], thread_buf, 128);
        }
        
        /* Memmove with overlap */
        #pragma omp single
        {
            __builtin_memmove(&shared_buf[128], &shared_buf[0], 256);
        }
    }
}

/* Multi-stage processing */
static unsigned long process_ast(struct ast_node *node) {
    if (!node) return 0;
    
    unsigned long hash = 0;
    
    /* Process data with builtins */
    char temp[256];
    size_t len = strlen(node->data);
    __builtin_memcpy(temp, node->data, len);
    
    /* Create hash */
    for (size_t i = 0; i < len; i++) {
        hash = hash * 31 + temp[i];
    }
    
    /* Recursive processing */
    hash += process_ast(node->left);
    hash += process_ast(node->right);
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Build recursive structure */
    struct ast_node *ast1 = build_ast(0, 4);
    struct ast_node *ast2 = build_ast(1, 3);
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Test goto edge cases */
    process_with_goto(ast1, ast2);
    
    /* Force memmove with overlap */
    {
        char overlap_buf[512];
        __builtin_memset(overlap_buf, 0xCC, sizeof(overlap_buf));
        __builtin_memmove(&overlap_buf[100], &overlap_buf[50], 200);
    }
    
    /* OpenMP parallel section */
    parallel_mem_ops();
    
    /* Process structures */
    unsigned long hash1 = process_ast(ast1);
    unsigned long hash2 = process_ast(ast2);
    
    /* Final builtin calls */
    char final_copy[1024];
    __builtin_memset(final_copy, 0, sizeof(final_copy));
    __builtin_memcpy(final_copy, ast1->data, g_memcpy_len % 256);
    __builtin_memmove(&final_copy[512], &final_copy[256], 256);
    
    /* Verify results */
    printf("AST1 hash: %lu\n", hash1);
    printf("AST2 hash: %lu\n", hash2);
    printf("Final buffer first byte: 0x%02X\n", (unsigned char)final_copy[0]);
    
    /* Cleanup */
    /* Note: In real ASAN, memory would be freed here */
    
    return 0;
}
