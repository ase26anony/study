/* asan_coverage_test.c - Comprehensive test for ASAN built-in redirection */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
static volatile size_t g_mem_size = 256;
static volatile int g_use_hwasan = 0;

/* Recursive AST-like structure */
typedef struct ASTNode {
    char data[64];
    struct ASTNode* left;
    struct ASTNode* right;
    size_t size;
} ASTNode;

/* Constructor function to force early initialization */
__attribute__((constructor))
static void init_asan_early(void) {
    volatile char buffer[32];
    /* Force memcpy redirection early */
    __builtin_memset(buffer, 0, sizeof(buffer));
    printf("[Constructor] Early ASAN initialization triggered\n");
}

/* Destructor for cleanup verification */
__attribute__((destructor))
static void cleanup_asan(void) {
    printf("[Destructor] ASAN cleanup completed\n");
}

/* Recursive function with memory operations */
static ASTNode* create_ast(int depth, const char* base_data) {
    if (depth <= 0) return NULL;
    
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    /* Use builtin memset for initialization */
    __builtin_memset(node, 0, sizeof(ASTNode));
    
    /* Copy data with builtin memcpy */
    size_t copy_len = strlen(base_data) + 1;
    if (copy_len > sizeof(node->data))
        copy_len = sizeof(node->data);
    
    __builtin_memcpy(node->data, base_data, copy_len);
    node->size = copy_len;
    
    /* Create children with goto for flow control */
    int create_left = 1;
    
    if (depth > 2) {
        goto create_children;
    }
    
    node->left = NULL;
    node->right = NULL;
    goto done;
    
create_children:
    node->left = create_ast(depth - 1, "left_child");
    node->right = create_ast(depth - 1, "right_child");
    
done:
    return node;
}

/* Function with goto jumping around memmove */
static void complex_memmove_with_goto(char* dest, char* src, size_t n) {
    volatile int condition = 1;
    
    if (condition) {
        goto block1;
    }
    
    /* This block should be skipped initially */
    __builtin_memmove(dest, src, n);
    return;
    
block1:
    /* Copy data first */
    __builtin_memcpy(dest, src, n);
    
    if (n > 16) {
        goto block2;
    }
    
    /* Small buffer handling */
    __builtin_memset(dest + n/2, 0xFF, n/2);
    return;
    
block2:
    /* Overlapping copy with memmove */
    char* mid = dest + n/4;
    __builtin_memmove(mid, dest, n/2);
    
    /* Jump back */
    goto block1;
}

/* OpenMP parallel memory operations */
static void parallel_memory_ops(void) {
    const size_t buffer_size = g_mem_size;
    char* buffers[4];
    
    #pragma omp parallel for
    for (int i = 0; i < 4; i++) {
        buffers[i] = malloc(buffer_size);
        if (buffers[i]) {
            /* Each thread uses different builtins */
            switch (i % 3) {
                case 0:
                    __builtin_memset(buffers[i], i, buffer_size);
                    break;
                case 1:
                    if (i > 0) {
                        __builtin_memcpy(buffers[i], buffers[i-1], buffer_size/2);
                    }
                    break;
                case 2:
                    __builtin_memmove(buffers[i] + 10, buffers[i], buffer_size - 10);
                    break;
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        if (buffers[i]) free(buffers[i]);
    }
}

/* Multi-stage initialization with different memory functions */
static void multi_stage_init(void) {
    volatile char stage1[128];
    volatile char stage2[128];
    volatile char stage3[128];
    
    /* Stage 1: Basic memset */
    __builtin_memset(stage1, 0xAA, sizeof(stage1));
    
    /* Stage 2: Memcpy from stage1 */
    __builtin_memcpy(stage2, stage1, sizeof(stage1)/2);
    
    /* Stage 3: Overlapping memmove */
    __builtin_memmove(stage3 + 32, stage2, 64);
    
    /* Verify with another memcpy */
    __builtin_memcpy(stage1 + 64, stage3, 32);
}

/* Main test driver */
int main(void) {
    printf("=== ASAN Built-in Redirection Test ===\n");
    
    /* Test 1: Recursive AST operations */
    printf("\n[Test 1] Creating recursive AST structure...\n");
    ASTNode* root = create_ast(4, "root_node");
    
    if (root) {
        /* Copy between AST nodes */
        ASTNode temp;
        __builtin_memcpy(&temp, root, sizeof(ASTNode));
        __builtin_memmove(root->data, temp.data, temp.size);
        
        /* Recursive cleanup */
        free(root);
    }
    
    /* Test 2: Goto-based control flow */
    printf("\n[Test 2] Testing goto with memmove...\n");
    {
        char src[100], dest[100];
        for (int i = 0; i < sizeof(src); i++) {
            src[i] = (char)i;
        }
        complex_memmove_with_goto(dest, src, sizeof(src));
    }
    
    /* Test 3: OpenMP parallel operations */
    printf("\n[Test 3] Running OpenMP memory operations...\n");
    parallel_memory_ops();
    
    /* Test 4: Multi-stage initialization */
    printf("\n[Test 4] Multi-stage memory initialization...\n");
    multi_stage_init();
    
    /* Test 5: Direct builtin calls with volatile */
    printf("\n[Test 5] Direct builtin calls...\n");
    {
        volatile char buffer1[256];
        volatile char buffer2[256];
        volatile size_t size = g_mem_size;
        
        __builtin_memset(buffer1, 0xCC, size);
        __builtin_memcpy(buffer2, buffer1, size);
        __builtin_memmove(buffer1 + 128, buffer2, 128);
    }
    
    /* Test 6: Nested memory operations */
    printf("\n[Test 6] Nested memory operations...\n");
    {
        struct {
            char a[50];
            char b[50];
            char c[50];
        } nested;
        
        __builtin_memset(&nested, 0, sizeof(nested));
        __builtin_memcpy(nested.a, "Test string for memcpy", 22);
        __builtin_memmove(nested.b, nested.a, 22);
        __builtin_memcpy(nested.c, nested.b, 22);
    }
    
    printf("\n=== All tests completed successfully ===\n");
    printf("Result hash: 0x%08X\n", 
           (unsigned int)(g_mem_size ^ (size_t)main ^ 0xDEADBEEF));
    
    return 0;
}
