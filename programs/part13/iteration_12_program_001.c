/* complex_gty_test.c - Test case for gengtype delimiter parsing */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter list */
typedef void (*GTY((user)) complex_fn_type)(
    int (*GTY((skip)) callback)(char[10]),
    struct GTY(()) inner_struct { int x; }
);

/* Test 2: Array with complex dimension expression */
struct GTY(()) outer_struct {
    int arr[(10 + sizeof(struct GTY(()) temp { int a; }))];
    void (*GTY((chain_next)) fn_ptr_array[5])(int[][10]);
};

/* Test 3: Deeply nested structure with all delimiter types */
union GTY((desc ("%1.type"), tag ("type"))) complex_union {
    struct GTY((user)) nested {
        int (*GTY((skip)) deep_fn)(
            struct {
                int x;
                char arr[((5 * 2) + 3)];
            } param
        );
        union GTY(()) inner_union {
            int i;
            char c;
        } u;
    } s;
    
    struct GTY((user)) another {
        void (*GTY((chain_prev, chain_next)) links[3])(
            int (*)(char (*)[5]),
            struct GTY(()) { int a; double b; } anonymous
        );
    } t;
};

/* Test 4: Multiple balanced delimiters in single declaration */
typedef struct GTY(()) {
    int (*GTY((user)) complex_member)(
        int arg1,
        int arg2[10][20],
        struct GTY(()) { 
            union { 
                int x; 
                long y; 
            } u; 
        } arg3
    );
    
    struct GTY((user)) {
        char * GTY((length ("len"))) data;
        int len;
    } *ptr_array[((sizeof(int) > 4) ? 8 : 4)];
} ultimate_type;

/* Test 5: Initializer with nested braces */
static const struct GTY(()) with_init = {
    .arr = {1, 2, {3, 4}, 5},
    .fn_ptr_array = {
        NULL,
        (void (*)(int[][10]))0x1234,
        NULL
    }
};

/* Test 6: Template-like macro expansion with delimiters */
#define GTY_ARRAY(type, size) type[size]
#define GTY_CALLBACK(ret, params) ret (*) params

struct GTY(()) macro_test {
    GTY_ARRAY(int, (10 + 5));
    GTY_CALLBACK(void, (int (*)(char[10]), struct { int x; }));
};

/* Test 7: Bit-field with complex expression */
struct GTY(()) bitfield_test {
    unsigned int flags : (sizeof(int) * 8 - 1);
    unsigned int : ((4 - (sizeof(struct { char c; }) % 4)) % 4);
    int (*GTY((skip)) handler)(int, ...);
};

/* Test 8: Multiple attribute lists */
typedef struct GTY((chain_next ("next"), chain_prev ("prev"))) linked_node {
    struct linked_node * GTY((skip)) next;
    struct linked_node * GTY((skip)) prev;
    void (* GTY((user)) notify)(
        struct linked_node * GTY((skip)),
        int data[((16 + 7) & ~7)]
    );
} linked_node_t;

/* Test 9: Nested GTY annotations */
struct GTY(()) container {
    struct GTY((user)) contained {
        union GTY((tag ("type"))) {
            int i;
            long l;
        } GTY((skip)) value;
    } GTY((user)) item;
    
    struct container * GTY((chain_next)) chain;
};

/* Test 10: All delimiters mixed together */
void (* GTY((user)) ultimate_test[3])(
    int,
    char *[],
    struct GTY(()) {
        int x;
        struct GTY((user)) { 
            void (*fn)(int[10]); 
        } inner;
    }
) = {
    NULL,
    NULL,
    NULL
};

/* Dummy main to make file compilable */
int main(void) {
    return 0;
}
