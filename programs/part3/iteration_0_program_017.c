/* ISO C99-compliant program targeting GCC ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_hooks(void) {
    volatile char buffer[128];
    /* Force __builtin_memset in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_hooks(void) {
    volatile char cleanup_buf[64];
    /* Force __builtin_memcpy in destructor */
    char src[] = "Cleanup data";
    __builtin_memcpy(cleanup_buf, src, sizeof(src));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for node initialization */
    __builtin_memcpy(node->data, base_data, strlen(base_data) + 1);
    node->size = strlen(base_data) + 1;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
create_children:
    if (create_left) {
        char left_data[64];
        __builtin_snprintf(left_data, sizeof(left_data), "%s-L%d", base_data, depth);
        node->left = create_ast(depth - 1, left_data);
        create_left = 0;
        goto create_children; /* Jump back */
    } else {
        char right_data[64];
        __builtin_snprintf(right_data, sizeof(right_data), "%s-R%d", base_data, depth);
        node->right = create_ast(depth - 1, right_data);
    }
    
    return node;
}

/* Function with goto jumping around memory operations */
static void process_with_goto(ASTNode* node1, ASTNode* node2) {
    volatile int state = 0;
    
    if (node1 == NULL || node2 == NULL) {
        goto cleanup;
    }
    
    /* Jump into memory operation block */
    goto mem_operation;
    
mem_operation:
    {
        /* Force __builtin_memmove with goto entry */
        volatile char temp[128];
        __builtin_memmove(temp, node1->data, node1->size);
        
        if (state == 0) {
            state = 1;
            goto copy_operation; /* Jump out of block */
        }
    }
    
copy_operation:
    /* Force __builtin_memcpy */
    __builtin_memcpy(node2->data, node1->data, 
                    node1->size < node2->size ? node1->size : node2->size);
    
    goto finalize;
    
cleanup:
    /* Force __builtin_memset on error path */
    if (node1) __builtin_memset(node1->data, 0, sizeof(node1->data));
    if (node2) __builtin_memset(node2->data, 0, sizeof(node2->data));
    
finalize:
    return;
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        volatile char thread_buf[256];
        volatile char thread_src[256];
        
        /* Initialize source with thread-specific pattern */
        __builtin_memset(thread_src, thread_id + 0x30, sizeof(thread_src));
        
        /* Parallel memory copy */
        __builtin_memcpy(thread_buf, thread_src, sizeof(thread_buf));
        
        /* Verify with memory move */
        volatile char verify_buf[256];
        __builtin_memmove(verify_buf, thread_buf, sizeof(verify_buf));
        
        #pragma omp barrier
        
        /* Cross-thread memory operation simulation */
        #pragma omp single
        {
            volatile char master_buf[512];
            __builtin_memset(master_buf, 0xFF, sizeof(master_buf));
        }
    }
}

/* Complex token processing with varied memory operations */
static size_t process_token_array(const char** tokens, size_t count) {
    size_t hash = 0;
    volatile char accum[1024] = {0};
    size_t accum_pos = 0;
    
    for (size_t i = 0; i < count; i++) {
        size_t token_len = strlen(tokens[i]) + 1;
        
        /* Use __builtin_memcpy for token accumulation */
        if (accum_pos + token_len < sizeof(accum)) {
            __builtin_memcpy(accum + accum_pos, tokens[i], token_len);
            accum_pos += token_len;
        }
        
        /* Use __builtin_memset for padding */
        if (i % 3 == 0) {
            volatile char pad[32];
            __builtin_memset(pad, i & 0xFF, sizeof(pad));
            hash += pad[0];
        }
        
        /* Use __builtin_memmove for overlapping regions */
        if (i > 0 && accum_pos > 64) {
            __builtin_memmove(accum, accum + 32, 64);
        }
    }
    
    /* Final hash calculation */
    for (size_t i = 0; i < sizeof(accum) && i < 256; i++) {
        hash = (hash * 31) + accum[i];
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Initialize complex token array */
    const char* tokens[] = {
        "MEMCPY_TEST", "MEMSET_OPERATION", "MEMMOVE_FLOW",
        "GOTO_BRANCH", "OPENMP_PARALLEL", "RECURSIVE_AST",
        "VOLATILE_VARS", "CONSTRUCTOR_INIT", "DESTRUCTOR_CLEAN"
    };
    size_t token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Create recursive AST structures */
    ASTNode* ast1 = create_ast(4, "ROOT1");
    ASTNode* ast2 = create_ast(3, "ROOT2");
    
    if (!ast1 || !ast2) {
        fprintf(stderr, "Failed to create AST structures\n");
        return 1;
    }
    
    /* Process with goto and memory operations */
    process_with_goto(ast1, ast2);
    
    /* Execute OpenMP parallel memory operations */
    parallel_memory_operations();
    
    /* Process token array with varied memory built-ins */
    size_t final_hash = process_token_array(tokens, token_count);
    
    /* Additional memory operations in main */
    volatile char final_buf[512];
    volatile char final_src[512];
    
    __builtin_memset(final_src, 0xCC, sizeof(final_src));
    __builtin_memcpy(final_buf, final_src, sizeof(final_buf));
    __builtin_memmove(final_src, final_buf, 256);
    
    /* Print verification result */
    printf("Final hash: %zu\n", final_hash);
    printf("Memory operations completed successfully\n");
    
    /* Cleanup */
    free(ast1);
    free(ast2);
    
    return 0;
}
