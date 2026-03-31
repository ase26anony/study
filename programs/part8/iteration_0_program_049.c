/* ISO C99-compliant test program for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 64;
volatile int g_use_hwasan = 0;

/* AST-like recursive structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    int id;
} ASTNode;

/* Global token array */
static char g_tokens[8][32] = {
    "memcpy_test_token_1",
    "memset_test_token_2",
    "memmove_test_token_3",
    "asan_redirection_4",
    "hwasan_branch_5",
    "flow_sensitive_6",
    "omp_parallel_7",
    "final_verification_8"
};

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_environment(void) {
    /* Force initialization of ASAN runtime */
    volatile char init_buf[16];
    __builtin_memset(init_buf, 0, sizeof(init_buf));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    /* Final memory operation to ensure destructor path is taken */
    volatile char cleanup_buf[32];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast_node(int depth, int id) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    node->id = id;
    
    /* Copy token data using __builtin_memcpy */
    int token_idx = id % 8;
    __builtin_memcpy(node->data, g_tokens[token_idx], 
                     sizeof(g_tokens[token_idx]));
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (id % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast_node(depth - 1, id * 2);
        node->right = create_ast_node(depth - 1, id * 2 + 1);
        
        if (use_goto) {
            create_children:
            /* Jump target with __builtin_memmove */
            ASTNode* temp = node->left;
            if (temp) {
                char temp_buf[256];
                __builtin_memmove(temp_buf, temp->data, sizeof(temp_buf));
                __builtin_memmove(temp->data, node->data, sizeof(node->data));
                __builtin_memmove(node->data, temp_buf, sizeof(temp_buf));
            }
        }
    }
    
    return node;
}

/* Complex memory operation with goto edge cases */
static void perform_memory_operations(volatile char* buffer, size_t size) {
    volatile char local_buf[128];
    volatile int operation = 0;
    
    /* Label for goto jumping into memory operation block */
    operation_block:
    
    switch (operation % 3) {
        case 0:
            /* Force __builtin_memcpy redirection */
            __builtin_memcpy((void*)local_buf, (void*)buffer, 
                           size < 128 ? size : 128);
            break;
        case 1:
            /* Force __builtin_memset redirection */
            __builtin_memset((void*)local_buf, operation, 
                           size < 128 ? size : 128);
            break;
        case 2:
            /* Force __builtin_memmove redirection with overlapping regions */
            __builtin_memmove((void*)buffer, (void*)(buffer + 32), 64);
            break;
    }
    
    operation++;
    
    /* Conditional goto to create complex control flow */
    if (operation < 6) {
        if (operation % 2 == 0) {
            goto operation_block;  /* Jump back */
        } else {
            goto alternate_path;
        }
    }
    
    alternate_path:
    /* Additional memory operation after goto */
    __builtin_memcpy((void*)buffer, (void*)local_buf, 64);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_dispatch(ASTNode* nodes[], int count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes)
    for (i = 0; i < count; i++) {
        if (nodes[i]) {
            volatile char thread_buf[256];
            volatile size_t op_size = g_mem_size + (i * 16);
            
            /* Mix of builtins in parallel region */
            __builtin_memset(thread_buf, i, sizeof(thread_buf));
            __builtin_memcpy(nodes[i]->data, thread_buf, 
                           op_size < 256 ? op_size : 256);
            
            /* Conditional memmove with volatile control */
            volatile int do_memmove = (i % 3 == 0);
            if (do_memmove && i > 0) {
                __builtin_memmove(nodes[i-1]->data, nodes[i]->data, 128);
            }
        }
    }
}

/* Main verification function */
static unsigned long compute_ast_hash(ASTNode* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    char* p = node->data;
    
    /* DJB2 hash algorithm */
    while (*p) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    hash += compute_ast_hash(node->left);
    hash += compute_ast_hash(node->right);
    
    return hash;
}

int main(void) {
    ASTNode* ast_nodes[4];
    unsigned long final_hash = 0;
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Create AST structure */
    for (int i = 0; i < 4; i++) {
        ast_nodes[i] = create_ast_node(3, i + 1);
    }
    
    /* Perform memory operations with goto edge cases */
    volatile char main_buffer[256];
    for (int i = 0; i < 256; i++) {
        main_buffer[i] = (char)(i % 256);
    }
    
    perform_memory_operations(main_buffer, g_mem_size);
    
    /* Execute OpenMP parallel section */
    #ifdef _OPENMP
    parallel_memory_dispatch(ast_nodes, 4);
    #endif
    
    /* Compute verification hash */
    for (int i = 0; i < 4; i++) {
        if (ast_nodes[i]) {
            final_hash ^= compute_ast_hash(ast_nodes[i]);
            
            /* Additional memory operation in main */
            volatile char verify_buf[128];
            __builtin_memcpy(verify_buf, ast_nodes[i]->data, 128);
            __builtin_memset(ast_nodes[i]->data + 64, 0xFF, 64);
        }
    }
    
    printf("Final verification hash: %lu\n", final_hash);
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        if (ast_nodes[i]) {
            /* Use memset before free */
            __builtin_memset(ast_nodes[i], 0, sizeof(ASTNode));
            free(ast_nodes[i]);
        }
    }
    
    /* Final builtin call to ensure all paths are taken */
    volatile char final_buf[64];
    __builtin_memcpy(final_buf, main_buffer, 64);
    __builtin_memset(main_buffer, 0, sizeof(main_buffer));
    
    return (final_hash != 0) ? 0 : 1;
}
