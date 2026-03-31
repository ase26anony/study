/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char global_tokens[1024];
static volatile int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_constructor(void) {
    /* Initialize with memset */
    __builtin_memset(global_tokens, 'A', sizeof(global_tokens));
    
    /* Use memcpy in constructor */
    char init_pattern[] = "INIT_PATTERN_1234567890";
    __builtin_memcpy(global_tokens + 100, init_pattern, sizeof(init_pattern) - 1);
    
    printf("Constructor initialized tokens\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    /* Use memset in destructor */
    __builtin_memset(global_tokens, 0, sizeof(global_tokens));
    printf("Destructor cleaned up\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with memset */
    __builtin_memset(node->data, 0, sizeof(node->data));
    
    /* Create pattern for this node */
    char pattern[32];
    snprintf(pattern, sizeof(pattern), "NODE_%d_DEPTH_%d", id, depth);
    
    /* Copy pattern using memcpy */
    __builtin_memcpy(node->data, pattern, strlen(pattern));
    
    node->id = id;
    
    /* Recursive creation with goto for control flow */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    /* Jump back into block with memmove */
    {
        char temp[32];
        __builtin_memcpy(temp, node->data, 32);
        __builtin_memmove(node->data + 10, temp, 20);
    }
    
    node->left = create_ast_node(depth - 1, id * 2);
    node->right = create_ast_node(depth - 1, id * 2 + 1);
    
done:
    return node;
}

/* Function with goto jumping around memmove */
static void complex_control_flow(char* dest, char* src, size_t len) {
    int use_memmove = volatile_flag;
    
    if (use_memmove) {
        goto use_memmove_block;
    } else {
        goto use_memcpy_block;
    }
    
use_memmove_block:
    /* This block contains __builtin_memmove */
    {
        char buffer[128];
        __builtin_memcpy(buffer, src, len > 128 ? 128 : len);
        __builtin_memmove(dest, buffer, len > 128 ? 128 : len);
    }
    goto after_operations;
    
use_memcpy_block:
    __builtin_memcpy(dest, src, len);
    goto after_operations;
    
after_operations:
    /* Additional operation */
    if (len > 50) {
        __builtin_memset(dest + len - 10, 'Z', 10);
    }
}

/* Parallel memory operations */
static void parallel_memory_operations(void) {
    const int num_arrays = 8;
    char* arrays[num_arrays];
    size_t sizes[num_arrays];
    
    /* Initialize arrays with different sizes */
    for (int i = 0; i < num_arrays; i++) {
        sizes[i] = (i + 1) * volatile_len;
        arrays[i] = malloc(sizes[i]);
        if (arrays[i]) {
            __builtin_memset(arrays[i], '0' + i, sizes[i]);
        }
    }
    
    /* OpenMP parallel region */
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs memory operations */
        for (int i = thread_id; i < num_arrays; i += 4) {
            if (arrays[i]) {
                /* Mix memcpy and memmove operations */
                if (i % 2 == 0) {
                    __builtin_memcpy(arrays[i] + 10, global_tokens + i * 32, 32);
                } else {
                    __builtin_memmove(arrays[i] + 5, arrays[i] + 15, 20);
                }
                
                /* Conditional memset */
                if (sizes[i] > 100) {
                    __builtin_memset(arrays[i] + sizes[i] - 20, thread_id + 'A', 10);
                }
            }
        }
    }
    
    /* Verify and clean up */
    unsigned long hash = 0;
    for (int i = 0; i < num_arrays; i++) {
        if (arrays[i]) {
            for (size_t j = 0; j < (sizes[i] > 100 ? 100 : sizes[i]); j++) {
                hash += (unsigned long)arrays[i][j];
            }
            free(arrays[i]);
        }
    }
    
    printf("Parallel operations hash: %lu\n", hash);
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Basic built-in calls */
    char buffer1[256];
    char buffer2[256];
    
    /* Force all three built-ins */
    __builtin_memset(buffer1, 'X', sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 50, buffer1, 100);
    
    /* Phase 2: Recursive AST operations */
    ASTNode* root = create_ast_node(4, 1);
    
    if (root) {
        /* Copy between AST nodes */
        if (root->left && root->right) {
            __builtin_memcpy(root->right->data, root->left->data, 128);
            __builtin_memmove(root->left->data + 64, root->right->data, 64);
        }
        
        /* Calculate hash from AST */
        unsigned long ast_hash = 0;
        ASTNode* nodes[16];
        nodes[0] = root;
        int count = 1;
        
        for (int i = 0; i < count && i < 15; i++) {
            ASTNode* node = nodes[i];
            for (int j = 0; j < 64; j++) {
                ast_hash += (unsigned long)node->data[j];
            }
            
            if (node->left && count < 15) nodes[count++] = node->left;
            if (node->right && count < 15) nodes[count++] = node->right;
        }
        
        printf("AST hash: %lu\n", ast_hash);
        
        /* Cleanup AST */
        /* ... cleanup code would go here ... */
    }
    
    /* Phase 3: Complex control flow with goto */
    complex_control_flow(buffer1, buffer2, volatile_len);
    
    /* Phase 4: Parallel operations */
    parallel_memory_operations();
    
    /* Phase 5: Final verification */
    unsigned long final_hash = 0;
    for (int i = 0; i < 256; i++) {
        final_hash += (unsigned long)buffer1[i];
        final_hash += (unsigned long)buffer2[i];
    }
    
    for (int i = 0; i < 1024; i += 32) {
        final_hash += (unsigned long)global_tokens[i];
    }
    
    printf("Final verification hash: %lu\n", final_hash);
    printf("Test completed successfully\n");
    
    return 0;
}
