/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

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

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "asan", "hwasan"};
static const int token_count = 5;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_hook(void) {
    volatile char buffer[32];
    /* Force initialization of memcpy redirection */
    __builtin_memcpy(buffer, tokens[0], 6);
    printf("[constructor] Initialized buffer: %s\n", buffer);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_sanitizer_hook(void) {
    volatile char cleanup_buf[16];
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
    printf("[destructor] Cleanup completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Copy token data with memcpy */
    const char* token = tokens[id % token_count];
    __builtin_memcpy(node->data, token, strlen(token) + 1);
    
    /* Create children with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, id * 2);
        node->right = create_ast(depth - 1, id * 2 + 1);
        
        if (use_goto) {
            skip_children:
            /* memmove between nodes if they exist */
            if (node->left && node->right) {
                volatile size_t move_size = sizeof(node->left->data);
                if (g_use_memmove) {
                    __builtin_memmove(node->right->data, 
                                     node->left->data, 
                                     move_size);
                }
            }
        }
        
        if (0) {
            create_children:
            node->left = create_ast(depth - 2, id * 3);
            node->right = create_ast(depth - 2, id * 3 + 1);
            goto skip_children;
        }
    }
    
    return node;
}

/* Parallel memory operations */
static void parallel_mem_operations(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        volatile char local_buf[128];
        volatile char src_buf[128];
        
        /* Initialize source buffer */
        for (int i = 0; i < 128; i++) {
            src_buf[i] = (char)((thread_id + i) % 256);
        }
        
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            /* Alternate between memory functions */
            switch (i % 3) {
                case 0:
                    __builtin_memcpy(local_buf, src_buf, 
                                    g_mem_size % 128);
                    break;
                case 1:
                    __builtin_memset(local_buf, 
                                    (char)(i % 256), 
                                    g_mem_size % 128);
                    break;
                case 2:
                    __builtin_memmove(local_buf + 32, 
                                     local_buf, 
                                     g_mem_size % 96);
                    break;
            }
        }
        
        /* Thread-local memory move with goto */
        if (thread_id % 2 == 0) {
            goto do_memmove;
        }
        
        __builtin_memcpy(local_buf + 64, src_buf, 32);
        
        if (thread_id % 3 == 0) {
            skip_memmove:
            __builtin_memset(local_buf, 0, 16);
            return;
        }
        
        if (0) {
            do_memmove:
            __builtin_memmove(local_buf, local_buf + 16, 48);
            goto skip_memmove;
        }
    }
}

/* Calculate hash of AST */
static int hash_ast(ASTNode* node) {
    if (!node) return 0;
    
    int hash = node->id;
    for (int i = 0; node->data[i] != '\0'; i++) {
        hash = hash * 31 + node->data[i];
    }
    
    /* Copy data to volatile buffer */
    volatile char tmp_buf[64];
    __builtin_memcpy((char*)tmp_buf, node->data, 64);
    
    return hash + hash_ast(node->left) + hash_ast(node->right);
}

/* Free AST */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    volatile char clear_buf[64];
    __builtin_memcpy((char*)clear_buf, node->data, 64);
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create recursive structure */
    ASTNode* root = create_ast(4, 1);
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Perform parallel memory operations */
    parallel_mem_operations();
    
    /* Calculate and print verification hash */
    int result_hash = hash_ast(root);
    printf("AST hash result: %d\n", result_hash);
    
    /* Additional memory operations in main */
    volatile char main_buf[256];
    volatile char src_data[256];
    
    for (int i = 0; i < 256; i++) {
        src_data[i] = (char)(i % 128);
    }
    
    /* Test all three built-ins */
    __builtin_memcpy(main_buf, src_data, g_mem_size);
    __builtin_memset(main_buf + 128, 0xAA, g_mem_size % 128);
    
    if (g_use_memmove) {
        __builtin_memmove(main_buf + 64, main_buf, 128);
    }
    
    /* Cleanup */
    free_ast(root);
    
    printf("Test completed successfully\n");
    return 0;
}
