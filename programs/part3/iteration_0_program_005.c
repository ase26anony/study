/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
struct ast_node {
    char data[64];
    struct ast_node *left;
    struct ast_node *right;
    int id;
};

/* Constructor function to force early initialization */
__attribute__((constructor))
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force memcpy initialization in constructor */
    __builtin_memcpy(buffer, "constructor_init", 16);
    printf("Constructor: Early ASAN initialization triggered\n");
}

/* Destructor to test cleanup paths */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
    printf("Destructor: Cleanup verification\n");
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(*node));
    node->id = id;
    
    /* Fill data with memcpy */
    char temp[64];
    snprintf(temp, sizeof(temp), "AST_Node_%d_Depth_%d", id, depth);
    __builtin_memcpy(node->data, temp, strlen(temp) + 1);
    
    /* Build children with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 2 == 0);
        
        if (use_goto) {
            goto build_left;
        } else {
            node->left = build_ast(depth - 1, id * 2);
            goto skip_left;
        }
        
    build_left:
        node->left = build_ast(depth - 1, id * 2);
        
    skip_left:
        node->right = build_ast(depth - 1, id * 2 + 1);
    }
    
    return node;
}

/* Function with goto jumping into memmove block */
static void test_goto_memmove(struct ast_node* src, struct ast_node* dst) {
    int condition = src->id % 3;
    
    if (condition == 0) {
        goto direct_copy;
    } else if (condition == 1) {
        /* Skip copy */
        return;
    } else {
        /* Fall through to memmove */
    }
    
    /* This block can be entered via goto */
    {
        char temp[64];
        __builtin_memmove(temp, src->data, sizeof(temp));
        __builtin_memmove(dst->data, temp, sizeof(temp));
    }
    return;
    
direct_copy:
    /* Jump here from above */
    __builtin_memmove(dst->data, src->data, sizeof(dst->data));
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    const int num_threads = 4;
    char* buffers[num_threads];
    size_t sizes[num_threads];
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        sizes[tid] = g_mem_size * (tid + 1);
        buffers[tid] = malloc(sizes[tid]);
        
        if (buffers[tid]) {
            /* Pattern initialization with memset */
            __builtin_memset(buffers[tid], tid + 'A', sizes[tid]);
            
            /* Inter-thread copying with memcpy */
            if (tid > 0) {
                size_t copy_size = sizes[tid] < sizes[tid-1] ? 
                                  sizes[tid] : sizes[tid-1];
                __builtin_memcpy(buffers[tid], buffers[tid-1], copy_size);
            }
            
            /* Boundary memmove within buffer */
            size_t move_size = sizes[tid] / 2;
            if (move_size > 0) {
                __builtin_memmove(buffers[tid] + move_size, 
                                 buffers[tid], move_size);
            }
        }
        
        #pragma omp barrier
        
        /* Verify with another memset */
        if (buffers[tid]) {
            __builtin_memset(buffers[tid] + sizes[tid]/4, 
                           'Z', sizes[tid]/4);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_threads; i++) {
        free(buffers[i]);
    }
}

/* Complex token processing with varied memory operations */
static unsigned long process_tokens(const char** tokens, int count) {
    unsigned long hash = 0xDEADBEEF;
    char accumulator[256] = {0};
    volatile size_t acc_pos = 0;  /* Volatile to prevent optimization */
    
    for (int i = 0; i < count; i++) {
        size_t token_len = strlen(tokens[i]) + 1;
        
        /* Use memcpy for token accumulation */
        __builtin_memcpy(accumulator + acc_pos, tokens[i], token_len);
        acc_pos += token_len;
        
        /* Occasionally use memmove to shift data */
        if (i % 5 == 0 && acc_pos > 64) {
            __builtin_memmove(accumulator, accumulator + 32, acc_pos - 32);
            acc_pos -= 32;
        }
        
        /* Hash calculation with memset between operations */
        for (size_t j = 0; j < token_len; j++) {
            hash = (hash << 5) + hash + accumulator[acc_pos - token_len + j];
        }
        
        /* Clear section with memset */
        if (i % 3 == 0) {
            __builtin_memset(accumulator + (acc_pos - 16), 0, 16);
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    struct ast_node* root = build_ast(4, 1);
    if (root) {
        struct ast_node* copy = malloc(sizeof(struct ast_node));
        if (copy) {
            /* Test goto/memmove patterns */
            test_goto_memmove(root, copy);
            
            /* Direct memory operations between nodes */
            __builtin_memcpy(copy->left, root->left, sizeof(struct ast_node));
            __builtin_memmove(root->right, copy, sizeof(struct ast_node));
            
            free(copy);
        }
        
        /* TODO: Add recursive free function */
        free(root);
    }
    
    /* Phase 2: Parallel memory stress test */
    parallel_memory_ops();
    
    /* Phase 3: Token processing with varied built-in usage */
    const char* tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "instrumentation", "redzone", "builtin", "coverage",
        "optimization", "volatile", "recursive", "parallel"
    };
    
    unsigned long result = process_tokens(tokens, 
                                        sizeof(tokens)/sizeof(tokens[0]));
    
    printf("Processing result: 0x%lx\n", result);
    printf("ASAN built-in redirection test completed\n");
    
    return 0;
}
