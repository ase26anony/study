/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_trigger = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early() {
    volatile char buffer1[128];
    volatile char buffer2[128];
    
    /* Force __builtin_memset initialization */
    __builtin_memset(buffer1, 0xAA, volatile_len);
    
    /* Jump logic to test flow sensitivity */
    if (volatile_trigger) {
        goto memcpy_block;
    }
    
memcpy_block:
    /* Force __builtin_memcpy initialization */
    __builtin_memcpy(buffer2, buffer1, volatile_len / 2);
    
    /* Complex goto pattern */
    if (volatile_trigger > 0) {
        goto memmove_block;
    }
    
    return;
    
memmove_block:
    /* Force __builtin_memmove initialization */
    __builtin_memmove(buffer1 + 32, buffer1, volatile_len / 4);
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_node(int id, const char* base_data) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Copy data with __builtin_memcpy */
    size_t copy_len = strlen(base_data) < 255 ? strlen(base_data) : 255;
    __builtin_memcpy(node->data, base_data, copy_len);
    node->data[copy_len] = '\0';
    
    return node;
}

/* Copy between tree nodes with overlapping regions */
static void copy_between_nodes(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use __builtin_memmove for potentially overlapping copies */
    size_t len = volatile_len % 128;
    if (len > 255) len = 255;
    
    __builtin_memmove(dest->data, src->data, len);
    
    /* Also test __builtin_memcpy */
    if (dest->left && src->right) {
        __builtin_memcpy(dest->left->data + 10, src->right->data, 32);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late() {
    volatile char final_buffer[256];
    
    /* Final memory operations in destructor */
    __builtin_memset(final_buffer, 0xFF, 128);
    __builtin_memcpy(final_buffer + 64, final_buffer, 64);
    __builtin_memmove(final_buffer, final_buffer + 32, 96);
}

/* Main execution with OpenMP parallelization */
int main(void) {
    const char* token_array[] = {"TOKEN_A", "TOKEN_B", "TOKEN_C", "TOKEN_D"};
    const int token_count = sizeof(token_array) / sizeof(token_array[0]);
    
    /* Create AST structure */
    ASTNode* root = create_node(0, "ROOT_NODE");
    ASTNode* nodes[4];
    
    for (int i = 0; i < 4; i++) {
        nodes[i] = create_node(i + 1, token_array[i % token_count]);
    }
    
    root->left = nodes[0];
    root->right = nodes[1];
    nodes[0]->left = nodes[2];
    nodes[0]->right = nodes[3];
    
    /* OpenMP parallel region with memory operations */
    #pragma omp parallel num_threads(2)
    {
        int thread_id = omp_get_thread_num();
        volatile char thread_buffer[512];
        
        /* Each thread uses builtins */
        if (thread_id == 0) {
            __builtin_memset(thread_buffer, thread_id, volatile_len * 2);
            __builtin_memcpy(thread_buffer + 128, thread_buffer, 64);
        } else {
            __builtin_memset(thread_buffer + 256, thread_id, volatile_len);
            __builtin_memmove(thread_buffer, thread_buffer + 300, 100);
        }
        
        /* Copy between AST nodes in parallel */
        #pragma omp for
        for (int i = 0; i < 3; i++) {
            if (nodes[i] && nodes[i + 1]) {
                copy_between_nodes(nodes[i], nodes[i + 1]);
            }
        }
    }
    
    /* Additional sequential memory operations with goto */
    volatile char sequential_buffer[1024];
    size_t offset = 0;
    
copy_loop:
    if (offset < 512) {
        __builtin_memcpy(sequential_buffer + offset, sequential_buffer, 64);
        offset += 64;
        
        /* Jump back to create loop with goto */
        if (volatile_trigger) {
            goto copy_loop;
        }
    }
    
move_section:
    __builtin_memmove(sequential_buffer + 256, sequential_buffer, 256);
    
    /* Complex goto pattern around memmove */
    if (volatile_len > 32) {
        goto final_operations;
    }
    
    __builtin_memset(sequential_buffer + 512, 0xCC, 128);
    
final_operations:
    /* Final mixed operations */
    __builtin_memcpy(sequential_buffer + 768, sequential_buffer + 256, 128);
    __builtin_memset(sequential_buffer + 896, 0xEE, 64);
    __builtin_memmove(sequential_buffer + 960, sequential_buffer + 800, 32);
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < 1024; i++) {
        hash += (unsigned long)sequential_buffer[i];
    }
    
    /* Add AST data to hash */
    for (int i = 0; i < 4; i++) {
        if (nodes[i]) {
            for (int j = 0; nodes[i]->data[j] && j < 255; j++) {
                hash += (unsigned long)nodes[i]->data[j];
            }
            hash += nodes[i]->id;
        }
    }
    
    printf("Verification hash: %lu\n", hash);
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(nodes[i]);
    }
    free(root);
    
    return 0;
}
