/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to add nesting depth */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define COMPLEX_BRACKET_EXPR [ (10) + (20) ]
#define BRACE_INITIALIZER { .x = 1, .y = 2 }
#define NESTED_MACRO(x) ((x) * (2 + (3)))

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int count;
    /* Flexible array member with complex length expression */
    int data GTY((length("(sizeof(int) * (2 + 3))")))[];
};

/* Additional parentheses case with multiple parenthesized groups */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Case 2: Brackets - array types with complex dimension expressions */
struct s3 GTY((user)) {
    /* Array with dimension containing parentheses */
    int arr1 GTY((tag("ARR1")))[ (2 * 3) + 4 ];
    
    /* Pointer to array with nested brackets */
    int (*arr2 GTY((length("N"))))[ (5) ][ (6) ];
    
    /* Multi-dimensional array */
    char matrix GTY((user))[ (1 + 2) ][ (3 * 4) ];
};

/* Typedef with array declarator containing brackets */
typedef struct s4 GTY((user)) {
    int values[ (sizeof(int) == 4) ? 10 : 20 ];
} s4_t;

/* Case 3: Braces - structure with nested struct definition */
struct outer GTY((user)) {
    int id;
    
    /* Nested structure definition (contains braces) */
    struct inner GTY((tag("INNER"))) {
        int x;
        int y;
        char name[32];
    } nested;
    
    /* Union with brace-enclosed initializer in comment/string */
    union {
        int i;
        float f;
    } u GTY((desc("%0.u.i ? 1 : 0")));
};

/* Function pointer typedef with complex argument list containing brackets */
typedef void (*complex_func_ptr GTY((callback)))(
    int (*)(int arr[ (4) + (5) ]),
    char (*)[ (2) * (3) ]
);

/* Union with array containing bracketed size expression */
union u1 GTY((user)) {
    int i;
    char arr[ 10 + (5 * 2) ];
    long long big[ (sizeof(long) == 8) ? 2 : 4 ];
};

/* Forward declared struct with GTY annotation containing nested parentheses */
struct forward_decl GTY((user));
struct forward_decl {
    int value;
    struct forward_decl *next GTY((skip));
};

/* Structure with GTY options containing all delimiter types */
struct all_delimiters GTY((
    user,
    desc("(nested (parentheses) here)"),
    length("(complex + (expression))")
)) {
    /* Array with bracketed size containing parentheses */
    int items[ (1 << 3) + (2 * 5) ];
    
    /* Nested structure (braces) */
    struct {
        int a;
        int b;
    } pair;
    
    /* Pointer to function with complex signature */
    void (*handler)(
        int x[ (10) ],
        struct { int y; } GTY((user)) *ptr
    ) GTY((callback));
};

/* Macro expansion within GTY annotation */
struct macro_test GTY((length("NESTED_PAREN_EXPR"))) {
    int data[];
};

/* GTY with string containing all delimiter types */
struct string_delimiters GTY((user)) {
    const char *pattern GTY((length("strlen(pattern) + (1)")));
    int flags[ (1) | (2) | (4) ];
};

/* Additional test cases to ensure coverage */

/* Nested parentheses in skip expression */
struct skip_test GTY((skip(("ptr") && ("next")))) {
    struct skip_test *next;
    void *ptr;
};

/* Array of pointers with GTY */
typedef struct node GTY((user)) {
    int value;
    struct node *children GTY((length("child_count")))[];
    int child_count;
} node_t;

/* GTY with conditional expression containing parentheses */
struct cond_expr GTY((desc("(%0.type == 1) ? \"TYPE1\" : \"TYPE2\""))) {
    int type;
    union {
        int i;
        float f;
        char *str GTY((length("strlen(%h.str) + (1)")));
    } data;
};

/* Test for deeply nested parentheses */
struct deep_nest GTY((user)) {
    int (*complex_array_ptr GTY((length("((((5) + (3)) * (2)) / (4))"))))[
        (((2) + (3)) * (4))
    ][
        (1 + (2 * (3 + (4))))
    ];
};

/* Structure with bitfield containing parenthesized expression */
struct bitfield_test GTY((user)) {
    unsigned int flags : (1 + 2 + 1);
    unsigned int mask : (sizeof(int) * 8 - 1);
};

/* End of test cases */

#endif /* TEST_PARSE_H */
