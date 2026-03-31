/* test_parse.h - Complex GTY annotations to exercise balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef unsigned int size_t;

/* Macros to introduce nested delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define COMPLEX_BRACKET_EXPR [ (10) + (20) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define CALLBACK_MACRO GTY((callback))

/* Case 1: Parentheses - struct with complex length expression */
struct s1 GTY((length("(sizeof(int) * (2 + 3))"))) {
    int data[];
};

/* More parentheses with multiple GTY options */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Nested parentheses via macro */
struct s3 GTY((length("NESTED_PAREN_EXPR"))) {
    char buffer[];
};

/* Case 2: Brackets - array types with complex dimensions */
struct s4 GTY(()) {
    int arr1[ (2 * 3) ];
    int arr2[ sizeof(int) * 2 ];
    int (*ptr_arr)[ (4 + 1) ];
};

/* Multi-dimensional array with nested brackets */
struct s5 GTY((user)) {
    int matrix[ (2) ][ (3 + 2) ];
    int (*dynamic_matrix)[ (5) ];
};

/* Array in typedef with GTY */
typedef int array_type GTY((length("10")))[ (2 + 3) * 2 ];

/* Case 3: Braces - structures with nested definitions */
struct s6 GTY((tag("LANG"))) {
    struct inner {
        int x;
        int y;
    } nested GTY((skip));
    
    union {
        int i;
        char c;
    } u GTY((desc("1")));
};

/* Union with array and GTY */
union u1 GTY((desc("$1"))) {
    int i;
    char arr[ 10 + (5) ];
    struct {
        int a;
        int b;
    } s;
};

/* Function pointer with complex signature (triggers parentheses) */
typedef void (*complex_func_ptr GTY((callback)))(
    int (*)(int [ (4) ]),
    void (*)(struct s1 *)
);

/* Forward declaration with GTY containing parentheses */
struct s7 GTY((user)) {
    struct s7 *next;
};

/* Structure with conditional in GTY option */
struct s8 GTY((length("(flag ? 10 : (20 + 5))"))) {
    int items[];
};

/* Nested structure definition inside GTY context */
struct outer GTY(()) {
    struct inner GTY((skip)) {
        int x;
        int y[ (2) ];
    } inner_struct;
    
    /* Array with size from macro containing parentheses */
    int data[ NESTED_PAREN_EXPR ];
};

/* Template-like macro usage (triggers parentheses) */
#define DEFINE_GTY_STRUCT(name, size) \
    struct name GTY((length(#size))) { \
        int data[size]; \
    }

/* Actually define a struct using the macro */
DEFINE_GTY_STRUCT(s9, (5 + 3));

/* Complex function pointer array */
struct s10 GTY(()) {
    void (*callbacks[ (3) ] GTY((callback)))(
        int,
        struct s1 * GTY((skip))
    );
};

/* Union with nested initializer-like syntax in comment (triggers brace scanning) */
union u2 GTY((desc("$1 == 0 ? 'A' : 'B'"))) {
    int type;
    char value;
    /* The following comment contains braces that might be scanned:
       Example: { .type = 1, .value = 'X' }
    */
};

/* Structure with attribute containing balanced delimiters */
struct s11 GTY((user, chain_next("((struct s11*)next)"))) {
    struct s11 *next;
    int data;
};

/* Final test: all delimiters in one type */
struct all_delimiters GTY((length("(sizeof(int) * [default: 10])"))) {
    int array[ (2) ][ (3) ];
    struct {
        int x;
        int y;
    } point;
    void (*func)(int [ (5) ]);
};

#endif /* TEST_PARSE_H */
