#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* AST-like recursive structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile int volatile_marker;  /* Prevent optimization */
    struct ASTNode* left;
    struct ASTNode* right;
    char padding[32];  /* Ensure size for memcpy testing */
} ASTNode;

/* Global token array */
volatile int global_tokens[256];
volatile int token_index = 0;

/* Constructor function - forces early initialization */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Use builtins in constructor to test early redirection */
    volatile char buffer[64];
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(&global_tokens[0], buffer, 16);
    
    /* Initialize tokens with pattern */
    for (int i = 0; i < 256; i++) {
        global_tokens[i] = i * 3 + 1;
    }
}

/* Destructor function - tests cleanup paths */
__attribute__((destructor))
static void cleanup_destructor(void) {
    volatile char cleanup_buf[128];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive AST creation with memory operations */
ASTNode* create_ast(int depth, int value) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = depth;
    node->value = value;
    node->volatile_marker = depth * 1000 + value;
    
    /* Create children with goto-based control flow */
    if (depth > 1) {
        int child_val = value * 2;
        
        /* Jump into block containing memmove */
        goto create_left;
        
        skip_left:
        child_val = value * 3;
        goto create_right;
        
        create_left:
        node->left = create_ast(depth - 1, child_val);
        goto skip_left;
        
        create_right:
        node->right = create_ast(depth - 1, child_val);
    }
    
    return node;
}

/* Copy AST node with memcpy between structures */
void copy_ast_node(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Direct structure copy using builtin memcpy */
    __builtin_memcpy(dest, src, sizeof(ASTNode));
    
    /* Additional memmove for overlapping regions */
    volatile char temp[sizeof(ASTNode)];
    __builtin_memcpy(temp, src->padding, sizeof(src->padding));
    __builtin_memmove(dest->padding, temp, sizeof(dest->padding));
}

/* Process AST recursively with OpenMP parallelization */
int process_ast_parallel(ASTNode* root, int level) {
    if (!root) return 0;
    
    int sum = 0;
    
    #pragma omp parallel reduction(+:sum)
    {
        int local_sum = 0;
        
        /* Each thread processes with memory operations */
        volatile char thread_buf[256];
        volatile int buf_size = 128 + (omp_get_thread_num() * 16);
        
        /* Use all three builtins in parallel region */
        __builtin_memset(thread_buf, omp_get_thread_num(), buf_size);
        
        /* Copy from global tokens */
        __builtin_memcpy(thread_buf + 64, 
                        (void*)&global_tokens[omp_get_thread_num() * 8], 
                        32);
        
        /* Move data around */
        __builtin_memmove(thread_buf, thread_buf + 32, 64);
        
        /* Process buffer */
        for (int i = 0; i < 64; i++) {
            local_sum += thread_buf[i];
        }
        
        sum += local_sum;
        
        #pragma omp barrier
        
        /* Additional memory ops after barrier */
        if (omp_get_thread_num() == 0) {
            volatile char sync_buf[512];
            __builtin_memset(sync_buf, 0xCC, sizeof(sync_buf));
            __builtin_memcpy(sync_buf + 256, sync_buf, 256);
        }
    }
    
    /* Recursive processing */
    sum += process_ast_parallel(root->left, level + 1);
    sum += process_ast_parallel(root->right, level + 1);
    
    return sum + root->value;
}

/* Function with complex control flow and memory operations */
int complex_memory_operations(void) {
    volatile int result = 0;
    volatile int use_memmove = 1;
    
    /* Array for testing */
    volatile char array1[1024];
    volatile char array2[1024];
    
    /* Initialize with memset */
    __builtin_memset((void*)array1, 0x11, sizeof(array1));
    __builtin_memset((void*)array2, 0x22, sizeof(array2));
    
    /* Conditional goto with memcpy */
    if (use_memmove) {
        goto do_memmove;
    } else {
        goto do_memcpy;
    }
    
    do_memmove:
    /* Overlapping memory move */
    __builtin_memmove((void*)(array1 + 256), array1, 512);
    goto after_memops;
    
    do_memcpy:
    /* Non-overlapping copy */
    __builtin_memcpy((void*)(array2 + 512), array1, 256);
    
    after_memops:
    
    /* Process results */
    for (int i = 0; i < 256; i++) {
        result += array1[i];
        result += array2[i];
    }
    
    /* Jump back for second pass */
    if (result < 10000) {
        use_memmove = 0;
        goto do_memcpy;
    }
    
    return result;
}

/* Main test driver */
int main(void) {
    int final_sum = 0;
    
    printf("Starting ASAN memory operation tests...\n");
    
    /* Phase 1: AST creation and copying */
    ASTNode* ast1 = create_ast(4, 42);
    ASTNode* ast2 = create_ast(3, 17);
    
    if (ast1 && ast2) {
        /* Copy between AST nodes */
        copy_ast_node(ast2, ast1);
        
        /* Process AST in parallel */
        final_sum += process_ast_parallel(ast1, 0);
        
        free(ast1);
        free(ast2);
    }
    
    /* Phase 2: Complex memory operations */
    final_sum += complex_memory_operations();
    
    /* Phase 3: Direct builtin calls with volatile control */
    volatile int dynamic_size = 128;
    volatile char* dynamic_buf1 = (volatile char*)malloc(dynamic_size * 2);
    volatile char* dynamic_buf2 = (volatile char*)malloc(dynamic_size * 2);
    
    if (dynamic_buf1 && dynamic_buf2) {
        /* Chain of memory operations */
        __builtin_memset((void*)dynamic_buf1, 0x33, dynamic_size);
        __builtin_memcpy((void*)dynamic_buf2, dynamic_buf1, dynamic_size);
        __builtin_memmove((void*)(dynamic_buf1 + 64), dynamic_buf1, dynamic_size - 64);
        
        /* Verify by summing */
        for (int i = 0; i < dynamic_size; i++) {
            final_sum += dynamic_buf1[i];
            final_sum += dynamic_buf2[i];
        }
        
        free((void*)dynamic_buf1);
        free((void*)dynamic_buf2);
    }
    
    /* Phase 4: OpenMP parallel memory stress test */
    #pragma omp parallel for reduction(+:final_sum)
    for (int i = 0; i < 100; i++) {
        volatile char local_buf[256];
        volatile int size = 64 + (i % 64);
        
        /* Mix of all three builtins */
        __builtin_memset(local_buf, i, size);
        
        if (i % 3 == 0) {
            __builtin_memcpy(local_buf + 128, local_buf, 64);
        } else if (i % 3 == 1) {
            __builtin_memmove(local_buf + 64, local_buf, 128);
        }
        
        for (int j = 0; j < 64; j++) {
            final_sum += local_buf[j];
        }
    }
    
    printf("Final checksum: %d\n", final_sum);
    printf("Tests completed.\n");
    
    return (final_sum > 0) ? 0 : 1;
}
