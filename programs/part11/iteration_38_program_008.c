/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to introduce nested delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define BRACKET_EXPR [ (2 * 3) + (4 / 2) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define COMPLEX_MACRO(x) ((x) + ( (x) * (x) ))

/* Macro that expands to brace-enclosed code (for testing brace consumption) */
#define GTY_BRACE_MARKER /* Intentionally empty - will be used in annotations */

/* ===== Test Case 1: Parentheses in GTY options ===== */

/* Struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int count;
    /* Array with length expression containing nested parentheses */
    int* data GTY((length("(sizeof(int) * (2 + 3))")));
};

/* Alternative: GTY with multiple parenthesized groups */
struct list_node GTY((chain_next("next"), chain_prev("prev"))) {
    int value;
    struct list_node* next;
    struct list_node* prev;
};

/* GTY with deeply nested parentheses */
struct deep_paren GTY((tag("NESTED_TAG"),
                       length("((((1) + (2)) * ((3) - (4))) / (5))"))) {
    int items[];
};

/* ===== Test Case 2: Brackets in type declarations ===== */

/* Array type with complex dimension expression inside brackets */
struct array_test GTY(()) {
    /* Direct array with bracketed expression */
    int arr1[ (2 * 3) + sizeof(int) ];
    
    /* Pointer to array with GTY annotation */
    int (*arr2 GTY((length("10"))))[ (5) ];
    
    /* Multi-dimensional array with nested brackets */
    int matrix GTY((length("N * M")))[ 3 ][ 2 + (3 * 4) ];
};

/* Typedef involving brackets */
typedef int matrix_t GTY(( )) [ (10) ][ (20) ];

/* Function pointer array with brackets */
struct func_table GTY(( )) {
    void (*callbacks[ (5) + (3) ])(void);
};

/* ===== Test Case 3: Braces in type definitions ===== */

/* Union with GTY and nested structure definition (contains braces) */
union u1 GTY((user)) {
    int i;
    /* Nested struct definition inside union (contains braces) */
    struct inner_struct {
        int x;
        int y;
    } nested GTY((tag("INNER")));
    
    /* Array with initializer-like syntax in GTY comment */
    char arr[10];
};

/* Struct containing another struct definition */
struct outer GTY(( )) {
    /* Anonymous struct with braces */
    struct {
        int a;
        int b;
    } inner GTY((skip));
    
    /* Another struct with initializer-like GTY option */
    struct point {
        int x;
        int y;
    } points[10] GTY((length("10")));
};

/* ===== Test Case 4: Mixed delimiters ===== */

/* Complex type mixing all delimiters */
struct mixed_delimiters GTY((user,
    tag("MIXED"),
    length("sizeof(struct { int a; char b; })"))) {
    /* Function pointer with complex signature */
    int (*complex_func GTY((callback)))(
        int param1,
        int (*nested_func)(int arr[ (10) + (20) ]),
        struct { int x; int y; } point
    );
    
    /* Array of pointers to arrays */
    int* (*array_ptr GTY((length("5"))))[ (3) + (2) ];
};

/* ===== Test Case 5: GTY on typedef with complex type ===== */

/* Typedef for function pointer with nested parentheses */
typedef void (*complex_callback GTY((callback)))(
    int (*)(int, char[ (sizeof(int)) ]),
    struct { int x; int y[ (10) ]; }*
);

/* Typedef with array and function mix */
typedef int (*func_array_t[ (5) ])(
    int param[ (3) * (2) ],
    void (*)(struct { int a; }*)
) GTY(( ));

/* ===== Test Case 6: Forward declarations with GTY ===== */

/* Forward declared struct with GTY */
struct forward_decl GTY((user, tag("FORWARD"))) ;

/* Later definition with complex GTY options */
struct forward_decl {
    int value;
    /* Conditional expression in length */
    char* data GTY((length("(value > 0) ? (value * 2) : (10)")));
    
    /* Nested anonymous union with braces */
    union {
        int as_int;
        float as_float;
    } converter;
};

/* ===== Test Case 7: GTY with macro expansions ===== */

/* Using macro that expands to parenthesized expression */
struct macro_test GTY((length("NESTED_PAREN_EXPR"))) {
    int items[];
};

/* GTY option with macro that might expand to braces */
struct brace_macro_test GTY((user)) {
    /* Note: Actual brace content would be in macro definition */
    int value;
};

/* ===== Test Case 8: Edge cases ===== */

/* Empty braces in struct definition */
struct empty_braces GTY(( )) {
    /* Nothing inside - just testing brace parsing */
};

/* Array with zero-sized bracket expression */
struct zero_array GTY((length("0"))) {
    char empty[ 0 ];
};

/* Nested GTY annotations (if supported) */
struct doubly_nested {
    struct inner GTY(( )) {
        int x;
    } inner_obj;
    
    struct outer GTY((user)) {
        struct inner* ptr;
    } outer_obj;
};

#endif /* TEST_PARSE_H */
