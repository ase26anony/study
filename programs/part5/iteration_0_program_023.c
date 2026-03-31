/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 256;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 192;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char *data;
    size_t data_len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[64];
    /* Force builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 32, buffer, 32);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    volatile char final_check[16];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = depth;
    node->data_len = g_memcpy_len % 256;
    node->data = malloc(node->data_len + 1);
    
    if (node->data) {
        /* Use builtins with volatile-controlled lengths */
        __builtin_memset(node->data, 0, node->data_len);
        __builtin_memcpy(node->data, base_data, 
                        node->data_len < strlen(base_data) ? 
                        node->data_len : strlen(base_data));
        node->data[node->data_len] = '\0';
    }
    
    /* Create children with goto-based control flow */
    node->left = NULL;
    node->right = NULL;
    
    if (depth > 1) {
        /* Jump into memory operation block */
        goto create_left;
        
        create_left:
        node->left = create_ast(depth - 1, base_data);
        
        /* Jump around memory operation */
        if (depth % 2) goto skip_right;
        
        node->right = create_ast(depth - 1, base_data);
        goto after_children;
        
        skip_right:
        /* Still need memmove with goto */
        volatile char temp[64];
        __builtin_memmove(temp, node->data, 
                         node->data_len < 64 ? node->data_len : 64);
        
        after_children:;
    }
    
    return node;
}

/* Process AST with memory operations */
static size_t process_ast(ASTNode *node, char *output, size_t out_len) {
    if (!node || !output || !out_len) return 0;
    
    size_t total = 0;
    
    /* Copy node data to output with builtin */
    size_t copy_len = node->data_len < out_len ? node->data_len : out_len;
    if (copy_len > 0) {
        __builtin_memcpy(output, node->data, copy_len);
        total += copy_len;
    }
    
    /* Process children */
    if (node->left) {
        total += process_ast(node->left, output + total, out_len - total);
    }
    
    /* Memmove between buffers */
    if (total > 32 && node->right) {
        volatile char shift_buffer[256];
        __builtin_memmove(shift_buffer, output, total < 256 ? total : 256);
    }
    
    if (node->right) {
        total += process_ast(node->right, output + total, out_len - total);
    }
    
    return total;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        volatile char thread_buf[512];
        
        /* Each thread uses all three builtins */
        __builtin_memset(thread_buf, tid, sizeof(thread_buf));
        
        #pragma omp barrier
        
        /* Memcpy between thread buffers with volatile lengths */
        volatile size_t len = g_memcpy_len % 512;
        if (len > 0) {
            __builtin_memcpy(thread_buf + 256, thread_buf, len);
        }
        
        /* Memmove with goto */
        if (tid % 2 == 0) {
            goto do_memmove;
            
            do_memmove:
            __builtin_memmove(thread_buf, thread_buf + 128, 
                             g_memmove_len % 384);
        }
        
        /* Memset again */
        __builtin_memset(thread_buf + 384, 0xFF, 128);
    }
}

/* Complex token array initialization */
static void init_token_array(char tokens[][64], size_t count) {
    for (size_t i = 0; i < count; i++) {
        /* Patterned initialization */
        __builtin_memset(tokens[i], (int)(i % 256), 64);
        
        /* Copy patterns between tokens */
        if (i > 0) {
            __builtin_memcpy(tokens[i] + 32, tokens[i-1], 32);
        }
        
        /* Move data around */
        if (i % 3 == 0) {
            __builtin_memmove(tokens[i], tokens[i] + 16, 48);
        }
    }
}

int main(void) {
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* 1. Initialize token array */
    char tokens[8][64];
    init_token_array(tokens, 8);
    
    /* 2. Create recursive AST */
    ASTNode *root = create_ast(4, "BaseDataForAST");
    
    /* 3. Process AST */
    char output[1024];
    __builtin_memset(output, 0, sizeof(output));
    size_t processed = process_ast(root, output, sizeof(output));
    
    /* 4. Execute parallel memory operations */
    parallel_memory_ops();
    
    /* 5. Additional builtin calls in main */
    volatile char main_buf[1024];
    volatile size_t len = g_memset_len;
    
    __builtin_memset(main_buf, 0xCC, len % 1024);
    __builtin_memcpy(main_buf + 512, main_buf, 256);
    __builtin_memmove(main_buf, main_buf + 256, 512);
    
    /* 6. Compute verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < processed && i < sizeof(output); i++) {
        hash = (hash * 31) + (unsigned char)output[i];
    }
    
    /* Mix in token data */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 64; j++) {
            hash ^= (hash << 5) + (unsigned char)tokens[i][j];
        }
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Processed %zu bytes from AST\n", processed);
    
    /* Cleanup */
    /* Note: AST cleanup omitted for brevity - would need recursive free */
    
    return 0;
}
