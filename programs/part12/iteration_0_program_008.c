/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Recursive AST-like structure */
struct ASTNode {
    int type;
    char data[256];
    struct ASTNode *left;
    struct ASTNode *right;
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buf1[256] = {0};
    volatile char buf2[256] = {0};
    
    /* Force builtin initialization in constructor context */
    __builtin_memcpy((void*)buf1, "constructor_init", 16);
    __builtin_memset((void*)buf2, 0xAA, (size_t)g_memset_len);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    volatile char buf[128];
    __builtin_memset((void*)buf, 0xFF, 128);
}

/* Recursive function with memory operations */
static struct ASTNode* build_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    struct ASTNode* node = malloc(sizeof(struct ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile lengths */
    __builtin_memset(node, 0, sizeof(struct ASTNode));
    __builtin_memcpy(node->data, base_data, strlen(base_data) + 1);
    
    node->type = depth;
    
    /* Recursive construction */
    char left_data[256];
    char right_data[256];
    __builtin_snprintf(left_data, sizeof(left_data), "%s_L%d", base_data, depth);
    __builtin_snprintf(right_data, sizeof(right_data), "%s_R%d", base_data, depth);
    
    node->left = build_ast(depth - 1, left_data);
    node->right = build_ast(depth - 1, right_data);
    
    return node;
}

/* Function with goto jumps around memmove */
static void test_goto_memmove(void* dst, void* src, size_t len) {
    int use_memmove = 1;
    
    if (len > 100) {
        goto skip_memmove;
    }
    
memmove_block:
    /* This memmove should be instrumented */
    __builtin_memmove(dst, src, len);
    goto after_memmove;
    
skip_memmove:
    if (use_memmove) {
        use_memmove = 0;
        goto memmove_block;
    }
    
after_memmove:
    return;
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[512];
        char shared_buf[512];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp master
        {
            __builtin_memset(shared_buf, 0xCC, sizeof(shared_buf));
        }
        
        #pragma omp barrier
        
        /* Mixed memory operations */
        __builtin_memcpy(local_buf + 256, shared_buf, 256);
        __builtin_memmove(local_buf, local_buf + 128, 128);
    }
}

/* Complex token processing */
static unsigned long process_tokens(const char** tokens, int count) {
    unsigned long hash = 0xDEADBEEF;
    char buffer[1024];
    char* ptr = buffer;
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Force different builtin usage patterns */
        if (i % 3 == 0) {
            __builtin_memcpy(ptr, tokens[i], len);
        } else if (i % 3 == 1) {
            __builtin_memset(ptr, tokens[i][0], len);
        } else {
            if (ptr > buffer) {
                __builtin_memmove(ptr, ptr - 64, 64);
            }
            __builtin_memcpy(ptr + 64, tokens[i], len);
        }
        
        /* Update hash */
        for (size_t j = 0; j < len; j++) {
            hash = (hash * 31) + ptr[j];
        }
        
        ptr += len + 1;
    }
    
    return hash;
}

int main(void) {
    const char* tokens[] = {
        "ASAN_TEST", "MEMCPY_FLOW", "MEMSET_PATTERN",
        "MEMMOVE_EDGE", "GOTO_CASE", "OPENMP_PARALLEL",
        "RECURSIVE_AST", "VOLATILE_LEN", "BUILTIN_REDIRECT"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* 1. Process tokens with various builtins */
    unsigned long hash1 = process_tokens(tokens, token_count);
    
    /* 2. Build recursive AST */
    struct ASTNode* root = build_ast(4, "ROOT");
    
    if (root) {
        /* Copy between AST nodes */
        char node_copy[256];
        __builtin_memcpy(node_copy, root->data, sizeof(node_copy));
        
        if (root->left && root->right) {
            __builtin_memmove(root->left->data, root->right->data, 128);
        }
        
        /* Test goto with memmove */
        test_goto_memmove(root->data, node_copy, (size_t)g_memmove_len);
    }
    
    /* 3. Parallel memory operations */
    parallel_memory_ops();
    
    /* 4. Additional builtin calls with volatile lengths */
    volatile char final_buf[1024];
    volatile char src_buf[1024];
    
    __builtin_memset(src_buf, 0x55, sizeof(src_buf));
    __builtin_memcpy((void*)final_buf, (void*)src_buf, (size_t)g_memcpy_len);
    __builtin_memmove((void*)final_buf + 512, (void*)final_buf, 256);
    
    /* 5. Compute final hash */
    unsigned long final_hash = hash1;
    for (int i = 0; i < 256; i++) {
        final_hash ^= ((unsigned long)final_buf[i] << (i % 56));
    }
    
    printf("Final hash: 0x%016lX\n", final_hash);
    
    /* Cleanup */
    /* ... recursive free would go here ... */
    
    return (final_hash != 0) ? 0 : 1;
}
