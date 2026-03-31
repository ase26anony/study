/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int volatile_len = 64;
static volatile char volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char token_pool[4096];
static int token_index = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_hook(void) {
    /* Force early initialization of memory functions */
    char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
    __builtin_memcpy(buffer, "init", 5);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_sanitizer_hook(void) {
    /* Final memory operations */
    char final_buf[16];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with builtin memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data)) copy_len = sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_len);
    
    node->id = depth;
    
    /* Create children with goto-controlled flow */
    if (depth > 1) {
        char child_data[64];
        __builtin_memset(child_data, 0, sizeof(child_data));
        snprintf(child_data, sizeof(child_data), "%s-%d", base_data, depth);
        
        /* Use goto to create unusual control flow */
        create_left:
        node->left = create_ast(depth - 1, child_data);
        
        /* Jump over right creation in some cases */
        if (volatile_flag) {
            goto skip_right;
        }
        
        create_right:
        node->right = create_ast(depth - 1, child_data);
        goto done;
        
        skip_right:
        node->right = NULL;
        goto create_right;  /* Jump back to create right */
        
        done:;
    } else {
        node->left = node->right = NULL;
    }
    
    return node;
}

/* Memory operation between AST nodes */
static void copy_ast_data(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use builtin memmove for overlapping regions */
    size_t len = volatile_len % sizeof(dest->data);
    __builtin_memmove(dest->data, src->data, len);
    
    /* Also copy using memcpy */
    __builtin_memcpy(dest->data + len, src->data + len, 
                    sizeof(dest->data) - len);
}

/* Process tokens with memory operations */
static void process_tokens(void) {
    char local_buf[512];
    int i;
    
    /* Initialize with memset */
    __builtin_memset(local_buf, 0, sizeof(local_buf));
    
    /* Fill with pattern using memcpy */
    for (i = 0; i < sizeof(local_buf) - 32; i += 32) {
        __builtin_memcpy(local_buf + i, "TOKEN_", 7);
    }
    
    /* Use memmove for overlapping copy */
    __builtin_memmove(local_buf + 16, local_buf, 256);
    
    /* Copy to global pool */
    __builtin_memcpy(token_pool + token_index, local_buf, 
                    sizeof(local_buf));
    token_index += sizeof(local_buf);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        char thread_buf[128];
        
        /* Each thread uses builtin memory functions */
        __builtin_memset(thread_buf, thread_id, sizeof(thread_buf));
        
        /* Conditional memcpy based on thread ID */
        if (thread_id % 2 == 0) {
            __builtin_memcpy(thread_buf + 64, "EVEN", 5);
        } else {
            __builtin_memcpy(thread_buf + 64, "ODD", 4);
        }
        
        /* Use memmove for intra-buffer operations */
        __builtin_memmove(thread_buf, thread_buf + 32, 64);
        
        #pragma omp critical
        {
            /* Copy to shared pool */
            size_t copy_size = sizeof(thread_buf);
            if (token_index + copy_size < sizeof(token_pool)) {
                __builtin_memcpy(token_pool + token_index, 
                               thread_buf, copy_size);
                token_index += copy_size;
            }
        }
    }
}

/* Complex memory dispatch with goto */
static void memory_dispatch(void) {
    char buffer_a[256];
    char buffer_b[256];
    char buffer_c[256];
    
    __builtin_memset(buffer_a, 'A', sizeof(buffer_a));
    __builtin_memset(buffer_b, 'B', sizeof(buffer_b));
    __builtin_memset(buffer_c, 'C', sizeof(buffer_c));
    
    /* Goto-based state machine for memory operations */
    int state = 0;
    
    dispatch_start:
    switch(state) {
        case 0:
            __builtin_memcpy(buffer_a, buffer_b, 128);
            state = 1;
            goto dispatch_start;
            
        case 1:
            __builtin_memmove(buffer_b + 64, buffer_b, 128);
            state = 2;
            goto dispatch_start;
            
        case 2:
            __builtin_memset(buffer_c, 0, 64);
            state = 3;
            goto dispatch_start;
            
        case 3:
            /* Overlapping copy with memmove */
            __builtin_memmove(buffer_a, buffer_a + 32, 192);
            state = 4;
            /* Fall through */
            
        case 4:
            /* Final memcpy */
            __builtin_memcpy(buffer_b, buffer_c, 64);
            break;
            
        default:
            __builtin_memset(buffer_a, 0xFF, sizeof(buffer_a));
    }
}

/* Calculate hash of token pool */
static unsigned long calculate_hash(void) {
    unsigned long hash = 5381;
    int i;
    
    for (i = 0; i < token_index; i++) {
        hash = ((hash << 5) + hash) + token_pool[i];
    }
    
    return hash;
}

int main(void) {
    ASTNode* root = NULL;
    ASTNode* copy = NULL;
    unsigned long final_hash;
    
    printf("Starting ASAN builtin redirection test...\n");
    
    /* Phase 1: Initialize and create AST */
    root = create_ast(4, "ROOT");
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    /* Phase 2: Process tokens */
    process_tokens();
    
    /* Phase 3: Parallel memory operations */
    #ifdef _OPENMP
    parallel_memory_ops();
    #endif
    
    /* Phase 4: Create copy and perform memory operations */
    copy = create_ast(3, "COPY");
    if (copy) {
        copy_ast_data(copy, root);
        
        /* Additional memory dispatch */
        memory_dispatch();
        
        free(copy);
    }
    
    /* Phase 5: More token processing */
    for (int i = 0; i < 3; i++) {
        process_tokens();
    }
    
    /* Calculate and print result */
    final_hash = calculate_hash();
    printf("Final hash: %lu\n", final_hash);
    printf("Token pool size: %d bytes\n", token_index);
    
    /* Cleanup */
    free(root);
    
    return 0;
}
