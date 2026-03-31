/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _OPENMP
#include <omp.h>
#endif

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char* data;
    size_t size;
    uint32_t hash;
} ASTNode;

/* Constructor attribute for early initialization */
static void __attribute__((constructor)) init_globals(void) {
    printf("Constructor: Initializing global state\n");
}

/* Destructor for cleanup */
static void __attribute__((destructor)) cleanup_globals(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->size = g_mem_size / (depth + 1);
    node->data = (char*)malloc(node->size);
    
    /* Use __builtin_memset to initialize */
    if (node->data) {
        __builtin_memset(node->data, 0, node->size);
        
        /* Copy base data using __builtin_memcpy */
        size_t copy_len = strlen(base_data);
        if (copy_len > node->size) copy_len = node->size;
        __builtin_memcpy(node->data, base_data, copy_len);
    }
    
    /* Recursive creation with goto for control flow */
    int use_goto = (depth % 2 == 0);
    
    if (use_goto) {
        goto create_left;
    }
    
    node->left = create_ast(depth - 1, base_data);
    
create_left:
    if (!use_goto) {
        node->right = create_ast(depth - 1, base_data);
    } else {
        node->left = create_ast(depth - 1, base_data);
        node->right = NULL;
    }
    
    /* Calculate hash using memory operations */
    node->hash = 0;
    if (node->data) {
        for (size_t i = 0; i < node->size; i++) {
            node->hash = (node->hash * 31) + (uint8_t)node->data[i];
        }
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    if (!src || !dst || !src->data || !dst->data) return;
    
    size_t copy_size = (src->size < dst->size) ? src->size : dst->size;
    
    /* Jump into memory operation */
    goto perform_copy;
    
perform_copy:
    if (g_use_memmove) {
        /* Use __builtin_memmove with overlapping regions */
        __builtin_memmove(dst->data, src->data, copy_size);
        
        /* Jump out and back in */
        goto after_copy;
    } else {
        __builtin_memcpy(dst->data, src->data, copy_size);
    }
    
after_copy:
    /* Modify source after copy to test memmove correctness */
    if (src->data && copy_size > 10) {
        __builtin_memset(src->data + 5, 'X', 10);
    }
}

/* Parallel memory operations */
static void parallel_memory_ops(ASTNode** nodes, int count) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        #pragma omp for
        for (int i = 0; i < count - 1; i++) {
            if (nodes[i] && nodes[i+1]) {
                /* Alternate between memcpy and memmove */
                if (i % 3 == 0) {
                    size_t size = (nodes[i]->size < nodes[i+1]->size) 
                                ? nodes[i]->size : nodes[i+1]->size;
                    __builtin_memcpy(nodes[i+1]->data, nodes[i]->data, size);
                } else if (i % 3 == 1) {
                    size_t size = (nodes[i]->size < nodes[i+1]->size) 
                                ? nodes[i]->size : nodes[i+1]->size;
                    __builtin_memmove(nodes[i+1]->data, nodes[i]->data, size);
                } else {
                    /* Initialize with memset */
                    __builtin_memset(nodes[i]->data, thread_id, 
                                   nodes[i]->size > 100 ? 100 : nodes[i]->size);
                }
            }
        }
    }
}

/* Complex initialization with multiple builtins */
static void initialize_token_array(char** tokens, int token_count) {
    volatile int init_mode = 0; /* Prevent constant folding */
    
    for (int i = 0; i < token_count; i++) {
        size_t token_size = 64 + (i * 8);
        tokens[i] = (char*)malloc(token_size);
        
        if (tokens[i]) {
            switch (init_mode++ % 3) {
                case 0:
                    __builtin_memset(tokens[i], 'A', token_size);
                    break;
                case 1:
                    __builtin_memcpy(tokens[i], "INIT_TOKEN", 11);
                    break;
                case 2:
                    /* Self-overlapping memmove */
                    if (token_size > 20) {
                        __builtin_memmove(tokens[i] + 10, tokens[i], 10);
                    }
                    break;
            }
            
            /* Ensure null termination */
            if (token_size > 0) {
                tokens[i][token_size - 1] = '\0';
            }
        }
    }
}

/* Main execution flow */
int main(void) {
    const int AST_DEPTH = 4;
    const int NODE_COUNT = 8;
    const int TOKEN_COUNT = 16;
    
    printf("Starting ASAN built-in redirection test\n");
    
    /* 1. Create recursive AST structures */
    ASTNode* nodes[NODE_COUNT];
    for (int i = 0; i < NODE_COUNT; i++) {
        char base[32];
        snprintf(base, sizeof(base), "Node%d_Data", i);
        nodes[i] = create_ast(AST_DEPTH, base);
    }
    
    /* 2. Initialize token array with various builtins */
    char* tokens[TOKEN_COUNT];
    initialize_token_array(tokens, TOKEN_COUNT);
    
    /* 3. Process with goto control flow */
    for (int i = 0; i < NODE_COUNT - 1; i += 2) {
        process_with_goto(nodes[i], nodes[i+1]);
    }
    
    /* 4. Execute parallel memory operations */
    parallel_memory_ops(nodes, NODE_COUNT);
    
    /* 5. Calculate verification hash */
    uint64_t total_hash = 0;
    for (int i = 0; i < NODE_COUNT; i++) {
        if (nodes[i]) {
            total_hash += nodes[i]->hash;
            
            /* Additional memory operation in verification */
            if (nodes[i]->data && nodes[i]->size > 50) {
                char temp[50];
                __builtin_memcpy(temp, nodes[i]->data, 50);
                __builtin_memset(nodes[i]->data + 25, 'V', 25);
                __builtin_memmove(nodes[i]->data, temp, 50);
            }
        }
    }
    
    printf("Verification hash: %llu\n", (unsigned long long)total_hash);
    
    /* 6. Cleanup */
    for (int i = 0; i < NODE_COUNT; i++) {
        if (nodes[i]) {
            free(nodes[i]->data);
            free(nodes[i]);
        }
    }
    
    for (int i = 0; i < TOKEN_COUNT; i++) {
        free(tokens[i]);
    }
    
    printf("Test completed successfully\n");
    return 0;
}
