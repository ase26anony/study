/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int value;
    uint8_t padding[32]; /* Ensure size for memcpy operations */
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
static const int g_token_count = 6;

/* Constructor function (runs before main) */
__attribute__((constructor))
static void init_sanitizer_hook(void) {
    printf("Constructor: Initializing sanitizer hooks\n");
    /* Force early initialization of memory functions */
    char buffer[16];
    __builtin_memset(buffer, 0, sizeof(buffer));
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_sanitizer_hook(void) {
    printf("Destructor: Cleaning up\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast(int depth, const char* token) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memset to initialize node */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy token using __builtin_memcpy */
    size_t len = strlen(token);
    if (len > sizeof(node->data) - 1)
        len = sizeof(node->data) - 1;
    __builtin_memcpy(node->data, token, len);
    node->data[len] = '\0';
    
    /* Recursive creation with goto for flow control */
    int create_left = 1;
    if (depth % 2 == 0) {
        goto skip_left;
    }
    
    node->left = create_ast(depth - 1, "left");
    create_left = 0;
    
skip_left:
    if (create_left) {
        node->left = create_ast(depth - 2, "left_alt");
    }
    
    /* Another goto jumping into block with memmove */
    if (depth > 3) {
        goto memmove_block;
    } else {
        node->right = create_ast(depth - 1, "right");
        goto after_memmove;
    }
    
memmove_block:
    {
        ASTNode temp_node;
        __builtin_memset(&temp_node, 0xA5, sizeof(temp_node));
        /* Use __builtin_memmove with overlapping regions */
        __builtin_memmove(node->data + 10, node->data, 20);
        node->right = create_ast(depth - 3, "right_moved");
    }
    
after_memmove:
    node->value = depth * 1000 + (int)len;
    return node;
}

/* Calculate hash of AST */
static uint64_t hash_ast(const ASTNode* node) {
    if (!node) return 0;
    
    uint64_t hash = 5381;
    const char* p = node->data;
    
    /* DJB2 hash algorithm */
    while (*p) {
        hash = ((hash << 5) + hash) + *p++;
    }
    
    hash ^= hash_ast(node->left);
    hash ^= hash_ast(node->right);
    hash ^= (uint64_t)node->value;
    
    return hash;
}

/* Free AST with memory clearing */
static void free_ast(ASTNode* node) {
    if (!node) return;
    
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear sensitive data before free */
    volatile char* data = (volatile char*)node->data;
    for (size_t i = 0; i < sizeof(node->data); i++) {
        data[i] = 0;
    }
    
    free(node);
}

/* Complex memory operation with OpenMP */
static void parallel_memory_operations(void) {
    const size_t buffer_size = (size_t)g_mem_size;
    char* src_buffer = (char*)malloc(buffer_size);
    char* dst_buffer = (char*)malloc(buffer_size);
    
    if (!src_buffer || !dst_buffer) {
        free(src_buffer);
        free(dst_buffer);
        return;
    }
    
    /* Initialize source with pattern */
    #pragma omp parallel for
    for (size_t i = 0; i < buffer_size; i++) {
        src_buffer[i] = (char)(i % 256);
    }
    
    /* OpenMP parallel region with memory builtins */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        size_t chunk_size = buffer_size / omp_get_num_threads();
        size_t start = thread_id * chunk_size;
        size_t end = (thread_id == omp_get_num_threads() - 1) ? 
                     buffer_size : start + chunk_size;
        
        /* Use all three builtins in parallel */
        __builtin_memset(dst_buffer + start, thread_id, end - start);
        
        /* Conditional memcpy based on thread ID */
        if (thread_id % 2 == 0) {
            __builtin_memcpy(src_buffer + start, dst_buffer + start, end - start);
        } else {
            /* Overlapping memmove */
            size_t move_size = (end - start) / 2;
            if (move_size > 0) {
                __builtin_memmove(src_buffer + start + move_size, 
                                 src_buffer + start, move_size);
            }
        }
    }
    
    /* Final verification memcpy */
    __builtin_memcpy(dst_buffer, src_buffer, buffer_size);
    
    /* Calculate checksum */
    uint64_t checksum = 0;
    #pragma omp parallel for reduction(+:checksum)
    for (size_t i = 0; i < buffer_size; i++) {
        checksum += (uint8_t)dst_buffer[i];
    }
    
    printf("Parallel checksum: %llu\n", (unsigned long long)checksum);
    
    free(src_buffer);
    free(dst_buffer);
}

/* Function with goto jumping around memcpy */
static void goto_memcpy_test(void) {
    char buffer1[256];
    char buffer2[256];
    int use_memcpy = 1;
    
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memset(buffer2, 0x55, sizeof(buffer2));
    
    if (g_use_hwasan) {
        goto alternative_path;
    }
    
    /* This memcpy should be redirected */
    __builtin_memcpy(buffer1, buffer2, 128);
    goto after_copy;
    
alternative_path:
    /* Alternative path with different memcpy size */
    __builtin_memcpy(buffer1 + 64, buffer2 + 64, 64);
    
after_copy:
    /* Verify the copy */
    int match = 1;
    for (int i = 0; i < 128; i++) {
        if (buffer1[i] != buffer2[i]) {
            match = 0;
            break;
        }
    }
    printf("Goto memcpy test: %s\n", match ? "PASS" : "FAIL");
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Phase 1: Recursive AST operations */
    printf("\nPhase 1: Creating AST structure\n");
    ASTNode* root = create_ast(5, g_tokens[0]);
    if (root) {
        uint64_t hash = hash_ast(root);
        printf("AST hash: 0x%016llx\n", (unsigned long long)hash);
        
        /* Copy entire AST node */
        ASTNode node_copy;
        __builtin_memcpy(&node_copy, root, sizeof(ASTNode));
        
        /* Move data within node */
        __builtin_memmove(root->data, root->data + 10, 20);
        
        free_ast(root);
    }
    
    /* Phase 2: Parallel memory operations */
    printf("\nPhase 2: Parallel memory operations\n");
    parallel_memory_operations();
    
    /* Phase 3: Goto flow control tests */
    printf("\nPhase 3: Goto flow control tests\n");
    goto_memcpy_test();
    
    /* Phase 4: Token array processing */
    printf("\nPhase 4: Token processing\n");
    char token_buffer[256] = {0};
    char* current = token_buffer;
    
    for (int i = 0; i < g_token_count; i++) {
        size_t len = strlen(g_tokens[i]);
        __builtin_memcpy(current, g_tokens[i], len);
        current += len;
        if (i < g_token_count - 1) {
            __builtin_memset(current, ':', 1);
            current += 1;
        }
    }
    
    printf("Concatenated tokens: %s\n", token_buffer);
    
    /* Phase 5: Variable-sized memory operations */
    printf("\nPhase 5: Variable-sized operations\n");
    volatile size_t dynamic_size = 512;
    char* dyn_buf1 = (char*)malloc(dynamic_size);
    char* dyn_buf2 = (char*)malloc(dynamic_size);
    
    if (dyn_buf1 && dyn_buf2) {
        __builtin_memset(dyn_buf1, 0xCC, dynamic_size);
        __builtin_memcpy(dyn_buf2, dyn_buf1, dynamic_size / 2);
        
        /* Overlapping memmove */
        __builtin_memmove(dyn_buf1 + dynamic_size/4, 
                         dyn_buf1, dynamic_size/4);
        
        free(dyn_buf1);
        free(dyn_buf2);
    }
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
