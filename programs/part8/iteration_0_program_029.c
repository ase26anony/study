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
struct ast_node {
    int type;
    char data[256];
    struct ast_node *left;
    struct ast_node *right;
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    volatile char buf1[256], buf2[256];
    
    /* Force __builtin_memcpy redirection in constructor */
    __builtin_memcpy(buf1, "constructor_data", 17);
    __builtin_memset(buf2, 0xCC, 32);
    
    printf("[constructor] Initialized ASAN buffers\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_destructor(void) {
    volatile char cleanup_buf[128];
    
    /* Force __builtin_memset redirection in destructor */
    __builtin_memset(cleanup_buf, 0xFF, 64);
    
    printf("[destructor] Cleaned up ASAN buffers\n");
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, int max_depth) {
    if (depth >= max_depth) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, 0, sizeof(struct ast_node));
    node->type = depth;
    
    /* Fill data with __builtin_memcpy */
    char pattern[32];
    __builtin_memset(pattern, 'A' + depth, 31);
    pattern[31] = '\0';
    __builtin_memcpy(node->data, pattern, 32);
    
    /* Recursive construction */
    node->left = build_ast(depth + 1, max_depth);
    node->right = build_ast(depth + 2, max_depth);
    
    return node;
}

/* Function with goto statements around __builtin_memmove */
static void memmove_with_goto(char* dest, char* src, size_t len) {
    int use_memmove = 1;
    
    if (len == 0) goto skip_operation;
    
    /* Jump into memory operation block */
    goto perform_op;
    
perform_op:
    if (use_memmove) {
        /* Force __builtin_memmove redirection */
        __builtin_memmove(dest, src, len);
        goto operation_done;
    }
    
skip_operation:
    __builtin_memset(dest, 0, len);
    
operation_done:
    /* Jump out of block */
    goto finalize;
    
finalize:
    return;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[512];
        char shared_buf[512];
        
        /* Each thread uses __builtin_memset */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp single
        {
            /* Master thread uses __builtin_memcpy */
            __builtin_memcpy(shared_buf, "shared_data", 12);
        }
        
        #pragma omp barrier
        
        /* All threads use __builtin_memmove */
        __builtin_memmove(local_buf + 256, shared_buf, 12);
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Basic built-in calls with volatile lengths */
    {
        char buf1[256], buf2[256];
        
        /* Force all three built-ins */
        __builtin_memset(buf1, 0xAA, g_memset_len);
        __builtin_memcpy(buf2, buf1, g_memcpy_len);
        __builtin_memmove(buf1 + 64, buf2, g_memmove_len);
        
        printf("Phase 1: Basic built-ins completed\n");
    }
    
    /* Phase 2: Recursive AST operations */
    {
        struct ast_node* root = build_ast(0, 4);
        if (root) {
            /* Copy between AST nodes */
            struct ast_node* copy = malloc(sizeof(struct ast_node));
            if (copy) {
                __builtin_memcpy(copy, root, sizeof(struct ast_node));
                
                /* Recursive copy of subtree */
                if (root->left && copy->left) {
                    __builtin_memcpy(copy->left, root->left, sizeof(struct ast_node));
                }
                
                free(copy);
            }
            
            /* TODO: Add proper AST cleanup */
            free(root);
        }
        printf("Phase 2: AST operations completed\n");
    }
    
    /* Phase 3: Goto flow control */
    {
        char src[128], dest[128];
        __builtin_memset(src, 'X', sizeof(src));
        
        memmove_with_goto(dest, src, 64);
        memmove_with_goto(dest + 64, src, 0); /* Edge case */
        
        printf("Phase 3: Goto flow control completed\n");
    }
    
    /* Phase 4: OpenMP parallel operations */
    {
        parallel_memory_ops();
        printf("Phase 4: OpenMP parallel operations completed\n");
    }
    
    /* Phase 5: Complex multi-stage interaction */
    {
        volatile int stage = 0;
        char final_buf[1024];
        
        for (stage = 0; stage < 3; stage++) {
            char temp[512];
            
            switch (stage) {
                case 0:
                    __builtin_memset(final_buf, stage, 256);
                    break;
                case 1:
                    __builtin_memcpy(temp, final_buf, 256);
                    __builtin_memmove(final_buf + 256, temp, 256);
                    break;
                case 2:
                    __builtin_memset(final_buf + 512, 0xFF, 512);
                    break;
            }
        }
        
        /* Compute verification hash */
        unsigned long hash = 0;
        for (size_t i = 0; i < sizeof(final_buf); i++) {
            hash += (unsigned long)final_buf[i];
        }
        printf("Phase 5: Verification hash = 0x%lx\n", hash);
    }
    
    printf("ASAN/HWASAN built-in redirection test completed successfully\n");
    return 0;
}
