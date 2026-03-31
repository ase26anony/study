/* asan_coverage.c - Comprehensive test for ASAN/HWASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations for recursive structures */
struct ASTNode;
typedef struct ASTNode ASTNode;

/* Complex token types for parser simulation */
typedef enum {
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_STRING,
    TOK_OPERATOR,
    TOK_KEYWORD,
    TOK_EOF
} TokenType;

/* Recursive AST-like structure */
struct ASTNode {
    TokenType type;
    char* value;
    size_t value_len;
    ASTNode* left;
    ASTNode* right;
    ASTNode* next;  /* For linked list of nodes */
    unsigned char metadata[64];  /* For memory operations */
};

/* Global volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_init_flag = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_constructor(void) {
    /* Force early initialization of ASAN structures */
    char buffer[128];
    volatile char* volatile_ptr = buffer;
    
    /* Use builtins in constructor to trigger early redirection */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 64, buffer, 32);
    
    g_init_flag = 1;
    printf("Constructor initialized ASAN structures\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_destructor(void) {
    volatile int cleanup[16];
    __builtin_memset((void*)cleanup, 0, sizeof(cleanup));
    printf("Destructor cleaned up\n");
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast_node(TokenType type, const char* value, size_t len) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = type;
    node->value_len = len;
    node->value = (char*)malloc(len + 1);
    
    if (node->value) {
        /* Use volatile to prevent folding */
        volatile size_t vlen = len;
        __builtin_memcpy(node->value, value, vlen);
        node->value[vlen] = '\0';
    }
    
    /* Initialize metadata with pattern */
    for (size_t i = 0; i < sizeof(node->metadata); i++) {
        node->metadata[i] = (unsigned char)(i ^ 0x5A);
    }
    
    return node;
}

/* Complex memory operation with goto flow control */
static void process_ast_with_goto(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    volatile int use_goto = (dest->type != src->type);
    
    if (use_goto) {
        goto copy_block;
    }
    
    /* Normal path */
    __builtin_memcpy(dest->metadata, src->metadata, 32);
    return;
    
copy_block:
    /* Jump target with memmove */
    size_t copy_len = (dest->value_len < src->value_len) ? 
                      dest->value_len : src->value_len;
    
    /* Force memmove with overlapping regions */
    char temp[128];
    volatile size_t v_len = copy_len;
    
    __builtin_memcpy(temp, src->value, v_len);
    __builtin_memmove(dest->value, temp, v_len);
    
    /* Jump back */
    goto finish;
    
    /* Unreachable in normal flow but tests compiler analysis */
    __builtin_memset(dest, 0, sizeof(ASTNode));  /* Never executed */
    
finish:
    /* Final memory operation */
    if (dest->value_len > 0) {
        __builtin_memset(dest->value + copy_len, '*', 
                        dest->value_len - copy_len);
    }
}

/* Recursive tree copy with memory operations */
static void copy_ast_tree(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    /* Copy node data with builtins */
    volatile size_t meta_size = sizeof(dest->metadata);
    __builtin_memcpy(dest->metadata, src->metadata, meta_size);
    
    /* Recursive copy of children */
    if (src->left && dest->left) {
        copy_ast_tree(dest->left, src->left);
    }
    
    if (src->right && dest->right) {
        copy_ast_tree(dest->right, src->right);
    }
    
    /* Handle linked list */
    if (src->next && dest->next) {
        /* Use memmove for potential overlap */
        char buffer[256];
        volatile size_t len = src->value_len;
        
        __builtin_memcpy(buffer, src->next->value, len);
        __builtin_memmove(dest->next->value, buffer, len);
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(ASTNode** nodes, size_t count) {
    int i;
    
    #pragma omp parallel for private(i) shared(nodes, count)
    for (i = 0; i < (int)count; i++) {
        if (nodes[i]) {
            volatile size_t op_size = g_mem_size % 128;
            
            /* Mixed builtin usage in parallel region */
            if (i % 3 == 0) {
                __builtin_memset(nodes[i]->metadata, i, op_size);
            } else if (i % 3 == 1) {
                ASTNode* next = (i + 1 < count) ? nodes[i + 1] : nodes[0];
                if (next) {
                    __builtin_memcpy(nodes[i]->metadata, next->metadata, op_size);
                }
            } else {
                /* Create overlapping regions for memmove */
                char* mid = (char*)nodes[i]->metadata + 16;
                __builtin_memmove(mid, nodes[i]->metadata, 32);
            }
        }
    }
}

/* Main test driver */
int main(void) {
    printf("Starting ASAN builtin redirection test\n");
    
    /* Initialize token array */
    const char* tokens[] = {
        "variable", "12345", "hello_world", 
        "+", "if", "end", "test123"
    };
    size_t token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Create AST forest */
    ASTNode* forest[16];
    size_t forest_size = 0;
    
    for (size_t i = 0; i < 8 && i < token_count; i++) {
        TokenType type = (TokenType)(i % TOK_EOF);
        forest[forest_size++] = create_ast_node(type, tokens[i], 
                                              strlen(tokens[i]));
        
        /* Create linked structure */
        if (i > 0) {
            forest[i-1]->next = forest[i];
        }
    }
    
    /* Create duplicate forest for copy operations */
    ASTNode* forest_copy[16];
    for (size_t i = 0; i < forest_size; i++) {
        forest_copy[i] = create_ast_node(forest[i]->type, 
                                       forest[i]->value,
                                       forest[i]->value_len);
    }
    
    /* Test 1: Direct builtin calls */
    printf("Test 1: Direct builtin calls\n");
    char buffer1[256], buffer2[256];
    volatile size_t size1 = 128;
    
    __builtin_memset(buffer1, 0xCC, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, size1);
    __builtin_memmove(buffer1 + 64, buffer1, 64);
    
    /* Test 2: Goto flow control with memmove */
    printf("Test 2: Goto flow control\n");
    for (size_t i = 0; i < forest_size; i += 2) {
        process_ast_with_goto(forest[i], 
                            forest[(i + 1) % forest_size]);
    }
    
    /* Test 3: Recursive tree copy */
    printf("Test 3: Recursive tree copy\n");
    for (size_t i = 0; i < forest_size; i++) {
        copy_ast_tree(forest_copy[i], forest[i]);
    }
    
    /* Test 4: OpenMP parallel operations */
    printf("Test 4: OpenMP parallel operations\n");
    parallel_memory_operations(forest, forest_size);
    
    /* Test 5: Mixed operations in complex loop */
    printf("Test 5: Mixed operations\n");
    unsigned long hash = 0;
    for (size_t i = 0; i < forest_size; i++) {
        ASTNode* node = forest[i];
        
        /* Mix of memory operations */
        if (node->value) {
            volatile size_t len = node->value_len;
            char temp[256];
            
            __builtin_memcpy(temp, node->value, len);
            __builtin_memset(node->value + len/2, '#', len/4);
            __builtin_memmove(node->value, temp, len);
            
            /* Compute verification hash */
            for (size_t j = 0; j < len && j < 32; j++) {
                hash = (hash * 31) + node->value[j];
            }
        }
        
        /* Process metadata */
        if (i > 0) {
            __builtin_memcpy(forest[i]->metadata, 
                           forest[i-1]->metadata, 32);
        }
    }
    
    /* Verification output */
    printf("Verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    for (size_t i = 0; i < forest_size; i++) {
        if (forest[i]) {
            free(forest[i]->value);
            free(forest[i]);
        }
        if (forest_copy[i]) {
            free(forest_copy[i]->value);
            free(forest_copy[i]);
        }
    }
    
    return 0;
}
