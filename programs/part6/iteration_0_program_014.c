/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early() {
    volatile char buffer[128];
    /* Force builtin calls in constructor context */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 32, buffer, 64);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_late() {
    volatile char final_buf[64];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_node(const char* src, size_t len) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use all three builtins in varied contexts */
    __builtin_memset(node, 0, sizeof(ASTNode));
    __builtin_memcpy(node->data, src, len < 256 ? len : 255);
    node->size = len;
    node->left = node->right = NULL;
    return node;
}

static void copy_tree_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Conditional memmove with goto for flow control */
    if (g_use_memmove) {
        goto use_memmove;
    } else {
        __builtin_memcpy(dest->data, src->data, src->size);
        return;
    }
    
use_memmove:
    /* Jump into memmove block */
    __builtin_memmove(dest->data, src->data, src->size);
    
    /* Jump out to different context */
    goto after_copy;
    
after_copy:
    dest->size = src->size;
}

/* Parallel memory operation dispatcher */
static void parallel_mem_ops(char* base_ptr, size_t total_size) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        size_t chunk = total_size / omp_get_num_threads();
        char* local_dest = base_ptr + (tid * chunk);
        char* local_src = base_ptr + ((tid + 1) * chunk) % total_size;
        
        /* Mixed builtin usage across threads */
        if (tid % 3 == 0) {
            __builtin_memset(local_dest, tid, chunk);
        } else if (tid % 3 == 1) {
            __builtin_memcpy(local_dest, local_src, chunk);
        } else {
            __builtin_memmove(local_dest, local_src, chunk);
        }
        
        #pragma omp barrier
        
        /* Cross-thread memory operations */
        if (tid == 0) {
            for (int i = 1; i < omp_get_num_threads(); i++) {
                char* src = base_ptr + (i * chunk);
                __builtin_memcpy(local_dest + (i * 16), src, 16);
            }
        }
    }
}

/* Complex token processing with goto patterns */
static size_t process_tokens(char** tokens, int count) {
    char buffer[512];
    size_t hash = 0;
    int i = 0;
    
    /* Goto-based state machine with memory ops */
process_loop:
    if (i >= count) goto finish;
    
    volatile size_t op_len = g_mem_size + i;
    
    if (i % 4 == 0) {
        __builtin_memset(buffer, tokens[i][0], op_len % 512);
        goto next_iter;
    } else if (i % 4 == 1) {
        __builtin_memcpy(buffer + 128, tokens[i], op_len % 128);
        goto next_iter;
    } else if (i % 4 == 2) {
        /* Nested goto for memmove path */
        if (op_len > 256) goto do_memmove;
        __builtin_memcpy(buffer, tokens[i], op_len % 256);
        goto next_iter;
        
    do_memmove:
        __builtin_memmove(buffer, tokens[i], op_len % 256);
        goto next_iter;
    } else {
        /* Overlapping copy with memmove */
        __builtin_memmove(buffer + 64, buffer + 32, op_len % 192);
    }
    
next_iter:
    for (int j = 0; j < op_len % 64; j++) {
        hash += buffer[j];
    }
    i++;
    goto process_loop;
    
finish:
    return hash;
}

int main(void) {
    /* Initialize token array */
    char* tokens[] = {
        "ASAN_TEST_STRING_1",
        "HWASAN_REDIRECTION_2",
        "MEMCPY_BUILTIN_3",
        "MEMMOVE_FLOW_4",
        "MEMSET_VOLATILE_5"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Create AST structures */
    ASTNode* root = create_node(tokens[0], strlen(tokens[0]));
    ASTNode* copy = create_node("", 0);
    
    if (!root || !copy) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Build tree */
    for (int i = 1; i < token_count; i++) {
        ASTNode* child = create_node(tokens[i], strlen(tokens[i]));
        if (child) {
            if (!root->left) root->left = child;
            else if (!root->right) root->right = child;
        }
    }
    
    /* Test tree copying with goto patterns */
    copy_tree_data(copy, root);
    if (root->left) {
        copy_tree_data(copy, root->left);
    }
    
    /* Parallel memory operations */
    size_t buffer_size = 4096;
    char* main_buffer = (char*)malloc(buffer_size);
    if (main_buffer) {
        __builtin_memset(main_buffer, 0xCC, buffer_size);
        parallel_mem_ops(main_buffer, buffer_size);
    }
    
    /* Process tokens with complex flow */
    size_t result_hash = process_tokens(tokens, token_count);
    
    /* Additional builtin calls in main context */
    volatile char final_check[256];
    __builtin_memset(final_check, 0x55, sizeof(final_check));
    __builtin_memcpy(final_check + 128, final_check, 64);
    __builtin_memmove(final_check + 64, final_check + 32, 96);
    
    /* Print verification result */
    printf("Result hash: %zu\n", result_hash);
    printf("Copy node size: %zu\n", copy->size);
    if (main_buffer) {
        printf("Buffer[0]: 0x%02x\n", (unsigned char)main_buffer[0]);
        free(main_buffer);
    }
    
    /* Cleanup */
    free(root->left);
    free(root->right);
    free(root);
    free(copy);
    
    return 0;
}
