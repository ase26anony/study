/* asan_coverage_test.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    int depth;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_test(void) {
    printf("Constructor: Initializing ASAN test environment\n");
    g_mem_size = 256 + (rand() % 128);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_test(void) {
    printf("Destructor: Cleaning up ASAN test\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->depth = depth;
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (depth % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int branch = depth % 3;
        
        if (branch == 0) {
            node->left = create_ast(depth - 1);
            goto skip_right;
        } else if (branch == 1) {
            node->right = create_ast(depth - 1);
            goto skip_left;
        }
        
        node->left = create_ast(depth - 1);
        node->right = create_ast(depth - 1);
        
        skip_left:
        skip_right:;
    }
    
    return node;
}

/* Function with goto jumping into memory operation block */
static void test_goto_memmove(void* dest, void* src, size_t n) {
    int use_memmove = 1;
    
    if (n == 0) goto skip_op;
    
    /* Jump into the middle of memory operation */
    if (use_memmove) {
        goto do_memmove;
    }
    
    skip_op:
    return;
    
    do_memmove:
    /* This tests flow-sensitivity of asan_memfn_rtls retrieval */
    __builtin_memmove(dest, src, n);
    goto skip_op;
}

/* Parallel memory operations using OpenMP */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread gets its own buffers */
        char* buf1 = (char*)malloc(g_mem_size);
        char* buf2 = (char*)malloc(g_mem_size);
        
        if (buf1 && buf2) {
            /* Force all three built-ins in parallel context */
            __builtin_memset(buf1, thread_id, g_mem_size);
            __builtin_memset(buf2, 0xFF, g_mem_size);
            
            /* Copy with overlap to force memmove */
            size_t half = g_mem_size / 2;
            __builtin_memcpy(buf1 + half, buf1, half);
            __builtin_memmove(buf2, buf1, g_mem_size);
            
            /* Verify with another memcpy */
            char verify[256];
            if (g_mem_size <= sizeof(verify)) {
                __builtin_memcpy(verify, buf2, g_mem_size);
            }
        }
        
        free(buf1);
        free(buf2);
    }
}

/* Complex token processing with memory operations */
static unsigned long process_tokens(const char** tokens, int count) {
    unsigned long hash = 0;
    char buffer[512];
    char* current = buffer;
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Use volatile to prevent constant folding */
        volatile size_t copy_len = len;
        if (copy_len > sizeof(buffer) - (current - buffer)) {
            copy_len = sizeof(buffer) - (current - buffer);
        }
        
        if (copy_len > 0) {
            /* Mix of memcpy and memmove with goto */
            if (i % 2 == 0) {
                __builtin_memcpy(current, tokens[i], copy_len);
            } else {
                /* Create overlap scenario for memmove */
                if (current > buffer + 10) {
                    __builtin_memmove(current - 5, tokens[i], copy_len);
                    current -= 5;
                } else {
                    __builtin_memcpy(current, tokens[i], copy_len);
                }
            }
            current += copy_len;
        }
        
        /* Update hash */
        for (size_t j = 0; j < len; j++) {
            hash = hash * 31 + tokens[i][j];
        }
    }
    
    /* Final memset on buffer */
    __builtin_memset(buffer + (current - buffer), 0, 
                     sizeof(buffer) - (current - buffer));
    
    return hash;
}

/* Multi-stage initialization */
static void initialize_system(void) {
    static int initialized = 0;
    
    if (!initialized) {
        /* Force early initialization of memory built-ins */
        char init_buf[16];
        __builtin_memset(init_buf, 0, sizeof(init_buf));
        __builtin_memcpy(init_buf, "INIT", 4);
        __builtin_memmove(init_buf + 4, init_buf, 4);
        
        initialized = 1;
    }
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Stage 1: System initialization */
    initialize_system();
    
    /* Stage 2: Create recursive AST structure */
    ASTNode* root = create_ast(4);
    
    if (root) {
        /* Copy between AST nodes */
        ASTNode node_copy;
        __builtin_memcpy(&node_copy, root, sizeof(ASTNode));
        
        /* Move within AST with overlap */
        if (root->left && root->right) {
            __builtin_memmove(root->left, root->right, sizeof(ASTNode));
        }
        
        /* Test goto with memmove */
        test_goto_memmove(root->data, root->data + 10, 32);
    }
    
    /* Stage 3: Token processing */
    const char* tokens[] = {
        "MEMCPY", "MEMSET", "MEMMOVE", "ASAN", "HWASAN",
        "BUILTIN", "REDIRECT", "COVERAGE", "TEST", "COMPLETE"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    unsigned long token_hash = process_tokens(tokens, token_count);
    printf("Token hash: %lu\n", token_hash);
    
    /* Stage 4: Parallel memory operations */
    printf("Running parallel memory operations...\n");
    parallel_mem_ops();
    
    /* Stage 5: Final verification with all three built-ins */
    char final_buf[1024];
    char src_buf[1024];
    
    /* Initialize source with pattern */
    for (size_t i = 0; i < sizeof(src_buf); i++) {
        src_buf[i] = (char)(i % 256);
    }
    
    /* Exercise all three built-ins in sequence */
    __builtin_memset(final_buf, 0, sizeof(final_buf));
    __builtin_memcpy(final_buf, src_buf, sizeof(final_buf));
    __builtin_memmove(final_buf + 512, final_buf, 512);
    
    /* Calculate checksum */
    unsigned long checksum = 0;
    for (size_t i = 0; i < sizeof(final_buf); i++) {
        checksum += (unsigned char)final_buf[i];
    }
    printf("Final checksum: %lu\n", checksum);
    
    /* Cleanup */
    /* Note: In real code, we would properly free the AST tree */
    
    printf("Test completed successfully\n");
    return 0;
}
