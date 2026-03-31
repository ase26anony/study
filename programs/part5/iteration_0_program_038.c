/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    char *data;
    size_t data_len;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_early(void) {
    volatile char buffer1[128];
    volatile char buffer2[128];
    
    /* Force builtin calls in constructor context */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1, buffer2, sizeof(buffer1));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan_late(void) {
    volatile char final_buf[32];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char *base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->data_len = volatile_len / (depth + 1);
    node->data = malloc(node->data_len);
    
    /* Use all three builtins with volatile control */
    __builtin_memset(node->data, depth, node->data_len);
    
    if (depth > 1 && base_data) {
        size_t copy_len = node->data_len < strlen(base_data) ? 
                         node->data_len : strlen(base_data);
        __builtin_memcpy(node->data, base_data, copy_len);
    }
    
    /* Create children with goto-based control flow */
    node->left = NULL;
    node->right = NULL;
    
    if (depth > 1) {
        /* Jump into memory operation block */
        goto create_left;
        
        create_left:
        node->left = create_ast(depth - 1, node->data);
        
        /* Jump out and back in */
        if (volatile_flag) {
            goto create_right;
        }
        
        create_right:
        node->right = create_ast(depth - 2, node->data);
        
        /* Copy between nodes */
        if (node->left && node->right) {
            size_t min_len = node->left->data_len < node->right->data_len ?
                           node->left->data_len : node->right->data_len;
            __builtin_memmove(node->right->data, node->left->data, min_len);
        }
    }
    
    return node;
}

/* Parallel memory operation dispatcher */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[256];
        char shared_buf[256];
        
        /* Each thread uses all three builtins */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp master
        {
            __builtin_memset(shared_buf, 0xCC, sizeof(shared_buf));
        }
        
        #pragma omp barrier
        
        __builtin_memcpy(local_buf + 128, shared_buf, 128);
        
        /* Conditional memmove with goto */
        if (thread_id % 2 == 0) {
            goto do_memmove;
        } else {
            goto skip_memmove;
        }
        
        do_memmove:
        __builtin_memmove(local_buf, local_buf + 64, 64);
        goto continue_exec;
        
        skip_memmove:
        /* Alternative path */
        continue_exec:;
        
        #pragma omp barrier
        
        /* Final overlapping copy */
        __builtin_memmove(local_buf + 32, local_buf, 96);
    }
}

/* Complex initialization with token processing */
static unsigned long process_tokens(const char **tokens, int count) {
    unsigned long hash = 0xDEADBEEF;
    char accum[512] = {0};
    size_t accum_pos = 0;
    
    for (int i = 0; i < count; i++) {
        size_t token_len = strlen(tokens[i]);
        
        /* Use builtins with volatile length */
        size_t copy_len = token_len < (volatile_len / 4) ? 
                         token_len : (volatile_len / 4);
        
        __builtin_memcpy(accum + accum_pos, tokens[i], copy_len);
        accum_pos += copy_len;
        
        /* Zero out remainder with memset */
        if (copy_len < token_len) {
            __builtin_memset(accum + accum_pos, 0, token_len - copy_len);
        }
        
        /* Move data around */
        if (i % 3 == 0 && accum_pos > 32) {
            __builtin_memmove(accum, accum + 16, accum_pos - 16);
            accum_pos -= 16;
        }
    }
    
    /* Compute hash from accumulated data */
    for (size_t i = 0; i < accum_pos; i++) {
        hash = (hash * 31) + accum[i];
    }
    
    return hash;
}

int main(void) {
    const char *tokens[] = {
        "ASAN", "HWASAN", "MEMCPY", "MEMSET", "MEMMOVE",
        "REDZONE", "INSTRUMENT", "BUILTIN", "COVERAGE"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    printf("Starting ASAN/HWASAN builtin redirection test...\n");
    
    /* Phase 1: Token processing */
    unsigned long token_hash = process_tokens(tokens, token_count);
    printf("Token hash: 0x%08lx\n", token_hash);
    
    /* Phase 2: AST creation with recursive memory ops */
    ASTNode *root = create_ast(5, "ROOT_DATA");
    
    /* Phase 3: Parallel memory operations */
    parallel_mem_ops();
    
    /* Phase 4: Cleanup with final builtin calls */
    char final_buffer[1024];
    char source_buffer[1024];
    
    __builtin_memset(source_buffer, 0x55, sizeof(source_buffer));
    __builtin_memcpy(final_buffer, source_buffer, sizeof(source_buffer));
    
    /* Overlapping move */
    __builtin_memmove(final_buffer + 256, final_buffer, 512);
    
    /* Verify by computing checksum */
    unsigned long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += final_buffer[i];
    }
    
    printf("Final checksum: %lu\n", checksum);
    printf("Token hash + checksum: %lu\n", token_hash + checksum);
    
    /* Cleanup */
    /* Note: Real AST cleanup omitted for brevity */
    
    return 0;
}
