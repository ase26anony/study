/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
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
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor and destructor functions */
__attribute__((constructor)) static void init_token_pool(void) {
    /* Initialize with pattern to detect corruption */
    for (int i = 0; i < sizeof(token_pool); i++) {
        token_pool[i] = (char)(i % 256);
    }
}

__attribute__((destructor)) static void verify_token_pool(void) {
    int errors = 0;
    for (int i = 0; i < sizeof(token_pool); i++) {
        if (token_pool[i] != (char)(i % 256)) {
            errors++;
        }
    }
    if (errors > 0) {
        fprintf(stderr, "Token pool corruption detected: %d errors\n", errors);
    }
}

/* Recursive parser with memory operations */
static ASTNode* parse_expression(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = depth;
    
    /* Copy data using __builtin_memcpy with volatile length */
    int copy_len = volatile_len % 256;
    if (copy_len > 255) copy_len = 255;
    
    __builtin_memcpy(node->data, &token_pool[token_index], copy_len);
    token_index = (token_index + copy_len) % sizeof(token_pool);
    
    /* Recursive parsing with goto for control flow testing */
    if (depth > 1) {
        int use_goto = volatile_flag;
        
        if (use_goto) {
            goto parse_left;
        } else {
            node->left = parse_expression(depth - 1);
            goto skip_left;
        }
        
    parse_left:
        node->left = parse_expression(depth - 1);
        
    skip_left:
        /* Use __builtin_memmove with goto jumping into block */
        if (node->left) {
            char temp[256];
            __builtin_memcpy(temp, node->left->data, sizeof(temp));
            
            goto move_data;
            
            /* This label is jumped into */
            move_data:
                __builtin_memmove(node->data + 128, temp, 128);
        }
        
        node->right = parse_expression(depth - 1);
    }
    
    return node;
}

/* Parallel memory operations using OpenMP */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread works on its own buffer */
        char buffer[512];
        char source[512];
        
        /* Initialize source with thread-specific pattern */
        for (int i = 0; i < sizeof(source); i++) {
            source[i] = (char)((i + thread_id) % 256);
        }
        
        /* Force built-in calls that should be redirected */
        __builtin_memset(buffer, thread_id, sizeof(buffer));
        __builtin_memcpy(buffer + 128, source, 256);
        
        /* Use memmove with overlapping regions */
        __builtin_memmove(buffer + 64, buffer + 32, 192);
        
        /* Verify the operations */
        int valid = 1;
        for (int i = 0; i < 64; i++) {
            if (buffer[i] != (char)thread_id) {
                valid = 0;
                break;
            }
        }
        
        #pragma omp critical
        {
            printf("Thread %d: %s\n", thread_id, valid ? "PASS" : "FAIL");
        }
    }
}

/* Complex memory dispatch with varied contexts */
static void memory_dispatch_test(void) {
    /* Array of function pointers for different memory operations */
    void (*mem_ops[3])(void*, const void*, size_t) = {
        (void (*)(void*, const void*, size_t))__builtin_memcpy,
        (void (*)(void*, const void*, size_t))__builtin_memset,
        (void (*)(void*, const void*, size_t))__builtin_memmove
    };
    
    /* Test buffers */
    char buffer1[1024];
    char buffer2[1024];
    
    /* Initialize with pattern */
    for (int i = 0; i < sizeof(buffer1); i++) {
        buffer1[i] = (char)(i % 256);
        buffer2[i] = (char)(255 - (i % 256));
    }
    
    /* Execute all memory operations in sequence */
    for (int op = 0; op < 3; op++) {
        size_t len = (volatile_len * (op + 1)) % 512;
        
        switch (op) {
            case 0: /* memcpy */
                mem_ops[op](buffer2, buffer1, len);
                break;
            case 1: /* memset */
                mem_ops[op](buffer2 + 256, 0, len);
                break;
            case 2: /* memmove with overlap */
                mem_ops[op](buffer1 + 128, buffer1 + 64, len);
                break;
        }
    }
    
    /* Calculate checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < sizeof(buffer1); i++) {
        checksum += (unsigned char)buffer1[i];
        checksum += (unsigned char)buffer2[i];
    }
    
    printf("Memory dispatch checksum: %u\n", checksum);
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Recursive parsing with AST operations */
    printf("\nPhase 1: Recursive AST parsing\n");
    ASTNode* root = parse_expression(4);
    
    if (root) {
        /* Traverse and compute hash */
        unsigned int ast_hash = 0;
        ASTNode* stack[16];
        int stack_ptr = 0;
        stack[stack_ptr++] = root;
        
        while (stack_ptr > 0) {
            ASTNode* node = stack[--stack_ptr];
            ast_hash += node->id;
            
            for (int i = 0; i < 64; i++) {
                ast_hash += (unsigned char)node->data[i];
            }
            
            if (node->right) stack[stack_ptr++] = node->right;
            if (node->left) stack[stack_ptr++] = node->left;
            
            /* Cleanup */
            free(node);
        }
        
        printf("AST traversal hash: %u\n", ast_hash);
    }
    
    /* Phase 2: OpenMP parallel memory operations */
    printf("\nPhase 2: Parallel memory operations\n");
    parallel_memory_operations();
    
    /* Phase 3: Complex memory dispatch */
    printf("\nPhase 3: Memory dispatch test\n");
    memory_dispatch_test();
    
    /* Phase 4: Direct built-in calls with volatile control */
    printf("\nPhase 4: Direct built-in calls\n");
    {
        char final_buffer[256];
        char source_buffer[256];
        
        /* Initialize with volatile-dependent pattern */
        int pattern = volatile_flag ? 0xAA : 0x55;
        __builtin_memset(final_buffer, pattern, sizeof(final_buffer));
        
        for (int i = 0; i < sizeof(source_buffer); i++) {
            source_buffer[i] = (char)((i * 13) % 256);
        }
        
        /* Chain of memory operations */
        __builtin_memcpy(final_buffer + 64, source_buffer, 128);
        __builtin_memmove(final_buffer + 32, final_buffer + 96, 64);
        __builtin_memset(final_buffer + 160, 0xFF, 32);
        
        /* Final verification */
        int final_sum = 0;
        for (int i = 0; i < sizeof(final_buffer); i++) {
            final_sum += (unsigned char)final_buffer[i];
        }
        printf("Final buffer sum: %d\n", final_sum);
    }
    
    printf("\nTest completed successfully\n");
    return 0;
}
