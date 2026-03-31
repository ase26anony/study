/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_memmove = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t id;
} ASTNode;

/* Global token array */
static const char* tokens[] = {"memcpy", "memset", "memmove", "test", "data"};
static const size_t num_tokens = sizeof(tokens)/sizeof(tokens[0]);

/* Constructor attribute for initialization */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    printf("Constructor: Initializing ASAN environment\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor)) 
static void cleanup_asan_environment(void) {
    printf("Destructor: ASAN cleanup completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(size_t depth, size_t* counter) {
    if (depth == 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Fill data with pattern using __builtin_memcpy */
    char pattern[64];
    __builtin_memset(pattern, 'A' + (depth % 26), sizeof(pattern));
    __builtin_memcpy(node->data, pattern, sizeof(node->data));
    
    node->id = (*counter)++;
    
    /* Recursive creation with goto for flow control */
    if (depth > 1) {
        int use_goto = (depth % 3 == 0);
        
        if (use_goto) {
            goto create_children;
        }
        
        node->left = create_ast(depth - 1, counter);
        node->right = create_ast(depth - 1, counter);
        return node;
        
    create_children:
        /* Jump target with __builtin_memmove */
        ASTNode temp;
        __builtin_memcpy(&temp, node, sizeof(ASTNode));
        __builtin_memset(node, 0, sizeof(ASTNode));
        __builtin_memmove(node, &temp, sizeof(ASTNode));
        
        node->left = create_ast(depth - 1, counter);
        node->right = create_ast(depth - 1, counter);
    }
    
    return node;
}

/* Function with complex memory operations and OpenMP */
static size_t process_ast_parallel(ASTNode* root) {
    if (!root) return 0;
    
    size_t total_hash = 0;
    volatile size_t local_size = g_mem_size;
    
    #pragma omp parallel reduction(+:total_hash)
    {
        int thread_id = omp_get_thread_num();
        char buffer[512];
        char dest_buffer[512];
        
        /* Initialize buffers with thread-specific pattern */
        __builtin_memset(buffer, thread_id, sizeof(buffer));
        
        #pragma omp for
        for (size_t i = 0; i < local_size; i++) {
            /* Vary memory operations based on conditions */
            if (i % 3 == 0) {
                /* Use __builtin_memcpy */
                __builtin_memcpy(dest_buffer, buffer, 
                    (i % 128) + 64);
            } else if (i % 3 == 1) {
                /* Use __builtin_memset */
                __builtin_memset(dest_buffer + (i % 256), 
                    (i % 26) + 'A', 32);
            } else {
                /* Use __builtin_memmove with overlap */
                size_t offset = (i % 128);
                __builtin_memmove(dest_buffer + offset, 
                    dest_buffer, 128);
            }
            
            /* Process buffer content */
            for (size_t j = 0; j < 64; j++) {
                total_hash += dest_buffer[j];
            }
        }
        
        /* Additional memory operation after loop */
        if (g_use_memmove) {
            char temp[256];
            __builtin_memcpy(temp, buffer, 256);
            __builtin_memmove(buffer + 128, buffer, 128);
            __builtin_memcpy(buffer, temp, 256);
        }
    }
    
    return total_hash;
}

/* Function with goto jumping into memory operation block */
static void test_goto_memmove(void) {
    char src[256], dst[256];
    
    /* Initialize source */
    for (int i = 0; i < 256; i++) {
        src[i] = i % 256;
    }
    
    int use_goto = 1;
    
    if (use_goto) {
        goto jump_into_memmove;
    }
    
    /* Normal path */
    __builtin_memcpy(dst, src, 256);
    return;
    
jump_into_memmove:
    /* Target label with __builtin_memmove */
    __builtin_memset(dst, 0, 256);
    __builtin_memmove(dst, src, 256);
    
    /* Jump out */
    goto exit_block;
    
exit_block:
    /* Verify the copy */
    for (int i = 0; i < 256; i++) {
        if (dst[i] != src[i]) {
            printf("Mismatch at position %d\n", i);
        }
    }
}

/* Main execution flow */
int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Initialize AST */
    size_t counter = 0;
    ASTNode* root = create_ast(4, &counter);
    
    if (!root) {
        fprintf(stderr, "Failed to create AST\n");
        return 1;
    }
    
    printf("Created AST with %zu nodes\n", counter);
    
    /* Test goto with memmove */
    test_goto_memmove();
    
    /* Process AST in parallel */
    size_t hash_result = process_ast_parallel(root);
    printf("Parallel processing hash: %zu\n", hash_result);
    
    /* Additional memory operations with volatile control */
    volatile size_t dynamic_size = 1024;
    char* dynamic_buf = (char*)malloc(dynamic_size);
    char* dynamic_buf2 = (char*)malloc(dynamic_size);
    
    if (dynamic_buf && dynamic_buf2) {
        /* Force all three built-ins */
        __builtin_memset(dynamic_buf, 0xCC, dynamic_size);
        __builtin_memcpy(dynamic_buf2, dynamic_buf, dynamic_size);
        
        /* Create overlapping regions for memmove */
        size_t overlap_point = dynamic_size / 2;
        __builtin_memmove(dynamic_buf + overlap_point, 
                         dynamic_buf, 
                         dynamic_size - overlap_point);
        
        /* Verify with simple checksum */
        size_t checksum = 0;
        for (size_t i = 0; i < dynamic_size; i++) {
            checksum += dynamic_buf[i];
        }
        printf("Dynamic buffer checksum: %zu\n", checksum);
    }
    
    /* Cleanup */
    free(dynamic_buf);
    free(dynamic_buf2);
    
    /* Note: In real code, would need to free AST recursively */
    
    printf("Test completed successfully\n");
    return 0;
}
