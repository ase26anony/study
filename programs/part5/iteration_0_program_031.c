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
    TOK_OPERATOR,
    TOK_STRING,
    TOK_END
} TokenType;

/* Token structure with volatile members */
typedef struct {
    volatile TokenType type;
    volatile char lexeme[64];
    volatile int line;
} Token;

/* Recursive AST node structure */
struct ASTNode {
    volatile int node_type;
    volatile char value[32];
    ASTNode* volatile left;
    ASTNode* volatile right;
    volatile size_t metadata[4];
};

/* Global volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_init_flag = 0;
volatile char g_global_buffer[1024];

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_globals(void) {
    volatile char local_buf[128];
    
    /* Force __builtin_memset in constructor */
    __builtin_memset(local_buf, 0xAA, sizeof(local_buf));
    
    /* Copy to global with __builtin_memcpy */
    __builtin_memcpy((void*)g_global_buffer, local_buf, 128);
    
    g_init_flag = 1;
    printf("[Constructor] ASAN globals initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_resources(void) {
    volatile char cleanup_buf[64];
    
    /* Use __builtin_memset in destructor */
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
    
    /* Overwrite global buffer */
    __builtin_memcpy((void*)g_global_buffer, cleanup_buf, 64);
    
    printf("[Destructor] ASAN resources cleaned up\n");
}

/* Recursive AST creation with memory operations */
static ASTNode* create_ast_node(const char* value, int depth) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy value with __builtin_memcpy */
    volatile size_t len = strlen(value) + 1;
    if (len > 31) len = 31;
    __builtin_memcpy(node->value, value, len);
    
    node->node_type = depth;
    
    if (depth > 0) {
        char left_val[32], right_val[32];
        snprintf(left_val, sizeof(left_val), "%s_L%d", value, depth);
        snprintf(right_val, sizeof(right_val), "%s_R%d", value, depth);
        
        node->left = create_ast_node(left_val, depth - 1);
        node->right = create_ast_node(right_val, depth - 1);
        
        /* Copy metadata between nodes using __builtin_memcpy */
        if (node->left && node->right) {
            __builtin_memcpy(node->metadata, node->left->metadata, 
                           sizeof(node->metadata));
        }
    }
    
    return node;
}

/* Complex parser with goto flow control */
static int parse_tokens(Token* tokens, int count, ASTNode** result) {
    volatile int i = 0;
    volatile char temp_buf[256];
    ASTNode* current = NULL;
    
    /* Initialize temp buffer */
    __builtin_memset(temp_buf, 0, sizeof(temp_buf));
    
parse_loop:
    if (i >= count) goto parse_done;
    
    /* Jump into block with __builtin_memmove */
    if (tokens[i].type == TOK_OPERATOR) {
        goto handle_operator;
    }
    
    /* Normal path with __builtin_memcpy */
    __builtin_memcpy(temp_buf, tokens[i].lexeme, 
                    strlen(tokens[i].lexeme) + 1);
    
    current = create_ast_node(temp_buf, 2);
    i++;
    goto parse_loop;

handle_operator:
    {
        volatile char op_buf[64];
        /* Use __builtin_memmove with overlapping regions */
        __builtin_memmove(op_buf, temp_buf + 16, 32);
        __builtin_memmove(temp_buf + 8, op_buf, 32);
        
        /* Create node with moved data */
        current = create_ast_node(temp_buf, 1);
        i++;
        
        /* Jump back to loop */
        goto parse_loop;
    }

parse_done:
    *result = current;
    return i;
}

/* OpenMP parallel memory operations */
static void parallel_memory_operations(void) {
    volatile size_t local_size = g_mem_size;
    volatile char* buffers[4];
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        /* Each thread allocates and initializes its buffer */
        buffers[tid] = malloc(local_size);
        if (buffers[tid]) {
            /* Use __builtin_memset in parallel region */
            __builtin_memset(buffers[tid], tid + 0x30, local_size);
            
            #pragma omp barrier
            
            /* Rotate buffers between threads using __builtin_memcpy */
            int src_tid = (tid + 1) % 4;
            if (buffers[src_tid]) {
                volatile char temp[128];
                __builtin_memcpy(temp, buffers[src_tid], 128);
                __builtin_memcpy(buffers[tid] + 64, temp, 128);
            }
        }
        
        #pragma omp barrier
        
        /* Final __builtin_memmove with overlap */
        if (tid == 0 && buffers[0] && buffers[1]) {
            __builtin_memmove(buffers[0] + 32, buffers[0], 96);
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(buffers[i]);
    }
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Initialize token array */
    Token tokens[5];
    volatile int token_count = 5;
    
    /* Initialize tokens with memory builtins */
    __builtin_memset(tokens, 0, sizeof(tokens));
    
    tokens[0].type = TOK_IDENTIFIER;
    __builtin_memcpy(tokens[0].lexeme, "variable_x", 11);
    tokens[0].line = 1;
    
    tokens[1].type = TOK_NUMBER;
    __builtin_memcpy(tokens[1].lexeme, "42", 3);
    tokens[1].line = 1;
    
    tokens[2].type = TOK_OPERATOR;
    __builtin_memcpy(tokens[2].lexeme, "+", 2);
    tokens[2].line = 1;
    
    tokens[3].type = TOK_NUMBER;
    __builtin_memcpy(tokens[3].lexeme, "23", 3);
    tokens[3].line = 1;
    
    tokens[4].type = TOK_END;
    __builtin_memset(tokens[4].lexeme, 0, sizeof(tokens[4].lexeme));
    
    /* Parse tokens (tests goto flow with memmove) */
    ASTNode* ast_root = NULL;
    int parsed = parse_tokens(tokens, token_count, &ast_root);
    printf("Parsed %d tokens, AST created: %s\n", 
           parsed, ast_root ? "YES" : "NO");
    
    /* Execute parallel memory operations */
    printf("Running parallel memory operations...\n");
    parallel_memory_operations();
    
    /* Complex memory operation sequence */
    volatile char final_buffer[512];
    volatile char source_buffer[512];
    
    /* Initialize source with pattern */
    for (volatile int i = 0; i < 512; i++) {
        source_buffer[i] = (i % 256);
    }
    
    /* Chain of memory builtins */
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer, source_buffer, 256);
    __builtin_memmove(final_buffer + 128, final_buffer, 128);
    __builtin_memcpy(final_buffer + 384, source_buffer + 128, 128);
    
    /* Compute verification hash */
    volatile unsigned long hash = 0;
    for (volatile int i = 0; i < 512; i++) {
        hash = (hash * 31) + final_buffer[i];
    }
    
    printf("Final buffer hash: 0x%08lx\n", (unsigned long)hash);
    printf("Global init flag: %d\n", (int)g_init_flag);
    
    /* Cleanup */
    free(ast_root);
    
    printf("=== Test Complete ===\n");
    return 0;
}
