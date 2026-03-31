/* complex-gty-test.c - Test file for gengtype delimiter parsing */

/* Include necessary headers for GTY annotations */
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Complex function pointer with nested parentheses */
typedef void (*GTY((user)) complex_func_ptr)(
    int (*GTY((skip)) nested_callback)(char GTY((length("strlen(param)+1"))) param[]),
    struct GTY((tag("1"))) { 
        int x; 
        union GTY((desc("%1.type"))) {
            int type;
            float value;
        } data;
    } context
);

/* Test 2: Array declarations with complex dimensions containing parentheses */
struct GTY(()) ArrayTest {
    /* Array with dimension containing parenthesized expression */
    int arr1[(10 + sizeof(struct { char pad; int value; }))];
    
    /* Multi-dimensional array with function pointer type */
    complex_func_ptr (*arr2[3][2])(int, float);
    
    /* Array of pointers to arrays */
    int (*arr3[5])[10];
};

/* Test 3: Deeply nested structure with all delimiter types */
struct GTY((chain_next("next"), chain_prev("prev"))) OuterStruct {
    /* Field with parentheses (function pointer) */
    void (*GTY((skip)) operation)(
        struct OuterStruct *GTY((tag("0"))) self,
        int params[],
        void (*callback)(int, char)
    );
    
    /* Field with brackets (complex array) */
    struct {
        int matrix[3][(2 * sizeof(int))];
        char *GTY((length("len"))) strings[];
    } GTY((tag("1"))) data_block;
    
    /* Field with braces (nested anonymous struct) */
    union {
        struct {
            int x;
            int y;
        } point;
        struct {
            int arr[10];
            struct GTY(()) {
                char *name;
                int id;
            } info;
        } data;
    } GTY((tag("2"))) variant;
    
    struct OuterStruct *next;
    struct OuterStruct *prev;
};

/* Test 4: Template-like pattern using nested types */
#define DECLARE_CONTAINER(TYPE) \
    struct GTY(()) Container_##TYPE { \
        TYPE *GTY((length("count"))) items; \
        int count; \
        void (*GTY((skip)) processor)(TYPE[], int); \
    }

/* Instantiate with complex type */
DECLARE_CONTAINER(struct {
    int key;
    char *GTY((string)) value;
    struct {
        float x, y;
    } coordinates[4];
});

/* Test 5: Complex initializer with nested braces */
static struct GTY(()) InitTest {
    int numbers[3];
    struct {
        char *name;
        int flags;
    } config;
} GTY((user)) init_var = {
    .numbers = {[(2 + 1) % 3] = 42, [0] = 13, [1] = 7},
    .config = {
        .name = "test",
        .flags = (1 << 3) | (1 << 5)
    }
};

/* Test 6: Union with nested function pointers and arrays */
union GTY((desc("%0.type"))) ComplexUnion {
    int type;
    struct {
        int (*GTY((skip)) compare)(
            const void *,
            const void *,
            int (*)(const char *, const char *)
        );
        char buffer[256];
    } GTY((tag("1"))) func_data;
    struct {
        int matrix[2][(3 + sizeof(void*))];
        struct GTY(()) {
            int id;
            char *GTY((string)) name;
        } entries[10];
    } GTY((tag("2"))) array_data;
};

/* Test 7: Recursive structure with all delimiter types */
struct GTY(()) TreeNode {
    char *GTY((string)) name;
    struct TreeNode *GTY((child)) children[];
    
    /* Method pointer with complex signature */
    void (*GTY((skip)) traverse)(
        struct TreeNode *,
        void (*)(char *, int),
        int options[3]
    );
    
    /* Anonymous union with initializer */
    union {
        int int_val;
        float float_val;
        struct {
            char *str;
            int len;
        } string_data;
    } value;
};

/* Test 8: Multiple nested GTY annotations */
typedef struct GTY((for_user)) {
    /* Pointer with chain_next inside nested struct */
    struct GTY((chain_next("link.next"), chain_prev("link.prev"))) {
        void *data;
        struct {
            struct GTY(()) Link *next;
            struct GTY(()) Link *prev;
        } link;
    } *GTY((tag("0"))) link_ptr;
    
    /* Array of function pointers */
    int (*GTY((skip)) handlers[5])(
        void *,
        struct { int x; int y; } point,
        char args[][32]
    );
} ComplexType;

/* Test 9: Structure with bit-fields and complex array dimensions */
struct GTY(()) BitFieldTest {
    unsigned int flags : 3;
    unsigned int : 5;  /* Padding */
    unsigned int mode : 4;
    
    /* Array dimension using bit-field size */
    char buffer[1 << 4];
    
    /* Nested structure with its own GTY annotation */
    struct GTY((user)) {
        int count;
        /* Pointer to array of pointers */
        int (**GTY((skip)) ptr_array)[10];
    } nested;
};

/* Test 10: Ultimate test - all delimiters deeply nested */
struct GTY(()) UltimateTest {
    /* Parentheses within brackets within braces */
    struct {
        /* Function returning pointer to array */
        int (*(*funcs[3])(int))[(sizeof(double) + 7) / 8];
        
        /* Union with anonymous struct containing array of function pointers */
        union {
            struct {
                void (*actions[2])(int, char);
                int (*calculators[])(float, double);
            };
            char data[256];
        } processor;
    } level1;
    
    /* Array of structures containing function pointers with complex params */
    struct {
        int (*callback)(
            struct { int a; int b; } pair,
            char *strings[],
            void (*cleanup)(void *)
        );
        int result;
    } operations[5];
    
    /* Final nested initializer */
    struct {
        int values[3];
        struct {
            char *name;
            int id;
        } items[2];
    } config = {
        .values = {1, 2, 3},
        .items = {
            [0] = {.name = "item1", .id = 100},
            [1] = {.name = "item2", .id = 200}
        }
    };
};

/* Main function to make the file compilable */
int main(void) {
    return 0;
}
