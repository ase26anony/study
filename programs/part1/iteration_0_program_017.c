/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[32];
    uint32_t hash;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[16];
    /* Force memcpy built-in in constructor */
    __builtin_memcpy(buffer, "constructor", 11);
    g_use_hwasan = (getenv("USE_HWASAN") != NULL);
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    volatile char buf[8];
    __builtin_memset(buf, 0, sizeof(buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with builtin memcpy */
    size_t len = strlen(base_data);
    if (len > 31) len = 31;
    __builtin_memcpy(node->data, base_data, len);
    node->data[len] = '\0';
    
    /* Create children with goto for flow control */
    if (depth > 1) {
        char child_data[32];
        snprintf(child_data, sizeof(child_data), "%s_L", base_data);
        
        /* Jump label for flow sensitivity test */
        create_left:
        node->left = create_ast(depth - 1, child_data);
        
        snprintf(child_data, sizeof(child_data), "%s_R", base_data);
        node->right = create_ast(depth - 1, child_data);
        
        /* Copy between nodes using memmove */
        if (node->left && node->right) {
            volatile size_t copy_len = 16;
            if (copy_len > 31) copy_len = 31;
            
            /* Test memmove built-in with goto jumping into block */
            if (depth == 3) goto do_memmove;
            
            continue_normal:
            __builtin_memmove(node->right->data + 8, 
                            node->left->data, 
                            copy_len);
        }
    }
    
    /* Compute hash */
    uint32_t hash = 0;
    for (int i = 0; i < 32 && node->data[i]; i++) {
        hash = (hash * 31) + node->data[i];
    }
    node->hash = hash;
    
    return node;

do_memmove:
    /* This goto target tests flow sensitivity */
    volatile char temp[32];
    __builtin_memcpy(temp, node->left->data, 16);
    __builtin_memmove(node->left->data, node->right->data, 16);
    __builtin_memmove(node->right->data, temp, 16);
    goto continue_normal;
}

/* Parallel memory operations */
static void parallel_mem_operations(void) {
    const int num_buffers = 8;
    char* buffers[num_buffers];
    volatile size_t sizes[num_buffers];
    
    /* Initialize sizes with volatile to prevent folding */
    for (int i = 0; i < num_buffers; i++) {
        sizes[i] = g_mem_size + i * 8;
        buffers[i] = (char*)malloc(sizes[i]);
        if (!buffers[i]) continue;
        
        /* Use builtin memset */
        __builtin_memset(buffers[i], i + 'A', sizes[i]);
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < num_buffers - 1; i++) {
            if (buffers[i] && buffers[i + 1]) {
                size_t copy_size = sizes[i];
                if (copy_size > sizes[i + 1]) 
                    copy_size = sizes[i + 1];
                
                /* Mix memcpy and memmove in parallel region */
                if (thread_id % 2 == 0) {
                    __builtin_memcpy(buffers[i + 1], buffers[i], copy_size);
                } else {
                    __builtin_memmove(buffers[i], buffers[i + 1], copy_size);
                }
                
                /* Additional memset in parallel */
                __builtin_memset(buffers[i] + copy_size / 2, 
                               'Z', 
                               copy_size / 4);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_buffers; i++) {
        if (buffers[i]) free(buffers[i]);
    }
}

/* Complex token processing with memory builtins */
static uint64_t process_tokens(const char** tokens, int count) {
    uint64_t total_hash = 0;
    char combined[256];
    volatile int offset = 0;
    
    for (int i = 0; i < count; i++) {
        size_t token_len = strlen(tokens[i]);
        volatile size_t copy_len = token_len;
        
        if (copy_len > 255 - offset) 
            copy_len = 255 - offset;
        
        /* Use all three builtins in sequence */
        if (i % 3 == 0) {
            __builtin_memcpy(combined + offset, tokens[i], copy_len);
        } else if (i % 3 == 1) {
            __builtin_memmove(combined + offset, tokens[i], copy_len);
        } else {
            /* Clear section then copy */
            __builtin_memset(combined + offset, 0, copy_len);
            __builtin_memcpy(combined + offset, tokens[i], copy_len);
        }
        
        offset += copy_len;
        
        /* Hash computation */
        for (size_t j = 0; j < copy_len; j++) {
            total_hash = (total_hash * 6364136223846793005ULL) + 
                        (uint64_t)combined[offset - copy_len + j];
        }
    }
    
    return total_hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: AST operations */
    ASTNode* root = create_ast(4, "root");
    
    /* Phase 2: Parallel memory operations */
    parallel_mem_operations();
    
    /* Phase 3: Token processing */
    const char* tokens[] = {
        "memcpy", "memset", "memmove",
        "asan", "hwasan", "instrumentation",
        "redzone", "shadow", "memory"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    uint64_t hash_result = process_tokens(tokens, token_count);
    
    /* Phase 4: Complex flow with gotos and builtins */
    volatile char final_buffer[128];
    volatile int use_memmove = 1;
    
    label_retry:
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    
    if (use_memmove) {
        char source[] = "Final verification string for ASAN coverage";
        __builtin_memmove(final_buffer, source, sizeof(source) - 1);
        use_memmove = 0;
        goto label_retry;  /* Test goto jumping back */
    } else {
        __builtin_memcpy(final_buffer, "Alternative path", 16);
    }
    
    /* Verify and output results */
    uint64_t final_hash = 0;
    for (int i = 0; i < 128; i++) {
        final_hash ^= (uint64_t)final_buffer[i] << ((i % 8) * 8);
    }
    
    printf("AST root hash: %u\n", root ? root->hash : 0);
    printf("Token hash: %llu\n", (unsigned long long)hash_result);
    printf("Final buffer hash: %llu\n", (unsigned long long)final_hash);
    printf("Total hash: %llu\n", 
           (unsigned long long)(hash_result ^ final_hash ^ (root ? root->hash : 0)));
    
    /* Cleanup AST */
    /* Simple recursive free - in real code would need proper traversal */
    if (root) free(root);
    
    return 0;
}
