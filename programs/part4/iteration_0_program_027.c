/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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
    size_t size;
} ASTNode;

/* Global token array */
static char global_tokens[4096];
static volatile size_t token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    /* Initialize with builtin memset */
    __builtin_memset(global_tokens, 0, sizeof(global_tokens));
    
    /* Fill with pattern using memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A', sizeof(pattern));
    
    for (size_t i = 0; i < sizeof(global_tokens); i += sizeof(pattern)) {
        size_t copy_len = sizeof(pattern);
        if (i + copy_len > sizeof(global_tokens)) {
            copy_len = sizeof(global_tokens) - i;
        }
        __builtin_memcpy(&global_tokens[i], pattern, copy_len);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    /* Use builtin memmove to shift data */
    size_t shift = volatile_len % 256;
    if (shift > 0) {
        __builtin_memmove(global_tokens, &global_tokens[shift], 
                         sizeof(global_tokens) - shift);
    }
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_recursive(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with variable length */
    size_t copy_len = (size_t)(volatile_len % 256);
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    
    /* Control flow with goto */
    if (volatile_flag) {
        goto copy_block;
    }
    
    __builtin_memset(node->data, 'X', sizeof(node->data));
    goto skip_copy;
    
copy_block:
    if (base_data) {
        __builtin_memcpy(node->data, base_data, copy_len);
    }
    
skip_copy:
    /* Recursive creation with memmove between nodes */
    node->left = create_ast_recursive(depth - 1, node->data);
    node->right = create_ast_recursive(depth - 1, node->data);
    
    /* Move data between children if they exist */
    if (node->left && node->right) {
        size_t move_len = sizeof(node->left->data) / 2;
        __builtin_memmove(node->left->data, node->right->data, move_len);
    }
    
    node->size = copy_len;
    return node;
}

/* Complex memory dispatch with OpenMP */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        /* Thread-local buffers */
        char local_buf1[512];
        char local_buf2[512];
        
        /* Initialize with builtin memset */
        __builtin_memset(local_buf1, 0, sizeof(local_buf1));
        __builtin_memset(local_buf2, 0, sizeof(local_buf2));
        
        /* Copy from global tokens with variable offset */
        size_t offset = (size_t)omp_get_thread_num() * 128;
        if (offset < sizeof(global_tokens)) {
            size_t copy_size = sizeof(local_buf1);
            if (offset + copy_size > sizeof(global_tokens)) {
                copy_size = sizeof(global_tokens) - offset;
            }
            
            /* Use all three builtins in different contexts */
            __builtin_memcpy(local_buf1, &global_tokens[offset], copy_size);
            
            /* Move data within buffer */
            __builtin_memmove(&local_buf1[128], local_buf1, 256);
            
            /* Set pattern */
            __builtin_memset(&local_buf2[256], 'T', 128);
            
            /* Copy back with overlap */
            __builtin_memcpy(&global_tokens[offset], local_buf2, copy_size);
        }
        
        #pragma omp barrier
        
        /* Additional memory operations after barrier */
        #pragma omp for
        for (int i = 0; i < 16; i++) {
            char temp[64];
            size_t len = (volatile_len + i) % 64;
            
            /* Jump into block with goto */
            if (i % 3 == 0) {
                goto memmove_block;
            }
            
            __builtin_memset(temp, i, len);
            goto continue_loop;
            
        memmove_block:
            __builtin_memmove(&temp[16], temp, len > 16 ? 16 : len);
            
        continue_loop:
            /* Copy to global array */
            size_t global_idx = (token_index + i * 64) % sizeof(global_tokens);
            size_t copy_len = len;
            if (global_idx + copy_len > sizeof(global_tokens)) {
                copy_len = sizeof(global_tokens) - global_idx;
            }
            __builtin_memcpy(&global_tokens[global_idx], temp, copy_len);
        }
    }
}

/* Calculate hash of operations */
static unsigned long compute_result_hash(void) {
    unsigned long hash = 5381;
    
    /* Process global tokens */
    for (size_t i = 0; i < sizeof(global_tokens); i++) {
        hash = ((hash << 5) + hash) + global_tokens[i];
    }
    
    /* Process through AST */
    ASTNode* root = create_ast_recursive(4, "BaseData");
    if (root) {
        /* Traverse AST and incorporate into hash */
        ASTNode* stack[32];
        int stack_ptr = 0;
        stack[stack_ptr++] = root;
        
        while (stack_ptr > 0) {
            ASTNode* current = stack[--stack_ptr];
            
            /* Process node data */
            for (size_t i = 0; i < current->size && i < sizeof(current->data); i++) {
                hash = ((hash << 3) + hash) + current->data[i];
            }
            
            /* Push children */
            if (current->right) stack[stack_ptr++] = current->right;
            if (current->left) stack[stack_ptr++] = current->left;
            
            /* Free node */
            free(current);
        }
    }
    
    return hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Initialize volatile control variables */
    volatile_len = 128;
    volatile_flag = 1;
    
    /* Create initial AST */
    ASTNode* test_tree = create_ast_recursive(3, "InitialData");
    if (test_tree) {
        /* Perform memory operations between nodes */
        if (test_tree->left && test_tree->right) {
            size_t move_size = test_tree->left->size;
            if (move_size > test_tree->right->size) {
                move_size = test_tree->right->size;
            }
            
            /* Use goto to jump into memmove */
            if (move_size > 0) {
                goto perform_memmove;
            }
            
            __builtin_memset(test_tree->left->data, 'L', move_size);
            goto skip_memmove;
            
        perform_memmove:
            __builtin_memmove(test_tree->right->data, test_tree->left->data, move_size);
            
        skip_memmove:
            /* Additional copy */
            __builtin_memcpy(test_tree->data, test_tree->right->data, 
                           sizeof(test_tree->data));
        }
        
        /* Free tree */
        free(test_tree);
    }
    
    /* Execute parallel memory operations */
    parallel_memory_operations();
    
    /* Update global tokens with final operations */
    for (int i = 0; i < 8; i++) {
        size_t offset = i * 512;
        if (offset < sizeof(global_tokens)) {
            size_t block_size = 512;
            if (offset + block_size > sizeof(global_tokens)) {
                block_size = sizeof(global_tokens) - offset;
            }
            
            /* Alternate between memset and memcpy */
            if (i % 2 == 0) {
                __builtin_memset(&global_tokens[offset], i + '0', block_size);
            } else {
                char src[512];
                __builtin_memset(src, i + 'A', sizeof(src));
                __builtin_memcpy(&global_tokens[offset], src, block_size);
            }
        }
    }
    
    /* Compute and print result */
    unsigned long result = compute_result_hash();
    printf("Result hash: %lu\n", result);
    printf("Test completed.\n");
    
    return 0;
}
