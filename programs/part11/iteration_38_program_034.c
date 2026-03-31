/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef unsigned int size_t;

/* Macros to introduce nested delimiter layers */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define COMPLEX_BRACKET_EXPR [ (2 * 3) + (4 / 2) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define ANOTHER_EXPR(a, b) ((a) + (b) * (2))

/* Forward declarations */
struct forward_decl GTY((user));

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((length("(1 << 5) + (sizeof(int) * (2 + 3))"))) {
    int data[];
};

/* More parentheses in chain options */
struct list_node GTY((chain_next("next"), chain_prev("prev"))) {
    int value;
    struct list_node *next;
    struct list_node *prev;
};

/* Case 2: Brackets - array types with complex dimension expressions */
struct s2 GTY((user)) {
    int arr1 GTY((length("N")))[ (2 * 3) + 1 ];
    char arr2 GTY((length("M")))[ sizeof(int) * 2 ];
    double *arr3 GTY((length("P")))[ (4) ][ (5) ];
};

/* Pointer to array with brackets in GTY */
typedef int (*array_ptr GTY((user)))[ (3 + 2) ];

/* Case 3: Braces - structure with nested struct definition inside GTY */
struct outer GTY((user)) {
    struct inner GTY((tag("LANG"))) {
        int x;
        int y[2];
    } nested;
    
    union inner_union GTY((desc("$1.type"))) {
        int i;
        float f;
        struct { char a; char b; } chars;
    } u;
};

/* Function pointer with complex argument list containing brackets */
typedef void (*complex_callback GTY((callback)))(
    int (*)(int arr[ (sizeof(int) > 4) ? 8 : 4 ]),
    struct s2 * GTY((skip))[ (2) ]
);

/* Union with array containing parenthesized size expression */
union u1 GTY((user)) {
    int i;
    char arr[ 10 + (5 * 2) ];
    long long big[ (sizeof(long) == 8) ? 2 : 4 ];
};

/* GTY with skip containing nested parentheses */
struct skipped GTY((skip("(&((type *)0)->field)"))) {
    void *field;
    int other;
};

/* Nested GTY annotations with multiple delimiter types */
struct deeply_nested GTY((user)) {
    /* Array of pointers with GTY */
    struct s1 * GTY((length("(depth + 1)"))) levels[];
    
    /* Function pointer member */
    complex_callback cb;
    
    /* Anonymous union with brace */
    union {
        int option1;
        struct { int a; int b; } option2;
    } choice;
};

/* Template-like macro usage with GTY */
#define DECLARE_VECTOR(type, name) \
    struct name ## _vector GTY((user)) { \
        type * GTY((length("capacity"))) data; \
        size_t capacity; \
        size_t size; \
    }

DECLARE_VECTOR(int, int);
DECLARE_VECTOR(struct s1, s1);

/* GTY with condition containing parentheses */
struct conditional GTY((if("FLAG_ENABLED"))) {
    int enabled_field;
    char data[ (FLAG_ENABLED) ? 100 : 50 ];
};

/* Multiple GTY options with various delimiters */
struct multi_option GTY(
    (chain_next("nxt")),
    (chain_prev("prv")),
    (length("cnt")),
    (skip("(&((struct multi_option *)0)->skip_me)"))
) {
    int value;
    struct multi_option *nxt;
    struct multi_option *prv;
    struct multi_option *skip_me;
    int cnt;
};

/* Array with computed size using macro containing parentheses */
struct computed_array GTY((user)) {
    int values[ ANOTHER_EXPR(5, 3) ];
    char buffer[ (ANOTHER_EXPR(2, 4)) + 1 ];
};

/* Forward-declared struct with GTY annotation containing expression */
struct forward_decl GTY((user, if("defined(USE_FORWARD)"))) {
    int id;
    struct forward_decl *next;
};

/* Typedef with function type containing brackets in parameters */
typedef int (array_func GTY((callback)))(int matrix[3][4], int vector[(1 + 2)]);

/* Structure with flexible array member and nested parentheses in length */
struct flex_array GTY((length("(offsetof(struct flex_array, data) + (count * sizeof(int)))"))) {
    int count;
    int data[];
};

/* Union with GTY and nested braces in initializer (for static data) */
union with_init GTY((user)) {
    struct {
        int x;
        int y;
    } point;
    int coordinates[2];
} global_init = BRACE_INITIALIZER;

#endif /* TEST_PARSE_H */
