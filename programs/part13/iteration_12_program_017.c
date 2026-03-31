/* complex_gty_test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Complex function pointer with nested parentheses */
typedef void GTY(()) (*complex_func_ptr)(
    int (* GTY((user)) nested_callback)(char GTY((length("strlen(arg)+1"))) buffer[256]),
    struct GTY(()) inner_struct { 
        int x; 
        void (* GTY((skip)) another_fn)(double);
    } * GTY((tag("1"))) inner
);

/* Test 2: Array with complex dimension expression containing parentheses */
struct GTY(()) ArrayTest {
    int GTY((skip)) arr1[(10 + sizeof(struct GTY(()) Temp { int a; }))];
    char GTY((length("(len > 0 ? len : 1)"))) * GTY((tag("2"))) dynamic_array;
    int len;
};

/* Test 3: Deeply nested structure with all delimiter types */
struct GTY(()) OuterStruct {
    /* Case '(': Function pointer with complex signature */
    int (* GTY((callback)) process)(
        struct GTY(()) Data {
            int values[5];
            struct GTY(()) Nested {
                char * GTY((string)) name;
                union GTY(()) ValueUnion {
                    int i;
                    double d;
                    void (* GTY((skip)) func)(int, char);
                } value;
            } nested;
        } * GTY((tag("3"))) data,
        int (* GTY((chain_next, chain_prev)) steps[])(void)
    );
    
    /* Case '[': Multi-dimensional array with nested brackets */
    int GTY((skip)) matrix[3][(2 * sizeof(int))][10];
    
    /* Case '{': Nested initializer-style anonymous struct */
    struct GTY(()) {
        struct GTY(()) {
            int x;
            struct GTY(()) {
                int y;
                int (* GTY((skip)) method)(int[][10]);
            } inner;
        } middle;
        union GTY(()) {
            long l;
            struct GTY(()) {
                short s;
                char c;
            } sc;
        } u;
    } anonymous;
};

/* Test 4: Template-like macro expansion with delimiters */
#define DECLARE_CONTAINER(TYPE, SIZE) \
    struct GTY(()) Container_##TYPE { \
        TYPE GTY((tag("4"))) items[SIZE]; \
        int (* GTY((compare))(const TYPE *, const TYPE *)) cmp; \
        struct GTY(()) { \
            TYPE min; \
            TYPE max; \
        } range; \
    }

DECLARE_CONTAINER(int, 100);
DECLARE_CONTAINER(double, (50 + sizeof(struct ArrayTest)));

/* Test 5: Union with function pointers and arrays */
union GTY(()) ComplexUnion {
    /* Multiple nested parentheses */
    void (* GTY((user)) signal_handler[5])(
        int signum,
        void (* GTY((skip)) old_handler)(int),
        const char * GTY((string)) message
    );
    
    /* Array of structs with initializer-like content */
    struct GTY(()) {
        int count;
        struct GTY(()) Entry {
            char key[256];
            void * GTY((tag("5"))) value;
        } entries[10];
    } lookup;
    
    /* Function returning pointer to array */
    int (* GTY((callback)) (*get_matrix)(int rows))[10];
};

/* Test 6: Typedef chain with all delimiters */
typedef struct GTY(()) Node Node;

struct GTY(()) Node {
    Node * GTY((chain_next, chain_prev)) next;
    Node * GTY((chain_prev)) prev;
    
    /* Complex member with all delimiters mixed */
    union GTY(()) {
        /* Case '{' inside '(' inside '[' */
        int (* GTY((skip)) handlers[3])(
            struct GTY(()) {
                int id;
                char name[50];
            } config
        );
        
        /* Case '[' inside '(' inside '{' */
        struct GTY(()) {
            int (* GTY((user)) process_values)(
                int values[],
                int (* GTY((skip)) validate)(int[10][10])
            );
        } processor;
    } u;
    
    /* Array dimension with parenthesized expression */
    int scores[(sizeof(Node) + 7) / 8];
};

/* Test 7: Multiple GTY annotations on nested types */
struct GTY(()) Outer {
    struct GTY((for_user)) Inner {
        struct GTY((skip)) Deeper {
            int x;
            /* Function pointer with attribute in parentheses */
            void (* GTY((user)) func)(
                __attribute__((aligned(16))) int *ptr
            );
        } GTY((tag("6"))) deeper;
        
        /* Array with designators (C99) */
        int arr[10] GTY((length("10")));
    } inner;
    
    /* Anonymous union with bitfields (uses braces) */
    union GTY(()) {
        int full:32;
        struct GTY(()) {
            unsigned low:16;
            unsigned high:16;
        } parts;
    } flags;
};

/* Test 8: Recursive structure with function pointer */
struct GTY(()) Tree {
    char * GTY((string)) data;
    struct GTY(()) Tree * GTY((chain_next)) children;
    
    /* Method with complex return type */
    struct GTY(()) Result {
        int status;
        char error[256];
        void (* GTY((skip)) cleanup)(struct Result *);
    } (* GTY((callback)) traverse)(
        int (* GTY((user)) visit)(struct Tree *, int level),
        int options[3]
    );
};

/* Test 9: Mix of all cases in single declaration */
static struct GTY(()) UltimateTest {
    /* All three delimiters in sequence: []({}) */
    void (* GTY((user)) (*callbacks[5])(int))(
        struct GTY(()) {
            int x;
            int y[10];
        } point
    );
    
    /* Nested: {[(())]} */
    struct GTY(()) {
        union GTY(()) {
            int (* GTY((skip)) funcs[2])(
                char param[(
                    sizeof(
                        struct GTY(()) { int a; int b; }
                    ) + 3
                ) / 4]
            );
            double matrix[3][4];
        } u;
    } nested;
} GTY((tag("final"))) ultimate_instance;

/* Test 10: Edge case - empty balanced delimiters */
struct GTY(()) EmptyDelims {
    void (* GTY((skip)) empty_func)();
    int empty_array[];
    struct GTY(()) {} empty_struct;
    union GTY(()) {} empty_union;
};

/* Main function to make the file compilable */
int main(void) {
    /* Dummy usage to avoid compiler warnings */
    struct OuterStruct os;
    struct Node node;
    struct UltimateTest ut;
    
    (void)os;
    (void)node;
    (void)ut;
    
    return 0;
}
