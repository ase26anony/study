/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    volatile char buffer[128];
    /* Force memcpy redirection in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 32, buffer, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    volatile char cleanup_buf[256];
    /* Test memmove in destructor */
    __builtin_memset(cleanup_buf, 0xFF, 128);
    __builtin_memmove(cleanup_buf + 64, cleanup_buf, 128);
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile control */
    size_t copy_len = (size_t)(volatile_len % 128) + 64;
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Conditional memcpy based on volatile flag */
    if (volatile_flag) {
        __builtin_memcpy(node->data, base_data, 
                        copy_len < sizeof(node->data) ? copy_len : sizeof(node->data) - 1);
    }
    
    node->size = copy_len;
    node->left = create_ast(depth - 1, node->data);
    node->right = create_ast(depth - 1, node->data + 32);
    
    return node;
}

/* Function with goto jumps around memmove */
static void test_goto_memmove(void) {
    volatile char src[512], dst[512];
    int use_memmove = 0;
    
    __builtin_memset(src, 0xCC, sizeof(src));
    
    /* Jump into block with memmove */
    if (volatile_flag) goto memmove_block;
    
    normal_path:
    __builtin_memcpy(dst, src, 256);
    return;
    
    memmove_block:
    /* This tests flow sensitivity */
    use_memmove = 1;
    if (use_memmove) {
        __builtin_memmove(dst, src, 384);
        goto normal_path;
    }
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        volatile char thread_buf[1024];
        int tid = omp_get_thread_num();
        
        /* Each thread uses builtins */
        __builtin_memset(thread_buf, tid, sizeof(thread_buf));
        
        #pragma omp barrier
        
        /* Memcpy between sections */
        __builtin_memcpy(thread_buf + 512, thread_buf, 512);
        
        /* Conditional memmove */
        if (tid % 2 == 0) {
            __builtin_memmove(thread_buf, thread_buf + 256, 256);
        }
    }
}

/* Complex token processing with varied memory patterns */
static unsigned long process_tokens(char** tokens, int count) {
    unsigned long hash = 0;
    volatile char accum[2048] = {0};
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Mix of memcpy and memset patterns */
        if (i % 3 == 0) {
            __builtin_memcpy(accum + hash % 1024, tokens[i], 
                           len < 256 ? len : 256);
        } else if (i % 3 == 1) {
            __builtin_memset(accum + (i * 64) % 1024, tokens[i][0], 128);
        } else {
            /* Overlapping memmove */
            __builtin_memmove(accum + 512, accum + 256, 384);
            __builtin_memcpy(accum + 256, tokens[i], len < 128 ? len : 128);
        }
        
        /* Update hash from buffer */
        for (size_t j = 0; j < sizeof(accum) && j < 1024; j++) {
            hash = (hash * 31) + accum[j];
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Basic builtin calls */
    volatile char buffer1[1024], buffer2[1024];
    
    __builtin_memset(buffer1, 0x11, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 256, buffer1, 512);
    
    /* Phase 2: Recursive AST with memory ops */
    ASTNode* root = create_ast(4, "AST Base Data for ASAN Testing");
    
    if (root) {
        /* Copy between AST nodes */
        __builtin_memcpy(root->left->data, root->data, 128);
        __builtin_memmove(root->right->data, root->left->data, 192);
        
        /* Free AST */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    /* Phase 3: Goto flow testing */
    test_goto_memmove();
    
    /* Phase 4: OpenMP parallel section */
    #ifdef _OPENMP
    parallel_memory_ops();
    #endif
    
    /* Phase 5: Token processing */
    char* tokens[] = {
        "memcpy", "memset", "memmove",
        "asan", "hwasan", "instrumentation",
        "redzone", "shadow", "memory"
    };
    
    unsigned long final_hash = process_tokens(tokens, 
                                            sizeof(tokens)/sizeof(tokens[0]));
    
    printf("Final hash: %lu\n", final_hash);
    printf("Test completed. Check ASAN instrumentation.\n");
    
    return 0;
}
