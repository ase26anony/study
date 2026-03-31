/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_redirection(void) {
    /* Force early initialization of memory builtins */
    char buffer[32];
    volatile char* volatile_ptr = buffer;
    
    /* Call all three builtins in constructor */
    __builtin_memset(volatile_ptr, 0xAA, sizeof(buffer));
    __builtin_memcpy(volatile_ptr + 16, volatile_ptr, 8);
    if (g_use_memmove) {
        __builtin_memmove(volatile_ptr + 8, volatile_ptr, 12);
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_cache(void) {
    /* Additional builtin calls in destructor */
    char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->id = (*counter)++;
    node->left = node->right = NULL;
    
    /* Initialize node data with memset */
    __builtin_memset(node->data, node->id % 256, sizeof(node->data));
    
    /* Create children recursively */
    node->left = create_ast(depth - 1, counter);
    node->right = create_ast(depth - 1, counter);
    
    /* Copy data between nodes if both children exist */
    if (node->left && node->right) {
        __builtin_memcpy(node->data + 32, node->left->data, 16);
        
        /* Use goto to create control flow edge cases */
        if (node->id % 3 == 0) {
            goto use_memmove_block;
        }
        
        __builtin_memcpy(node->right->data, node->data + 16, 16);
        goto skip_memmove;
        
    use_memmove_block:
        /* Jump target with memmove */
        __builtin_memmove(node->right->data + 8, node->left->data + 8, 16);
        
    skip_memmove:
        /* Continue normal execution */
        ;
    }
    
    return node;
}

/* Function with complex memory operations and OpenMP */
static size_t process_ast_parallel(ASTNode* root) {
    size_t total_hash = 0;
    volatile size_t chunk_size = g_mem_size / 4;
    
    #pragma omp parallel reduction(+:total_hash)
    {
        char local_buffer[128];
        volatile char* volatile_dest = local_buffer;
        
        /* Initialize local buffer */
        __builtin_memset(volatile_dest, 0, sizeof(local_buffer));
        
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            size_t offset = i * chunk_size;
            
            /* Different memory operations based on thread ID */
            if (i % 3 == 0) {
                __builtin_memcpy(volatile_dest + offset, 
                               root->data + offset, 
                               chunk_size);
            } else if (i % 3 == 1) {
                __builtin_memset(volatile_dest + offset, 
                               i, 
                               chunk_size);
            } else {
                /* Use goto for flow control inside parallel region */
                if (offset > 0) {
                    goto memmove_section;
                }
                __builtin_memcpy(volatile_dest + offset, 
                               root->data, 
                               chunk_size);
                goto continue_loop;
                
            memmove_section:
                __builtin_memmove(volatile_dest + offset, 
                                volatile_dest, 
                                chunk_size);
                
            continue_loop:
                ;
            }
            
            /* Compute hash from buffer */
            for (size_t j = 0; j < chunk_size; j++) {
                total_hash += (size_t)volatile_dest[offset + j];
            }
        }
    }
    
    return total_hash;
}

/* Multi-stage initialization function */
static void initialize_token_array(char tokens[][64], int count) {
    volatile int init_value = 0x42;
    
    for (int i = 0; i < count; i++) {
        /* Alternate between memset and memcpy patterns */
        if (i % 2 == 0) {
            __builtin_memset(tokens[i], init_value + i, 64);
        } else {
            if (i > 0) {
                __builtin_memcpy(tokens[i], tokens[i-1], 32);
                /* Another goto example */
                if (i % 5 == 0) {
                    goto do_memmove;
                }
                __builtin_memcpy(tokens[i] + 32, tokens[0], 32);
                goto skip_memmove2;
                
            do_memmove:
                __builtin_memmove(tokens[i] + 32, tokens[i], 32);
                
            skip_memmove2:
                ;
            }
        }
    }
}

int main(void) {
    char tokens[8][64];
    int node_counter = 1;
    size_t final_hash = 0;
    
    /* Stage 1: Initialize token array with memory operations */
    initialize_token_array(tokens, 8);
    
    /* Stage 2: Create recursive AST structure */
    ASTNode* root = create_ast(4, &node_counter);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Stage 3: Copy data from tokens to AST root */
    for (int i = 0; i < 4 && i < 8; i++) {
        __builtin_memcpy(root->data + (i * 16), tokens[i], 16);
    }
    
    /* Stage 4: Parallel processing with OpenMP */
    final_hash = process_ast_parallel(root);
    
    /* Stage 5: Additional memory operations in main */
    char final_buffer[256];
    volatile char* volatile_final = final_buffer;
    
    __builtin_memset(volatile_final, 0, sizeof(final_buffer));
    __builtin_memcpy(volatile_final, root->data, 64);
    
    /* Conditional memmove with goto */
    if (g_use_memmove) {
        goto final_memmove;
    }
    
    __builtin_memcpy(volatile_final + 128, volatile_final, 64);
    goto skip_final;
    
final_memmove:
    __builtin_memmove(volatile_final + 128, volatile_final, 64);
    
skip_final:
    /* Add final buffer to hash */
    for (int i = 0; i < 192; i++) {
        final_hash += (size_t)volatile_final[i];
    }
    
    /* Cleanup */
    /* Note: In real code, you'd want to properly free the AST */
    
    printf("Final hash: %zu\n", final_hash);
    return 0;
}
