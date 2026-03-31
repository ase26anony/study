/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ast_node {
    int type;
    char data[32];
    struct ast_node *left;
    struct ast_node *right;
} ast_node_t;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Force early initialization of memory builtins */
    char buf1[32], buf2[32];
    __builtin_memset(buf1, 'A', sizeof(buf1));
    __builtin_memcpy(buf2, buf1, sizeof(buf1));
    __builtin_memmove(buf1, buf2, sizeof(buf1));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    /* Final memory operations */
    volatile char final_buf[128];
    __builtin_memset(final_buf, 0, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ast_node_t* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) return NULL;
    
    /* Use all three builtins in varied contexts */
    __builtin_memset(node, 0, sizeof(ast_node_t));
    node->type = depth;
    
    /* Fill data with pattern */
    for (int i = 0; i < sizeof(node->data) - 1; i++) {
        node->data[i] = 'A' + (i % 26);
    }
    node->data[sizeof(node->data) - 1] = '\0';
    
    /* Create children with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 2 == 0);
        
        if (use_goto) {
            /* Jump into block with memmove */
            goto create_left;
        } else {
            node->left = create_ast(depth - 1);
            goto skip_left;
        }
        
    create_left:
        {
            ast_node_t* temp = create_ast(depth - 1);
            /* Use memmove with goto context */
            if (temp) {
                __builtin_memmove(&node->left, &temp, sizeof(temp));
            }
        }
    skip_left:
        node->right = create_ast(depth - 1);
    }
    
    return node;
}

/* Function with complex control flow using goto */
static void process_with_goto(ast_node_t* node, char* output) {
    if (!node) return;
    
    int i = 0;
    
    /* Jump into memory operation block */
    if (node->type % 3 == 0) {
        goto do_memcpy;
    } else if (node->type % 3 == 1) {
        goto do_memset;
    } else {
        goto do_memmove;
    }
    
do_memcpy:
    {
        char temp[32];
        __builtin_memcpy(temp, node->data, sizeof(node->data));
        __builtin_memcpy(output + i, temp, sizeof(node->data));
        i += sizeof(node->data);
        goto next;
    }
    
do_memset:
    {
        char temp[32];
        __builtin_memset(temp, node->type, sizeof(temp));
        __builtin_memcpy(output + i, temp, sizeof(temp));
        i += sizeof(temp);
        goto next;
    }
    
do_memmove:
    {
        char buffer[64];
        __builtin_memset(buffer, 0, sizeof(buffer));
        __builtin_memcpy(buffer, node->data, sizeof(node->data));
        __builtin_memmove(output + i, buffer + 16, sizeof(node->data));
        i += sizeof(node->data);
        /* Jump out of block */
        goto next;
    }
    
next:
    /* Process children */
    process_with_goto(node->left, output + i);
    process_with_goto(node->right, output + i);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char local_buf[128];
        char shared_buf[128];
        
        /* Each thread uses different builtins */
        switch (thread_id % 3) {
            case 0:
                __builtin_memset(local_buf, thread_id, sizeof(local_buf));
                __builtin_memcpy(shared_buf, local_buf, sizeof(local_buf));
                break;
            case 1:
                __builtin_memcpy(local_buf, volatile_src, volatile_len);
                __builtin_memmove(shared_buf, local_buf, volatile_len);
                break;
            case 2:
                __builtin_memset(shared_buf, 0xFF, sizeof(shared_buf));
                __builtin_memcpy(local_buf, shared_buf, sizeof(shared_buf));
                __builtin_memmove(volatile_dest, local_buf, volatile_len);
                break;
        }
        
        /* Barrier to ensure all threads complete */
        #pragma omp barrier
        
        /* Combined operation */
        #pragma omp single
        {
            char combined[256];
            __builtin_memset(combined, 0, sizeof(combined));
            __builtin_memcpy(combined, shared_buf, sizeof(shared_buf));
            __builtin_memmove(volatile_dest, combined, volatile_len);
        }
    }
}

/* Multi-stage initialization */
static void initialize_data(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < sizeof(volatile_src); i++) {
        volatile_src[i] = (char)(i % 256);
    }
    
    /* Use builtins with volatile lengths */
    __builtin_memset(volatile_dest, 0, sizeof(volatile_dest));
    __builtin_memcpy((char*)volatile_dest, (char*)volatile_src, volatile_len);
    __builtin_memmove((char*)volatile_dest + 32, (char*)volatile_src, volatile_len / 2);
}

/* Main execution flow */
int main(void) {
    /* Initialize complex data */
    initialize_data();
    
    /* Create recursive AST */
    ast_node_t* root = create_ast(4);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process AST with goto control flow */
    char output_buffer[1024] = {0};
    process_with_goto(root, output_buffer);
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Additional memory operations in main */
    char main_buf[512];
    char* ptr1 = main_buf;
    char* ptr2 = main_buf + 256;
    
    /* Chain of builtin calls */
    __builtin_memset(ptr1, 0xAA, 128);
    __builtin_memcpy(ptr2, ptr1, 128);
    __builtin_memmove(ptr1 + 64, ptr2, 64);
    __builtin_memset(ptr2 + 64, 0xBB, 64);
    __builtin_memcpy(ptr1, ptr2 + 32, 96);
    __builtin_memmove(ptr2, ptr1, 128);
    
    /* Calculate verification hash */
    unsigned long hash = 0;
    for (int i = 0; i < sizeof(output_buffer); i++) {
        hash = hash * 31 + (unsigned char)output_buffer[i];
    }
    
    /* Also hash the volatile buffers */
    for (int i = 0; i < volatile_len; i++) {
        hash = hash * 17 + (unsigned char)volatile_dest[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: In real ASAN, this would detect leaks */
    
    return 0;
}
