/* complex_gty_test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Complex function pointer with nested parentheses */
typedef void (*GTY((chain_next)) complex_func_ptr)(
    int (*GTY((chain_prev)) nested_callback)(char[10][20], 
                                             struct { int x; double y; }),
    union {
        long a;
        struct { short b; char c; } inner;
    } u
);

/* Test 2: Structure with multiple nested delimiter types */
struct GTY(()) outer_struct {
    /* Array with complex dimension containing parentheses */
    int arr1[(10 + (sizeof(struct inner) / 4))];
    
    /* Function pointer array with nested parameter list */
    void (*GTY((skip)) fn_array[5])(
        int matrix[][10],
        struct GTY((for_user)) param_struct {
            int count;
            char *GTY((length("count"))) data;
        } *params
    );
    
    /* Nested union with initializer-like braces in type */
    union {
        struct {
            int (*comparator)(const void *, const void *);
            char name[50];
        } s;
        double values[100];
    } GTY((tag("union_tag"))) data_union;
};

/* Test 3: Deeply nested template-like pattern using macros */
#define DECLARE_VECTOR(TYPE, SIZE) \
    struct GTY(()) vector_##TYPE { \
        TYPE elements[SIZE]; \
        int (*GTY((callback)) sort_func)(TYPE arr[], int len); \
    }

DECLARE_VECTOR(int, 100);
DECLARE_VECTOR(struct outer_struct *, 50);

/* Test 4: Structure with all three delimiters in single member */
struct GTY(()) delimiter_test {
    /* Contains: (), [], {} all together */
    void (*(*complex_member)[3])(
        int arg1,
        struct {
            char array_field[10][20];
            union {
                float f;
                int i;
            } u;
        } arg2
    )[5];
    
    /* Nested initializer in type definition */
    struct nested {
        int x;
        struct {
            char a;
            char b;
        } pair;
    } GTY((default)) defaults[2] = {
        {1, {'a', 'b'}},
        {2, {'c', 'd'}}
    };
};

/* Test 5: Recursive structure with function pointers */
typedef struct GTY((chain_next)) tree_node {
    void *GTY((chain_next)) data;
    struct tree_node *GTY((chain_prev)) left;
    struct tree_node *GTY((chain_prev)) right;
    
    /* Function with complex return type containing arrays */
    struct result {
        int scores[100];
        char (*names)[50];
    } (*GTY((callback)) process)(
        int (*filter)(int value, char buffer[][255]),
        struct config {
            int flags;
            union settings {
                int int_val;
                double dbl_val;
            } s;
        } cfg
    );
} tree_node_t;

/* Test 6: Union with anonymous struct containing arrays */
union GTY(()) complex_union {
    struct {
        int (*compare[10])(const char *, const char *);
        void (*handlers[5])(
            struct event {
                int type;
                union event_data {
                    int i;
                    double d;
                    char str[100];
                } data;
            } ev
        );
    } callbacks;
    
    struct container {
        int count;
        /* Multi-dimensional array with complex access */
        char *items[][3][2];
    } *GTY((reorder)) data_container;
};

/* Test 7: Typedef with deeply nested attributes */
typedef struct GTY((user)) base {
    int id;
    char *GTY((length("id * 2"))) name;
} base_t;

typedef base_t *(*GTY((user)) factory_func)(
    int count,
    struct options {
        int flags;
        char *env_vars[];
    } opts
);

/* Test 8: Structure with bit-fields and arrays */
struct GTY(()) packed_data {
    unsigned int flags : 3;
    unsigned int : 5;  /* Padding */
    unsigned int count : 8;
    
    /* Array of function pointers with nested return type */
    struct {
        int x;
        int y;
    } (*GTY((skip)) operations[10])(
        int param1,
        int param2[][5]
    );
    
    /* Nested anonymous struct */
    struct {
        union {
            int i;
            float f;
        } value;
        char tag;
    } items[20];
};

/* Test 9: Multiple levels of indirection with all delimiters */
typedef void (*(*(*deeply_nested)[5])[10])(
    int,
    char *argv[],
    struct {
        int argc;
        char **GTY((length("argc"))) argv;
    } args
)[3];

/* Test 10: Final comprehensive test combining everything */
struct GTY(()) ultimate_test {
    /* Test all three cases in sequence */
    int (*func_ptr_array[3][2])(
        struct param1 {
            int x;
            int y[10];
        } p1,
        union param2 {
            struct {
                char a;
                char b;
            } chars;
            int number;
        } p2
    )[5][4];
    
    /* Nested structure with initializer */
    struct config {
        int version;
        struct {
            char *name;
            int values[100];
        } sections[10];
    } cfg = {
        1,
        {
            {"main", {1, 2, 3}},
            {"backup", {4, 5, 6}}
        }
    };
    
    /* Union containing array of function pointers */
    union {
        int (*int_funcs[5])(int, int);
        void (*void_funcs[3])(
            char *,
            int[],
            struct { int len; char *data; }
        );
    } callback_union;
};

/* Dummy main to make file compilable */
int main(void) {
    return 0;
}
