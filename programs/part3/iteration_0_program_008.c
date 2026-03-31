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
static void init_asan_globals(void) {
    volatile char buffer[128];
    /* Force __builtin_memset in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    g_mem_size = 128;
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_asan_globals(void) {
    volatile char final_buffer[64];
    __builtin_memset(final_buffer, 0xFF, sizeof(final_buffer));
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use __builtin_memcpy with volatile size */
    volatile size_t copy_size = (depth * 16) % 256;
    __builtin_memcpy(node->data, base_data, copy_size);
    node->size = copy_size;
    
    /* Initialize remaining with __builtin_memset */
    if (copy_size < sizeof(node->data)) {
        size_t remaining = sizeof(node->data) - copy_size;
        __builtin_memset(node->data + copy_size, depth, remaining);
    }
    
    /* Recursive creation with goto for control flow */
    int create_left = 1;
    
    if (depth > 3) {
        goto skip_left;
    }
    
    create_left = 0;
    
skip_left:
    if (create_left) {
        node->left = create_ast(depth - 1, base_data);
        
        /* Complex goto pattern around __builtin_memmove */
        if (node->left) {
            char temp[256];
            volatile int use_memmove = 1;
            
            if (use_memmove) {
                goto do_memmove;
            }
            
            __builtin_memcpy(temp, node->left->data, node->left->size);
            goto after_memmove;
            
        do_memmove:
            /* This should trigger the memmove redirection */
            __builtin_memmove(temp, node->left->data, node->left->size);
            
        after_memmove:
            /* Use the moved data */
            __builtin_memcpy(node->data + 128, temp, 32);
        }
    } else {
        node->left = NULL;
    }
    
    node->right = create_ast(depth - 2, base_data);
    
    return node;
}

/* Function with OpenMP parallel section */
static void parallel_memory_operations(void) {
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        volatile char local_buf[256];
        volatile char src_buf[256];
        
        /* Initialize source with pattern */
        for (int i = 0; i < 256; i++) {
            src_buf[i] = (char)((i + thread_id) & 0xFF);
        }
        
        /* Force all three builtins in parallel region */
        __builtin_memset(local_buf, thread_id, sizeof(local_buf));
        
        volatile size_t copy_len = g_mem_size;
        if (copy_len > 256) copy_len = 256;
        
        __builtin_memcpy(local_buf, src_buf, copy_len);
        
        /* Conditional memmove with goto */
        if (thread_id % 2 == 0) {
            goto use_memmove;
        }
        
        __builtin_memcpy(src_buf, local_buf, copy_len);
        goto after_parallel_move;
        
    use_memmove:
        __builtin_memmove(src_buf, local_buf, copy_len);
        
    after_parallel_move:
        /* Use the result */
        volatile char sum = 0;
        for (int i = 0; i < copy_len; i++) {
            sum += src_buf[i];
        }
    }
}

/* Multi-stage initialization function */
static void initialize_complex_buffer(char* buffer, size_t size) {
    volatile char pattern[512];
    volatile size_t pattern_size = size % 512;
    
    /* Nested memory operations */
    __builtin_memset(pattern, 0xCC, sizeof(pattern));
    
    for (int i = 0; i < 3; i++) {
        volatile size_t offset = (i * 64) % 256;
        
        if (i == 1) {
            /* Jump into memcpy block */
            goto do_memcpy;
        }
        
        __builtin_memset(buffer + offset, i, 64);
        continue;
        
    do_memcpy:
        __builtin_memcpy(buffer + offset, pattern + offset, 64);
        
        /* Another goto to test flow sensitivity */
        if (offset > 128) {
            goto large_offset;
        }
        
        __builtin_memset(buffer + offset + 32, 0xFF, 32);
        goto after_large_offset;
        
    large_offset:
        __builtin_memmove(buffer + offset - 32, pattern, 32);
        
    after_large_offset:
        ; /* Empty statement for label */
    }
}

int main(void) {
    printf("Starting ASAN built-in redirection test...\n");
    
    /* Phase 1: Basic built-in usage */
    volatile char buffer1[1024];
    volatile char buffer2[1024];
    
    __builtin_memset(buffer1, 0x11, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1, buffer2, sizeof(buffer1));
    
    /* Phase 2: Recursive AST operations */
    ASTNode* root = create_ast(5, "AST Base Data for ASAN Testing");
    
    if (root) {
        /* Copy between AST nodes */
        if (root->left && root->right) {
            size_t copy_size = root->left->size;
            if (copy_size > root->right->size) {
                copy_size = root->right->size;
            }
            
            /* This should trigger memcpy redirection */
            __builtin_memcpy(root->right->data, root->left->data, copy_size);
        }
        
        /* Free AST recursively */
        ASTNode* nodes[32];
        int node_count = 0;
        ASTNode* current = root;
        
        while (current || node_count > 0) {
            while (current) {
                nodes[node_count++] = current;
                current = current->left;
            }
            
            if (node_count > 0) {
                current = nodes[--node_count];
                ASTNode* right = current->right;
                
                /* Clear node data before free */
                __builtin_memset(current->data, 0, sizeof(current->data));
                free(current);
                current = right;
            }
        }
    }
    
    /* Phase 3: OpenMP parallel operations */
    #ifdef _OPENMP
    parallel_memory_operations();
    #endif
    
    /* Phase 4: Complex buffer initialization */
    char complex_buffer[2048];
    initialize_complex_buffer(complex_buffer, sizeof(complex_buffer));
    
    /* Phase 5: Verify results with checksum */
    unsigned long checksum = 0;
    for (size_t i = 0; i < sizeof(complex_buffer); i++) {
        checksum += (unsigned char)complex_buffer[i];
    }
    
    /* Also check volatile buffers */
    for (size_t i = 0; i < sizeof(buffer1); i += 64) {
        checksum += (unsigned char)buffer1[i];
    }
    
    printf("Test completed. Checksum: %lu\n", checksum);
    printf("If compiled with -fsanitize=address or -fsanitize=kernel-hwaddress,\n");
    printf("this should trigger ASAN/HWASAN built-in redirection logic.\n");
    
    return 0;
}
