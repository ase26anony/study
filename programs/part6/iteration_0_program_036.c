/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    int type;
    int value;
    char *data;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *next;
} ASTNode;

/* Token array for parser simulation */
typedef struct {
    int type;
    char *lexeme;
    size_t length;
} Token;

/* Global token array */
static Token g_tokens[100];
static volatile size_t g_token_count = 0;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Initialize token array with volatile control */
    volatile size_t init_size = 50;
    
    for (size_t i = 0; i < init_size; i++) {
        g_tokens[i].type = (int)(i % 5);
        g_tokens[i].length = (i * 7) % 32 + 1;
        
        /* Use __builtin_memset in constructor */
        char *buf = malloc(g_tokens[i].length + 1);
        if (buf) {
            __builtin_memset(buf, 'A' + (i % 26), g_tokens[i].length);
            buf[g_tokens[i].length] = '\0';
            g_tokens[i].lexeme = buf;
        }
        g_token_count++;
    }
    
    /* Force memcpy in constructor context */
    Token dest_token;
    if (g_token_count > 1) {
        __builtin_memcpy(&dest_token, &g_tokens[0], sizeof(Token));
    }
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    for (size_t i = 0; i < g_token_count; i++) {
        if (g_tokens[i].lexeme) {
            free(g_tokens[i].lexeme);
        }
    }
}

/* Recursive parser with goto control flow */
static ASTNode* parse_expression(size_t *pos) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    if (*pos >= g_token_count) {
        node->type = 0;
        return node;
    }
    
    Token current = g_tokens[*pos];
    (*pos)++;
    
    /* Goto-based control flow around memmove */
    if (current.type == 2) {
        goto handle_special;
    }
    
    node->type = current.type;
    node->value = (int)current.length;
    
    /* Allocate and copy data with __builtin_memcpy */
    node->data = malloc(current.length + 1);
    if (node->data && current.lexeme) {
        __builtin_memcpy(node->data, current.lexeme, current.length);
        node->data[current.length] = '\0';
    }
    
    if (*pos < g_token_count && g_tokens[*pos].type == 1) {
        node->left = parse_expression(pos);
    }
    
    if (*pos < g_token_count && g_tokens[*pos].type == 3) {
        node->right = parse_expression(pos);
    }
    
    return node;
    
handle_special:
    /* Jump target with __builtin_memmove */
    ASTNode temp_node;
    __builtin_memset(&temp_node, 0, sizeof(ASTNode));
    temp_node.type = 99;
    
    /* Use memmove for overlapping regions */
    if (node) {
        __builtin_memmove(node, &temp_node, sizeof(ASTNode));
        /* Jump back */
        goto normal_return;
    }
    
normal_return:
    return node;
}

/* Parallel memory operations with OpenMP */
static void parallel_memory_operations(void) {
    int results[4] = {0};
    
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        char *buffer = malloc(g_mem_size);
        
        if (buffer) {
            /* Each thread uses different builtins */
            switch (tid % 3) {
                case 0:
                    __builtin_memset(buffer, tid + '0', g_mem_size);
                    break;
                case 1:
                    if (tid > 0) {
                        char *src = buffer + g_mem_size/2;
                        __builtin_memcpy(buffer, src, g_mem_size/4);
                    }
                    break;
                case 2:
                    /* Overlapping memmove */
                    __builtin_memmove(buffer, buffer + g_mem_size/4, g_mem_size/2);
                    break;
            }
            
            /* Compute hash */
            for (size_t i = 0; i < g_mem_size; i++) {
                results[tid] += buffer[i];
            }
            
            free(buffer);
        }
    }
    
    /* Verify parallel execution */
    int total = 0;
    for (int i = 0; i < 4; i++) {
        total += results[i];
    }
    printf("Parallel hash sum: %d\n", total);
}

/* Complex data structure manipulation */
static void manipulate_ast(ASTNode *root) {
    if (!root) return;
    
    ASTNode stack[10];
    int top = 0;
    
    /* Push root */
    __builtin_memcpy(&stack[top++], root, sizeof(ASTNode));
    
    while (top > 0) {
        ASTNode current;
        __builtin_memcpy(&current, &stack[--top], sizeof(ASTNode));
        
        /* Process children with memmove for structure copying */
        if (current.left) {
            ASTNode temp;
            __builtin_memcpy(&temp, current.left, sizeof(ASTNode));
            __builtin_memmove(&stack[top++], &temp, sizeof(ASTNode));
        }
        
        if (current.right) {
            ASTNode temp;
            __builtin_memcpy(&temp, current.right, sizeof(ASTNode));
            __builtin_memmove(&stack[top++], &temp, sizeof(ASTNode));
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Recursive parsing */
    size_t pos = 0;
    ASTNode *ast = parse_expression(&pos);
    
    /* Phase 2: AST manipulation */
    if (ast) {
        manipulate_ast(ast);
        
        /* Cleanup AST */
        ASTNode *current = ast;
        while (current) {
            ASTNode *next = current->next;
            if (current->data) free(current->data);
            free(current);
            current = next;
        }
    }
    
    /* Phase 3: Parallel operations */
    parallel_memory_operations();
    
    /* Phase 4: Direct built-in calls with volatile control */
    volatile char test_buf[128];
    volatile char src_buf[128];
    
    for (volatile int i = 0; i < 128; i++) {
        src_buf[i] = (char)(i % 256);
    }
    
    /* Force all three builtins in main context */
    __builtin_memset((void*)test_buf, 0x42, sizeof(test_buf));
    __builtin_memcpy((void*)test_buf, (void*)src_buf, 64);
    __builtin_memmove((void*)test_buf + 32, (void*)test_buf, 64);
    
    /* Compute verification hash */
    unsigned long hash = 0;
    for (volatile int i = 0; i < 128; i++) {
        hash = hash * 31 + test_buf[i];
    }
    
    printf("Final verification hash: %lu\n", hash);
    printf("Test completed successfully\n");
    
    return 0;
}
