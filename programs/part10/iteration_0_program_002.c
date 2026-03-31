/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 1024;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char *data;
    size_t len;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *parent;
} ASTNode;

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_environment(void) {
    volatile char init_buf[64];
    /* Force __builtin_memset in constructor */
    __builtin_memset(init_buf, 0xAA, sizeof(init_buf));
    
    /* Test memcpy in constructor context */
    volatile char src_buf[32];
    __builtin_memcpy(init_buf + 16, src_buf, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_environment(void) {
    volatile char cleanup_buf[128];
    __builtin_memset(cleanup_buf, 0xFF, sizeof(cleanup_buf));
}

/* Recursive tree manipulation with memory operations */
static ASTNode* create_node(const char *data, size_t len) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->data = malloc(len + 1);
    if (!node->data) {
        free(node);
        return NULL;
    }
    
    /* Use __builtin_memcpy for data initialization */
    __builtin_memcpy(node->data, data, len);
    node->data[len] = '\0';
    node->len = len;
    node->left = node->right = node->parent = NULL;
    
    return node;
}

static void copy_node_data(ASTNode *dest, ASTNode *src) {
    if (dest->data) free(dest->data);
    dest->data = malloc(src->len + 1);
    dest->len = src->len;
    
    /* Test __builtin_memmove with overlapping regions */
    if (dest == src->parent) {
        char temp[256];
        __builtin_memcpy(temp, src->data, src->len);
        __builtin_memmove(dest->data, temp, src->len);
    } else {
        __builtin_memcpy(dest->data, src->data, src->len);
    }
    dest->data[src->len] = '\0';
}

/* Function with goto jumps around memory operations */
static void test_goto_memmove(void) {
    volatile char buffer1[256];
    volatile char buffer2[256];
    int use_memmove = 1;
    
    /* Initialize buffers */
    __builtin_memset(buffer1, 0x11, sizeof(buffer1));
    __builtin_memset(buffer2, 0x22, sizeof(buffer2));
    
    goto jump_point;
    
no_memmove:
    __builtin_memcpy(buffer1, buffer2, 128);
    goto end;
    
jump_point:
    if (use_memmove) {
        /* Jump into memmove block */
        goto do_memmove;
    } else {
        goto no_memmove;
    }
    
do_memmove:
    /* Overlapping memory regions to force memmove */
    __builtin_memmove((char*)buffer1 + 64, buffer1, 128);
    goto end;
    
end:
    /* Final memset */
    __builtin_memset(buffer1, 0, 64);
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    volatile size_t local_size = g_mem_size;
    volatile char *parallel_buf = malloc(local_size);
    
    if (!parallel_buf) return;
    
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        size_t chunk = local_size / omp_get_num_threads();
        size_t start = tid * chunk;
        
        /* Each thread uses builtins */
        __builtin_memset(parallel_buf + start, tid, chunk);
        
        #pragma omp barrier
        
        /* Test memcpy between threads' regions */
        if (tid % 2 == 0) {
            size_t target = ((tid + 1) % omp_get_num_threads()) * chunk;
            __builtin_memcpy(parallel_buf + target, 
                           parallel_buf + start, 
                           chunk > 64 ? 64 : chunk);
        }
        
        #pragma omp barrier
        
        /* Test memmove within thread's region */
        if (chunk > 128) {
            __builtin_memmove(parallel_buf + start + 32,
                            parallel_buf + start,
                            64);
        }
    }
    
    /* Verify with checksum */
    unsigned long sum = 0;
    for (size_t i = 0; i < local_size; i++) {
        sum += parallel_buf[i];
    }
    printf("Parallel checksum: %lu\n", sum);
    
    free((void*)parallel_buf);
}

/* Complex token parsing with memory operations */
static unsigned long parse_tokens(const char **tokens, int count) {
    volatile char parse_buffer[512];
    unsigned long hash = 5381;
    
    for (int i = 0; i < count; i++) {
        size_t len = strlen(tokens[i]);
        volatile size_t copy_len = len > 64 ? 64 : len;
        
        /* Clear buffer section */
        __builtin_memset(parse_buffer + (i * 64), 0, 64);
        
        /* Copy token */
        __builtin_memcpy(parse_buffer + (i * 64), tokens[i], copy_len);
        
        /* Move data around */
        if (i > 0) {
            __builtin_memmove(parse_buffer + (i * 64) - 16,
                            parse_buffer + (i * 64),
                            32);
        }
        
        /* Update hash */
        for (size_t j = 0; j < copy_len; j++) {
            hash = ((hash << 5) + hash) + parse_buffer[(i * 64) + j];
        }
    }
    
    return hash;
}

/* Multi-stage initialization */
static void initialize_memory_system(void) {
    volatile char stage1[256];
    volatile char stage2[256];
    
    /* Stage 1: Basic memset/memcpy */
    __builtin_memset(stage1, 0xCC, sizeof(stage1));
    __builtin_memcpy(stage2, stage1, sizeof(stage1));
    
    /* Stage 2: Overlapping memmove */
    __builtin_memmove(stage1 + 128, stage1, 128);
    
    /* Stage 3: Reverse copy */
    for (int i = 0; i < 4; i++) {
        __builtin_memcpy(stage2 + (i * 64), 
                       stage1 + (3 - i) * 64, 
                       64);
    }
}

int main(void) {
    printf("Starting ASAN built-in redirection test\n");
    
    /* Initialize complex token array */
    const char *tokens[] = {
        "memcpy", "memset", "memmove", "asan", "hwasan",
        "instrumentation", "redzone", "builtin", "coverage"
    };
    int token_count = sizeof(tokens) / sizeof(tokens[0]);
    
    /* Stage 1: Token parsing with memory ops */
    unsigned long token_hash = parse_tokens(tokens, token_count);
    printf("Token hash: %lu\n", token_hash);
    
    /* Stage 2: Recursive AST operations */
    ASTNode *root = create_node("root", 4);
    ASTNode *left = create_node("left", 4);
    ASTNode *right = create_node("right", 5);
    
    if (root && left && right) {
        root->left = left;
        root->right = right;
        left->parent = root;
        right->parent = root;
        
        /* Test node data copying */
        copy_node_data(left, right);
        
        /* Create circular reference for memmove test */
        ASTNode temp_node;
        temp_node.data = malloc(32);
        temp_node.len = 32;
        __builtin_memset(temp_node.data, 0x55, 32);
        
        copy_node_data(root, &temp_node);
        free(temp_node.data);
    }
    
    /* Stage 3: Goto flow control test */
    test_goto_memmove();
    
    /* Stage 4: Multi-stage initialization */
    initialize_memory_system();
    
    /* Stage 5: OpenMP parallel operations */
    parallel_memory_ops();
    
    /* Cleanup */
    if (root) {
        if (root->data) free(root->data);
        if (root->left) {
            if (root->left->data) free(root->left->data);
            free(root->left);
        }
        if (root->right) {
            if (root->right->data) free(root->right->data);
            free(root->right);
        }
        free(root);
    }
    
    printf("Test completed successfully\n");
    return 0;
}
