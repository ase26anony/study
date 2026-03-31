/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void *ptr_t;

/* Macros to add nesting depth */
#define NESTED_PAREN_EXPR ( (10) + (20) * (30) )
#define BRACKET_EXPR [ (2 * 3) + (4) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define COMPLEX_MACRO(x) ( (x) * ((x) + 1) / 2 )

/* Case 1: Parentheses - struct with complex length expression */
struct s1 GTY((user)) {
    int count;
    /* Multiple nested parentheses in length expression */
    int data GTY((length("(sizeof(int) * (2 + (3 * 4)))"))) [];
};

/* Case 2: Parentheses - multiple GTY options with nested parens */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    struct s2 *next;
    struct s2 *prev;
    int value GTY((skip))[ (1 << 2) + (3 * 4) ];
};

/* Case 3: Parentheses - callback with function pointer containing parens */
typedef int (*complex_func_ptr GTY((callback)))(
    int (*)(int, char (*)[ (2) + (3) ]),
    void *(*)(struct s1 *)
);

/* Case 4: Brackets - array with complex dimension expression */
struct s3 GTY(()) {
    /* Array with nested brackets and parentheses in dimension */
    int matrix GTY((length("NESTED_PAREN_EXPR")))[ (2 * 3) ][ (4 + 5) ];
    /* Pointer to array with brackets */
    int (*ptr_to_array) GTY((skip))[ (10) ];
};

/* Case 5: Brackets - multi-dimensional array with macro expansion */
#define ARRAY_DIM ( (5) * sizeof(int) )
struct s4 GTY((user)) {
    /* Using macro that expands to bracketed expression */
    char buffer GTY((length("ARRAY_DIM")))[ sizeof(int) * (2 + 3) ];
    /* Nested array declarator */
    int (*nested_array)[ (3) ][ (4) ];
};

/* Case 6: Braces - union with nested struct definition */
union u1 GTY((desc("1"))) {
    int i;
    /* Nested struct definition with braces */
    struct inner GTY((tag("0"))) {
        int x;
        int y GTY((length("(2)")))[2];
    } nested;
    /* Array with brace-enclosed size in macro */
    char arr[ 10 + (5) ];
};

/* Case 7: Braces - struct with nested anonymous struct */
struct s5 GTY((user)) {
    /* Anonymous struct with braces */
    struct GTY((tag("1"))) {
        int a;
        int b GTY((length("(sizeof(int))")))[1];
    } inner;
    /* Another level of nesting */
    union {
        struct {
            int x;
        } s;
        int y;
    } u;
};

/* Case 8: Complex combination - all delimiters in one type */
struct s6 GTY((chain_next("n"), chain_prev("p"))) {
    struct s6 *n;
    struct s6 *p;
    /* Array with parenthesized length and bracketed dimension */
    void *data GTY((length("( (sizeof(void *) * (16)) )")))[ (16) ];
    /* Function pointer with complex signature */
    void (*handler GTY((callback)))(
        int,
        char *(*)[ (2) + (3) ],
        struct { int x; int y; } *
    );
};

/* Case 9: Typedef with nested parentheses in array dimension */
typedef struct s7 GTY((user)) {
    int value;
    /* Very complex dimension expression */
    double weights GTY((length("(N * sizeof(double) / (2))")))[ 
        ( (16) * (sizeof(double)) / (8) ) 
    ];
} s7_t;

/* Case 10: Forward declaration with GTY annotation containing expr */
struct s8 GTY((user));
struct s8 {
    /* Conditional expression in length */
    int *items GTY((length("(count > 0 ? count : (1))")));
    int count;
};

/* Case 11: Nested GTY markers with different delimiters */
struct outer GTY((user)) {
    struct middle GTY((tag("MIDDLE"))) {
        struct inner GTY((tag("INNER"))) {
            /* Array with all delimiters in expressions */
            int arr GTY((length("( (2) * [3] ? 4 : {5} )")))[ 
                (2) * (3) + (4) 
            ];
        } inner_struct;
    } middle_struct;
};

/* Case 12: Function-like macro in GTY argument */
#define LENGTH_EXPR(x) ( (x) * ((x) + 1) / 2 )
struct s9 GTY((user)) {
    /* Using function-like macro that expands to parenthesized expr */
    int *dynamic_array GTY((length("LENGTH_EXPR(10)")));
};

/* Case 13: Pointer to array with nested brackets */
typedef int (*array_ptr_t GTY((skip)))[ (3) ][ (4) ][ (5) ];

/* Case 14: Union with array containing parenthesized size */
union u2 GTY((desc("2"))) {
    long long big;
    /* Array size from macro containing parentheses */
    char bytes[ (sizeof(long long)) ];
    struct {
        int a GTY((length("(2)")))[2];
        int b;
    } parts;
};

/* Case 15: Final complex case - deeply nested expressions */
struct s10 GTY((user)) {
    /* Multiple levels of parentheses */
    int (*complex_ptr GTY((callback)))(
        int (*)(int, int (*[ (2) ])(char *)),
        struct { 
            int x GTY((length("( (1) + (2) )")))[3]; 
        } *
    );
    
    /* Array with macro containing parentheses */
    int data GTY((length("COMPLEX_MACRO(5)")))[];
};

#endif /* TEST_PARSE_H */
