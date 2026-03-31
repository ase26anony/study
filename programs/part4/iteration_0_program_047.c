/* ISO C99-compliant program targeting ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

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
    /* Force initialization of ASAN runtime */
    volatile char buffer[64];
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char buffer[64];
    __builtin_memset(buffer, 0xFF, sizeof(buffer));
    printf("Destructor: ASAN environment cleaned up\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast_tree(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Create pattern in data using __builtin_memcpy */
    char pattern[256];
    for (int i = 0; i < 256; i++) {
        pattern[i] = (char)(i % 256);
    }
    __builtin_memcpy(node->data, pattern, 256);
    
    /* Set size using volatile variable */
    node->size = g_mem_size % 256;
    
    /* Recursive creation with goto for control flow */
    if (depth > 1) {
        int create_left = 1;
        
        /* Goto to test flow sensitivity */
        if (depth % 2 == 0) goto create_children;
        
        node->left = create_ast_tree(depth - 1);
        create_children:
        node->right = create_ast_tree(depth - 2);
        
        /* Memory move between nodes if both exist */
        if (node->left && node->right) {
            size_t move_size = (node->left->size < node->right->size) ? 
                              node->left->size : node->right->size;
            __builtin_memmove(node->right->data, node->left->data, move_size);
        }
    }
    
    return node;
}

/* Function with complex memory operations and OpenMP */
static void process_ast_parallel(ASTNode* root) {
    if (!root) return;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("OpenMP: %d threads available\n", omp_get_num_threads());
        }
        
        #pragma omp for
        for (int i = 0; i < 4; i++) {
            volatile char local_buffer[128];
            volatile size_t local_size = 64 + i * 16;
            
            /* Force all three built-ins in parallel region */
            __builtin_memset(local_buffer, i, local_size);
            
            char src_buffer[128];
            for (int j = 0; j < 128; j++) {
                src_buffer[j] = (char)((i + j) % 256);
            }
            
            __builtin_memcpy((void*)local_buffer, src_buffer, local_size);
            
            /* Use goto to jump around memory operations */
            if (i % 2 == 0) {
                goto skip_move;
            }
            
            char temp_buffer[128];
            __builtin_memmove(temp_buffer, local_buffer, local_size);
            __builtin_memcpy((void*)local_buffer, temp_buffer, local_size);
            
            skip_move:
            /* Additional operation after label */
            __builtin_memset(((char*)local_buffer) + 32, 0xFF, 32);
        }
    }
    
    /* Process tree recursively */
    if (root->left) {
        volatile size_t copy_size = root->size;
        if (copy_size > 256) copy_size = 256;
        
        char temp[256];
        __builtin_memcpy(temp, root->data, copy_size);
        __builtin_memmove(root->left->data, temp, copy_size);
    }
}

/* Calculate hash of AST tree */
static unsigned long long hash_ast_tree(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long long hash = 5381;
    
    /* Hash node data */
    for (size_t i = 0; i < node->size && i < 256; i++) {
        hash = ((hash << 5) + hash) + (unsigned char)node->data[i];
    }
    
    /* Recursive hash with goto for control flow */
    if (node->left || node->right) {
        int hash_right = 0;
        
        if (node->left) {
            hash += hash_ast_tree(node->left);
            if (node->right) {
                goto hash_both;
            }
        }
        
        if (node->right) {
            hash_right = 1;
            hash_both:
            hash ^= hash_ast_tree(node->right);
            
            /* Memory operation after label */
            if (hash_right) {
                char temp[64];
                __builtin_memset(temp, 0, 64);
                __builtin_memcpy(temp, node->right->data, 
                               (node->right->size < 64) ? node->right->size : 64);
            }
        }
    }
    
    return hash;
}

/* Free AST tree */
static void free_ast_tree(ASTNode* node) {
    if (!node) return;
    
    /* Clear data before free */
    __builtin_memset(node->data, 0, 256);
    
    free_ast_tree(node->left);
    free_ast_tree(node->right);
    
    /* Final clear of entire node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    free(node);
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Initialize token array */
    volatile char token_array[512];
    for (volatile int i = 0; i < 512; i++) {
        token_array[i] = (char)(i % 128);
    }
    
    /* Create recursive AST structure */
    ASTNode* root = create_ast_tree(5);
    if (!root) {
        fprintf(stderr, "Failed to create AST tree\n");
        return 1;
    }
    
    printf("AST tree created successfully\n");
    
    /* Process with OpenMP */
    process_ast_parallel(root);
    
    /* Calculate and print verification hash */
    unsigned long long hash = hash_ast_tree(root);
    printf("Verification hash: %llu\n", hash);
    
    /* Additional memory operations in main */
    volatile char final_buffer[256];
    volatile size_t op_size = g_mem_size % 128;
    
    /* Test all three built-ins in sequence */
    __builtin_memset(final_buffer, 0xAA, op_size);
    
    char source[256];
    for (int i = 0; i < 256; i++) {
        source[i] = (char)(255 - i);
    }
    
    __builtin_memcpy((void*)final_buffer, source, op_size);
    
    /* Use goto to create complex control flow around memmove */
    int use_memmove = 1;
    if (hash % 2 == 0) {
        goto skip_final_move;
    }
    
    char temp[256];
    __builtin_memmove(temp, final_buffer, op_size);
    __builtin_memcpy((void*)final_buffer, temp, op_size);
    use_memmove = 0;
    
    skip_final_move:
    /* Operation after label */
    __builtin_memset(((char*)final_buffer) + op_size/2, 0xCC, op_size/4);
    
    /* Cleanup */
    free_ast_tree(root);
    
    printf("Test completed successfully\n");
    return 0;
}
