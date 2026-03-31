/* ISO C99-compliant program targeting ASAN/HWASAN built-in redirection logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile size_t volatile_len = 256;
volatile int use_memcpy = 1;
volatile int use_memset = 1;
volatile int use_memmove = 1;

/* AST-like recursive structure */
typedef struct Node {
    char *data;
    size_t size;
    struct Node *left;
    struct Node *right;
    int id;
} Node;

/* Constructor function (runs before main) */
__attribute__((constructor)) static void init_asan_early(void) {
    printf("Constructor: Initializing ASAN/HWASAN environment\n");
    
    /* Force early built-in usage in constructor */
    char buffer1[64];
    char buffer2[64];
    
    __builtin_memset(buffer1, 0xAA, sizeof(buffer1));
    __builtin_memcpy(buffer2, buffer1, sizeof(buffer1));
    __builtin_memmove(buffer1, buffer2, sizeof(buffer1));
}

/* Destructor function (runs after main) */
__attribute__((destructor)) static void cleanup_asan_late(void) {
    printf("Destructor: Cleaning up ASAN/HWASAN resources\n");
}

/* Recursive tree manipulation with memory operations */
static Node* create_node(int id, size_t size) {
    Node *node = malloc(sizeof(Node));
    if (!node) return NULL;
    
    node->data = malloc(size);
    node->size = size;
    node->left = NULL;
    node->right = NULL;
    node->id = id;
    
    /* Use built-ins with volatile control */
    __builtin_memset(node->data, id % 256, size);
    
    return node;
}

static void copy_node_data(Node *dest, Node *src) {
    if (!dest || !src || dest->size != src->size) return;
    
    /* This should trigger ASAN built-in redirection */
    __builtin_memcpy(dest->data, src->data, dest->size);
}

static void move_node_data(Node *dest, Node *src) {
    if (!dest || !src || dest->size != src->size) return;
    
    /* Force memmove usage with overlapping regions */
    size_t half = dest->size / 2;
    __builtin_memmove(dest->data, src->data, half);
    __builtin_memmove(dest->data + half, src->data + half, dest->size - half);
}

/* Function with goto edge cases */
static void process_with_goto(Node *nodes[], int count) {
    int i = 0;
    
start_label:
    if (i >= count) goto end_label;
    
    /* Jump into memory operation block */
    if (nodes[i]) {
        char temp[128];
        
        /* Use volatile to control which built-in is called */
        if (use_memcpy && i % 2 == 0) {
            __builtin_memcpy(temp, nodes[i]->data, 
                           nodes[i]->size < 128 ? nodes[i]->size : 128);
        }
        
        if (use_memset && i % 3 == 0) {
            __builtin_memset(nodes[i]->data + 10, 0xCC, 
                           nodes[i]->size > 20 ? nodes[i]->size - 20 : 1);
        }
        
        /* Jump out to skip memmove sometimes */
        if (i % 5 == 0) goto skip_memmove;
        
        if (use_memmove && nodes[i]->size > 50) {
            __builtin_memmove(nodes[i]->data + 20, nodes[i]->data + 10, 30);
        }
        
    skip_memmove:
        i++;
        goto start_label;
    }
    
end_label:
    return;
}

/* OpenMP parallel section with memory operations */
static void parallel_memory_operations(Node *nodes[], int count) {
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for
        for (int i = 0; i < count; i++) {
            if (nodes[i]) {
                /* Each thread uses different built-ins */
                switch (thread_id % 3) {
                    case 0:
                        __builtin_memset(nodes[i]->data, thread_id, 
                                       nodes[i]->size > 100 ? 100 : nodes[i]->size);
                        break;
                    case 1:
                        if (i > 0 && nodes[i-1]) {
                            __builtin_memcpy(nodes[i]->data, nodes[i-1]->data,
                                           nodes[i]->size < nodes[i-1]->size ? 
                                           nodes[i]->size : nodes[i-1]->size);
                        }
                        break;
                    case 2:
                        if (nodes[i]->size > 60) {
                            __builtin_memmove(nodes[i]->data + 30, nodes[i]->data, 30);
                        }
                        break;
                }
            }
        }
        
        /* Barrier to ensure all memory ops complete */
        #pragma omp barrier
        
        /* Additional inter-thread memory operations */
        #pragma omp single
        {
            if (count >= 2 && nodes[0] && nodes[1]) {
                __builtin_memcpy(nodes[0]->data, nodes[1]->data,
                               nodes[0]->size < nodes[1]->size ? 
                               nodes[0]->size : nodes[1]->size);
            }
        }
    }
}

/* Complex initialization with varied memory patterns */
static void initialize_token_array(char *tokens[], int token_count) {
    const char *patterns[] = {"ASAN", "HWASAN", "MEMCPY", "MEMSET", "MEMMOVE"};
    
    for (int i = 0; i < token_count; i++) {
        size_t len = (i * 17 + 23) % 128 + 16; /* Non-trivial length calculation */
        tokens[i] = malloc(len + 1);
        
        if (tokens[i]) {
            /* Mix of built-in usage */
            __builtin_memset(tokens[i], 0, len + 1);
            
            const char *pattern = patterns[i % 5];
            size_t pattern_len = strlen(pattern);
            
            /* Copy pattern multiple times */
            for (size_t pos = 0; pos < len; pos += pattern_len) {
                size_t copy_len = pattern_len;
                if (pos + copy_len > len) copy_len = len - pos;
                __builtin_memcpy(tokens[i] + pos, pattern, copy_len);
            }
            
            /* Move data around within the buffer */
            if (len > 20) {
                __builtin_memmove(tokens[i] + 5, tokens[i], 15);
            }
        }
    }
}

int main(void) {
    const int NODE_COUNT = 8;
    const int TOKEN_COUNT = 12;
    
    printf("Starting ASAN/HWASAN built-in redirection test\n");
    
    /* Create AST-like node structures */
    Node *nodes[NODE_COUNT];
    for (int i = 0; i < NODE_COUNT; i++) {
        size_t size = (i * 50 + 64) % 256 + 32; /* Varied sizes */
        nodes[i] = create_node(i, size);
    }
    
    /* Initialize token array with memory operations */
    char *tokens[TOKEN_COUNT];
    initialize_token_array(tokens, TOKEN_COUNT);
    
    /* Process nodes with goto edge cases */
    process_with_goto(nodes, NODE_COUNT);
    
    /* Perform parallel memory operations */
    parallel_memory_operations(nodes, NODE_COUNT);
    
    /* Additional recursive tree operations */
    for (int i = 0; i < NODE_COUNT - 1; i++) {
        if (nodes[i] && nodes[i + 1]) {
            copy_node_data(nodes[i], nodes[i + 1]);
            if (i % 2 == 0) {
                move_node_data(nodes[i], nodes[i + 1]);
            }
        }
    }
    
    /* Compute verification hash */
    unsigned long long hash = 0;
    for (int i = 0; i < NODE_COUNT; i++) {
        if (nodes[i] && nodes[i]->data) {
            for (size_t j = 0; j < nodes[i]->size && j < 64; j++) {
                hash = (hash * 31 + nodes[i]->data[j]) % 1000000007;
            }
        }
    }
    
    for (int i = 0; i < TOKEN_COUNT; i++) {
        if (tokens[i]) {
            for (int j = 0; tokens[i][j] && j < 32; j++) {
                hash = (hash * 37 + tokens[i][j]) % 1000000007;
            }
        }
    }
    
    printf("Verification hash: %llu\n", hash);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    for (int i = 0; i < NODE_COUNT; i++) {
        if (nodes[i]) {
            free(nodes[i]->data);
            free(nodes[i]);
        }
    }
    
    for (int i = 0; i < TOKEN_COUNT; i++) {
        free(tokens[i]);
    }
    
    return 0;
}
