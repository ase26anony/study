/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void* my_ptr;

/* Macros to add nesting depth */
#define NESTED_EXPR ( (10) + (20) )
#define COMPLEX_SIZE (sizeof(int) * (2 + 3))
#define ARRAY_DIM ( (5) * (3) )
#define BRACE_BLOCK { int x = 0; }
#define CALLBACK_ARG int (*)(int [ (4) ])

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int count;
    /* Flexible array member with complex length expression */
    int data GTY((length("(sizeof(int) * (2 + 3))")))[];
};

/* Case 2: More parentheses - doubly linked list with chain_next/chain_prev */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    int value;
    struct s2 *next;
    struct s2 *prev;
};

/* Case 3: Parentheses with macro expansion */
struct s3 GTY((length("NESTED_EXPR"))) {
    int items GTY((length("NESTED_EXPR")))[];
};

/* Case 4: Brackets - array with complex dimension expression */
struct s4 GTY(()) {
    /* Array with dimension containing parentheses */
    int arr GTY((length("N")))[ (2 * 3) + (4 - 1) ];
    int N;
};

/* Case 5: Nested brackets - pointer to array */
typedef int (*array_ptr GTY((user)))[ (sizeof(int) > 4) ? 10 : 20 ];

/* Case 6: Multi-dimensional array with brackets */
struct s5 GTY(()) {
    /* 2D array with computed dimensions */
    int matrix GTY((length("rows"), param_is("struct s5 *")))[ ARRAY_DIM ][ (3 + 2) ];
    int rows;
};

/* Case 7: Function pointer with brackets in parameter */
typedef void (*complex_func GTY((callback)))(int (*)(int [ (4) ][ (2) ]));

/* Case 8: Braces - union with nested structure definition */
union u1 GTY((desc("0"))) {
    int i;
    /* Nested structure with braces */
    struct inner GTY((tag("1"))) {
        int x;
        int y;
    } nested;
    char arr[ 10 + (5) ];
};

/* Case 9: Structure containing another structure with braces */
struct s6 GTY((user)) {
    /* Direct nested structure definition */
    struct GTY((tag("LANG"))) {
        int a;
        int b GTY((length("(a > 0) ? a : 1")))[];
    } inner;
    int outer_field;
};

/* Case 10: Complex callback with all delimiters */
typedef void (*callback_func GTY((callback)))(
    int param1,
    int param2[],
    struct { int x; int y; } point
);

/* Case 11: Variable length array in structure */
struct s7 GTY((variable_size)) {
    int length;
    /* VLA with parenthesized size expression */
    char data GTY((length("(length + 7) & ~7")))[];
};

/* Case 12: Union with array containing parenthesized expression */
union u2 GTY((desc("$1 == 0"))) {
    int type;
    float values[ (int)(sizeof(float) * (2)) ];
};

/* Case 13: Structure with skip field containing complex expression */
struct s8 GTY((skip("( (char *)&((type *)0)->field - (char *)0 )"))) {
    int field1;
    int field2;
    char field3;
};

/* Case 14: Nested parentheses in skip expression */
struct s9 GTY((skip("(offsetof(struct s9, end) - offsetof(struct s9, start))"))) {
    int start;
    int middle GTY((length("(10)")))[];
    int end;
};

/* Case 15: Array of function pointers */
typedef int (*func_array GTY((user))[ (3) ])(int, int);

/* Forward declarations with GTY annotations containing delimiters */
struct forward1 GTY((user));
struct forward2 GTY((chain_next("next")));

/* Case 16: Conditional expression in array dimension */
struct s10 GTY(()) {
    int conditional_array GTY((length("cond ? 10 : 20")))[ 
        (sizeof(int) == 4) ? 100 : 200 
    ];
    int cond;
};

/* Case 17: Multiple nested parentheses */
struct s11 GTY((length("((((5) + (3)) * (2)) - (1))"))) {
    int deeply_nested GTY((length("((((5) + (3)) * (2)) - (1))")))[];
};

/* Case 18: Structure with embedded anonymous union (braces) */
struct s12 GTY((user)) {
    int tag;
    union GTY((desc("tag"))) {
        int as_int;
        float as_float;
        char as_char GTY((length("(tag + 1)")))[];
    } value;
};

/* Case 19: Pointer to array with computed size */
typedef struct s13 {
    int count;
    /* Pointer to array with parenthesized size */
    int (*data_ptr GTY((length("count"))))[ (sizeof(int) * 2) ];
} s13_t;

/* Case 20: Final test - mixing all delimiters */
struct s14 GTY((user, skip("(offsetof(struct s14, data) + (N * sizeof(int)))"))) {
    int N;
    int header[ (N > 0) ? N : 1 ];
    struct {
        int x GTY((length("(N + 2)")))[];
        int y;
    } data;
};

#endif /* TEST_PARSE_H */
