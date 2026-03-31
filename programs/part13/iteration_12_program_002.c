/* complex_gty_test.c - Test file for gengtype delimiter parsing */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists */
typedef void (*GTY(()) complex_fn_ptr)(
    int (*GTY(()) nested_callback)(char[10][20]),
    struct GTY(()) inner_struct { int x; double y; }
);

/* Test 2: Structure with multiple nested delimiter types */
struct GTY(()) outer_struct {
    /* Array with complex dimension expression */
    int arr1[(10 + sizeof(struct GTY(()) temp { int a; }))];
    
    /* Function pointer array */
    void (*GTY((chain_next, chain_prev)) fn_array[5])(
        int matrix[][10],
        struct GTY(()) config { int mode; char name[50]; }
    );
    
    /* Nested union with initializer-like syntax in comments */
    union GTY(()) {
        struct GTY(()) {
            long *GTY((tag("0"))) ptr_field;
            short bits:4;
        } s;
        double dbl_array[2][3];
    } nested_union;
};

/* Test 3: Deeply nested parentheses in type expressions */
typedef int (*(*GTY(()) deep_nested_fn)(void (*)(int)))(char, float);

/* Test 4: Structure with attribute-packed nested struct */
struct GTY(()) container {
    struct GTY(()) __attribute__((packed)) packed_member {
        unsigned char data[256];
        int (*GTY(()) handlers[4])(void);
    } pm;
    
    /* Multi-dimensional array with function pointers */
    void (*(*GTY(()) complex_array[2][3])(int))[5];
};

/* Test 5: Union containing anonymous struct with bitfields */
union GTY(()) bitfield_union {
    struct GTY(()) {
        unsigned int a:1;
        unsigned int b:2;
        unsigned int c:3;
        unsigned int d:26;
    } bits;
    unsigned int full;
};

/* Test 6: Type definition with nested array dimensions */
typedef char (*(*GTY(()) string_table)[10][20])[30];

/* Test 7: Structure with nested initializer-like constructs in comments */
struct GTY(()) has_comments {
    int x;
    /* Comment with balanced delimiters: { [ ( test ) ] } */
    int y;
    /* Another: int arr[] = {1, {2, 3}, 4}; */
    int z;
};

/* Test 8: Recursive structure with function pointer */
struct GTY((chain_next, chain_prev)) recursive_node {
    struct recursive_node *GTY((chain_next)) next;
    struct recursive_node *GTY((chain_prev)) prev;
    void (*GTY(()) process)(struct recursive_node *self, int data[]);
    int values[10][(sizeof(void *) * 2)];
};

/* Test 9: Complex typedef with all delimiter types */
typedef struct GTY(()) {
    int (*GTY(()) compare)(
        const void *,
        const void *,
        int options[3]
    );
    void (*GTY(())) cleanup(void);
    char name[(sizeof(int) + sizeof(long))];
} complex_callback_t;

/* Test 10: Template-like macro expansion (C style) */
#define DECLARE_VECTOR(TYPE) \
    struct GTY(()) vector_##TYPE { \
        TYPE *GTY((length("size"))) data; \
        size_t size; \
        size_t capacity; \
        void (*GTY(())) resize(struct vector_##TYPE *v, size_t new_cap); \
    }

DECLARE_VECTOR(int);
DECLARE_VECTOR(double);
DECLARE_VECTOR(struct outer_struct);

/* Test 11: Nested structures with array of function pointers */
struct GTY(()) module {
    struct GTY(()) api {
        int (*GTY(()) init)(void);
        int (*GTY(()) process)(int input[], size_t len);
        void (*GTY(()) shutdown)(void);
    } *apis[5];
    
    struct GTY(()) config {
        char *GTY((length("strlen(name)+1"))) name;
        int settings[10][20];
        struct GTY(()) option {
            int id;
            char value[50];
        } options[];
    } cfg;
};

/* Test 12: Union with anonymous struct containing nested arrays */
union GTY(()) data_container {
    struct GTY(()) {
        int (*GTY(())) handlers[2][3])(
            union data_container *self,
            int param
        );
        char buffer[1024][(256 / sizeof(char))];
    };
    void *ptr_array[(sizeof(void *) * 16)];
};

/* Test 13: Structure with deeply nested attribute lists */
struct GTY((user)) user_defined {
    void *GTY((skip)) opaque_data;
    int (*GTY((callback))) validate(
        struct user_defined *ud,
        int criteria[],
        struct GTY(()) result { int code; char msg[100]; } *output
    );
};

/* Test 14: Mix of all delimiters in single member declaration */
struct GTY(()) delimiter_test {
    /* This should trigger all three consume_balanced calls */
    void (*(*GTY(()) extreme_member)(
        int param1,
        char param2[][10],
        struct GTY(()) { int x; } inner
    )[(sizeof(void *) + 8)])[5])(
        float matrix[3][4],
        void (*callback)(void)
    );
};

/* Test 15: Variable declaration with initializer (in comments to avoid compilation errors) */
/* static struct outer_struct GTY(()) instance = {
    .arr1 = {1, 2, [5] = 3, 4},
    .fn_array = {NULL, NULL, NULL, NULL, NULL},
    .nested_union = {.s = {.ptr_field = NULL, .bits = 0}}
}; */

/* Main function to make the file compilable */
int main(void) {
    return 0;
}
