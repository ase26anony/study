/* complex_gty_test.c - Test file for gengtype delimiter parsing */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists */
typedef void (*GTY((user)) complex_func_ptr)(
    int (*GTY((skip)) nested_callback)(char param[10]),
    struct GTY(()) inner_struct { int x; }
);

/* Test 2: Array with complex dimension expression */
struct GTY(()) array_container {
    int arr[(10 + sizeof(struct GTY(()) temp { int a; }))];
    char * GTY((length("strlen(%h.field) + 1"))) field;
};

/* Test 3: Nested structures with all delimiter types */
struct GTY(()) outer_struct {
    /* Function pointer array */
    void (* GTY((tag("1"))) func_array[5])(
        int matrix[][10],
        struct GTY(()) { int y; } anonymous
    );
    
    /* Nested union with bitfields */
    union GTY((desc("%1.union_tag"))) nested_union {
        int GTY((skip)) i;
        struct GTY(()) {
            unsigned int flag:1;
            unsigned int value:31;
        } GTY((tag("0"))) bits;
        void (* GTY((user)) callback)(
            char buffer[],
            int sizes[3]
        );
    } GTY((tag("2"))) u;
    
    /* Complex initializer (will be in source file) */
    struct GTY(()) init_example {
        int values[3];
        struct GTY(()) { char *name; } meta;
    } data;
};

/* Test 4: Chain of pointers with nested attributes */
struct GTY((chain_next("next"), chain_prev("prev"))) linked_node {
    int value;
    struct linked_node * GTY((skip)) next;
    struct linked_node * GTY((skip)) prev;
    
    /* Array of function pointers */
    int (* GTY((user)) operations[3])(
        struct linked_node * GTY((skip)) node,
        int params[]
    );
};

/* Test 5: Deeply nested parentheses in type expressions */
typedef struct GTY(()) deep_nested {
    /* Pointer to function returning pointer to array */
    int (*(* GTY((user)) complex_decl)(
        void (*)(int, char),
        int
    ))[10];
    
    /* Nested structure with array of structures */
    struct GTY(()) {
        struct GTY(()) inner_inner {
            int matrix[2][(5 + 3)];
            void (* GTY((user)) method)(void);
        } items[4];
    } container;
} deep_nested_t;

/* Test 6: Multiple balanced delimiters in single declaration */
struct GTY(()) all_delimiters {
    /* Contains: (), [], {} all together */
    void (* (* GTY((user)) nested_array_func[2])(
        int param,
        struct GTY(()) { 
            char data[100]; 
            union GTY(()) { 
                int x; 
                double y; 
            } value;
        } context
    ))[3][4];
    
    /* Initializer with designators */
    struct GTY(()) init_struct {
        int array[5];
        struct GTY(()) { int a; char b; } nested;
    } init;
};

/* Test 7: Template-like pattern using nested types */
#ifdef __cplusplus
template<typename T>
struct GTY(()) template_container {
    T data;
    T* GTY((skip)) next;
};
#else
/* C version with macro */
#define DECLARE_CONTAINER(type) \
    struct GTY(()) container_##type { \
        type data; \
        struct container_##type* GTY((skip)) next; \
    }

DECLARE_CONTAINER(int);
DECLARE_CONTAINER(struct outer_struct);
#endif

/* Test 8: Attribute lists within attribute lists */
struct GTY((user,
    desc("%1.type"),
    maybe_undef,
    skip_if("skip_check(%h.flag)"))) attr_nesting {
    int type;
    int flag;
    
    /* Field with its own GTY attributes */
    struct GTY((chain_next("chain"))) chain_item {
        int value;
        struct chain_item* GTY((skip)) chain;
    }* GTY((length("count"))) items;
    int count;
};

/* Source file with initializers to trigger brace handling */
#ifdef IMPLEMENTATION

/* Complex initializer with nested braces */
struct GTY(()) outer_struct global_var = {
    .func_array = { NULL, NULL, NULL, NULL, NULL },
    .u = {
        .callback = NULL
    },
    .data = {
        .values = {1, 2, 3},
        .meta = {
            .name = "test"
        }
    }
};

/* Array initializer with designators */
struct GTY(()) all_delimiters delimiters_var = {
    .nested_array_func = { NULL, NULL },
    .init = {
        .array = {[0] = 1, [4] = 5},
        .nested = {
            .a = 42,
            .b = 'x'
        }
    }
};

/* Nested initializer */
deep_nested_t nested_var = {
    .complex_decl = NULL,
    .container = {
        .items = {
            [0] = {
                .matrix = {{1, 2, 3, 4, 5, 6, 7, 8}, {9, 10, 11, 12, 13, 14, 15, 16}},
                .method = NULL
            },
            [3] = {
                .matrix = {{0}},
                .method = NULL
            }
        }
    }
};

#endif /* IMPLEMENTATION */

/* Main function to make file compilable */
int main(void) {
    return 0;
}
