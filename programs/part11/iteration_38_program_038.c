/* test_parse.h - Complex GTY annotations to test delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void *my_ptr;

/* Macros to add nesting depth for delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define COMPLEX_BRACKET_EXPR [ (10) + (20) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define ANOTHER_EXPR(a, b) ((a) + (b) * (2))

/* ========== PARENTHESES TESTING ========== */
/* Struct with GTY annotation containing nested parentheses */
struct s1 GTY((user)) {
    int data GTY((length("(sizeof(int) * (2 + 3))")))[];
    struct s1 *next GTY((skip));
};

/* Another struct with multiple parenthesized groups in GTY options */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    int value;
    struct s2 *next;
    struct s2 *prev;
};

/* Function pointer typedef with complex parentheses */
typedef void (*complex_func_ptr GTY((callback)))(
    int (*)(int, char),
    void *(*)(int[ (4) ])
);

/* ========== BRACKETS TESTING ========== */
/* Array type with complex dimension expressions */
struct array_test GTY(()) {
    int arr1 GTY((length("N")))[ (2 * 3) ];
    char arr2 GTY((length("(1 << 5)")))[ NESTED_PAREN_EXPR ];
    double *arr3 GTY((length("sizeof(double) * 8")))[];
};

/* Pointer to array with nested brackets */
typedef int (*ptr_to_array GTY((skip)))[ (10) + (5) ];

/* Multi-dimensional array */
struct md_array GTY((user)) {
    int matrix GTY((length("ROWS"), param_is("COLS")))[ (3) ][ (4) ];
};

/* ========== BRACES TESTING ========== */
/* Union with nested structure definition (contains braces) */
union u1 GTY((user)) {
    int i;
    struct inner1 GTY((tag("LANG"))) {
        int x;
        int y;
    } nested;
    char arr[ 10 + (5) ];
};

/* Struct containing another struct definition with braces */
struct outer_struct GTY((user)) {
    struct inner_struct GTY((tag("1"))) {
        int a;
        int b GTY((length("(2)")))[];
    } inner;
    int count;
};

/* Another approach: Use GTY marker on a type with initializer-like syntax */
struct brace_test GTY((user)) {
    int values[3];
} GTY((user)) my_brace_test = { {1, 2, 3} };

/* ========== COMBINED DELIMITER TESTING ========== */
/* Complex type mixing all delimiters */
struct complex_type GTY((user)) {
    /* Parentheses in function pointer */
    int (*compare GTY((callback)))(const void *, const void *);
    
    /* Brackets in array with parenthesized size */
    void *buffer GTY((length("(PAGE_SIZE * (2))")))[];
    
    /* Nested struct with braces */
    struct {
        int id;
        char name[ (32) ];
    } GTY((tag("INFO"))) info;
    
    /* Union with array */
    union {
        int num;
        char str[ (64) ];
    } GTY((tag("DATA"))) data;
};

/* ========== FORWARD DECLARATIONS WITH GTY ========== */
/* Forward declared struct with GTY annotation containing nested expression */
struct forward_decl GTY((user));
struct forward_decl {
    int value;
    struct forward_decl *next GTY((skip("(value > 0) ? next : NULL")));
};

/* ========== TYPEDEF WITH COMPLEX GTY ========== */
typedef struct list_node GTY((user)) {
    int data;
    struct list_node *next GTY((skip));
    struct list_node *prev GTY((skip));
} list_node_t;

/* Array of pointers typedef */
typedef list_node_t *node_array GTY((length("COUNT")))[ (10) ];

/* ========== MACRO-EXPANDED DELIMITERS ========== */
/* Use macros that expand to delimiter-heavy expressions */
struct macro_test GTY((user)) {
    int computed_length GTY((length("ANOTHER_EXPR(5, 10)")));
    char dynamic_array GTY((length("NESTED_PAREN_EXPR")))[];
};

/* ========== FUNCTION-LIKE MACRO IN GTY ========== */
#define GTY_SPECIAL(a) ((a) + 1)
struct macro_in_gty GTY((user)) {
    int special_value GTY((length("GTY_SPECIAL( (5) )")));
};

#endif /* TEST_PARSE_H */
