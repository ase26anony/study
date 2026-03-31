/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _OPENMP
#include <omp.h>
#endif

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int type;
    size_t size;
} ASTNode;

/* Token array for parser simulation */
typedef struct {
    char tokens[32][16];
    int count;
} TokenArray;

/* Global constructor/destructor functions */
void __attribute__((constructor)) init_asan_test(void) {
    printf("ASAN Test Constructor: Initializing test environment\n");
}

void __attribute__((destructor)) cleanup_asan_test(void) {
    printf("ASAN Test Destructor: Cleaning up\n");
}

/* Recursive parser with memory operations */
ASTNode* parse_expression(TokenArray* tokens, int* pos) {
    if (*pos >= tokens->count) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token data with memcpy */
    __builtin_memcpy(node->data, tokens->tokens[*pos], 
                     sizeof(tokens->tokens[*pos]));
    
    (*pos)++;
    
    /* Recursive parsing with goto for flow control */
    if (*pos < tokens->count) {
        int next_pos = *pos;
        
        /* Goto block for memmove testing */
        if (tokens->tokens[next_pos][0] == '(') {
            goto handle_paren;
        }
        
        node->left = parse_expression(tokens, pos);
        
        if (*pos < tokens->count && tokens->tokens[*pos][0] == '+') {
            (*pos)++;
            node->right = parse_expression(tokens, pos);
        }
        
        return node;
        
    handle_paren:
        /* Jump target with memmove operation */
        char temp[64];
        __builtin_memmove(temp, node->data, sizeof(node->data));
        __builtin_memset(node->data, 0, sizeof(node->data));
        __builtin_memcpy(node->data, temp + 1, sizeof(node->data) - 1);
        (*pos)++;
        goto end_parse;
    }
    
end_parse:
    return node;
}

/* Complex memory operation with OpenMP parallelization */
void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Thread-local buffers */
        char src_buf[512];
        char dst_buf[512];
        
        /* Initialize with pattern */
        for (int i = 0; i < 512; i++) {
            src_buf[i] = (char)((i + thread_id) % 256);
        }
        
        /* Force built-in usage with volatile size */
        volatile size_t copy_size = g_mem_size;
        
        /* Test all three built-ins in parallel */
        #pragma omp sections
        {
            #pragma omp section
            {
                __builtin_memset(dst_buf, thread_id, copy_size);
            }
            
            #pragma omp section
            {
                __builtin_memcpy(dst_buf + 128, src_buf + 64, copy_size / 2);
            }
            
            #pragma omp section
            {
                /* Overlapping memory test with memmove */
                __builtin_memmove(src_buf + 100, src_buf + 50, copy_size / 4);
            }
        }
        
        /* Barrier to ensure all operations complete */
        #pragma omp barrier
        
        /* Verify results with checksum */
        uint32_t checksum = 0;
        for (int i = 0; i < 256; i++) {
            checksum += (uint8_t)dst_buf[i];
        }
        
        #pragma omp critical
        {
            printf("Thread %d checksum: %u\n", thread_id, checksum);
        }
    }
}

/* Function with goto jumping into memory operation block */
void goto_memory_test(void) {
    char buffer1[256];
    char buffer2[256];
    int use_memmove = 0;
    
    /* Initialize buffers */
    for (int i = 0; i < 256; i++) {
        buffer1[i] = i;
        buffer2[i] = 255 - i;
    }
    
    /* Goto jumping into different memory operation contexts */
    if (g_use_hwasan) {
        goto hwasan_path;
    } else {
        goto asan_path;
    }
    
asan_path:
    /* Standard ASAN path with memcpy */
    __builtin_memcpy(buffer1, buffer2, 128);
    goto cleanup;
    
hwasan_path:
    /* HWASAN path with overlapping memmove */
    __builtin_memmove(buffer1 + 64, buffer1 + 32, 96);
    goto cleanup;
    
cleanup:
    /* Final operation that could be optimized */
    volatile size_t final_size = 64;
    __builtin_memset(buffer1 + 192, 0, final_size);
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Initialize token array */
    TokenArray tokens;
    tokens.count = 10;
    const char* sample_tokens[] = {"var1", "+", "var2", "*", "(", "var3", "+", "var4", ")", ";"};
    
    for (int i = 0; i < tokens.count; i++) {
        __builtin_strcpy(tokens.tokens[i], sample_tokens[i]);
    }
    
    /* Parse recursive structure */
    int pos = 0;
    ASTNode* ast = parse_expression(&tokens, &pos);
    
    if (ast) {
        /* Perform memory operations between AST nodes */
        ASTNode* ast_copy = (ASTNode*)malloc(sizeof(ASTNode));
        if (ast_copy) {
            /* Test memcpy between complex structures */
            __builtin_memcpy(ast_copy, ast, sizeof(ASTNode));
            
            /* Test overlapping memmove */
            __builtin_memmove(ast->data + 10, ast->data, 32);
            
            free(ast_copy);
        }
        
        free(ast);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Test goto-based control flow */
    goto_memory_test();
    
    /* Additional stress test with varying sizes */
    char* dynamic_buf1 = (char*)malloc(1024);
    char* dynamic_buf2 = (char*)malloc(1024);
    
    if (dynamic_buf1 && dynamic_buf2) {
        volatile size_t sizes[] = {16, 32, 64, 128, 256, 512};
        
        for (int i = 0; i < 6; i++) {
            size_t current_size = sizes[i];
            
            /* Mix of all three built-ins */
            __builtin_memset(dynamic_buf1, i, current_size);
            __builtin_memcpy(dynamic_buf2, dynamic_buf1, current_size);
            
            if (i % 2 == 0) {
                __builtin_memmove(dynamic_buf1 + current_size/2, 
                                 dynamic_buf1, current_size/2);
            }
        }
    }
    
    free(dynamic_buf1);
    free(dynamic_buf2);
    
    printf("ASAN test completed successfully\n");
    return 0;
}
