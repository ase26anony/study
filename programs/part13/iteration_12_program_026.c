/* complex_gty_test.c - Test file to exercise gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Complex function pointer with nested parentheses */
typedef void (*GTY(()) complex_func_ptr)(
    int (*GTY(()) nested_callback)(char[10][20], 
                                   struct GTY(()) inner { int x; } *),
    union GTY(()) data_union {
        long l;
        double d;
        struct GTY(()) { 
            short s; 
            char c[5]; 
        } nested;
    } *
);

/* Test 2: Structure with array declarations containing parenthesized expressions */
struct GTY(()) outer_struct {
    /* Array with size containing parentheses */
    int arr1[(10 + (sizeof(struct GTY(()) dummy { int a; }) / sizeof(int)))];
    
    /* Multi-dimensional array */
    char arr2[5][(2 * (3 + 4))];
    
    /* Function pointer array */
    void (*GTY(()) func_array[3])(
        int param1,
        struct GTY(()) { 
            float f; 
            int arr[((5) + (3))]; 
        } param2
    );
    
    /* Nested structure with bit-field containing parenthesized expression */
    struct GTY(()) nested {
        unsigned int flags : (sizeof(int) * 8 - 1);
        int (*GTY(()) method)(int, ...);
    } inner;
};

/* Test 3: Union with complex initializer-like braces structure */
union GTY(()) complex_union {
    struct GTY(()) {
        int x;
        int y[((2)+(3))];
        struct GTY(()) point {
            int coord[2][2];
            void (*GTY(()) draw)(struct point *);
        } *p;
    } data;
    
    long long big_array[((10) * (2))];
    
    /* Anonymous struct with function pointer returning array pointer */
    struct GTY(()) {
        int (*GTY(()) get_matrix(void))[10][10];
        void (*GTY(()) set_values(int matrix[][10], 
                                  struct GTY(()) config { 
                                      int size; 
                                      char name[50]; 
                                  }));
    } ops;
};

/* Test 4: Typedef with deeply nested delimiters */
typedef struct GTY(()) node {
    struct node *GTY((chain_next, chain_prev)) next;
    struct node *GTY((chain_next, chain_prev)) prev;
    
    /* Mixed delimiters in single declaration */
    void (*(*GTY(()) complex_array[5])(int, ...))[10];
    
    /* Structure containing array of function pointers */
    struct GTY(()) handler_set {
        int (*GTY(()) handlers[3])(
            char *,
            int[],
            struct GTY(()) context {
                void *data;
                size_t size;
            }
        );
        int count;
    } handlers;
    
    /* Union with nested initializer-like syntax in comment position */
    union GTY(()) value {
        int i;
        float f;
        char str[100];
        struct GTY(()) {
            int type;
            void *ptr;
        } obj;
    } data;
} *node_ptr;

/* Test 5: Template-like macro expansion with balanced delimiters */
#define DECLARE_VECTOR(TYPE, SIZE) \
    struct GTY(()) vector_##TYPE { \
        TYPE data[(SIZE)]; \
        int (*GTY(()) compare)(const TYPE *, const TYPE *); \
        void (*GTY(()) sort)(TYPE arr[], int (*)(const TYPE *, const TYPE *)); \
    }

/* Instantiate the macro with complex types */
DECLARE_VECTOR(struct GTY(()) pair {
    int key;
    char value[50];
    struct GTY(()) link {
        struct pair *next;
        struct pair *prev;
    } *links;
}, 100);

/* Test 6: Structure with attribute-packed and nested arrays */
struct GTY(()) __attribute__((packed)) packed_struct {
    char id[((16) + (1))];
    unsigned int : (8 * sizeof(int) - 24);  /* bitfield with parenthesized expression */
    
    /* Nested structure with flexible array member */
    struct GTY(()) flexible {
        int length;
        int data[];  /* Flexible array member */
    } *flex;
    
    /* Function pointer with nested parameter */
    int (*GTY(()) validator)(
        struct packed_struct *self,
        int options[((4) + (2))],
        void (*GTY(()) on_error)(const char *, ...)
    );
};

/* Test 7: Recursive structure with complex function pointer */
struct GTY(()) tree_node {
    char *GTY((length)) name;
    struct tree_node *GTY((chain_next)) children;
    
    /* Method with array parameter and nested struct */
    void (*GTY(()) traverse)(
        struct tree_node *root,
        void (*GTY(()) visit)(
            struct tree_node *,
            struct GTY(()) visitor_data {
                int depth;
                void *user_data;
            }
        ),
        int options
    );
    
    /* Union with anonymous struct containing array */
    union GTY(()) node_data {
        int int_val;
        double dbl_val;
        struct GTY(()) {
            char *str;
            int len;
        } str_val;
        int array_val[((3) * (2))];
    } data;
};

/* Test 8: Global variable with complex type and initializer */
static struct GTY(()) global_config {
    int version;
    char *name;
    struct GTY(()) settings {
        int timeout;
        int retries;
        struct GTY(()) limits {
            int min;
            int max;
        } limits[2];
    } settings;
} test_config = {
    .version = 1,
    .name = "test",
    .settings = {
        .timeout = 1000,
        .retries = 3,
        .limits = {
            [0] = { .min = 0, .max = 100 },
            [1] = { .min = -50, .max = 50 }
        }
    }
};

/* Test 9: Typedef for function returning pointer to array */
typedef int (*GTY(()) matrix_generator(int size))[][10];

/* Test 10: Structure with all delimiter types mixed */
struct GTY(()) delimiter_test {
    /* Parentheses in function pointer */
    void (*func1)(int, char *);
    
    /* Brackets in array declaration */
    int array2d[5][10];
    
    /* Braces in nested structure */
    struct GTY(()) {
        /* Parentheses in array size calculation */
        int sized_array[(sizeof(int) * 10)];
        
        /* Brackets in function pointer array */
        void (*callbacks[3])(void);
        
        /* Braces in anonymous union */
        union GTY(()) {
            int x;
            float y;
        } value;
    } nested;
    
    /* Mixed: function returning pointer to array */
    int (*(*get_matrix)(void))[10];
    
    /* Complex: array of function pointers returning structures */
    struct GTY(()) result {
        int status;
        char message[100];
    } (*(*operations[5])(int, ...))(void);
};

/* Main function to make the file compilable */
int main(void) {
    return 0;
}
