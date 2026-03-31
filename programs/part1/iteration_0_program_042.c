#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* AST-like recursive structure */
typedef struct ASTNode {
    int type;
    int value;
    volatile size_t size;  /* volatile to prevent optimization */
    struct ASTNode* left;
    struct ASTNode* right;
    char padding[32];      /* Ensure size for memcpy operations */
} ASTNode;

/* Global volatile variables to prevent constant folding */
volatile size_t g_mem_size = 64;
volatile int g_init_value = 0x42;
volatile int g_copy_offset = 8;

/* Token array for parser simulation */
static const char* tokens[] = {
    "IDENT", "NUMBER", "STRING", "LPAREN", "RPAREN",
    "PLUS", "MINUS", "MULT", "DIV", "ASSIGN"
};
static const int token_count = 10;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_asan_globals(void) {
    /* Force initialization of ASAN structures */
    volatile char buffer[128];
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("Constructor: Initialized ASAN globals\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    printf("Destructor: Cleaning up ASAN resources\n");
}

/* Recursive AST creation with memory operations */
ASTNode* create_ast(int depth, int* counter) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile size to prevent optimization */
    volatile size_t node_size = sizeof(ASTNode);
    
    /* Initialize with __builtin_memset */
    __builtin_memset(node, g_init_value, node_size);
    
    node->type = (*counter)++ % 5;
    node->value = depth * 100 + *counter;
    node->size = node_size;
    
    /* Create children with goto-based control flow */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    } else {
        goto skip_children;
    }
    
create_children:
    node->left = create_ast(depth - 1, counter);
    
    /* Jump around memory operation */
    if (depth % 2 == 0) {
        goto memmove_block;
    }
    
skip_children:
    node->right = create_ast(depth - 1, counter);
    return node;
    
memmove_block:
    {
        /* Force __builtin_memmove with goto into block */
        char temp[sizeof(ASTNode)];
        __builtin_memmove(temp, node, sizeof(ASTNode));
        __builtin_memmove(node, temp, sizeof(ASTNode));
    }
    goto skip_children;
}

/* Complex memory operation function with OpenMP */
static void parallel_memory_operations(void) {
    const int array_size = 1024;
    char* src = (char*)malloc(array_size);
    char* dst = (char*)malloc(array_size);
    
    if (!src || !dst) {
        free(src);
        free(dst);
        return;
    }
    
    /* Initialize source with pattern */
    for (int i = 0; i < array_size; i++) {
        src[i] = (char)(i % 256);
    }
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int chunk_size = array_size / omp_get_num_threads();
        int start = thread_id * chunk_size;
        
        /* Each thread performs different memory operations */
        switch (thread_id % 3) {
            case 0:
                __builtin_memcpy(dst + start, src + start, chunk_size);
                break;
            case 1:
                __builtin_memset(dst + start, thread_id, chunk_size);
                break;
            case 2:
                __builtin_memmove(dst + start, src + start, chunk_size);
                /* Additional goto for flow control */
                if (chunk_size > 64) {
                    goto large_chunk;
                }
                break;
            large_chunk:
                /* Extra memory operation for large chunks */
                __builtin_memcpy(dst + start + 32, src + start + 32, chunk_size - 64);
                break;
        }
    }
    
    /* Verify copy with volatile access */
    volatile char verify_char = dst[array_size / 2];
    (void)verify_char;  /* Prevent unused warning */
    
    free(src);
    free(dst);
}

/* Recursive tree copy with memory operations */
void copy_ast(ASTNode* dest, const ASTNode* src) {
    if (!dest || !src) return;
    
    /* Direct structure copy using __builtin_memcpy */
    __builtin_memcpy(dest, src, sizeof(ASTNode));
    
    /* Recursive copy of children */
    if (src->left) {
        dest->left = (ASTNode*)malloc(sizeof(ASTNode));
        if (dest->left) {
            copy_ast(dest->left, src->left);
        }
    }
    
    if (src->right) {
        dest->right = (ASTNode*)malloc(sizeof(ASTNode));
        if (dest->right) {
            copy_ast(dest->right, src->right);
        }
    }
}

/* Compute hash of AST for verification */
uint32_t hash_ast(const ASTNode* node) {
    if (!node) return 0;
    
    uint32_t hash = 2166136261u;
    
    /* Hash node contents */
    hash = (hash ^ node->type) * 16777619u;
    hash = (hash ^ node->value) * 16777619u;
    hash = (hash ^ (uint32_t)node->size) * 16777619u;
    
    /* Recursive hash of children */
    hash ^= hash_ast(node->left);
    hash ^= hash_ast(node->right);
    
    return hash;
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN memory operation tests\n");
    
    /* Initialize token processing */
    volatile int token_index = 0;
    char token_buffer[256];
    
    /* Process tokens with memory operations */
    for (int i = 0; i < token_count; i++) {
        size_t len = strlen(tokens[i]);
        __builtin_memcpy(token_buffer + token_index, tokens[i], len);
        token_index += len;
        token_buffer[token_index++] = ' ';
    }
    token_buffer[token_index] = '\0';
    
    /* Create and manipulate AST */
    int counter = 0;
    ASTNode* root = create_ast(4, &counter);
    
    if (root) {
        /* Create copy for memmove testing */
        ASTNode* copy = (ASTNode*)malloc(sizeof(ASTNode));
        if (copy) {
            /* Test __builtin_memmove with overlapping regions */
            char* overlap_src = (char*)root;
            char* overlap_dst = overlap_src + g_copy_offset;
            __builtin_memmove(overlap_dst, overlap_src, sizeof(ASTNode) - g_copy_offset);
            
            copy_ast(copy, root);
            
            /* Compute and print verification hash */
            uint32_t original_hash = hash_ast(root);
            uint32_t copy_hash = hash_ast(copy);
            
            printf("AST Hash Verification:\n");
            printf("  Original: %u\n", original_hash);
            printf("  Copy:     %u\n", copy_hash);
            printf("  Match:    %s\n", original_hash == copy_hash ? "YES" : "NO");
            
            free(copy);
        }
        
        /* Execute parallel memory operations */
        parallel_memory_operations();
        
        /* Cleanup */
        free(root);
    }
    
    /* Final memory operation with goto */
    volatile char final_buffer[128];
    
    goto final_memset;
    
final_memset:
    __builtin_memset(final_buffer, 0xFF, sizeof(final_buffer));
    
    /* One last memcpy for good measure */
    char verify_buffer[128];
    __builtin_memcpy(verify_buffer, final_buffer, sizeof(final_buffer));
    
    printf("Test completed successfully\n");
    return 0;
}
