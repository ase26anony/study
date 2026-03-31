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
typedef struct ASTNode {
    int type;
    char data[32];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_hooks(void) {
    volatile char buf1[256], buf2[256];
    
    /* Force initialization of asan_memfn_rtls cache */
    __builtin_memset(buf1, 0xAA, sizeof(buf1));
    __builtin_memcpy(buf2, buf1, sizeof(buf1));
    __builtin_memmove(buf1, buf2, sizeof(buf1));
    
    printf("[constructor] Initialized sanitizer hooks\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_sanitizer_hooks(void) {
    printf("[destructor] Cleaning up sanitizer hooks\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtins with volatile lengths */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->type = depth;
    
    /* Fill data with pattern */
    for (int i = 0; i < 31; i++) {
        node->data[i] = 'A' + (depth + i) % 26;
    }
    node->data[31] = '\0';
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth - 1);
    
    /* Memory copy between nodes if both exist */
    if (node->left && depth > 3) {
        ASTNode temp;
        __builtin_memcpy(&temp, node->left, sizeof(ASTNode));
        __builtin_memmove(node->left, &temp, sizeof(ASTNode));
    }
    
    node->right = create_ast(depth - 2);
    
    /* Another goto for control flow */
    if (node->right) {
        goto adjust_right;
    }
    
done:
    return node;

adjust_right:
    /* Modify right child data */
    __builtin_memset(node->right->data, 'X', 16);
    goto done;
}

/* Parallel memory operations */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[512];
        char src_buf[512];
        
        /* Initialize source buffer */
        #pragma omp for
        for (int i = 0; i < 512; i++) {
            src_buf[i] = (char)(i % 256);
        }
        
        /* Use all three builtins with volatile lengths */
        __builtin_memset(local_buf, thread_id, g_memset_len);
        __builtin_memcpy(local_buf + 128, src_buf, g_memcpy_len);
        
        /* Conditional memmove with goto */
        if (thread_id % 2 == 0) {
            goto do_memmove;
        }
        
        __builtin_memcpy(local_buf + 256, src_buf + 128, 64);
        goto skip_memmove;
        
    do_memmove:
        __builtin_memmove(local_buf + 192, local_buf + 64, g_memmove_len);
        
    skip_memmove:
        /* Verify the operations */
        int sum = 0;
        for (int i = 0; i < 256; i++) {
            sum += local_buf[i];
        }
        
        #pragma omp critical
        {
            printf("Thread %d: memory ops completed, checksum = %d\n", 
                   thread_id, sum);
        }
    }
}

/* Complex token processing */
static int process_tokens(const char** tokens, int count) {
    char buffer[1024];
    int offset = 0;
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        
        /* Use builtins with variable lengths */
        if (i % 3 == 0) {
            __builtin_memcpy(buffer + offset, tokens[i], len);
        } else if (i % 3 == 1) {
            __builtin_memset(buffer + offset, '#', len);
        } else {
            if (offset > 16) {
                __builtin_memmove(buffer + offset - 8, buffer + offset, len);
                offset -= 8;
            }
            __builtin_memcpy(buffer + offset, tokens[i], len);
        }
        
        /* Goto for error handling simulation */
        if (offset + len >= sizeof(buffer) - 64) {
            goto buffer_full;
        }
        
        offset += len;
        buffer[offset++] = ' ';
    }
    
    buffer[offset] = '\0';
    return offset;

buffer_full:
    /* Handle buffer full condition */
    __builtin_memset(buffer + offset - 32, 0, 32);
    return -1;
}

int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Initialize token array */
    const char* tokens[] = {
        "MEMCPY", "MEMSET", "MEMMOVE", "REDZONE",
        "INSTRUMENT", "SANITIZE", "BUILTIN", "HWASAN",
        "ADDRESS", "SHADOW", "QUARANTINE", "ALLOCATOR"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Process tokens */
    int result = process_tokens(tokens, token_count);
    printf("Token processing result: %d bytes processed\n", result);
    
    /* Create recursive AST */
    ASTNode* root = create_ast(5);
    if (root) {
        /* Copy AST data */
        ASTNode copy;
        __builtin_memcpy(&copy, root, sizeof(ASTNode));
        
        /* Verify copy */
        int match = __builtin_memcmp(root, &copy, sizeof(ASTNode)) == 0;
        printf("AST copy verification: %s\n", match ? "PASS" : "FAIL");
        
        /* Cleanup */
        free(root);
    }
    
    /* Execute parallel memory operations */
    printf("\nParallel memory operations:\n");
    parallel_memory_ops();
    
    /* Final builtin calls with volatile parameters */
    char final_buf[1024];
    char src_buf[1024];
    
    __builtin_memset(src_buf, 0xCC, sizeof(src_buf));
    __builtin_memcpy(final_buf, src_buf, g_memcpy_len * 2);
    __builtin_memmove(final_buf + 512, final_buf, g_memmove_len);
    
    /* Compute final checksum */
    unsigned long long checksum = 0;
    for (int i = 0; i < 1024; i++) {
        checksum += final_buf[i];
    }
    
    printf("\nFinal checksum: 0x%016llx\n", checksum);
    printf("=== Test completed ===\n");
    
    return 0;
}
