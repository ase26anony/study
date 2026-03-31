/* asan_coverage.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_dest[128];
static volatile char volatile_src[128];

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "test", "data"};
static const int token_count = 5;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize volatile source with pattern */
    for (int i = 0; i < sizeof(volatile_src); i++) {
        volatile_src[i] = (char)(i % 256);
    }
    
    /* Force early built-in usage in constructor */
    __builtin_memset(volatile_dest, 0xAA, sizeof(volatile_dest));
    __builtin_memcpy((char*)volatile_dest + 32, volatile_src, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    /* Final built-in usage in destructor */
    __builtin_memset(volatile_dest, 0, sizeof(volatile_dest));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with built-ins */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern in data */
    char pattern[32];
    __builtin_memset(pattern, 'A' + (id % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    node->id = id;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = NULL;
        
        create_children:
        /* Jump target with memmove operation */
        ASTNode temp_node;
        if (node->left) {
            __builtin_memmove(&temp_node, node->left, sizeof(ASTNode));
            __builtin_memcpy(node->left, &temp_node, sizeof(ASTNode));
        }
        
        node->right = create_ast(depth - 1, id * 2 + 1);
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Function with complex control flow and goto */
static void process_with_goto(ASTNode* node) {
    if (!node) return;
    
    int state = 0;
    
    start_processing:
    switch(state) {
        case 0:
            /* Copy data between nodes using built-ins */
            if (node->left && node->right) {
                __builtin_memcpy(node->right->data, node->data, 16);
                state = 1;
                goto start_processing;
            }
            break;
            
        case 1:
            /* Move data with memmove */
            char buffer[32];
            __builtin_memmove(buffer, node->data, sizeof(node->data));
            __builtin_memcpy(node->data + 8, buffer, 16);
            state = 2;
            goto start_processing;
            
        case 2:
            /* Final memset */
            __builtin_memset(node->data + 24, 0xFF, 8);
            break;
    }
    
    /* Jump out of switch to process children */
    goto process_children;
    
    process_children:
    process_with_goto(node->left);
    process_with_goto(node->right);
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    int i;
    char local_buf[256];
    char result_buf[256] = {0};
    
    #pragma omp parallel private(i) shared(result_buf)
    {
        #pragma omp for
        for (i = 0; i < 256; i++) {
            local_buf[i] = (char)(i % 128);
        }
        
        /* Barrier to ensure initialization */
        #pragma omp barrier
        
        #pragma omp single
        {
            /* Master thread copies using built-ins */
            __builtin_memcpy(result_buf, local_buf, 128);
            __builtin_memset(result_buf + 128, 0xCC, 64);
            __builtin_memmove(result_buf + 64, local_buf + 64, 64);
        }
        
        #pragma omp barrier
        
        /* All threads verify and modify */
        #pragma omp for
        for (i = 0; i < 256; i++) {
            if (i < 128) {
                result_buf[i] ^= 0x55;
            } else {
                result_buf[i] += (char)i;
            }
        }
    }
    
    /* Final built-in outside parallel region */
    __builtin_memcpy((void*)volatile_dest, result_buf, 256);
}

/* Multi-stage initialization with varied built-in usage */
static void multi_stage_processing(void) {
    /* Stage 1: Direct built-in calls */
    char stage1[512];
    __builtin_memset(stage1, 0, sizeof(stage1));
    __builtin_memcpy(stage1, tokens[0], strlen(tokens[0]));
    
    /* Stage 2: Volatile length built-in */
    int len = volatile_len;
    __builtin_memset(stage1 + 100, 0xAA, len);
    __builtin_memmove(stage1 + 200, stage1 + 100, len / 2);
    
    /* Stage 3: Nested calls */
    char stage2[256];
    for (int i = 0; i < 4; i++) {
        __builtin_memcpy(stage2 + i * 64, stage1 + i * 64, 64);
        if (i % 2 == 0) {
            __builtin_memset(stage2 + i * 64 + 32, i, 16);
        }
    }
    
    /* Stage 4: Conditional built-in */
    if (token_count > 3) {
        __builtin_memmove(stage1, stage2, 128);
    }
}

/* Main execution flow */
int main(void) {
    int result_hash = 0;
    
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* 1. Create recursive AST structure */
    ASTNode* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* 2. Process with goto control flow */
    process_with_goto(root);
    
    /* 3. Execute OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_operations();
    #endif
    
    /* 4. Multi-stage processing */
    multi_stage_processing();
    
    /* 5. Compute verification hash */
    for (int i = 0; i < token_count; i++) {
        for (const char* p = tokens[i]; *p; p++) {
            result_hash += *p;
        }
    }
    
    /* Add volatile buffer contents to hash */
    for (int i = 0; i < 64; i++) {
        result_hash += volatile_dest[i];
    }
    
    /* Traverse AST and add to hash */
    ASTNode* stack[32];
    int top = 0;
    stack[top++] = root;
    
    while (top > 0) {
        ASTNode* node = stack[--top];
        if (node) {
            for (int i = 0; i < 16; i++) {
                result_hash += node->data[i];
            }
            result_hash += node->id;
            
            if (node->right) stack[top++] = node->right;
            if (node->left) stack[top++] = node->left;
        }
    }
    
    printf("Test completed. Result hash: %d\n", result_hash);
    printf("Expected range: 5000-15000 (implementation dependent)\n");
    
    /* Cleanup */
    /* Note: Proper AST cleanup omitted for brevity */
    
    return 0;
}
