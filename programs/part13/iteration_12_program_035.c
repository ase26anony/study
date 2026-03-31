/* complex_gty_test.c - Test file for gengtype delimiter parsing */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter lists and attributes */
typedef void (*GTY((user)) complex_fn_type)(
    int (*GTY((skip)) callback)(char GTY((length("10"))) [10]),
    struct GTY((tag("1"))) { int x; } param
);

struct GTY((user)) test_struct_1 {
    /* Function pointer array with complex signature */
    complex_fn_type GTY((tag("2"))) fn_array[5];
    
    /* Nested function pointer declaration */
    int (*GTY((skip)) nested_fn[3])(
        struct GTY((tag("3"))) { 
            int a; 
            char b[((10 + sizeof(struct test_struct_1)) * 2)]; 
        } param
    );
};

/* Test 2: Array declarations with complex size expressions */
struct GTY((user)) test_struct_2 {
    /* Array with size containing parentheses expression */
    int arr1[(10 + sizeof(struct test_struct_1))];
    
    /* Multi-dimensional array with complex dimensions */
    char arr2[5][(sizeof(int) * 2)][10];
    
    /* Array of pointers to arrays */
    int (*GTY((skip)) arr3[(2 * 3)])[(5 + 1)];
};

/* Test 3: Deeply nested structures and unions */
struct GTY((user)) outer_struct {
    struct GTY((tag("4"))) middle_struct {
        union GTY((tag("5"))) inner_union {
            int x;
            struct GTY((tag("6"))) {
                int a;
                struct GTY((tag("7"))) {
                    int b;
                    char c[({ int y = 5; y * 2; })];
                } deepest;
            } y;
        } u;
        
        /* Bit-field with complex expression */
        unsigned int bits : (sizeof(int) * 8 - 1);
    } mid;
    
    /* Pointer to function returning pointer to array */
    int (*(*GTY((chain_next, chain_prev)) complex_ptr)(
        void (*)(int[][10])
    ))[10];
};

/* Test 4: Multiple balanced delimiters in single declaration */
struct GTY((user)) delimiter_test {
    /* Combination of all three delimiter types */
    void (*GTY((skip)) fn_array[5])(
        int GTY((length("((2 * 3) + 4)"))) [][10],
        struct GTY((tag("8"))) { 
            int x[({ int a = 1, b = 2; a + b; })]; 
        } param
    );
    
    /* Initializer-like nested braces */
    struct GTY((tag("9"))) {
        int values[3];
        struct GTY((tag("10"))) {
            char *name;
            int id;
        } info;
    } data = { 
        {1, 2, 3}, 
        {"test", 42} 
    };
};

/* Test 5: Template-like macro patterns with nested delimiters */
#define GTY_TEMPLATE(type) \
    struct GTY((user)) template_##type { \
        type GTY((skip)) data; \
        struct GTY((tag("11"))) { \
            type (*process)(type[], int); \
        } ops; \
    }

/* Instantiate template-like structures */
GTY_TEMPLATE(int);
GTY_TEMPLATE(char *);

/* Test 6: Complex typedef with nested attributes */
typedef struct GTY((user)) {
    /* Member with GTY annotation containing nested parentheses */
    struct test_struct_1 * GTY((chain_next("next"), chain_prev("prev"))) ptr;
    
    /* Array of function pointers */
    int (*GTY((skip)) callbacks[3])(
        char GTY((length("({ int x = 5; x * 2; })"))) []
    );
} complex_typedef_t;

/* Test 7: Union with variant containing all delimiter types */
union GTY((user)) all_delimiters {
    /* Case 1: Parentheses-heavy */
    int (*func_ptr)(
        int (*)(char[10]), 
        struct { int x; }
    );
    
    /* Case 2: Brackets-heavy */
    int multi_array[3][(5 + 2)][10];
    
    /* Case 3: Braces-heavy */
    struct GTY((tag("12"))) {
        struct GTY((tag("13"))) {
            int a;
            struct GTY((tag("14"))) {
                int b;
            } inner;
        } nested;
    } str;
    
    /* Case 4: Mixed */
    void (*mixed[2])(
        int[][({ int y = 3; y + 2; })],
        struct { char c; }
    );
};

/* Test 8: Recursive structure with complex member */
struct GTY((user)) recursive_struct {
    struct recursive_struct *GTY((skip)) next;
    
    /* Complex array declaration inside recursive struct */
    int (*matrix[(sizeof(struct recursive_struct) / 4)])[
        (10 * ({ int factor = 2; factor * 3; }))
    ];
    
    /* Nested anonymous struct with initializer */
    struct GTY((tag("15"))) {
        int x;
        int y[({ int size = 5; size; })];
    } point = {0, {1, 2, 3, 4, 5}};
};

/* Test 9: Multiple GTY annotations with nested attribute lists */
struct GTY((user, desc("test"))) multi_annotation {
    /* Field with chain_next/prev that have parameters */
    struct multi_annotation *GTY((chain_next("next_field"), 
                                 chain_prev("prev_field"))) chain;
    
    /* Skip with reason in parentheses */
    void *GTY((skip("because it's complex"))) skipped_ptr;
    
    /* Length attribute with expression */
    char *GTY((length("(len + 1)"))) variable_len_str;
    
    /* Nested GTY annotation scenario */
    struct GTY((tag("16"))) {
        int *GTY((skip)) data;
    } container;
};

/* Test 10: Extreme nesting of all delimiter types */
struct GTY((user)) extreme_nesting {
    /* Function returning pointer to array of function pointers */
    int (*(*(*GTY((skip)) level1)(void))[5])(
        struct GTY((tag("17"))) {
            int (*(*level2)(int (*)(char[][10])))[10];
            struct GTY((tag("18"))) {
                void (*level3[3])(
                    int[][({ int d = 4; d * 2; })],
                    struct { 
                        union { 
                            int x; 
                            char y[((5 * 2) + 3)]; 
                        } u; 
                    }
                );
            } inner;
        }
    );
};

/* Test 11: Initializers with designators and nested braces */
struct GTY((user)) with_initializer {
    int array[3];
    struct GTY((tag("19"))) {
        char *name;
        int id;
    } info;
} init_var = {
    .array = {[(2 - 1)] = 42, [0] = 1, [2] = 3},
    .info = {"initialized", 100},
    /* Additional nested initializer */
    .info.name = (char[({ int len = 10; len + 1; })]){"test"},
};

/* Test 12: Type definition containing all delimiter cases */
typedef union GTY((user)) {
    /* Parentheses case */
    int (*func)(int, 
                void (*)(char[], 
                         struct { int x; }
                        )
               );
    
    /* Brackets case */
    long double multi_dim[2][(sizeof(int) + 2)][10];
    
    /* Braces case */
    struct GTY((tag("20"))) {
        struct GTY((tag("21"))) {
            struct GTY((tag("22"))) {
                int deepest;
            } level3;
        } level2;
    } level1;
} ultimate_delimiter_test_t;

/* Main function to make the file compilable (though GTY expands to nothing normally) */
int main(void) {
    /* Reference all types to avoid unused warnings */
    struct test_struct_1 ts1 = {0};
    struct test_struct_2 ts2 = {0};
    struct outer_struct os = {0};
    struct delimiter_test dt = {0};
    template_int ti = {0};
    template_char_ptr tcp = {0};
    complex_typedef_t ct = {0};
    union all_delimiters ad = {0};
    struct recursive_struct rs = {0};
    struct multi_annotation ma = {0};
    struct extreme_nesting en = {0};
    struct with_initializer wi = init_var;
    ultimate_delimiter_test_t udt = {0};
    
    return 0;
}
