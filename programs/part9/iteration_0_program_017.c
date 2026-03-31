/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    /* Force initialization of ASAN runtime */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    /* Final memory operation to ensure cleanup path */
    volatile char final_buf[16];
    __builtin_memset((void*)final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for initialization */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->size = copy_len;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        create_left_label:
        node->left = create_ast(depth - 1, "left_branch");
        if (!create_left) goto skip_right;
    }
    
    node->right = create_ast(depth - 1, "right_branch");
    
    skip_right:
    return node;
}

/* Function with goto jumping around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst) return;
    
    int use_memmove = 1;
    
    if (src->size > 0) {
        goto do_memmove;
    } else {
        goto skip_operation;
    }
    
    do_memmove:
    /* This should trigger ASAN's memmove redirection */
    __builtin_memmove(dst->data, src->data, 
                     src->size < dst->size ? src->size : dst->size);
    
    if (!use_memmove) {
        goto skip_operation;
    }
    
    skip_operation:
    /* Additional operation after goto */
    volatile int temp = 0;
    __builtin_memset(&temp, 0x42, sizeof(temp));
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
        char dest_buf[128];
        
        /* Initialize with memset */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        /* Copy between buffers */
        __builtin_memcpy(dest_buf, local_buf, sizeof(local_buf));
        
        /* Move data around */
        if (thread_id % 2 == 0) {
            __builtin_memmove(local_buf + 32, local_buf, 64);
        }
        
        /* Volatile access to prevent optimization */
        volatile char* vptr = local_buf;
        vptr[0] = 0;
    }
}

/* Complex token processing with varied memory operations */
static size_t process_tokens(const char** tokens, int count) {
    size_t hash = 0;
    char accum[512] = {0};
    size_t accum_pos = 0;
    
    for (int i = 0; i < count; i++) {
        size_t token_len = strlen(tokens[i]);
        
        /* Use memcpy for accumulation */
        if (accum_pos + token_len < sizeof(accum)) {
            __builtin_memcpy(accum + accum_pos, tokens[i], token_len);
            accum_pos += token_len;
            
            /* Occasionally use memmove to shift data */
            if (i % 3 == 0 && accum_pos > 128) {
                __builtin_memmove(accum, accum + 64, accum_pos - 64);
                accum_pos -= 64;
            }
        }
        
        /* Compute simple hash */
        for (size_t j = 0; j < token_len; j++) {
            hash = hash * 31 + tokens[i][j];
        }
    }
    
    /* Final memset on accumulated data */
    if (accum_pos > 0) {
        __builtin_memset(accum + accum_pos - 16, 0, 16);
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize token array */
    const char* tokens[] = {
        "memcpy_test", "memset_operation", "memmove_data",
        "asan_instrumentation", "hwasan_kernel", "builtin_redirect",
        "rtl_modification", "decl_symbol", "coverage_target"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Create recursive AST structure */
    ASTNode* root = create_ast(4, "root_node");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    ASTNode* copy_node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!copy_node) {
        free(root);
        return 1;
    }
    
    /* Test goto with memmove */
    process_with_goto(root, copy_node);
    
    /* Execute OpenMP parallel section */
    parallel_memory_ops();
    
    /* Process tokens with various memory operations */
    size_t result_hash = process_tokens(tokens, token_count);
    
    /* Additional explicit built-in calls */
    volatile char explicit_buf[256];
    volatile char explicit_dst[256];
    
    /* Force all three built-ins in sequence */
    __builtin_memset((void*)explicit_buf, 0xAA, sizeof(explicit_buf));
    __builtin_memcpy((void*)explicit_dst, (void*)explicit_buf, sizeof(explicit_buf));
    __builtin_memmove((void*)explicit_buf + 128, (void*)explicit_buf, 128);
    
    /* Print verification result */
    printf("Result hash: %zu\n", result_hash);
    printf("Explicit buffer first byte: 0x%02x\n", (unsigned char)explicit_buf[0]);
    
    /* Cleanup */
    free(root);
    free(copy_node);
    
    printf("Test completed successfully.\n");
    return 0;
}
