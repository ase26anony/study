/* asan_coverage.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile int g_volatile_size = 64;
static volatile char g_volatile_flag = 1;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[256];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_constructor(void) {
    volatile char buffer[128];
    /* Force __builtin_memset in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    printf("Constructor: Initialized buffer with memset\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_destructor(void) {
    volatile int cleanup_buf[32];
    /* Force __builtin_memcpy in destructor */
    int src[32];
    for (int i = 0; i < 32; i++) src[i] = i * 2;
    __builtin_memcpy(cleanup_buf, src, sizeof(src));
    printf("Destructor: Cleaned up with memcpy\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy for node initialization */
    size_t copy_size = strlen(base_data) + 1;
    if (copy_size > sizeof(node->data)) copy_size = sizeof(node->data);
    __builtin_memcpy(node->data, base_data, copy_size);
    
    /* Use volatile-controlled memset */
    volatile size_t pad_size = g_volatile_size % 128;
    __builtin_memset(node->data + copy_size, 0, pad_size);
    
    node->size = copy_size + pad_size;
    node->left = create_ast(depth - 1, "left_child");
    node->right = create_ast(depth - 1, "right_child");
    
    return node;
}

/* Function with goto jumps around memmove */
static void test_goto_memmove(void) {
    char buffer1[512];
    char buffer2[512];
    volatile int use_memmove = g_volatile_flag;
    
    /* Initialize buffers */
    for (int i = 0; i < 512; i++) {
        buffer1[i] = i % 256;
        buffer2[i] = 255 - (i % 256);
    }
    
    int state = 0;
    
    /* Jump into block with memmove */
    if (use_memmove) goto memmove_block;
    
normal_path:
    /* Use __builtin_memcpy */
    __builtin_memcpy(buffer1 + 100, buffer2 + 200, 128);
    state = 1;
    goto continue_test;
    
memmove_block:
    /* This block should trigger __builtin_memmove */
    __builtin_memmove(buffer1 + 50, buffer1 + 150, 256);
    state = 2;
    
    /* Jump out to avoid optimization */
    if (state == 2) goto continue_test;
    
continue_test:
    /* Verify the operation */
    volatile int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += buffer1[i];
    }
    printf("Goto test completed with state=%d, sum=%d\n", state, sum);
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_ops(void) {
    const int NUM_THREADS = 4;
    char thread_buffers[NUM_THREADS][1024];
    volatile int results[NUM_THREADS];
    
    #pragma omp parallel for
    for (int i = 0; i < NUM_THREADS; i++) {
        /* Each thread uses different builtins */
        volatile size_t op_size = (g_volatile_size * (i + 1)) % 512;
        
        switch (i % 3) {
            case 0:
                __builtin_memset(thread_buffers[i], i, op_size);
                break;
            case 1:
                if (i > 0) {
                    __builtin_memcpy(thread_buffers[i], 
                                   thread_buffers[i-1], 
                                   op_size);
                }
                break;
            case 2:
                __builtin_memmove(thread_buffers[i] + 100,
                                thread_buffers[i],
                                op_size);
                break;
        }
        
        /* Compute checksum */
        int sum = 0;
        for (int j = 0; j < op_size && j < 1024; j++) {
            sum += thread_buffers[i][j];
        }
        results[i] = sum;
    }
    
    /* Aggregate results */
    int total = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        total += results[i];
    }
    printf("Parallel ops total checksum: %d\n", total);
}

/* Complex token processing with nested memory ops */
static void process_tokens(char** tokens, int count) {
    char combined[2048];
    volatile size_t offset = 0;
    
    for (int i = 0; i < count; i++) {
        size_t token_len = strlen(tokens[i]);
        
        /* Use goto for flow control */
        if (token_len > 100) goto large_token;
        
        /* Normal path with memcpy */
        __builtin_memcpy(combined + offset, tokens[i], token_len);
        offset += token_len;
        continue;
        
    large_token:
        /* Handle large token with memmove */
        if (offset > 0) {
            __builtin_memmove(combined + 50, combined, offset);
            offset = 50;
        }
        size_t copy_len = token_len;
        if (copy_len > 512) copy_len = 512;
        __builtin_memcpy(combined + offset, tokens[i], copy_len);
        offset += copy_len;
    }
    
    /* Final memset to pad */
    volatile size_t pad = g_volatile_size % 64;
    __builtin_memset(combined + offset, 0xFF, pad);
    
    /* Compute final hash */
    unsigned long hash = 0;
    for (size_t i = 0; i < offset + pad && i < sizeof(combined); i++) {
        hash = (hash * 31) + combined[i];
    }
    printf("Token processing hash: 0x%lx\n", hash);
}

int main(void) {
    printf("=== Starting ASAN built-in redirection test ===\n");
    
    /* 1. Test recursive AST operations */
    ASTNode* root = create_ast(3, "root_node");
    if (root && root->left && root->right) {
        /* Copy between AST nodes */
        volatile size_t copy_size = root->size;
        if (copy_size > root->left->size) copy_size = root->left->size;
        __builtin_memcpy(root->right->data, root->left->data, copy_size);
        
        /* Move within node */
        __builtin_memmove(root->data + 10, root->data, 100);
    }
    
    /* 2. Test goto with memmove */
    test_goto_memmove();
    
    /* 3. Test OpenMP parallel operations */
    parallel_memory_ops();
    
    /* 4. Test token processing */
    char* tokens[] = {
        "short",
        "medium_length_token_here",
        "very_long_token_" 
        "that_continues_for_many_characters_" 
        "to_test_memmove_operations_properly",
        "another",
        "final"
    };
    process_tokens(tokens, 5);
    
    /* 5. Direct built-in calls with volatile sizes */
    volatile char final_buffer[1024];
    volatile char src_buffer[1024];
    
    for (int i = 0; i < 1024; i++) {
        src_buffer[i] = (i * 7) % 256;
    }
    
    /* Chain all three builtins */
    volatile size_t op_size = g_volatile_size;
    __builtin_memset(final_buffer, 0, op_size);
    __builtin_memcpy(final_buffer + 100, src_buffer, op_size);
    __builtin_memmove(final_buffer, final_buffer + 50, op_size);
    
    /* Compute verification sum */
    unsigned long long total_sum = 0;
    for (int i = 0; i < 1024; i++) {
        total_sum += final_buffer[i];
    }
    
    printf("Final buffer checksum: %llu\n", total_sum);
    printf("=== Test completed ===\n");
    
    /* Cleanup */
    /* Note: In real code, you'd want to free the AST properly */
    
    return 0;
}
