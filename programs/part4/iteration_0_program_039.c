/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 64;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    /* Force initialization of ASAN runtime */
    volatile char buffer[32];
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("Constructor: ASAN environment initialized\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Destructor: ASAN environment cleanup\n");
}

/* Complex memory operation with goto control flow */
static void complex_mem_operations(void) {
    char src[256], dst[256];
    volatile int use_memmove = 1;
    
    /* Initialize source with pattern */
    for (size_t i = 0; i < sizeof(src); i++) {
        src[i] = (char)(i % 256);
    }
    
    /* Label for goto jumping */
    use_memcpy:
    __builtin_memcpy(dst, src, g_mem_size);
    
    if (use_memmove) {
        /* Jump into memmove block */
        goto do_memmove;
    }
    
    /* This should be skipped by goto */
    __builtin_memset(dst + 32, 0xFF, 16);
    
    do_memmove:
    /* Overlapping memory regions to force memmove */
    __builtin_memmove(dst + 16, dst + 8, 48);
    
    /* Jump back */
    if (use_memmove) {
        use_memmove = 0;
        goto use_memcpy;
    }
}

/* Recursive AST manipulation with memory operations */
static ASTNode* create_ast_node(const char* data, size_t depth) {
    if (depth == 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data using builtin memcpy */
    size_t copy_len = strlen(data) < sizeof(node->data) - 1 ? 
                     strlen(data) : sizeof(node->data) - 1;
    __builtin_memcpy(node->data, data, copy_len);
    node->data[copy_len] = '\0';
    node->size = copy_len;
    
    /* Recursive creation */
    char left_data[64], right_data[64];
    snprintf(left_data, sizeof(left_data), "%s-L%d", data, (int)depth);
    snprintf(right_data, sizeof(right_data), "%s-R%d", data, (int)depth);
    
    node->left = create_ast_node(left_data, depth - 1);
    node->right = create_ast_node(right_data, depth - 1);
    
    return node;
}

/* Copy between AST nodes */
static void copy_ast_data(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Use volatile to prevent optimization */
    volatile size_t copy_size = src->size < sizeof(dest->data) ? 
                               src->size : sizeof(dest->data);
    
    __builtin_memcpy(dest->data, src->data, copy_size);
    
    /* Recursive copy */
    if (src->left && dest->left) {
        copy_ast_data(dest->left, src->left);
    }
    if (src->right && dest->right) {
        copy_ast_data(dest->right, src->right);
    }
}

/* Calculate hash of AST */
static size_t ast_hash(const ASTNode* node) {
    if (!node) return 0;
    
    size_t hash = 5381;
    for (size_t i = 0; i < node->size; i++) {
        hash = ((hash << 5) + hash) + node->data[i];
    }
    
    return hash + ast_hash(node->left) + ast_hash(node->right);
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear memory before free */
    __builtin_memset(node->data, 0, sizeof(node->data));
    free(node);
}

/* Main test function with OpenMP parallelization */
static void parallel_memory_operations(void) {
    const int num_arrays = 8;
    char* arrays[num_arrays];
    
    /* Allocate arrays */
    for (int i = 0; i < num_arrays; i++) {
        arrays[i] = (char*)malloc(g_mem_size * 2);
        if (arrays[i]) {
            __builtin_memset(arrays[i], i, g_mem_size * 2);
        }
    }
    
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < num_arrays - 1; i++) {
            if (arrays[i] && arrays[i + 1]) {
                /* Use all three builtins in parallel */
                __builtin_memcpy(arrays[i] + g_mem_size, 
                               arrays[i + 1], 
                               g_mem_size);
                
                __builtin_memset(arrays[i], 
                               (char)(i * 31), 
                               g_mem_size / 2);
                
                /* Overlapping copy */
                __builtin_memmove(arrays[i] + g_mem_size / 4,
                                arrays[i] + g_mem_size / 2,
                                g_mem_size / 2);
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_arrays; i++) {
        if (arrays[i]) {
            free(arrays[i]);
        }
    }
}

/* Token parser with memory operations */
static size_t parse_tokens(const char** tokens, size_t count) {
    char buffer[512];
    size_t total_hash = 0;
    
    for (size_t i = 0; i < count; i++) {
        volatile size_t token_len = strlen(tokens[i]);
        volatile size_t copy_pos = i * 16 % 256;
        
        /* Ensure we don't overflow buffer */
        if (copy_pos + token_len < sizeof(buffer)) {
            __builtin_memcpy(buffer + copy_pos, 
                           tokens[i], 
                           token_len);
            
            /* Zero terminate */
            if (copy_pos + token_len + 1 < sizeof(buffer)) {
                __builtin_memset(buffer + copy_pos + token_len, 
                               0, 1);
            }
            
            /* Calculate simple hash */
            for (size_t j = 0; j < token_len; j++) {
                total_hash += buffer[copy_pos + j];
            }
        }
    }
    
    return total_hash;
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Phase 1: Complex control flow with goto */
    complex_mem_operations();
    
    /* Phase 2: Recursive AST operations */
    ASTNode* ast1 = create_ast_node("ROOT", 3);
    ASTNode* ast2 = create_ast_node("COPY", 3);
    
    if (ast1 && ast2) {
        /* Copy data between ASTs */
        copy_ast_data(ast2, ast1);
        
        /* Calculate and print hash */
        size_t hash1 = ast_hash(ast1);
        size_t hash2 = ast_hash(ast2);
        printf("AST Hash 1: %zu\n", hash1);
        printf("AST Hash 2: %zu\n", hash2);
        printf("Hash difference: %zu\n", 
               hash1 > hash2 ? hash1 - hash2 : hash2 - hash1);
        
        free_ast(ast1);
        free_ast(ast2);
    }
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_operations();
    
    /* Phase 4: Token parsing */
    const char* tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "instrumentation", "redzone", "builtin", "coverage",
        "optimization", "volatile", "recursive", "parallel"
    };
    
    size_t token_hash = parse_tokens(tokens, 
                                    sizeof(tokens)/sizeof(tokens[0]));
    printf("Token hash: %zu\n", token_hash);
    
    /* Final verification */
    volatile char final_check[128];
    __builtin_memset(final_check, 0xAA, sizeof(final_check));
    __builtin_memcpy(final_check + 64, final_check, 32);
    __builtin_memmove(final_check + 32, final_check + 16, 48);
    
    size_t final_sum = 0;
    for (size_t i = 0; i < sizeof(final_check); i++) {
        final_sum += final_check[i];
    }
    printf("Final checksum: %zu\n", final_sum);
    
    printf("ASAN test completed successfully\n");
    return 0;
}
