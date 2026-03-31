/* complex_gty_test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with deeply nested parameter lists */
typedef void (*GTY((user)) complex_func_ptr)(
    int (*GTY((skip)) nested_callback)(char[10][20], 
        struct GTY(()) inner_struct { 
            int x; 
            double y[5]; 
        } *arg),
    union GTY(()) data_union {
        int ival;
        void (*GTY((user)) func)(int, float);
    } u
);

/* Test 2: Array with complex dimension expressions */
struct GTY(()) array_test {
    /* Array dimension with parentheses */
    int arr1[(10 + sizeof(struct GTY(()) temp { int a; double b; }))];
    
    /* Multi-dimensional array with nested brackets */
    char arr2[5][(2 * sizeof(int))][10];
    
    /* Pointer to array of function pointers */
    void (*(*GTY((user)) fn_ptr_array[3])(int))[5];
};

/* Test 3: Nested structures with all delimiter types */
struct GTY(()) outer_struct {
    /* Simple field */
    int id;
    
    /* Nested anonymous struct with initializer-like designators */
    struct GTY(()) {
        int x;
        struct GTY(()) deeper {
            float f;
            char c[4];
        } d;
    } nested;
    
    /* Union containing array of structs */
    union GTY(()) {
        struct GTY(()) {
            int count;
            void *GTY((tag("0"))) data;
        } items[10];
        
        /* Function pointer with complex return type */
        struct GTY(()) result *(*GTY((user)) processor)(
            int param,
            char buffer[],
            void (*GTY((skip)) callback)(void)
        );
    } u;
    
    /* Bit-field with attribute */
    unsigned int flags:4 GTY((skip));
};

/* Test 4: Complex typedef with multiple nested delimiters */
typedef struct GTY(()) node {
    struct node *GTY((tag("1"))) next;
    struct node *GTY((tag("2"))) prev;
    
    /* Array of pointers to functions returning pointers to arrays */
    int (*(*(*GTY((user)) operations[5])(int arg_count))[10])(float);
    
    /* Nested union with anonymous struct */
    union GTY(()) {
        struct GTY(()) {
            int type;
            char *GTY((length("strlen($)"))) name;
        } s;
        
        /* Pointer to array with computed size */
        int (*GTY((user)) dynamic_array)[sizeof(struct GTY(()) header {
            int magic;
            int size;
        })];
    } data;
} *GTY((user)) node_ptr;

/* Test 5: Template-like macro expansion (simulating C++ templates in C) */
#define DECLARE_CONTAINER(TYPE) \
    struct GTY(()) container_##TYPE { \
        TYPE *GTY((tag("0"))) items; \
        int (*(*GTY((user)) comparer)(TYPE a, TYPE b))[2]; \
        struct GTY(()) { \
            TYPE min; \
            TYPE max; \
        } range; \
    }

/* Instantiate with complex type */
DECLARE_CONTAINER(struct GTY(()) complex_type {
    int id;
    void (*GTY((user)) methods[3])(struct complex_type *self, int arg);
});

/* Test 6: Structure with complex initializer (for brace handling) */
static struct GTY(()) initialized_struct = {
    .id = 42,
    .nested = {
        .x = 100,
        .d = {
            .f = 3.14,
            .c = {'a', 'b', 'c', '\0'}
        }
    },
    .u = {
        .items = {
            [0] = { .count = 1, .data = NULL },
            [5] = { .count = 2, .data = (void *)0x1000 }
        }
    },
    .flags = 0xF
};

/* Test 7: Multiple balanced delimiters in single declaration */
struct GTY(()) ultimate_test {
    /* Combination: pointer to array of function pointers with parameters */
    void (*(*GTY((user)) complex_member[3][2])(
        int param1,
        struct GTY(()) config {
            int version;
            char options[10];
        } cfg,
        void (*GTY((skip)) handlers[])(void)
    ))[5];
    
    /* Nested anonymous union with struct containing array */
    union GTY(()) {
        struct GTY(()) {
            int matrix[3][3];
            struct GTY(()) {
                float coords[2];
                int id;
            } points[5];
        } s;
        
        /* Function returning pointer to array of structs */
        struct GTY(()) entry *(*GTY((user)) get_entries)(int count)[10];
    } u;
};

/* Test 8: Recursive structure with function pointer */
struct GTY(()) tree_node {
    char *GTY((length("strlen($)"))) name;
    struct tree_node *GTY((tag("1"))) left;
    struct tree_node *GTY((tag("2"))) right;
    
    /* Visitor function pointer with nested parameter */
    void (*GTY((user)) visit)(
        struct tree_node *node,
        void *GTY((skip)) context,
        int (*GTY((skip)) should_stop)(struct tree_node *, int depth)
    );
    
    /* Array of child processors */
    struct GTY(()) {
        int (*GTY((user)) process)(char *input, int len);
        void (*GTY((skip)) cleanup)(void);
    } processors[5];
};

/* Test 9: Type definition with attributes in GTY annotation */
typedef struct GTY((chain_next("next"), chain_prev("prev"))) linked_item {
    struct linked_item *next;
    struct linked_item *prev;
    int data;
    
    /* Callback with its own GTY annotation */
    void (*GTY((user)) on_change)(
        struct linked_item *GTY((tag("0"))) item,
        int old_value,
        int new_value
    );
} linked_item_t;

/* Test 10: Union with nested anonymous struct containing array of function pointers */
union GTY(()) variant_data {
    int int_val;
    double double_val;
    
    struct GTY(()) {
        char *GTY((length("strlen($)"))) str;
        int (*GTY((user)) operations[4])(char *, int);
        struct GTY(()) {
            int x, y;
            void (*GTY((skip)) draw)(int, int);
        } pos;
    } complex;
};

/* Main function to make the file compilable (though GTY expands to nothing normally) */
int main(void) {
    /* These structures would normally be used by GCC's garbage collector */
    struct outer_struct os = {0};
    node_ptr np = NULL;
    linked_item_t li = {0};
    
    return 0;
}
