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
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* Global token array */
static char g_token_array[4096];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Initialize token array with pattern */
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        g_token_array[i] = (char)(i % 256);
    }
    printf("Constructor: Token array initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_destructor(void) {
    /* Verify array integrity */
    int sum = 0;
    for (size_t i = 0; i < sizeof(g_token_array); i++) {
        sum += g_token_array[i];
    }
    printf("Destructor: Token array checksum = %d\n", sum);
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(int depth) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize node data with memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Set node type */
    node->type = depth;
    
    /* Copy data from global array using memcpy */
    size_t copy_len = (size_t)(depth * 16) % 256;
    __builtin_memcpy(node->data, g_token_array, copy_len);
    
    /* Recursive creation */
    node->left = create_ast_node(depth - 1);
    node->right = create_ast_node(depth - 1);
    
    return node;
}

/* Function with goto jumps around memmove */
static void process_with_goto(ASTNode* src, ASTNode* dst) {
    int use_memmove = 1;
    
    if (src == NULL || dst == NULL) {
        goto skip_memmove;
    }
    
    /* Jump into block with memmove */
    goto do_memmove;
    
skip_memmove:
    printf("Skipping memmove\n");
    return;
    
do_memmove:
    /* Perform memmove between nodes */
    size_t move_len = g_memmove_len;
    if (move_len > sizeof(src->data)) move_len = sizeof(src->data);
    
    __builtin_memmove(dst->data, src->data, move_len);
    
    /* Jump out */
    goto finish;
    
finish:
    printf("Memmove completed\n");
}

/* Parallel memory dispatch */
static void parallel_memory_ops(void) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        char local_buf[512];
        char src_buf[512];
        
        /* Initialize source buffer */
        for (int i = 0; i < 512; i++) {
            src_buf[i] = (char)((i + thread_id) % 256);
        }
        
        /* Use all three builtins in parallel region */
        #pragma omp barrier
        
        /* Memcpy with volatile length */
        size_t cp_len = g_memcpy_len;
        if (cp_len > 512) cp_len = 512;
        __builtin_memcpy(local_buf, src_buf, cp_len);
        
        /* Memset with volatile length */
        size_t set_len = g_memset_len;
        if (set_len > 512) set_len = 512;
        __builtin_memset(local_buf + 256, thread_id, set_len % 256);
        
        /* Memmove within buffer */
        __builtin_memmove(local_buf, local_buf + 128, 128);
        
        #pragma omp critical
        {
            printf("Thread %d completed memory ops\n", thread_id);
        }
    }
}

/* Complex initialization with multiple memory operations */
static int initialize_system(void) {
    char buffer1[1024];
    char buffer2[1024];
    char* dynamic_buf = NULL;
    
    /* Pattern 1: Direct builtin calls */
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1 + 512, buffer1, 512);
    
    /* Pattern 2: Dynamic allocation with builtins */
    dynamic_buf = (char*)malloc(2048);
    if (dynamic_buf) {
        __builtin_memset(dynamic_buf, 0x55, 2048);
        __builtin_memcpy(dynamic_buf + 1024, buffer2, 512);
        free(dynamic_buf);
    }
    
    /* Pattern 3: Nested calls */
    char temp[256];
    __builtin_memset(temp, 0, sizeof(temp));
    __builtin_memcpy(temp, "TestString", 10);
    __builtin_memmove(temp + 5, temp, 5);
    
    return 1;
}

int main(void) {
    printf("=== ASAN/HWASAN Built-in Redirection Test ===\n");
    
    /* Stage 1: System initialization */
    if (!initialize_system()) {
        fprintf(stderr, "Initialization failed\n");
        return 1;
    }
    
    /* Stage 2: Create AST structure */
    ASTNode* root = create_ast_node(4);
    if (!root) {
        fprintf(stderr, "AST creation failed\n");
        return 1;
    }
    
    /* Stage 3: Process with goto jumps */
    ASTNode* copy_node = (ASTNode*)malloc(sizeof(ASTNode));
    if (copy_node) {
        process_with_goto(root, copy_node);
        free(copy_node);
    }
    
    /* Stage 4: Parallel operations */
    parallel_memory_ops();
    
    /* Stage 5: Compute verification hash */
    unsigned long hash = 0;
    if (root) {
        for (size_t i = 0; i < sizeof(root->data); i++) {
            hash = (hash * 31) + (unsigned char)root->data[i];
        }
        
        /* Cleanup AST */
        free(root->left);
        free(root->right);
        free(root);
    }
    
    printf("Final hash: %lu\n", hash);
    printf("=== Test completed ===\n");
    
    return 0;
}
