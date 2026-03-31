/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 64;
volatile int volatile_switch = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buf[32];
    /* Force __builtin_memset in constructor */
    __builtin_memset(buf, 0xAA, sizeof(buf));
    printf("[Constructor] Initialized buffer\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    volatile char buf[16];
    /* Force __builtin_memcpy in destructor */
    char src[] = "DESTRUCTOR";
    __builtin_memcpy(buf, src, sizeof(src));
    printf("[Destructor] Cleanup completed\n");
}

/* Function with goto jumping into memmove block */
static void goto_memmove_test(char* dest, char* src, size_t n) {
    int use_memmove = volatile_switch;
    
    if (use_memmove) {
        goto do_memmove;
    } else {
        goto skip_memmove;
    }
    
do_memmove:
    /* This block should be reached via goto */
    __builtin_memmove(dest, src, n);
    goto end;
    
skip_memmove:
    __builtin_memcpy(dest, src, n);
    
end:
    return;
}

/* Recursive function copying between AST nodes */
static void recursive_ast_copy(ASTNode* dest, ASTNode* src, int depth) {
    if (depth <= 0 || !dest || !src) return;
    
    /* Copy node data using builtins */
    __builtin_memcpy(dest->data, src->data, sizeof(dest->data));
    
    /* Recursive calls */
    if (dest->left && src->left) {
        recursive_ast_copy(dest->left, src->left, depth - 1);
    }
    
    if (dest->right && src->right) {
        recursive_ast_copy(dest->right, src->right, depth - 1);
    }
}

/* OpenMP parallel memory operations */
static void parallel_mem_ops(void) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char local_buf[128];
        char shared_buf[128];
        
        /* Each thread uses builtins */
        __builtin_memset(local_buf, tid, sizeof(local_buf));
        
        #pragma omp barrier
        
        #pragma omp critical
        {
            __builtin_memcpy(shared_buf, local_buf, sizeof(local_buf));
            __builtin_memset(local_buf, 0, sizeof(local_buf));
            __builtin_memcpy(local_buf, shared_buf, sizeof(shared_buf));
        }
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Direct builtin calls with volatile lengths */
    char buffer1[256];
    char buffer2[256];
    size_t len = volatile_len;
    
    __builtin_memset(buffer1, 0xCC, len);
    __builtin_memcpy(buffer2, buffer1, len);
    
    /* Phase 2: Goto-controlled memmove */
    goto_memmove_test(buffer1, buffer2, len / 2);
    
    /* Phase 3: AST structure operations */
    ASTNode* node1 = calloc(1, sizeof(ASTNode));
    ASTNode* node2 = calloc(1, sizeof(ASTNode));
    
    if (node1 && node2) {
        __builtin_memset(node1->data, 'A', sizeof(node1->data));
        __builtin_memset(node2->data, 'B', sizeof(node2->data));
        
        /* Create simple tree structure */
        node1->left = calloc(1, sizeof(ASTNode));
        node1->right = calloc(1, sizeof(ASTNode));
        node2->left = calloc(1, sizeof(ASTNode));
        node2->right = calloc(1, sizeof(ASTNode));
        
        recursive_ast_copy(node2, node1, 3);
        
        /* Cleanup */
        free(node1->left);
        free(node1->right);
        free(node2->left);
        free(node2->right);
    }
    
    free(node1);
    free(node2);
    
    /* Phase 4: OpenMP parallel section */
    printf("Starting parallel memory operations...\n");
    parallel_mem_ops();
    printf("Parallel operations completed.\n");
    
    /* Phase 5: Mixed builtin usage in loops */
    char final_buf[512];
    for (int i = 0; i < 10; i++) {
        if (i % 3 == 0) {
            __builtin_memset(final_buf + i * 32, i, 32);
        } else if (i % 3 == 1) {
            __builtin_memcpy(final_buf + i * 32, buffer1, 32);
        } else {
            __builtin_memmove(final_buf + i * 32, final_buf + (i-1) * 32, 32);
        }
    }
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < sizeof(final_buf); i++) {
        hash = (hash * 31) + final_buf[i];
    }
    
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully.\n");
    
    return 0;
}
