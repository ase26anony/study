/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to add nesting depth */
#define NESTED_EXPR ( (10) + (20) )
#define ARRAY_SIZE_EXPR [ (2 * 3) + (4 / 2) ]
#define BRACE_INITIALIZER { .x = (1 + 2), .y = {3, 4} }
#define COMPLEX_PAREN ((a) * ((b) + (c)))

/* Test case 1: Parentheses in GTY length option */
struct s1 GTY((length("(sizeof(int) * (2 + 3))"))) {
    int data[];
};

/* Test case 2: Multiple parenthesized groups in GTY options */
struct s2 GTY((chain_next("next"), chain_prev("prev"), user)) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Test case 3: Nested parentheses using macro */
struct s3 GTY((length("NESTED_EXPR"))) {
    my_int items[];
};

/* Test case 4: Brackets in array dimension with GTY */
struct s4 GTY((user)) {
    int arr GTY((length("N")))[ (2 * 3) ];
    int N;
};

/* Test case 5: Complex array declarator with nested brackets */
typedef int (*complex_array_ptr GTY((user)))[ (sizeof(int) > 4) ? 8 : 16 ];

/* Test case 6: Pointer to array with bracketed size expression */
struct s5 GTY((user)) {
    int (*matrix GTY((length("rows * cols"))))[ (10 + 5) ];
    int rows;
    int cols;
};

/* Test case 7: Function pointer with GTY callback and complex argument list */
typedef void (*fnptr GTY((callback)))(
    int (*)(int [ (4) ]), 
    void * GTY((skip))
);

/* Test case 8: Union with array containing parenthesized size */
union u1 GTY((user)) {
    int i;
    char arr[ 10 + (5) ];
    long * GTY((skip)) ptr;
};

/* Test case 9: Structure with nested structure definition (braces) */
struct s6 GTY((user)) {
    struct inner GTY((tag("LANG"))) {
        int x;
        int y GTY((length("(x > 0) ? x : 1")))[];
    } nested;
    int count;
};

/* Test case 10: GTY with skip option containing conditional expression */
struct s7 GTY((skip("((type) == TYPE_A) ? &skip_a : &skip_b"))) {
    int type;
    void *data;
};

/* Test case 11: Multiple nested delimiters in single GTY annotation */
struct s8 GTY((
    length("(sizeof(struct header) + (count * sizeof(int)))"),
    tag("NESTED"),
    user
)) {
    struct header {
        int magic;
        int count;
    } hdr;
    int items[];
};

/* Test case 12: Array of pointers with GTY and complex size calculation */
typedef struct node *node_array GTY((length("(capacity + 1)")))[
    (16 * sizeof(void*)) / sizeof(struct node*)
];

/* Test case 13: Forward declaration with GTY containing parenthesized expr */
struct s9 GTY((user, chain_next("((struct s9*)next)->link"))) {
    struct s9 *link;
    int value;
};

/* Test case 14: GTY with desc option containing nested parentheses */
enum my_enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
};

struct s10 GTY((desc("%1.type"), user)) {
    enum my_enum type;
    union {
        int ival;
        char * GTY((tag("VALUE_C"))) str;
    } data;
};

/* Test case 15: Macro expansion with all delimiter types */
#define COMPLEX_GTY_ANNOTATION GTY((length("(1 << 5)"), user))

struct s11 COMPLEX_GTY_ANNOTATION {
    int buffer[ (1 << 5) ];
};

/* Test case 16: Conditional expression in array dimension */
struct s12 GTY((user)) {
    int flags GTY((length("(mode == 0) ? 8 : 16")))[
        (sizeof(int) * 8)
    ];
    int mode;
};

/* Test case 17: Pointer to function returning pointer to array */
typedef int (*(*complex_func_ptr GTY((callback)))(
    int, 
    void * GTY((skip))
))[ (10) ];

/* Test case 18: Nested structure with array of structures */
struct outer GTY((user)) {
    struct inner {
        int id;
        char name[ (32 + 8) ];
    } GTY((length("count"))) items[];
    int count;
};

/* Test case 19: GTY with param_is option containing cast expression */
typedef struct base {
    int type;
} base_t;

struct derived GTY((user, param_is("(base_t*)&%h"))) {
    base_t base;
    int extra;
};

/* Test case 20: Union with GTY and nested initializer-like syntax in comment */
union u2 GTY((user)) {
    int x;
    struct {
        int a;
        int b;
    } s;
    /* This looks like an initializer but is in comment: { .a = 1, .b = 2 } */
};

#endif /* TEST_PARSE_H */
