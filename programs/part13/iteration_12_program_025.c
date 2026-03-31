/* complex_gty_test.c - Test file for gengtype delimiter parsing */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists */
typedef void (*GTY(()) complex_func_ptr)(
    int (*GTY(()) nested_callback)(char GTY(()) buffer[10]),
    struct GTY(()) { 
        int x; 
        union GTY(()) { 
            long a; 
            double b; 
        } u;
    } param
);

/* Test 2: Array with complex dimension expression */
struct GTY(()) ArrayTest {
    int arr1[(10 + sizeof(struct GTY(()) { char c; int i; }))];
    char arr2[5][(2 * sizeof(int))];
};

/* Test 3: Nested structures with all delimiter types */
struct GTY(()) OuterStruct {
    /* Function pointer array */
    void (*GTY((chain_next, chain_prev)) fp_array[5])(
        int matrix[][10],
        struct GTY(()) { 
            int flags; 
        } config
    );
    
    /* Union with bitfields and arrays */
    union GTY(()) {
        struct GTY(()) {
            unsigned int a:4;
            unsigned int b:4[(2+3)];
        } bits;
        long GTY(()) values[3];
    } data;
    
    /* Pointer to nested anonymous struct */
    struct GTY(()) {
        int x;
        struct GTY(()) {
            double y;
            int (*GTY(()) method)(void);
        } GTY(()) inner;
    } *GTY((length("len"))) nested_ptr;
};

/* Test 4: Complex typedef with multiple nested delimiters */
typedef struct GTY(()) TreeNode *GTY((user)) TreeNodePtr;

struct GTY(()) TreeNode {
    TreeNodePtr GTY((chain_next, chain_prev)) next;
    TreeNodePtr GTY((chain_next, chain_prev)) prev;
    
    /* Array of function pointers returning function pointers */
    int (*(*GTY(()) complex_array[3])(float))(
        char param[(sizeof(int) + 2)],
        void (*GTY(()) callback)(struct GTY(()) { int id; } )
    );
    
    /* Nested initializer-like structure */
    struct GTY(()) {
        int indices[2][2];
        struct GTY(()) {
            char *GTY(()) name;
            int value;
        } entries[4];
    } config;
};

/* Test 5: Template-like macro pattern with nested delimiters */
#define DECLARE_GTY_VECTOR(TYPE) \
    struct GTY(()) vector_##TYPE { \
        TYPE *GTY((length("size"))) data; \
        size_t size; \
        int (*GTY(()) compare)(TYPE a, TYPE b); \
    }

DECLARE_GTY_VECTOR(int);
DECLARE_GTY_VECTOR(struct GTY(()) { int x; double y; });

/* Test 6: Structure with complex initializer (if supported) */
struct GTY(()) InitTest {
    int matrix[2][3];
    struct GTY(()) {
        char *name;
        int id;
    } info;
};

/* Test 7: Multiple levels of nested parentheses in function types */
typedef void (*(*GTY(()) signal_handler_factory)(int signum))(
    void *GTY(()) context,
    struct GTY(()) {
        int flags;
        void (*GTY(()) cleanup)(void);
    } *GTY(()) config
);

/* Test 8: Union containing arrays of structures with function pointers */
union GTY(()) MegaUnion {
    struct GTY(()) {
        int (*(*GTY(()) callbacks[2][2])(void))(
            char buffer[],
            int lengths[][5]
        );
    } funcs;
    
    struct GTY(()) {
        struct GTY(()) {
            int x;
            int y[(2 + (3 * sizeof(char)))];
        } points[10];
    } data;
};

/* Test 9: Attribute lists with nested parentheses */
struct GTY((user, reorder, skip)) UserType {
    void *GTY((tag("0"), desc("1"))) opaque;
    int GTY((length("((nested) + 1)"))) *dynamic_array;
};

/* Test 10: Deeply nested all delimiter types in single member */
struct GTY(()) UltimateTest {
    /* This should trigger consume_balanced multiple times */
    int (*(*(*GTY(()) ultra_complex)[5])(int arg[2][(3+4)]))(
        struct GTY(()) {
            union GTY(()) {
                char c;
                int i[2];
            } u;
        } param
    )[10];
};

/* Test 11: Mixed delimiters in array dimensions and initializers */
struct GTY(()) MixedDelimiters {
    /* Array with parenthesized size expression */
    int arr1[((sizeof(double) > 4) ? 8 : 4)];
    
    /* Pointer to array of pointers to functions */
    void (*(*GTY(()) func_table[3])[2])(int);
    
    /* Nested anonymous struct with bitfield array */
    struct GTY(()) {
        unsigned int flags:3[2];
        struct GTY(()) {
            char *name;
            int values[];
        } *flex;
    } nested;
};

/* Test 12: Recursive structure with function pointer */
struct GTY(()) RecursiveStruct {
    struct RecursiveStruct *GTY((chain_next)) next;
    int (*GTY(()) processor)(
        struct RecursiveStruct *self,
        int data[],
        void (*GTY(())) callback(
            struct GTY(()) { int result; } *
        )
    );
};

/* Dummy main to make file compilable */
int main(void) {
    return 0;
}
