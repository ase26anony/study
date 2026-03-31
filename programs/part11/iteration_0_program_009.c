/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

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
    printf("Constructor: Initializing ASAN test environment\n");
    /* Force early initialization of memory functions */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* prefix) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Build node data with __builtin_memcpy */
    char temp[256];
    __builtin_memset(temp, 'A', sizeof(temp));
    __builtin_memcpy(node->data, temp, sizeof(node->data));
    
    /* Copy prefix with __builtin_memmove */
    size_t prefix_len = strlen(prefix);
    if (prefix_len > 0) {
        __builtin_memmove(node->data, prefix, prefix_len < 255 ? prefix_len : 255);
    }
    
    node->size = sizeof(ASTNode);
    node->left = create_ast(depth - 1, "LEFT_");
    node->right = create_ast(depth - 1, "RIGHT_");
    
    return node;
}

/* Function with goto statements for flow control */
static void test_goto_memmove(void) {
    char src[128], dst[128];
    volatile int condition = 1;
    
    __builtin_memset(src, 'X', sizeof(src));
    
    start_label:
    if (condition) {
        /* Jump into memory operation block */
        __builtin_memmove(dst, src, g_mem_size);
        condition = 0;
        goto end_label;
    }
    
    middle_label:
    __builtin_memmove(dst + 32, src + 32, g_mem_size - 32);
    
    end_label:
    /* Jump out and back in */
    if (condition == 0) {
        goto middle_label;
    }
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(void) {
    const int array_size = 1024;
    char* buffers[4];
    
    /* Allocate buffers */
    for (int i = 0; i < 4; i++) {
        buffers[i] = (char*)malloc(array_size);
        __builtin_memset(buffers[i], i + '0', array_size);
    }
    
    #pragma omp parallel num_threads(2)
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            /* Each thread performs different memory operations */
            if (tid == 0) {
                /* Thread 0: memcpy operations */
                __builtin_memcpy(buffers[i] + 128, buffers[(i + 1) % 4], 256);
            } else {
                /* Thread 1: memset operations */
                __builtin_memset(buffers[i] + 384, 'Z', 128);
            }
            
            /* Barrier to ensure interleaving */
            #pragma omp barrier
            
            /* Both threads do memmove */
            __builtin_memmove(buffers[i] + 512, buffers[i], 256);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(buffers[i]);
    }
}

/* Complex token processing with varied memory operations */
static unsigned long process_tokens(const char** tokens, int count) {
    unsigned long hash = 5381;
    char buffer[512];
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Clear buffer with __builtin_memset */
        __builtin_memset(buffer, 0, sizeof(buffer));
        
        /* Copy token with __builtin_memcpy */
        __builtin_memcpy(buffer, tokens[i], len > 511 ? 511 : len);
        
        /* Move data around with __builtin_memmove */
        if (len > 256) {
            __builtin_memmove(buffer + 128, buffer, 256);
        }
        
        /* Update hash */
        for (size_t j = 0; j < len && j < sizeof(buffer); j++) {
            hash = ((hash << 5) + hash) + buffer[j];
        }
    }
    
    return hash;
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Basic built-in calls */
    printf("Phase 1: Basic built-in memory operations\n");
    {
        char src[256], dst[256];
        
        __builtin_memset(src, 'S', sizeof(src));
        __builtin_memcpy(dst, src, g_mem_size);
        __builtin_memmove(dst + 64, src, g_mem_size);
        
        /* Verify with volatile check */
        volatile char check = dst[0];
        (void)check; /* Suppress unused warning */
    }
    
    /* Phase 2: Goto flow control */
    printf("Phase 2: Testing goto with memmove\n");
    test_goto_memmove();
    
    /* Phase 3: Recursive AST operations */
    printf("Phase 3: Recursive AST memory operations\n");
    ASTNode* root = create_ast(3, "ROOT_");
    
    if (root) {
        /* Copy between AST nodes */
        ASTNode temp;
        __builtin_memcpy(&temp, root, sizeof(ASTNode));
        __builtin_memmove(root->data, temp.data, sizeof(temp.data));
        
        /* Recursive cleanup would go here */
        free(root);
    }
    
    /* Phase 4: OpenMP parallel operations */
    printf("Phase 5: OpenMP parallel memory operations\n");
    parallel_memory_operations();
    
    /* Phase 5: Token processing */
    printf("Phase 5: Token processing with memory built-ins\n");
    const char* tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "instrumentation", "redzone", "builtin", "coverage"
    };
    
    unsigned long final_hash = process_tokens(tokens, 
        sizeof(tokens) / sizeof(tokens[0]));
    
    printf("Final hash: %lu\n", final_hash);
    printf("=== Test completed ===\n");
    
    return 0;
}
