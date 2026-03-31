/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* Volatile to prevent optimization */
    struct ASTNode *left;
    struct ASTNode *right;
    char padding[32];      /* Ensure size for memcpy operations */
} ASTNode;

/* Global volatile variables to control memory operations */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* Token array for parser simulation */
static const char* tokens[] = {
    "memcpy", "memset", "memmove", "parse", "build", "traverse"
};
#define TOKEN_COUNT (sizeof(tokens)/sizeof(tokens[0]))

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_sanitizer_hook(void) {
    printf("Constructor: Initializing sanitizer hooks\n");
    /* Force early initialization of memory functions */
    char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_sanitizer_hook(void) {
    printf("Destructor: Cleaning up sanitizer state\n");
}

/* Recursive AST builder with memory operations */
static ASTNode* build_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = depth * 10;
    node->size = g_mem_size + depth;  /* Volatile size */
    
    /* Build children with goto-based control flow */
    if (depth < max_depth - 1) {
        goto build_left;
        
    build_left:
        node->left = build_ast(depth + 1, max_depth);
        goto build_right;
        
    build_right:
        node->right = build_ast(depth + 1, max_depth);
        goto done;
    }
    
done:
    return node;
}

/* AST copy function with __builtin_memcpy */
static void copy_ast(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Direct memcpy of structure */
    __builtin_memcpy(dest, src, sizeof(ASTNode));
    
    /* Conditional goto for control flow testing */
    if (src->left) {
        goto copy_left;
    copy_left:
        dest->left = (ASTNode*)malloc(sizeof(ASTNode));
        if (dest->left) {
            __builtin_memcpy(dest->left, src->left, sizeof(ASTNode));
        }
    }
    
    if (src->right) {
        goto copy_right;
    copy_right:
        dest->right = (ASTNode*)malloc(sizeof(ASTNode));
        if (dest->right) {
            __builtin_memcpy(dest->right, src->right, sizeof(ASTNode));
        }
    }
}

/* AST traversal with memory operations */
static int traverse_ast(const ASTNode* node, int* sum) {
    if (!node) return 0;
    
    int local_sum = 0;
    char temp_buffer[128];
    
    /* Use __builtin_memset on local buffer */
    __builtin_memset(temp_buffer, node->value, sizeof(temp_buffer));
    
    /* Calculate checksum from buffer */
    for (size_t i = 0; i < sizeof(temp_buffer); i++) {
        local_sum += temp_buffer[i];
    }
    
    *sum += local_sum + node->value;
    
    /* Recursive traversal */
    traverse_ast(node->left, sum);
    traverse_ast(node->right, sum);
    
    return *sum;
}

/* Memory operation dispatcher with OpenMP */
static void dispatch_memory_operations(void) {
    const size_t buffer_size = 1024;
    char* src_buffer = (char*)malloc(buffer_size);
    char* dst_buffer = (char*)malloc(buffer_size);
    
    if (!src_buffer || !dst_buffer) {
        free(src_buffer);
        free(dst_buffer);
        return;
    }
    
    /* Initialize source with pattern */
    for (size_t i = 0; i < buffer_size; i++) {
        src_buffer[i] = (char)(i % 256);
    }
    
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs different memory operations */
        #pragma omp sections
        {
            #pragma omp section
            {
                /* Section 1: memcpy operations */
                volatile size_t copy_size = g_mem_size + thread_id;
                __builtin_memcpy(dst_buffer + thread_id * 64, 
                               src_buffer + thread_id * 64, 
                               copy_size);
            }
            
            #pragma omp section
            {
                /* Section 2: memset operations */
                volatile size_t set_size = g_mem_size + thread_id * 2;
                __builtin_memset(dst_buffer + 256 + thread_id * 64, 
                               thread_id, 
                               set_size);
            }
            
            #pragma omp section
            {
                /* Section 3: memmove with overlap */
                volatile size_t move_size = g_mem_size + thread_id * 3;
                __builtin_memmove(dst_buffer + 512, 
                                dst_buffer + 500, 
                                move_size);
            }
        }
        
        /* Barrier to ensure all operations complete */
        #pragma omp barrier
        
        /* Additional memcpy with goto for flow control */
        if (thread_id % 2 == 0) {
            goto even_thread;
        even_thread:
            __builtin_memcpy(dst_buffer + 768, 
                           src_buffer + 768, 
                           g_mem_size);
        } else {
            goto odd_thread;
        odd_thread:
            __builtin_memset(dst_buffer + 896, 
                           thread_id, 
                           g_mem_size);
        }
    }
    
    /* Verify operations with checksum */
    unsigned long checksum = 0;
    for (size_t i = 0; i < buffer_size; i++) {
        checksum += (unsigned char)dst_buffer[i];
    }
    printf("Memory operations checksum: %lu\n", checksum);
    
    free(src_buffer);
    free(dst_buffer);
}

/* Parser simulation with complex control flow */
static void simulate_parser(void) {
    char parse_buffer[256];
    char* buffers[TOKEN_COUNT];
    
    /* Allocate and initialize buffers */
    for (int i = 0; i < TOKEN_COUNT; i++) {
        buffers[i] = (char*)malloc(128);
        if (buffers[i]) {
            /* Use different builtins based on index */
            switch (i % 3) {
                case 0:
                    __builtin_memset(buffers[i], i, 128);
                    break;
                case 1:
                    if (i > 0) {
                        __builtin_memcpy(buffers[i], buffers[i-1], 128);
                    }
                    break;
                case 2:
                    __builtin_memmove(buffers[i], parse_buffer, 128);
                    break;
            }
        }
    }
    
    /* Complex goto network for control flow testing */
    int state = 0;
    
start:
    if (state >= TOKEN_COUNT) goto finish;
    
    /* Copy token to parse buffer */
    size_t token_len = strlen(tokens[state]);
    volatile size_t copy_len = token_len < 255 ? token_len : 255;
    
    __builtin_memcpy(parse_buffer, tokens[state], copy_len);
    parse_buffer[copy_len] = '\0';
    
    state++;
    
    /* Jump based on token content */
    if (parse_buffer[0] == 'm') {  /* Memory-related tokens */
        goto memory_op;
    } else {
        goto other_op;
    }
    
memory_op:
    /* Perform memory operation on buffer */
    __builtin_memset(parse_buffer + 10, 'X', 20);
    goto next_token;
    
other_op:
    /* Move data around */
    __builtin_memmove(parse_buffer + 5, parse_buffer, 50);
    goto next_token;
    
next_token:
    goto start;
    
finish:
    /* Cleanup */
    for (int i = 0; i < TOKEN_COUNT; i++) {
        free(buffers[i]);
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: AST operations */
    printf("\nPhase 1: AST construction and copying\n");
    ASTNode* original = build_ast(0, 3);
    ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
    
    if (original && copy) {
        copy_ast(copy, original);
        
        int sum1 = 0, sum2 = 0;
        traverse_ast(original, &sum1);
        traverse_ast(copy, &sum2);
        
        printf("AST checksums: original=%d, copy=%d\n", sum1, sum2);
    }
    
    /* Phase 2: OpenMP memory operations */
    printf("\nPhase 2: Parallel memory operations\n");
    dispatch_memory_operations();
    
    /* Phase 3: Parser simulation */
    printf("\nPhase 3: Parser simulation with goto\n");
    simulate_parser();
    
    /* Phase 4: Direct built-in calls with volatile sizes */
    printf("\nPhase 4: Direct built-in function calls\n");
    {
        char buffer1[256], buffer2[256];
        volatile size_t op_sizes[] = {16, 32, 64, 128};
        
        for (int i = 0; i < 4; i++) {
            volatile size_t sz = op_sizes[i];
            
            /* Cycle through all three builtins */
            __builtin_memset(buffer1, i, sz);
            __builtin_memcpy(buffer2, buffer1, sz);
            __builtin_memmove(buffer1 + 10, buffer1, sz - 10);
            
            /* Jump to skip some operations */
            if (i == 2) goto skip_memmove;
            __builtin_memmove(buffer2 + 5, buffer2, sz - 5);
        skip_memmove:
            continue;
        }
        
        /* Final verification */
        unsigned long final_sum = 0;
        for (int i = 0; i < 256; i++) {
            final_sum += (unsigned char)buffer1[i];
            final_sum += (unsigned char)buffer2[i];
        }
        printf("Final buffer checksum: %lu\n", final_sum);
    }
    
    /* Cleanup */
    free(original);
    free(copy);
    
    printf("\nTest completed successfully\n");
    return 0;
}
