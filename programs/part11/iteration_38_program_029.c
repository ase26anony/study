/* test_parse.h - Complex GTY annotations to test delimiter handling */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to add nesting depth */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define NESTED_BRACKET_EXPR [ (2 * 3) + (4 / 2) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define COMPLEX_MACRO(x) ((x) + (x * 2))

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int count;
    /* Flexible array member with complex length expression */
    int data GTY((length("(sizeof(int) * (2 + 3))")))[];
};

/* Another parentheses case: multiple parenthesized groups */
struct list_node GTY((chain_next("next"), chain_prev("prev"))) {
    int value;
    struct list_node *next;
    struct list_node *prev;
};

/* Case 2: Brackets - array types with complex dimension expressions */
struct s2 GTY(()) {
    /* Array with dimension containing parentheses */
    int arr1 GTY((length("N")))[ (2 * 3) ];
    
    /* Pointer to array with nested brackets in type */
    int (*arr2 GTY((length("M"))))[ (4 + 1) ];
    
    /* Multi-dimensional array */
    int matrix GTY((length("ROWS * COLS")))[ (3) ][ (2 + 2) ];
};

/* Using macro with brackets */
struct s3 GTY(()) {
    char buffer GTY((length("sizeof(char) * 256")))[ NESTED_BRACKET_EXPR[0] ];
};

/* Case 3: Braces - structure with nested struct definition inside GTY */
struct outer GTY((user)) {
    int id;
    
    /* Nested structure definition (contains braces) */
    struct inner GTY((tag("NESTED"))) {
        int x;
        int y[2];
    } nested;
};

/* Union with brace-enclosed initializer in macro expansion */
union u1 GTY(()) {
    int i;
    char str[32];
    struct {
        float f;
        double d;
    } complex;
};

/* Function pointer type with complex signature containing parentheses and brackets */
typedef void (*complex_func_ptr GTY((callback)))(
    int (*)(int arr[ (sizeof(int) > 4) ? 8 : 4 ]),
    void (*)(struct s1 * GTY((skip)))
);

/* GTY annotation with conditional expression containing parentheses */
struct conditional GTY((if("(defined(FLAG1) && defined(FLAG2)) || !defined(FLAG3)"))) {
    int value;
};

/* Template-like macro usage with GTY */
#define DEFINE_GTY_STRUCT(name, size) \
    struct name GTY(()) { \
        my_int data[size]; \
        struct name *next; \
    }

/* Instantiate the macro */
DEFINE_GTY_STRUCT(macro_struct, (10 + 5));

/* Another complex case: GTY with skip containing function-like macro */
struct skipped_fields GTY((skip("COMPLEX_MACRO(offset) && other_check()"))) {
    int visible;
    /* These fields would be skipped by GC */
    int hidden1;
    char hidden2;
    void *hidden3;
};

/* Array of pointers with GTY length containing arithmetic */
struct container GTY(()) {
    void *items GTY((length("(item_count + 7) & ~7"))) /* round up to multiple of 8 */[];
    int item_count;
};

/* Nested GTY annotations */
struct deeply_nested GTY((user)) {
    struct level1 GTY((tag("LEVEL1"))) {
        struct level2 GTY((tag("LEVEL2"))) {
            int value;
            struct level2 *link GTY((chain_next("next")));
        } *entry;
    } *top;
};

/* Test case with all delimiters mixed */
struct all_delimiters GTY((user)) {
    /* Parentheses in length expression */
    int *array GTY((length("(width * height)")))[];
    
    /* Brackets in array dimension */
    char buffer[ (32 + (16 / 2)) ];
    
    /* Braces in nested struct */
    struct {
        int x;
        int y;
    } point;
    
    /* Function pointer with parentheses */
    void (*callback GTY((skip)))(int, char *);
};

/* Forward declaration with GTY annotation containing parentheses */
struct forward_decl GTY((if("(SIZE > 0)")));

/* Actual definition */
struct forward_decl {
    int data;
    struct forward_decl *next GTY((chain_next("next")));
};

/* Union with array containing bracket expression */
union mixed_types GTY(()) {
    int ints[ (sizeof(void*) == 8) ? 2 : 1 ];
    char chars[ (1 << 3) ];
    struct s1 *ptr;
};

/* Final test: GTY with embedded string containing delimiters */
struct string_test GTY((user)) {
    /* The string contains delimiter characters but they're in quotes */
    const char *pattern GTY((skip)) = "{[(])}";
    
    /* Array with computed size */
    int computed[ (int)(sizeof(pattern) / sizeof(pattern[0])) ];
};

#endif /* TEST_PARSE_H */
