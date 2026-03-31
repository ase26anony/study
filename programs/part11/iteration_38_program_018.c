/* test_parse.h - Complex GTY annotations to test delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to add nesting depth */
#define NESTED_EXPR ( (10) + (20) )
#define ARRAY_SIZE_EXPR ( (2 * 3) + (4 / 2) )
#define COMPLEX_BRACE { struct inner { int x; }; }
#define BRACKET_EXPR [ (sizeof(int) * (2 + 3)) ]

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int count;
    /* Flexible array member with complex length expression */
    int data GTY((length("(sizeof(int) * (2 + 3))"))) [];
};

/* Case 2: Parentheses - GTY with multiple parenthesized groups */
struct list_node GTY((chain_next("next"), chain_prev("prev"))) {
    int value;
    struct list_node *next;
    struct list_node *prev;
};

/* Case 3: Parentheses - Function pointer with nested parentheses in arguments */
typedef void (*complex_func_ptr GTY((callback)))(
    int (*)(int [ (sizeof(int) * 2) ]),
    void (*)(struct s1 *)
);

/* Case 4: Brackets - Array with complex dimension expression inside GTY */
struct array_struct GTY((user)) {
    int size;
    /* Array with dimension containing parentheses */
    int arr GTY((length("NESTED_EXPR")))[ (2 * 3) + (4 / 2) ];
};

/* Case 5: Brackets - Pointer to array with nested brackets */
typedef int (*ptr_to_array GTY(()))[ (1 << 3) ][ (2 + 2) ];

/* Case 6: Brackets - Multi-dimensional array with complex expressions */
struct md_array GTY((user)) {
    int matrix GTY((length("(width * height)")))[ (10 + 5) ][ (20 - 4) ];
    int width;
    int height;
};

/* Case 7: Braces - Structure with nested struct definition inside GTY */
struct outer_struct GTY((user)) {
    /* Nested structure definition with braces */
    struct inner_struct GTY((tag("LANG"))) {
        int x;
        int y;
    } nested;
    
    /* Another nested struct with initializer-like syntax in comment */
    struct another_inner GTY((user)) {
        int data[ (sizeof(int) + 1) ];
    } another GTY((length("sizeof(struct another_inner)")));
};

/* Case 8: Braces - Union with GTY and nested braces */
union complex_union GTY((user)) {
    int i;
    struct {
        char a;
        char b;
    } chars;
    long array[ (sizeof(long) > 4) ? 2 : 1 ];
};

/* Case 9: All delimiters combined - Complex type with mixed delimiters */
struct mixed_delimiters GTY((user)) {
    /* Parentheses in function pointer */
    void (*callback GTY((callback)))(int, char);
    
    /* Brackets in array with parenthesized size */
    int items[ (10) + (20) ];
    
    /* Nested struct with braces */
    struct {
        int x GTY((length("(sizeof(int) * 2)")))[ (1 + 2) ];
        int y;
    } nested;
};

/* Case 10: GTY with string containing delimiters */
struct string_delimiters GTY((user)) {
    /* String containing delimiter characters */
    const char *pattern GTY((length("strlen(\"(pattern)\") + 1")));
    int values[ (3) ];
};

/* Case 11: Forward declaration with GTY containing parenthesized expression */
struct forward_decl GTY((user));
struct forward_decl {
    int data;
    struct forward_decl *next GTY((chain_next("next")));
};

/* Case 12: Typedef with GTY and complex array declarator */
typedef struct {
    int count;
    double values GTY((length("count")))[ (sizeof(double) * 8) ];
} complex_typedef GTY((user));

/* Case 13: GTY option with nested parentheses in skip expression */
struct skip_example GTY((skip(("(ptr)->field != 0")))) {
    void *ptr;
    int field;
};

/* Case 14: Array of function pointers with GTY */
typedef int (*func_array GTY((callback)))[ (5) ](int, int);

/* Case 15: Structure containing anonymous union with braces */
struct with_anon_union GTY((user)) {
    int type;
    union {
        int int_val;
        double dbl_val;
        char str_val[ (32) ];
    } data;
};

#endif /* TEST_PARSE_H */
