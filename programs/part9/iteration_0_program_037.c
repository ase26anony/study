/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
struct ast_node {
    char *data;
    size_t size;
    struct ast_node *left;
    struct ast_node *right;
    int id;
};

/* Constructor attribute for early initialization */
__attribute__((constructor))
static void init_globals(void) {
    g_init_flag = 1;
    printf("Constructor: Global init flag set\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_globals(void) {
    printf("Destructor: Program cleanup\n");
}

/* Recursive AST manipulation with memory operations */
static struct ast_node* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    struct ast_node *node = malloc(sizeof(*node));
    if (!node) return NULL;
    
    node->size = g_mem_size / (depth + 1);
    node->data = malloc(node->size);
    node->id = id;
    
    /* Use __builtin_memset to initialize node data */
    if (node->data) {
        __builtin_memset(node->data, id % 256, node->size);
    }
    
    /* Recursive creation with goto for control flow complexity */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth - 1, id * 2);
    node->right = create_ast(depth - 1, id * 2 + 1);
    
done:
    return node;
}

/* Function with goto jumping around memmove */
static void manipulate_ast(struct ast_node *src, struct ast_node *dst) {
    if (!src || !dst || !src->data || !dst->data) return;
    
    size_t copy_size = src->size < dst->size ? src->size : dst->size;
    
    /* Complex control flow with goto */
    if (copy_size > 100) {
        goto large_copy;
    }
    
    /* Small copy using memcpy */
    __builtin_memcpy(dst->data, src->data, copy_size);
    goto done;
    
large_copy:
    /* Use memmove with potential overlap */
    if (src->data + copy_size > dst->data && src->data < dst->data + copy_size) {
        __builtin_memmove(dst->data, src->data, copy_size);
    } else {
        __builtin_memcpy(dst->data, src->data, copy_size);
    }
    
    /* Jump back for additional operation */
    if (dst->id % 3 == 0) {
        goto extra_op;
    }
    
done:
    return;
    
extra_op:
    /* Extra memset on part of buffer */
    __builtin_memset(dst->data + copy_size/2, 0xFF, copy_size/4);
    goto done;
}

/* Parallel memory operations using OpenMP */
static void parallel_mem_operations(void) {
    const int num_buffers = 16;
    char *buffers[num_buffers];
    volatile size_t local_size = g_mem_size;
    
    #pragma omp parallel for
    for (int i = 0; i < num_buffers; i++) {
        buffers[i] = malloc(local_size);
        if (buffers[i]) {
            /* Each thread uses builtins */
            __builtin_memset(buffers[i], i, local_size);
            
            /* Create overlap for memmove testing */
            if (i > 0 && i % 4 == 0) {
                #pragma omp critical
                {
                    __builtin_memmove(buffers[i-1], buffers[i], local_size/2);
                }
            }
        }
    }
    
    /* Verify and cleanup */
    #pragma omp parallel for
    for (int i = 0; i < num_buffers; i++) {
        if (buffers[i]) {
            /* Final memcpy between buffers */
            if (i < num_buffers - 1 && buffers[i+1]) {
                __builtin_memcpy(buffers[i+1], buffers[i], local_size/4);
            }
            free(buffers[i]);
        }
    }
}

/* Multi-stage initialization and processing */
static unsigned long process_tokens(const char *tokens[], int count) {
    unsigned long hash = 0;
    char buffer[512];
    volatile int use_memmove = 0;
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]) + 1;
        
        /* Alternate between memcpy and memmove */
        if (use_memmove && i > 0) {
            __builtin_memmove(buffer, tokens[i], len);
        } else {
            __builtin_memcpy(buffer, tokens[i], len);
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < len && j < sizeof(buffer); j++) {
            hash = (hash * 31) + buffer[j];
        }
        
        use_memmove = !use_memmove;
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Initialize complex token array */
    const char *tokens[] = {
        "memcpy_test_token_1",
        "memset_operation_token",
        "memmove_overlap_token",
        "asan_instrumentation",
        "hwasan_kernel_mode",
        "builtin_redirection",
        "control_flow_goto",
        "openmp_parallel_section"
    };
    
    /* Stage 1: Token processing */
    unsigned long token_hash = process_tokens(tokens, 
        sizeof(tokens)/sizeof(tokens[0]));
    printf("Token hash: %lu\n", token_hash);
    
    /* Stage 2: AST creation and manipulation */
    struct ast_node *ast1 = create_ast(4, 1);
    struct ast_node *ast2 = create_ast(3, 2);
    
    if (ast1 && ast2) {
        manipulate_ast(ast1, ast2);
        
        /* Additional direct memory operations */
        if (ast1->data && ast2->data) {
            size_t min_size = ast1->size < ast2->size ? ast1->size : ast2->size;
            __builtin_memcpy(ast1->data + min_size/3, ast2->data, min_size/3);
            __builtin_memset(ast2->data + min_size/2, 0xAA, min_size/4);
        }
    }
    
    /* Stage 3: Parallel operations */
    parallel_mem_operations();
    
    /* Stage 4: Final verification with all builtins */
    char final_buffer[1024];
    volatile size_t final_size = 256;
    
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, tokens[0], strlen(tokens[0]));
    __builtin_memmove(final_buffer + 128, final_buffer, 64);
    
    /* Compute final checksum */
    unsigned long final_sum = token_hash;
    for (size_t i = 0; i < sizeof(final_buffer); i++) {
        final_sum += (unsigned char)final_buffer[i];
    }
    
    printf("Final checksum: %lu\n", final_sum);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    /* Note: Proper AST cleanup omitted for brevity */
    
    return 0;
}
