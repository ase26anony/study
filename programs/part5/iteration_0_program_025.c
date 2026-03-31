/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[256];
static volatile char volatile_src[256];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "test", "asan"};
static const int token_count = 5;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < 256; i++) {
        volatile_src[i] = (char)(i % 256);
    }
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Destructor: ASAN environment cleaned up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Create pattern in data using __builtin_memcpy */
    char pattern[32];
    for (int i = 0; i < 32; i++) {
        pattern[i] = (char)((id + i) % 256);
    }
    __builtin_memcpy(node->data, pattern, 32);
    
    /* Recursive creation with goto for control flow */
    int left_id = id * 2;
    int right_id = id * 2 + 1;
    
    if (depth > 1) {
        goto create_left;
        
        skip_left:
        goto create_right;
        
        skip_right:
        goto done;
        
        create_left:
        node->left = create_ast(depth - 1, left_id);
        goto skip_left;
        
        create_right:
        node->right = create_ast(depth - 1, right_id);
        goto skip_right;
    }
    
    done:
    return node;
}

/* Function with complex memory operations and goto */
static void process_ast_with_goto(ASTNode* node1, ASTNode* node2) {
    if (!node1 || !node2) return;
    
    volatile int use_memmove = 0;
    
    start:
    if (use_memmove) {
        /* Jump into block with __builtin_memmove */
        goto memmove_block;
    }
    
    /* Normal path with __builtin_memcpy */
    __builtin_memcpy(node2->data, node1->data, 32);
    
    if (node1->left && node2->left) {
        process_ast_with_goto(node1->left, node2->left);
    }
    
    use_memmove = 1;
    goto start;
    
    memmove_block:
    /* This tests the flow-sensitivity of ASAN logic */
    __builtin_memmove(node1->data, node2->data, 32);
    
    if (node1->right && node2->right) {
        process_ast_with_goto(node1->right, node2->right);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    int i;
    char buffer1[1024];
    char buffer2[1024];
    
    /* Initialize buffers */
    for (i = 0; i < 1024; i++) {
        buffer1[i] = (char)(i % 256);
        buffer2[i] = 0;
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int chunk_size = 1024 / omp_get_num_threads();
        int start = thread_id * chunk_size;
        int end = (thread_id == omp_get_num_threads() - 1) ? 1024 : start + chunk_size;
        
        /* Each thread performs memory operations */
        for (int i = start; i < end; i += 64) {
            int len = (i + 64 <= end) ? 64 : end - i;
            
            /* Force ASAN to handle __builtin_memcpy */
            __builtin_memcpy(&buffer2[i], &buffer1[i], len);
            
            /* Force ASAN to handle __builtin_memset */
            __builtin_memset(&buffer1[i], thread_id, len);
            
            /* Force ASAN to handle __builtin_memmove with overlap */
            if (i + 32 < end) {
                __builtin_memmove(&buffer1[i], &buffer1[i + 16], 32);
            }
        }
    }
    
    /* Verify results */
    int sum = 0;
    for (i = 0; i < 1024; i++) {
        sum += buffer1[i] + buffer2[i];
    }
    printf("Parallel operations checksum: %d\n", sum);
}

/* Function with volatile-controlled memory operations */
static void volatile_memory_ops(void) {
    char local_buf[128];
    int len = volatile_len;
    
    if (len > 128) len = 128;
    if (len < 0) len = 0;
    
    /* Use volatile length with builtins */
    __builtin_memcpy((void*)volatile_dest, (void*)volatile_src, len);
    __builtin_memcpy(local_buf, (void*)volatile_dest, len);
    __builtin_memset((void*)volatile_dest, 0xAA, len);
    __builtin_memmove(local_buf + 16, local_buf, len - 16);
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Recursive AST operations */
    printf("\nPhase 1: Recursive AST operations\n");
    ASTNode* ast1 = create_ast(4, 1);
    ASTNode* ast2 = create_ast(4, 100);
    
    if (ast1 && ast2) {
        process_ast_with_goto(ast1, ast2);
        
        /* Calculate hash of AST data */
        unsigned long hash = 0;
        ASTNode* nodes[100];
        int node_count = 0;
        nodes[node_count++] = ast1;
        nodes[node_count++] = ast2;
        
        for (int i = 0; i < node_count; i++) {
            for (int j = 0; j < 32; j++) {
                hash = hash * 31 + nodes[i]->data[j];
            }
        }
        printf("AST hash: %lu\n", hash);
    }
    
    /* Phase 2: Volatile memory operations */
    printf("\nPhase 2: Volatile memory operations\n");
    volatile_memory_ops();
    
    /* Phase 3: OpenMP parallel operations */
    printf("\nPhase 3: OpenMP parallel operations\n");
    parallel_memory_operations();
    
    /* Phase 4: Direct built-in calls with various sizes */
    printf("\nPhase 4: Direct built-in calls\n");
    char test_buf1[256], test_buf2[256];
    
    for (int size = 1; size <= 128; size *= 2) {
        __builtin_memset(test_buf1, size, 256);
        __builtin_memcpy(test_buf2, test_buf1, size);
        __builtin_memmove(test_buf1 + 64, test_buf1, size);
    }
    
    /* Phase 5: Token processing with memory ops */
    printf("\nPhase 5: Token processing\n");
    char token_buffer[512] = {0};
    char* dest = token_buffer;
    
    for (int i = 0; i < token_count; i++) {
        size_t len = strlen(tokens[i]);
        __builtin_memcpy(dest, tokens[i], len);
        dest += len;
        if (i < token_count - 1) {
            __builtin_memset(dest, '|', 1);
            dest += 1;
        }
    }
    printf("Token buffer: %s\n", token_buffer);
    
    /* Cleanup */
    /* Note: In real ASAN, memory would be automatically checked */
    
    printf("\n=== Test completed ===\n");
    return 0;
}
