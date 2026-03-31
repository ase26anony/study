/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t g_memcpy_len = 64;
volatile size_t g_memset_len = 128;
volatile size_t g_memmove_len = 96;

/* Recursive AST-like structure */
struct ast_node {
    int type;
    char data[256];
    struct ast_node *left;
    struct ast_node *right;
};

/* Constructor function (runs before main) */
__attribute__((constructor)) 
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force early builtin usage in constructor */
    __builtin_memset(buffer, 0xAA, sizeof(buffer));
    __builtin_memcpy(buffer + 16, buffer, 16);
}

/* Destructor function (runs after main) */
__attribute__((destructor)) 
static void cleanup_asan_late(void) {
    volatile char final_check[64];
    __builtin_memset(final_check, 0xFF, sizeof(final_check));
}

/* Recursive function with memory operations */
static struct ast_node* build_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    struct ast_node* node = malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    
    /* Initialize with builtins */
    __builtin_memset(node, 0, sizeof(*node));
    node->type = depth;
    
    /* Copy data with volatile length */
    volatile size_t copy_len = g_memcpy_len % 256;
    __builtin_memcpy(node->data, base_data, copy_len);
    
    /* Build children recursively */
    node->left = build_ast(depth - 1, base_data + 1);
    node->right = build_ast(depth - 1, base_data + 2);
    
    return node;
}

/* Function with goto edge cases */
static void process_with_goto(struct ast_node* src, struct ast_node* dst) {
    int use_memmove = 0;
    
    /* Jump into block with memmove */
    if (src && dst) {
        goto memmove_block;
    }
    
normal_path:
    /* Regular memcpy */
    if (src && dst) {
        __builtin_memcpy(dst->data, src->data, g_memcpy_len % 256);
    }
    return;
    
memmove_block:
    /* Overlapping copy with goto out */
    use_memmove = 1;
    if (dst > src && (char*)dst < (char*)src + sizeof(struct ast_node)) {
        __builtin_memmove(dst, src, g_memmove_len % sizeof(struct ast_node));
        goto after_memmove;
    }
    
    /* Jump back */
    goto normal_path;
    
after_memmove:
    /* Additional processing */
    __builtin_memset(src->data, 0xCC, g_memset_len % 256);
}

/* OpenMP parallel section */
static void parallel_memory_ops(struct ast_node** nodes, int count) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Mix of builtins in parallel region */
                volatile char local_buf[128];
                
                __builtin_memset(local_buf, tid, sizeof(local_buf));
                __builtin_memcpy(nodes[i]->data, local_buf, 
                                g_memcpy_len % sizeof(local_buf));
                
                /* Conditional memmove with goto */
                if (i > 0 && nodes[i-1]) {
                    __builtin_memmove(nodes[i-1]->data + 64,
                                     nodes[i]->data,
                                     g_memmove_len % 128);
                }
            }
        }
        
        /* Thread-private memset */
        #pragma omp single
        {
            volatile char single_buf[256];
            __builtin_memset(single_buf, 0xDD, sizeof(single_buf));
        }
    }
}

/* Multi-stage processing */
static unsigned long compute_ast_hash(struct ast_node* node) {
    if (!node) return 0;
    
    unsigned long hash = 5381;
    volatile char* data = node->data;
    
    /* Process data with builtin-assisted loop */
    for (size_t i = 0; i < g_memcpy_len % 256; i++) {
        hash = ((hash << 5) + hash) + data[i];
    }
    
    /* Recursive hash combination */
    unsigned long left_hash = compute_ast_hash(node->left);
    unsigned long right_hash = compute_ast_hash(node->right);
    
    /* Combine with memcpy to temporary buffer */
    volatile char combine_buf[16];
    __builtin_memcpy(combine_buf, &left_hash, sizeof(left_hash));
    __builtin_memcpy(combine_buf + 8, &right_hash, sizeof(right_hash));
    
    return hash ^ left_hash ^ right_hash;
}

int main(void) {
    const int num_nodes = 8;
    struct ast_node* nodes[num_nodes];
    
    printf("Starting ASAN/HWASAN builtin redirection test\n");
    
    /* Phase 1: Build AST structures */
    for (int i = 0; i < num_nodes; i++) {
        char base_data[256];
        __builtin_memset(base_data, 'A' + i, sizeof(base_data));
        nodes[i] = build_ast(3, base_data);
    }
    
    /* Phase 2: Goto-based processing */
    for (int i = 1; i < num_nodes; i++) {
        process_with_goto(nodes[i-1], nodes[i]);
    }
    
    /* Phase 3: OpenMP parallel operations */
    parallel_memory_ops(nodes, num_nodes);
    
    /* Phase 4: Verification and cleanup */
    unsigned long total_hash = 0;
    for (int i = 0; i < num_nodes; i++) {
        if (nodes[i]) {
            total_hash ^= compute_ast_hash(nodes[i]);
            
            /* Final builtin usage before free */
            volatile char check_buf[128];
            __builtin_memcpy(check_buf, nodes[i]->data, 
                            g_memcpy_len % sizeof(check_buf));
            
            free(nodes[i]);
        }
    }
    
    /* Force one more builtin in main */
    volatile char final_buffer[1024];
    __builtin_memset(final_buffer, 0, sizeof(final_buffer));
    __builtin_memcpy(final_buffer + 512, final_buffer, 512);
    __builtin_memmove(final_buffer + 256, final_buffer + 768, 256);
    
    printf("Total hash: %lu\n", total_hash);
    printf("Test completed - check for ASAN/HWASAN redirection\n");
    
    return 0;
}
