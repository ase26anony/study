/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 1024;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Initializing ASAN environment...\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Cleaning up ASAN environment...\n");
}

/* Function with goto statements for flow control */
static void process_with_goto(char* dest, const char* src, size_t n) {
    int use_builtin = 1;
    
    if (n == 0) goto skip_copy;
    
    /* Jump into memory operation block */
    goto do_copy;
    
copy_block:
    /* Force builtin usage with goto */
    if (use_builtin) {
        __builtin_memcpy(dest, src, n);
    }
    goto after_copy;
    
do_copy:
    /* This tests flow-sensitivity of asan_memfn_rtls logic */
    if (g_use_memmove) {
        __builtin_memmove(dest, src, n);
    } else {
        __builtin_memcpy(dest, src, n);
    }
    goto copy_block;
    
after_copy:
    /* memset with volatile size */
    __builtin_memset(dest + n - 1, 0, 1);
    
skip_copy:
    return;
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast_node(const char* data, size_t depth) {
    if (depth == 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with builtin */
    size_t len = strlen(data);
    if (len > 255) len = 255;
    __builtin_memcpy(node->data, data, len);
    node->data[len] = '\0';
    node->size = len;
    
    /* Recursive creation */
    char left_data[256];
    char right_data[256];
    __builtin_snprintf(left_data, sizeof(left_data), "%s-L%d", data, (int)depth);
    __builtin_snprintf(right_data, sizeof(right_data), "%s-R%d", data, (int)depth);
    
    node->left = create_ast_node(left_data, depth - 1);
    node->right = create_ast_node(right_data, depth - 1);
    
    return node;
}

/* Copy between AST nodes using builtins */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Direct builtin memcpy */
    __builtin_memcpy(dest->data, src->data, src->size);
    
    /* Recursive copy */
    if (src->left && dest->left) {
        copy_ast_data(dest->left, src->left);
    }
    if (src->right && dest->right) {
        copy_ast_data(dest->right, src->right);
    }
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

/* Main processing with OpenMP parallelization */
static void parallel_memory_operations(void) {
    const size_t buffer_size = 4096;
    char* buffer1 = (char*)malloc(buffer_size);
    char* buffer2 = (char*)malloc(buffer_size);
    
    if (!buffer1 || !buffer2) {
        free(buffer1);
        free(buffer2);
        return;
    }
    
    /* Initialize with builtin memset */
    __builtin_memset(buffer1, 'A', buffer_size);
    __builtin_memset(buffer2, 'B', buffer_size);
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        size_t chunk_size = buffer_size / 4;
        size_t offset = thread_id * chunk_size;
        
        if (offset + chunk_size <= buffer_size) {
            /* Mix of builtin calls */
            __builtin_memcpy(buffer2 + offset, buffer1 + offset, chunk_size);
            
            /* Conditional memmove */
            if (thread_id % 2 == 0) {
                __builtin_memmove(buffer1 + offset + 100, 
                                 buffer1 + offset, 
                                 chunk_size - 100);
            }
            
            /* Final memset */
            __builtin_memset(buffer2 + offset + chunk_size - 10, 
                            thread_id + '0', 10);
        }
    }
    
    /* Verify with checksum */
    unsigned long sum = 0;
    for (size_t i = 0; i < buffer_size; i++) {
        sum += (unsigned char)buffer2[i];
    }
    printf("Buffer checksum: %lu\n", sum);
    
    free(buffer1);
    free(buffer2);
}

/* Complex token processing */
static size_t process_tokens(const char** tokens, size_t count) {
    size_t total_len = 0;
    char combined[2048] = {0};
    char* pos = combined;
    
    for (size_t i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        if (pos + len < combined + sizeof(combined)) {
            __builtin_memcpy(pos, tokens[i], len);
            pos += len;
            total_len += len;
            
            /* Add separator */
            if (i < count - 1) {
                __builtin_memset(pos, '|', 1);
                pos++;
                total_len++;
            }
        }
    }
    
    /* Create overlapping copy */
    if (total_len > 100) {
        __builtin_memmove(combined + 50, combined, total_len - 50);
    }
    
    return total_len;
}

int main(void) {
    printf("Starting ASAN builtin redirection test...\n");
    
    /* 1. Test goto with memory operations */
    char src[256] = "Test data for goto flow control";
    char dest[256];
    process_with_goto(dest, src, strlen(src) + 1);
    printf("Goto test complete: %s\n", dest);
    
    /* 2. Create and manipulate AST */
    ASTNode* ast1 = create_ast_node("Root", 3);
    ASTNode* ast2 = create_ast_node("Copy", 3);
    
    if (ast1 && ast2) {
        /* Copy between AST nodes */
        copy_ast_data(ast2, ast1);
        printf("AST copy complete\n");
    }
    
    /* 3. Process token array */
    const char* tokens[] = {
        "memcpy", "memset", "memmove", "builtin",
        "asan", "hwasan", "redzone", "instrumentation"
    };
    size_t token_count = sizeof(tokens) / sizeof(tokens[0]);
    size_t total_len = process_tokens(tokens, token_count);
    printf("Token processing complete: %zu bytes\n", total_len);
    
    /* 4. Parallel memory operations */
    parallel_memory_operations();
    
    /* 5. Direct builtin calls with volatile sizes */
    volatile size_t dynamic_size = g_mem_size % 512 + 64;
    char* dyn_buf1 = (char*)malloc(dynamic_size);
    char* dyn_buf2 = (char*)malloc(dynamic_size);
    
    if (dyn_buf1 && dyn_buf2) {
        __builtin_memset(dyn_buf1, 0xCC, dynamic_size);
        __builtin_memcpy(dyn_buf2, dyn_buf1, dynamic_size);
        
        /* Overlapping move */
        if (dynamic_size > 100) {
            __builtin_memmove(dyn_buf1 + 50, dyn_buf1, dynamic_size - 50);
        }
        
        free(dyn_buf1);
        free(dyn_buf2);
    }
    
    /* Cleanup */
    free_ast(ast1);
    free_ast(ast2);
    
    printf("Test completed successfully\n");
    return 0;
}
