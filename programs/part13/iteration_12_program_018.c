/* complex_gty_test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists and attributes */
typedef int (*GTY((chain_next, chain_prev)) complex_callback_t)(
    char data[10],
    struct GTY(()) inner_struct {
        int x;
        double y;
    } *context
);

/* Test 2: Array with complex dimension expression containing parentheses */
struct GTY(()) array_container {
    int arr[(10 + sizeof(struct GTY(()) size_calc { int a; double b; }))];
    long * GTY((length("arr[0]"))) ptr_array[];
};

/* Test 3: Nested structures with multiple delimiter types */
struct GTY(()) outer_struct {
    /* Function pointer array */
    void (* GTY((tag("1"))) fn_array[5])(
        int matrix[][10],
        struct GTY(()) param_struct {
            unsigned count;
            char buffer[256];
        } params
    );
    
    /* Union with nested initializer-like syntax in comments */
    union GTY((desc("0"))) nested_union {
        int as_int;
        struct GTY(()) {
            float x;
            float y;
            float z;
        } as_vec;
        complex_callback_t as_callback;
    } data;
    
    /* Pointer to array of function pointers */
    int (*(* GTY((skip)) complex_ptr)[3])(
        char input[],
        int options[][(sizeof(int) * 2)]
    );
};

/* Test 4: Deeply nested parentheses in type definition */
typedef struct GTY(()) node {
    struct node * GTY((chain_next)) next;
    struct node * GTY((chain_prev)) prev;
    
    /* Multi-dimensional array with computed size */
    unsigned char * GTY((length("((depth * width * height) + 7) / 8"))) bitmap;
    
    /* Function with complex return type */
    struct GTY(()) result {
        int status;
        char message[256];
    } (* GTY((callback)) processor)(
        int (* GTY((param)) helper)(char data[], void *context),
        void * GTY((skip)) user_data
    );
    
    /* Nested anonymous struct with bit-fields */
    struct {
        unsigned int flag:1;
        unsigned int count:7;
        unsigned int :0;  /* Force alignment */
        unsigned long values[4];
    } flags;
} node_t;

/* Test 5: Multiple balanced delimiters in single declaration */
union GTY(()) ultimate_test {
    /* Combination of all delimiter types */
    void (*(*ultimate_array[2][3])(int param_list[(sizeof(double) > 4) ? 8 : 4]))(
        struct GTY(()) {
            int x[10];
            struct GTY(()) inner {
                char c;
                short s;
            } nested;
        } config
    );
    
    /* Initializer-style nested braces (in comment to show syntax) */
    struct GTY(()) init_example {
        int a;
        struct {
            float b;
            char c[10];
        } inner;
    } example;
};

/* Test 6: Template-like macro expansion with nested delimiters */
#define DECLARE_CONTAINER(TYPE, SIZE) \
    struct GTY(()) container_##TYPE { \
        TYPE data[SIZE]; \
        int (*comparator)(TYPE a, TYPE b); \
        struct { \
            size_t count; \
            size_t capacity; \
        } GTY((skip)) metadata; \
    }

/* Instantiate the macro with complex types */
DECLARE_CONTAINER(struct GTY(()) pair {
    int key;
    char * GTY((length("strlen(value) + 1"))) value;
}, 100);

/* Test 7: Attribute lists with nested parentheses */
struct GTY((chain_next("next"), chain_prev("prev"),
           length("count"), skip)) linked_list {
    struct linked_list *next;
    struct linked_list *prev;
    int count;
    
    /* Array of pointers to functions returning pointers to arrays */
    int (*(*(*callbacks[10]))[5])(void);
    
    /* Nested struct with its own GTY annotation */
    struct GTY(()) payload {
        enum { TYPE_INT, TYPE_FLOAT, TYPE_STRING } type;
        union {
            int as_int;
            float as_float;
            char * GTY((length("strlen(str) + 1"))) as_string;
        } value;
    } data[];
};

/* Test 8: Recursive type definitions */
typedef struct GTY(()) tree_node tree_node_t;

struct GTY(()) tree_node {
    tree_node_t * GTY((left)) left;
    tree_node_t * GTY((right)) right;
    
    /* Function pointer with nested attribute */
    void (* GTY((callback("traverse"))) visitor)(
        tree_node_t *node,
        int level,
        void * GTY((skip)) context
    );
    
    /* Anonymous union with bit-fields and array */
    union {
        struct {
            unsigned int :4;
            unsigned int tag:4;
            unsigned int size:24;
        } bits;
        unsigned int raw;
        char padding[4];
    } header;
};

/* Test 9: Complex initializer (static) - triggers brace handling */
static const struct GTY(()) complex_init {
    int id;
    struct {
        float coordinates[3];
        char name[50];
    } location;
    void (*actions[3])(int, char *);
} GTY((const)) default_value = {
    .id = 42,
    .location = {
        .coordinates = {1.0, 2.0, 3.0},
        .name = "test\0point"
    },
    .actions = {NULL, NULL, NULL}
};

/* Test 10: Multiple levels of nested delimiters */
struct GTY(()) level1 {
    struct GTY(()) level2 {
        struct GTY(()) level3 {
            int (*(*level4)[10])(
                struct GTY(()) {
                    int a[5][5];
                    struct {
                        char b[100];
                    } nested;
                } param
            );
            union {
                long l;
                double d;
                void *p;
            } data;
        } *deep;
        short array[sizeof(struct level3*)];
    } *deeper;
    char * GTY((string)) name;
};

/* Main function to make the file compilable */
int main(void) {
    /* Dummy usage to avoid compiler warnings */
    node_t node = {0};
    struct outer_struct outer = {0};
    struct linked_list list = {0};
    
    (void)node;
    (void)outer;
    (void)list;
    (void)default_value;
    
    return 0;
}
