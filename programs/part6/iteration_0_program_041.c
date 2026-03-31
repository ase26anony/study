/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    struct ASTNode* left;
    struct ASTNode* right;
    char data[64];
    int type;
    size_t size;
} ASTNode;

/* Global token array */
static const char* g_tokens[] = {
    "memcpy", "memset", "memmove", "asan", "hwasan", "test"
};
static const int g_token_count = sizeof(g_tokens) / sizeof(g_tokens[0]);

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_sanitizer_hook(void) {
    /* Force initialization of sanitizer runtime */
    volatile char dummy[16];
    __builtin_memset(dummy, 0, sizeof(dummy));
    printf("Constructor: Initialized sanitizer hooks\n");
}

/* Destructor function (runs after main) */
__attribute__((destructor))
static void cleanup_sanitizer_hook(void) {
    printf("Destructor: Cleaning up sanitizer state\n");
}

/* Recursive parser with memory operations */
static ASTNode* create_ast_node(const char* data, int depth) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use volatile to prevent folding */
    volatile size_t copy_size = sizeof(node->data) - 1;
    if (copy_size > strlen(data)) copy_size = strlen(data);
    
    /* Force __builtin_memcpy call */
    __builtin_memcpy(node->data, data, copy_size);
    node->data[copy_size] = '\0';
    
    /* Initialize with __builtin_memset */
    __builtin_memset(&node->type, 0, sizeof(node->type));
    node->type = depth;
    node->size = copy_size;
    
    node->left = NULL;
    node->right = NULL;
    
    if (depth > 0) {
        /* Create children with goto for flow control */
        int create_left = 1;
        
        /* Jump into memory operation block */
        if (depth % 2 == 0) {
            goto create_child;
        }
        
        create_child:
        if (create_left) {
            char child_data[128];
            /* Use __builtin_memmove for overlapping buffers */
            __builtin_memmove(child_data, data, copy_size);
            child_data[copy_size] = '_';
            child_data[copy_size + 1] = 'L';
            child_data[copy_size + 2] = '\0';
            
            node->left = create_ast_node(child_data, depth - 1);
            create_left = 0;
            goto create_child; /* Jump back */
        } else {
            char child_data[128];
            __builtin_memmove(child_data, data, copy_size);
            child_data[copy_size] = '_';
            child_data[copy_size + 1] = 'R';
            child_data[copy_size + 2] = '\0';
            
            node->right = create_ast_node(child_data, depth - 1);
        }
    }
    
    return node;
}

/* Complex memory operation with goto jumps */
static void perform_memory_operations(void* buffer1, void* buffer2, size_t size) {
    volatile int operation_phase = 0;
    
    phase_start:
    switch (operation_phase) {
        case 0:
            /* Clear buffer with __builtin_memset */
            __builtin_memset(buffer1, 0xAA, size);
            operation_phase = 1;
            goto phase_start;
            
        case 1:
            /* Copy with __builtin_memcpy */
            __builtin_memcpy(buffer2, buffer1, size / 2);
            operation_phase = 2;
            goto phase_start;
            
        case 2: {
            /* Overlapping copy with __builtin_memmove */
            void* mid_point = (char*)buffer1 + size / 4;
            __builtin_memmove(mid_point, buffer1, size / 2);
            
            /* Jump out of switch block */
            if (size > 100) {
                goto phase_complete;
            }
            operation_phase = 3;
            goto phase_start;
        }
            
        case 3:
            /* Additional memset */
            __builtin_memset(buffer2, 0xBB, size / 4);
            break;
    }
    
    phase_complete:
    return;
}

/* Calculate hash from AST */
static uint64_t hash_ast(ASTNode* node) {
    if (!node) return 0;
    
    uint64_t hash = 5381;
    char* ptr = node->data;
    
    /* Simple hash calculation */
    while (*ptr) {
        hash = ((hash << 5) + hash) + *ptr++;
    }
    
    /* Recursive hash combination */
    uint64_t left_hash = hash_ast(node->left);
    uint64_t right_hash = hash_ast(node->right);
    
    /* Mix hashes with memory operations */
    volatile uint64_t mixed[3] = {hash, left_hash, right_hash};
    volatile uint64_t result = 0;
    
    /* Use builtins on volatile arrays */
    __builtin_memcpy(&result, mixed, sizeof(uint64_t));
    __builtin_memset((void*)(mixed + 1), 0, sizeof(uint64_t) * 2);
    
    return result ^ left_hash ^ right_hash;
}

/* Free AST recursively */
static void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    
    /* Clear data before free */
    volatile char* data = node->data;
    __builtin_memset(data, 0, sizeof(node->data));
    free(node);
}

int main(void) {
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create buffers with volatile size */
    volatile size_t buf_size = g_mem_size;
    char* buffer1 = malloc(buf_size);
    char* buffer2 = malloc(buf_size);
    
    if (!buffer1 || !buffer2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize buffers with different patterns */
    __builtin_memset(buffer1, 0xCC, buf_size);
    __builtin_memset(buffer2, 0xDD, buf_size);
    
    /* Create recursive AST */
    ASTNode* root = create_ast_node(g_tokens[0], 3);
    
    /* Parallel memory operations using OpenMP */
    #pragma omp parallel
    {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        
        /* Each thread performs memory operations */
        size_t thread_buf_size = buf_size / 4;
        char* thread_buf = malloc(thread_buf_size);
        
        if (thread_buf) {
            /* Force builtin calls in parallel context */
            __builtin_memset(thread_buf, thread_id, thread_buf_size);
            
            /* Copy between buffers */
            if (thread_id % 2 == 0) {
                __builtin_memcpy(thread_buf, buffer1, thread_buf_size);
            } else {
                __builtin_memmove(thread_buf, buffer2, thread_buf_size);
            }
            
            free(thread_buf);
        }
    }
    
    /* Perform complex memory operations with goto */
    perform_memory_operations(buffer1, buffer2, buf_size);
    
    /* Calculate and print hash result */
    uint64_t hash_result = hash_ast(root);
    printf("AST Hash Result: 0x%016llx\n", (unsigned long long)hash_result);
    
    /* Additional memory operations in cleanup */
    volatile char final_check[32];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
    __builtin_memcpy(final_check, buffer1, sizeof(final_check) < buf_size ? sizeof(final_check) : buf_size);
    
    /* Cleanup */
    free_ast(root);
    free(buffer1);
    free(buffer2);
    
    printf("Test completed successfully\n");
    return 0;
}
