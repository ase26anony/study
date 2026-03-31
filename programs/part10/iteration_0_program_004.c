/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile size_t g_mem_size = 256;
volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    uint32_t hash;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    /* Force early initialization of ASAN runtime */
    volatile char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    /* Final memory operation to ensure coverage */
    volatile char final_buf[8];
    __builtin_memset(final_buf, 0xFF, sizeof(final_buf));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for initialization */
    __builtin_memcpy(node->data, base_data, strlen(base_data) + 1);
    
    /* Create hash using memset/memcpy pattern */
    uint32_t temp_hash = 0;
    __builtin_memset(&temp_hash, depth, sizeof(temp_hash));
    
    /* Conditional memmove with goto for flow control */
    if (g_use_memmove) {
        uint32_t hash_src = temp_hash;
        uint32_t hash_dst = 0;
        
        /* Jump into memory operation block */
        goto memmove_block;
        
        memmove_block:
            __builtin_memmove(&hash_dst, &hash_src, sizeof(uint32_t));
            node->hash = hash_dst;
            goto after_memmove;
        
        after_memmove:;
    } else {
        __builtin_memcpy(&node->hash, &temp_hash, sizeof(uint32_t));
    }
    
    /* Recursive creation with volatile size control */
    volatile size_t child_size = g_mem_size / 2;
    char child_data[64];
    __builtin_snprintf(child_data, sizeof(child_data), "%s-%d", base_data, depth);
    
    node->left = create_ast(depth - 1, child_data);
    node->right = create_ast(depth - 1, child_data);
    
    return node;
}

/* Function with OpenMP parallel section */
static void parallel_memory_operations(ASTNode* nodes[], size_t count) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (size_t i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Thread-specific memory operations */
                char thread_buf[128];
                volatile size_t op_size = g_mem_size + thread_id;
                
                /* Mix of memory builtins */
                __builtin_memset(thread_buf, thread_id, op_size % 128);
                
                if (i % 3 == 0) {
                    __builtin_memcpy(nodes[i]->data, thread_buf, 
                                   op_size % sizeof(nodes[i]->data));
                } else if (i % 3 == 1) {
                    char temp[64];
                    __builtin_memcpy(temp, nodes[i]->data, sizeof(temp));
                    __builtin_memmove(nodes[i]->data + 8, temp, 
                                     op_size % (sizeof(nodes[i]->data) - 8));
                }
            }
        }
    }
}

/* Complex token processing with goto jumps */
static uint32_t process_tokens(const char** tokens, size_t token_count) {
    uint32_t result = 0xDEADBEEF;
    char buffer[256];
    size_t offset = 0;
    
    for (size_t i = 0; i < token_count; i++) {
        size_t token_len = strlen(tokens[i]);
        
        /* Jump-based flow control around memmove */
        if (token_len > 32) {
            goto use_memmove;
        } else {
            goto use_memcpy;
        }
        
        use_memmove:
            __builtin_memmove(buffer + offset, tokens[i], token_len);
            offset += token_len;
            goto next_token;
        
        use_memcpy:
            __builtin_memcpy(buffer + offset, tokens[i], token_len);
            offset += token_len;
            goto next_token;
        
        next_token:
            /* Mix memset into buffer */
            if (i % 5 == 0) {
                __builtin_memset(buffer + offset, 'X', 4);
                offset += 4;
            }
    }
    
    /* Final hash calculation */
    for (size_t i = 0; i < offset; i++) {
        result = (result << 5) - result + buffer[i];
    }
    
    return result;
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Phase 1: Recursive AST creation */
    ASTNode* root = create_ast(3, "ROOT");
    
    /* Phase 2: Array of AST nodes for parallel processing */
    ASTNode* nodes[8];
    nodes[0] = root;
    for (int i = 1; i < 8; i++) {
        char name[32];
        __builtin_snprintf(name, sizeof(name), "NODE-%d", i);
        nodes[i] = create_ast(2, name);
    }
    
    /* Phase 3: Parallel memory operations */
    parallel_memory_operations(nodes, 8);
    
    /* Phase 4: Token processing with goto flow */
    const char* tokens[] = {
        "TOKEN1", "LONG_TOKEN_FOR_MEMMOVE_TRIGGER", "SHORT",
        "ANOTHER_LONG_TOKEN_TO_FORCE_REDIRECTION", "END"
    };
    
    uint32_t token_hash = process_tokens(tokens, 
                                        sizeof(tokens)/sizeof(tokens[0]));
    
    /* Phase 5: Direct built-in calls with volatile control */
    volatile char src[512];
    volatile char dst[512];
    
    for (volatile int i = 0; i < 100; i++) {
        __builtin_memset((void*)src, i, sizeof(src));
        
        if (i % 2 == 0) {
            __builtin_memcpy((void*)dst, (void*)src, g_mem_size);
        } else {
            __builtin_memmove((void*)dst + 64, (void*)src, g_mem_size);
        }
    }
    
    /* Calculate final verification hash */
    uint32_t final_hash = token_hash;
    for (int i = 0; i < 8; i++) {
        if (nodes[i]) {
            for (int j = 0; j < 64; j++) {
                final_hash = (final_hash << 3) ^ nodes[i]->data[j];
            }
            free(nodes[i]);
        }
    }
    
    printf("Final hash: 0x%08X\n", final_hash);
    printf("Test completed - check ASAN/HWASAN instrumentation\n");
    
    return (final_hash != 0) ? 0 : 1;
}
