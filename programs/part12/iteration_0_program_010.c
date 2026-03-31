/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int depth;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char buffer[32];
    /* Force memcpy built-in in constructor */
    __builtin_memcpy(buffer, "CONSTRUCTOR_INIT", 16);
    g_use_hwasan = (buffer[0] == 'C') ? 1 : 0;
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[64];
    /* Force memset built-in in destructor */
    __builtin_memset(cleanup_buf, 0, sizeof(cleanup_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for node initialization */
    __builtin_memcpy(node->data, base_data, strlen(base_data) + 1);
    node->depth = depth;
    
    /* Create children with goto-controlled flow */
    int create_left = 1;
    
    if (depth > 2) {
        goto skip_left;
    }
    
    create_left = 0;
    
skip_left:
    if (create_left) {
        char child_data[256];
        volatile size_t copy_len = g_mem_size % 128;
        __builtin_memcpy(child_data, "LEFT_CHILD_", 11);
        __builtin_memcpy(child_data + 11, base_data, copy_len);
        node->left = create_ast(depth - 1, child_data);
    } else {
        node->left = NULL;
    }
    
    /* Use goto to jump into memmove block */
    if (depth % 3 == 0) {
        goto memmove_block;
    }
    
    char temp_data[256];
    __builtin_memset(temp_data, 'X', sizeof(temp_data));
    node->right = create_ast(depth - 2, temp_data);
    goto after_memmove;
    
memmove_block:
    {
        volatile char src[128], dst[128];
        __builtin_memset(src, 'M', sizeof(src));
        /* Force __builtin_memmove with overlapping regions */
        __builtin_memmove(dst, src, sizeof(src));
        __builtin_memmove(src + 32, src, 64);  /* Overlapping move */
        __builtin_memcpy(temp_data, dst, 64);
    }
    node->right = create_ast(depth - 1, temp_data);
    
after_memmove:
    return node;
}

/* Function with OpenMP parallel section */
static void parallel_memory_operations(void) {
    volatile size_t local_size = g_mem_size;
    char* buffers[8];
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < 8; i++) {
            buffers[i] = (char*)malloc(local_size);
            if (buffers[i]) {
                /* Mix memcpy and memset in parallel */
                if (i % 3 == 0) {
                    __builtin_memset(buffers[i], tid + '0', local_size);
                } else if (i % 3 == 1) {
                    char pattern[32];
                    __builtin_memset(pattern, 'A' + tid, sizeof(pattern));
                    __builtin_memcpy(buffers[i], pattern, 
                                   local_size < 32 ? local_size : 32);
                } else {
                    /* Use memmove for self-copy */
                    __builtin_memmove(buffers[i], buffers[i] + 16, 
                                    local_size > 16 ? local_size - 16 : 1);
                }
            }
        }
        
        #pragma omp barrier
        
        /* Cross-thread memory operations */
        #pragma omp single
        {
            for (int i = 0; i < 7; i++) {
                if (buffers[i] && buffers[i+1]) {
                    volatile size_t copy_len = (local_size * (i + 1)) % 128;
                    __builtin_memcpy(buffers[i+1], buffers[i], copy_len);
                }
            }
        }
        
        #pragma omp for
        for (int i = 0; i < 8; i++) {
            if (buffers[i]) {
                free(buffers[i]);
            }
        }
    }
}

/* Complex token processing with goto jumps */
static int process_tokens(char** tokens, int count) {
    int result = 0;
    char combined[512];
    volatile int use_memmove = 0;
    
    __builtin_memset(combined, 0, sizeof(combined));
    
    for (int i = 0; i < count; i++) {
        if (tokens[i] == NULL) continue;
        
        /* Jump to different memory operation based on condition */
        if (i % 4 == 0) {
            goto do_memcpy;
        } else if (i % 4 == 1) {
            goto do_memset;
        } else if (i % 4 == 2) {
            use_memmove = 1;
            goto do_memmove;
        } else {
            /* Default path */
            size_t len = strlen(tokens[i]);
            if (len > 0) {
                __builtin_memcpy(combined + result, tokens[i], len);
                result += len;
            }
            continue;
        }
        
    do_memcpy:
        {
            size_t len = strlen(tokens[i]);
            volatile size_t copy_len = len % 256;
            __builtin_memcpy(combined + result, tokens[i], copy_len);
            result += copy_len;
        }
        continue;
        
    do_memset:
        {
            int pattern = (i * 17) % 256;
            volatile size_t set_len = (result + 32) % sizeof(combined);
            __builtin_memset(combined + result, pattern, set_len);
            result += set_len;
        }
        continue;
        
    do_memmove:
        if (use_memmove && result > 64) {
            /* Overlapping move within buffer */
            __builtin_memmove(combined + 32, combined, 64);
            use_memmove = 0;
        }
        continue;
    }
    
    return result;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST operations */
    ASTNode* root = create_ast(5, "ROOT_NODE");
    
    if (root) {
        /* Copy between AST nodes */
        char temp_buffer[512];
        volatile size_t copy_size = g_mem_size;
        
        __builtin_memcpy(temp_buffer, root->data, 
                        copy_size < sizeof(root->data) ? copy_size : sizeof(root->data));
        
        if (root->left) {
            __builtin_memcpy(root->left->data, temp_buffer, 128);
        }
        
        if (root->right) {
            __builtin_memmove(root->right->data, root->data, 64);
        }
        
        /* Recursive cleanup */
        void cleanup_ast(ASTNode* node) {
            if (!node) return;
            cleanup_ast(node->left);
            cleanup_ast(node->right);
            
            /* Use memset before free */
            volatile char clear_pattern = 0;
            __builtin_memset(node->data, clear_pattern, sizeof(node->data));
            free(node);
        }
        
        cleanup_ast(root);
    }
    
    /* Phase 2: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 3: Token processing with control flow */
    char* tokens[] = {
        "TOKEN_ALPHA",
        "TOKEN_BETA",
        "TOKEN_GAMMA",
        "TOKEN_DELTA",
        "TOKEN_EPSILON",
        "TOKEN_ZETA",
        "TOKEN_ETA",
        "TOKEN_THETA"
    };
    
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    int processed = process_tokens(tokens, token_count);
    
    /* Final verification with mixed built-ins */
    volatile char final_buf[1024];
    volatile char src_buf[1024];
    
    __builtin_memset(src_buf, 'S', sizeof(src_buf));
    __builtin_memcpy(final_buf, src_buf, g_mem_size);
    __builtin_memmove(final_buf + 256, final_buf, 512);
    
    /* Calculate simple hash for verification */
    unsigned int hash = 0;
    for (size_t i = 0; i < sizeof(final_buf); i++) {
        hash = (hash * 31) + final_buf[i];
    }
    
    printf("Test completed. Processed %d token bytes. Final hash: %u\n", 
           processed, hash);
    
    return (hash != 0) ? 0 : 1;
}
