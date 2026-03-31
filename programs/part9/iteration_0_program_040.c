/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
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
    ASTNode* children[4];
    unsigned char metadata[32];
};

/* Global volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_hwasan = 0;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_sanitizer_hook(void) {
    volatile char init_buf[64];
    /* Force builtin memset initialization */
    __builtin_memset(init_buf, 0xAA, sizeof(init_buf));
    
    /* Copy initialization pattern */
    volatile char src_buf[64];
    __builtin_memset(src_buf, 0xBB, sizeof(src_buf));
    __builtin_memcpy(init_buf, src_buf, sizeof(src_buf) / 2);
    
    printf("[Constructor] Initialized sanitizer hooks\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_sanitizer_hook(void) {
    volatile char cleanup_buf[128];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    printf("[Destructor] Cleaned up sanitizer state\n");
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast_node(TokenType type, const char* value, size_t len) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with builtin memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    node->type = type;
    node->value_len = len;
    
    /* Allocate and copy value with builtin memcpy */
    if (value && len > 0) {
        node->value = (char*)malloc(len + 1);
        if (node->value) {
            __builtin_memcpy(node->value, value, len);
            node->value[len] = '\0';
        }
    }
    
    /* Initialize metadata with pattern */
    for (size_t i = 0; i < sizeof(node->metadata); i++) {
        node->metadata[i] = (unsigned char)(i ^ 0x55);
    }
    
    return node;
}

/* Complex memory operation with goto flow control */
static void process_ast_with_goto(ASTNode* dest, ASTNode* src) {
    if (!dest || !src) return;
    
    volatile int use_memmove = 0;
    
    /* Jump into memory operation block */
    if (dest->value && src->value) {
        goto copy_values;
    }
    
    skip_copy:
    /* Normal path */
    __builtin_memcpy(dest->metadata, src->metadata, 
                     sizeof(dest->metadata));
    return;
    
    copy_values:
    /* Goto target with memmove */
    size_t copy_len = dest->value_len < src->value_len ? 
                      dest->value_len : src->value_len;
    
    /* Force memmove usage */
    use_memmove = 1;
    
    if (use_memmove) {
        __builtin_memmove(dest->value, src->value, copy_len);
        /* Jump back out */
        goto skip_copy;
    }
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(ASTNode** nodes, size_t count) {
    volatile size_t local_size = g_mem_size;
    
    #pragma omp parallel
    {
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Each thread uses builtins */
                volatile char thread_buf[256];
                
                /* Initialize buffer */
                __builtin_memset(thread_buf, i, sizeof(thread_buf));
                
                /* Copy to node metadata */
                __builtin_memcpy(nodes[i]->metadata, thread_buf,
                               sizeof(nodes[i]->metadata) < sizeof(thread_buf) ?
                               sizeof(nodes[i]->metadata) : sizeof(thread_buf));
                
                /* Move data within node */
                if (i > 0 && nodes[i-1]) {
                    __builtin_memmove(nodes[i]->children, 
                                    nodes[i-1]->children,
                                    sizeof(nodes[i]->children));
                }
            }
        }
        
        /* Barrier with memory operation */
        #pragma omp barrier
        
        #pragma omp single
        {
            volatile char sync_buf[64];
            __builtin_memset(sync_buf, 0xCC, sizeof(sync_buf));
        }
    }
}

/* Multi-stage initialization with varied memory patterns */
static ASTNode** create_token_ast(const char** tokens, size_t token_count) {
    ASTNode** nodes = (ASTNode**)malloc(sizeof(ASTNode*) * token_count);
    if (!nodes) return NULL;
    
    /* Initialize array with memset */
    __builtin_memset(nodes, 0, sizeof(ASTNode*) * token_count);
    
    for (size_t i = 0; i < token_count; i++) {
        size_t len = tokens[i] ? strlen(tokens[i]) : 0;
        nodes[i] = create_ast_node(i % 6, tokens[i], len);
        
        /* Create circular references */
        if (i > 0) {
            nodes[i]->left = nodes[i-1];
            if (i > 3) {
                nodes[i-3]->right = nodes[i];
            }
        }
        
        /* Fill children array */
        for (int j = 0; j < 4 && (i + j) < token_count; j++) {
            nodes[i]->children[j] = nodes[(i + j) % token_count];
        }
    }
    
    return nodes;
}

/* Compute verification hash */
static unsigned long compute_ast_hash(ASTNode** nodes, size_t count) {
    unsigned long hash = 0xDEADBEEF;
    volatile unsigned char* ptr;
    
    for (size_t i = 0; i < count; i++) {
        if (!nodes[i]) continue;
        
        /* Hash metadata */
        ptr = nodes[i]->metadata;
        for (size_t j = 0; j < sizeof(nodes[i]->metadata); j++) {
            hash = (hash << 5) + hash + ptr[j];
        }
        
        /* Hash value if present */
        if (nodes[i]->value) {
            size_t len = nodes[i]->value_len;
            for (size_t j = 0; j < len && j < 256; j++) {
                hash = (hash << 3) + hash + nodes[i]->value[j];
            }
        }
    }
    
    return hash;
}

/* Main execution flow */
int main(void) {
    const char* tokens[] = {
        "variable", "12345", "hello_world", "+", "if", "else",
        "while", "function", "return", "struct", "typedef", "enum",
        "const", "volatile", "static", "extern", "sizeof", "offsetof"
    };
    size_t token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Stage 1: Create AST structures */
    ASTNode** nodes = create_token_ast(tokens, token_count);
    if (!nodes) {
        fprintf(stderr, "Failed to create AST nodes\n");
        return 1;
    }
    
    /* Stage 2: Process with goto flow control */
    for (size_t i = 1; i < token_count; i += 2) {
        process_ast_with_goto(nodes[i], nodes[i-1]);
    }
    
    /* Stage 3: Parallel memory operations */
    printf("Launching OpenMP parallel memory operations...\n");
    parallel_memory_operations(nodes, token_count);
    
    /* Stage 4: Additional builtin stress */
    volatile char final_buf[512];
    volatile char src_buf[512];
    
    /* Chain of memory operations */
    __builtin_memset(src_buf, 0xAA, sizeof(src_buf));
    __builtin_memcpy(final_buf, src_buf, sizeof(final_buf));
    __builtin_memmove(final_buf + 128, final_buf, 256);
    
    /* Copy to AST nodes */
    for (size_t i = 0; i < token_count && i < 8; i++) {
        if (nodes[i]) {
            __builtin_memcpy(nodes[i]->metadata, final_buf + i * 32,
                           sizeof(nodes[i]->metadata));
        }
    }
    
    /* Stage 5: Verification */
    unsigned long hash = compute_ast_hash(nodes, token_count);
    printf("AST verification hash: 0x%08lX\n", hash);
    
    /* Cleanup */
    for (size_t i = 0; i < token_count; i++) {
        if (nodes[i]) {
            if (nodes[i]->value) free(nodes[i]->value);
            free(nodes[i]);
        }
    }
    free(nodes);
    
    printf("Test completed successfully.\n");
    return 0;
}
