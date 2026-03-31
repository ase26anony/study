/* ISO C99-compliant test program for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
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
    char buffer[64];
    __builtin_memset(buffer, 0, sizeof(buffer));
    g_init_flag = 1;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_globals(void) {
    /* Final memory operation to ensure cleanup path */
    volatile char final_buf[32];
    __builtin_memset((void*)final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy with volatile size */
    volatile size_t copy_size = sizeof(node->data) - 1;
    __builtin_memcpy(node->data, base_data, copy_size);
    node->data[copy_size] = '\0';
    
    /* Initialize with __builtin_memset */
    __builtin_memset(&node->size, 0, sizeof(node->size));
    node->size = copy_size;
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    return node;
    
create_children:
    /* Jump back into block with memory operation */
    node->left = create_ast(depth - 1, base_data);
    
    /* Use __builtin_memmove between nodes */
    if (node->left && depth > 3) {
        ASTNode temp;
        __builtin_memcpy(&temp, node->left, sizeof(ASTNode));
        __builtin_memmove(node->data + 128, temp.data, temp.size % 128);
    }
    
    node->right = create_ast(depth - 2, base_data);
    return node;
}

/* Function with goto jumping around memory operations */
static void process_with_goto(ASTNode* node) {
    if (!node) return;
    
    volatile int state = 0;
    
start:
    if (state == 0) {
        /* First memory operation */
        char local_buf[512];
        __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
        state = 1;
        goto middle;
    }
    
    /* This should be skipped on first pass */
    __builtin_memcpy(node->data, "SKIPPED", 8);
    
middle:
    if (state == 1) {
        /* Jump into block with memmove */
        char src[256], dst[256];
        __builtin_memset(src, 0xBB, sizeof(src));
        __builtin_memmove(dst, src, g_mem_size % 256);
        state = 2;
        goto end;
    }
    
end:
    /* Final memory operation */
    __builtin_memset(node->data + 100, 0, 50);
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
        char thread_buf[1024];
        char thread_src[512];
        
        /* Initialize with builtins */
        __builtin_memset(thread_buf, thread_id, sizeof(thread_buf));
        __builtin_memset(thread_src, 0xFF, sizeof(thread_src));
        
        /* Copy between buffers */
        volatile size_t copy_len = (thread_id * 64 + 128) % 512;
        __builtin_memcpy(thread_buf + 256, thread_src, copy_len);
        
        /* Move data around */
        __builtin_memmove(thread_buf, thread_buf + 128, 256);
        
        #pragma omp barrier
        
        /* Verify with another memset */
        __builtin_memset(thread_buf + 512, 0xCC, 256);
    }
}

/* Complex token processing */
static unsigned long process_tokens(const char** tokens, int count) {
    unsigned long hash = 0xDEADBEEF;
    char buffer[2048];
    volatile size_t offset = 0;
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        volatile size_t copy_size = len % 512;
        
        /* Use all three builtins in sequence */
        __builtin_memset(buffer + offset, 0, copy_size);
        __builtin_memcpy(buffer + offset, tokens[i], copy_size);
        
        if (i > 0 && offset > 256) {
            __builtin_memmove(buffer, buffer + 128, 256);
        }
        
        /* Update hash */
        for (size_t j = 0; j < copy_size; j++) {
            hash = (hash * 31) + buffer[offset + j];
        }
        
        offset = (offset + copy_size) % 1024;
    }
    
    return hash;
}

int main(void) {
    /* Initialize token array */
    const char* tokens[] = {
        "ASAN_TEST_STRING_1",
        "BUILTIN_MEMCPY_FLOW",
        "GOTO_MEMORY_OPERATION",
        "OPENMP_PARALLEL_SECTION",
        "RECURSIVE_AST_NODES",
        "VOLATILE_SIZE_CONTROL",
        "CONSTRUCTOR_INITIALIZED"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create recursive AST structure */
    ASTNode* root = create_ast(5, "BASE_AST_NODE_DATA");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Process with goto flow control */
    process_with_goto(root);
    
    /* Execute parallel memory operations */
    parallel_memory_ops();
    
    /* Process tokens with memory operations */
    unsigned long final_hash = process_tokens(tokens, token_count);
    
    /* Additional memory operations in main */
    char main_buffer[4096];
    volatile size_t main_size = g_mem_size % 2048;
    
    __builtin_memset(main_buffer, 0, sizeof(main_buffer));
    __builtin_memcpy(main_buffer, root->data, root->size % 1024);
    __builtin_memmove(main_buffer + 1024, main_buffer, 512);
    
    /* Cleanup */
    free(root);
    
    printf("Test completed successfully. Final hash: 0x%08lX\n", final_hash);
    printf("ASAN built-in redirection paths exercised.\n");
    
    return 0;
}
