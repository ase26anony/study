#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Complex AST-like structure for data structure access */
typedef struct ASTNode {
    char token[32];
    int value;
    volatile int flags;
    struct ASTNode* left;
    struct ASTNode* right;
    unsigned char padding[64]; /* Ensure size for memcpy testing */
} ASTNode;

/* Global volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Function attributes for structural diversity */
__attribute__((constructor)) static void init_globals(void) {
    printf("Constructor: Initializing ASAN test environment\n");
}

__attribute__((destructor)) static void cleanup_globals(void) {
    printf("Destructor: Cleaning up ASAN test environment\n");
}

/* Recursive AST builder with memory operations */
static ASTNode* build_ast(int depth, const char* base_token) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Construct token with __builtin_memcpy */
    char temp_token[32];
    __builtin_memcpy(temp_token, base_token, strlen(base_token) + 1);
    __builtin_memcpy(node->token, temp_token, sizeof(node->token));
    
    node->value = depth;
    node->flags = depth * 2;
    
    /* Recursive construction with goto for control flow */
    int build_left = 1;
    
    if (depth > 3) {
        build_left = 0;
        goto skip_left;
    }
    
    node->left = build_ast(depth - 1, "LEFT");
    
skip_left:
    if (build_left) {
        /* Jump back into block with memmove */
        volatile int counter = 5;
        while (counter-- > 0) {
            ASTNode dummy;
            __builtin_memmove(&dummy, node, sizeof(ASTNode));
            node->value += dummy.value;
        }
    }
    
    /* Right child with different pattern */
    if (depth % 2 == 0) {
        node->right = build_ast(depth - 2, "RIGHT");
    }
    
    return node;
}

/* Memory operation dispatcher with OpenMP */
static void dispatch_memory_ops(ASTNode* nodes[], int count) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Create local copy with __builtin_memcpy */
                ASTNode local_copy;
                __builtin_memcpy(&local_copy, nodes[i], sizeof(ASTNode));
                
                /* Modify and copy back with __builtin_memmove */
                local_copy.value += thread_id;
                __builtin_memmove(nodes[i], &local_copy, sizeof(ASTNode));
                
                /* Use volatile size for memset */
                volatile size_t clear_size = sizeof(nodes[i]->padding);
                __builtin_memset(nodes[i]->padding, thread_id, clear_size);
            }
        }
        
        /* Barrier to synchronize */
        #pragma omp barrier
        
        /* Additional memory operations between threads */
        #pragma omp single
        {
            ASTNode shared_node;
            volatile size_t shared_size = g_mem_size % 128;
            
            __builtin_memset(&shared_node, 0xFF, sizeof(ASTNode));
            
            for (int i = 0; i < count && i < 4; i++) {
                if (nodes[i]) {
                    __builtin_memcpy(&shared_node, nodes[i], shared_size);
                    __builtin_memmove(nodes[i], &shared_node, shared_size);
                }
            }
        }
    }
}

/* Complex token parser with goto jumps */
static int parse_tokens(const char* tokens[], int token_count) {
    int result = 0;
    int i = 0;
    
    /* Jump table simulation */
    parse_loop:
    if (i >= token_count) goto parse_done;
    
    char buffer[64];
    volatile int use_memmove = (i % 3 == 0);
    
    if (use_memmove) {
        /* Jump into memmove block */
        goto use_memmove_op;
    } else {
        /* Use memcpy */
        __builtin_memcpy(buffer, tokens[i], strlen(tokens[i]) + 1);
        goto process_token;
    }
    
use_memmove_op:
    {
        char temp[64];
        __builtin_memset(temp, 0, sizeof(temp));
        __builtin_memmove(buffer, temp, sizeof(buffer));
        __builtin_memmove(buffer, tokens[i], strlen(tokens[i]) + 1);
    }
    
process_token:
    for (int j = 0; buffer[j]; j++) {
        result += buffer[j];
    }
    
    i++;
    goto parse_loop;
    
parse_done:
    return result;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Build AST structures */
    ASTNode* nodes[8] = {0};
    for (int i = 0; i < 8; i++) {
        char token[32];
        __builtin_memset(token, 0, sizeof(token));
        snprintf(token, sizeof(token), "NODE_%d", i);
        nodes[i] = build_ast(5 + (i % 3), token);
    }
    
    /* Phase 2: Parse tokens with control flow */
    const char* tokens[] = {
        "MEMCPY_TEST", "MEMSET_OPERATION", "MEMMOVE_DATA",
        "ASAN_BUILTIN", "HWASAN_CHECK", "REDIRECTION"
    };
    int token_hash = parse_tokens(tokens, 
                    sizeof(tokens)/sizeof(tokens[0]));
    
    /* Phase 3: Parallel memory operations */
    dispatch_memory_ops(nodes, 8);
    
    /* Phase 4: Verify and compute result */
    unsigned long long total = token_hash;
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            total += nodes[i]->value;
            total += nodes[i]->flags;
            
            /* Final memory operations */
            ASTNode verify;
            volatile size_t verify_size = sizeof(ASTNode) - 16;
            __builtin_memcpy(&verify, nodes[i], verify_size);
            __builtin_memset(nodes[i]->padding, 0, 
                           sizeof(nodes[i]->padding));
            
            free(nodes[i]);
        }
    }
    
    printf("Test completed. Result hash: %llu\n", total);
    printf("Expected ASAN/HWASAN built-in redirections:\n");
    printf("  - __builtin_memcpy -> __asan_memcpy/__hwasan_memcpy\n");
    printf("  - __builtin_memset -> __asan_memset/__hwasan_memset\n");
    printf("  - __builtin_memmove -> __asan_memmove/__hwasan_memmove\n");
    
    return 0;
}
