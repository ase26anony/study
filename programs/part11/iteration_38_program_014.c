/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void *ptr_t;

/* Macros to add nesting depth for delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define BRACKET_EXPR [ (10) + (20) ]
#define BRACE_INITIALIZER { .x = 1, .y = 2 }
#define COMPLEX_MACRO(x) ((x) * (2 + (3)))

/* Forward declarations */
struct forward_decl GTY((user));

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((length("(sizeof(int) * (2 + 3))"))) {
    int data[];
};

/* More complex parentheses nesting */
struct s2 GTY((chain_next("next"), chain_prev("prev"), 
               user("my_user_function((void*)ptr)"))) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Case 2: Brackets - array types with complex dimension expressions */
typedef int arr_type GTY((length("N")))[ (2 * 3) + 4 ];

/* Pointer to array with nested brackets */
typedef int (*parr_type GTY((user)))[ (sizeof(int) > 4) ? 10 : 20 ];

/* Multi-dimensional array with brackets */
struct s3 GTY((user)) {
    int matrix GTY((length("rows * cols")))[ (5) ][ (10) ];
    int rows;
    int cols;
};

/* Case 3: Braces - structure with nested struct definition inside GTY */
struct s4 GTY((tag("NESTED"), 
               user("{ .type = 1, .sub = { .x = 0 } }"))) {
    struct inner {
        int x;
        int y;
    } GTY((tag("INNER"))) nested;
    int value;
};

/* Union with brace-enclosed initializer in comment/string */
union u1 GTY((desc("(%s ? 1 : 0)"), 
              param("(int)({ int temp = 5; temp; })"))) {
    int i;
    char arr[ 10 + (5) ];
    long l;
};

/* Function pointer type with complex parentheses */
typedef void (*complex_callback GTY((callback)))(
    int (*)(int [ (sizeof(int) == 4) ? 10 : 20 ]),
    void * GTY((skip))
);

/* GTY with all delimiters combined */
struct s5 GTY((chain_next("next"),
               length("(count > 0) ? count : (DEFAULT_SIZE)"),
               user("{{ .tag = 0 }, { .tag = 1 }}"),
               param("array[(index % 2)]"))) {
    struct s5 *next;
    int count;
    int data[];
};

/* Macro expansion with delimiters */
struct s6 GTY((length("NESTED_PAREN_EXPR"))) {
    int items[];
};

/* Nested structure with array of function pointers */
struct s7 GTY((user)) {
    int (*callbacks[ (3) + (2) ] GTY((callback)))(int, int);
    struct {
        int x GTY((skip));
        int y;
    } point;
};

/* Template-like macro usage */
#define DECLARE_GTY_STRUCT(name, size) \
    struct name GTY((length(#size))) { \
        int data[size]; \
        int count; \
    }

/* Instantiate the macro */
DECLARE_GTY_STRUCT(s8, (5 * 2));

/* Another complex case: conditional in array size */
struct s9 {
    int flags;
    char buffer GTY((length("(flags & 1) ? 100 : 200")))[ 
        (sizeof(char) * 100) 
    ];
};

/* Typedef with function type containing nested parentheses */
typedef int (func_type GTY((callback)))(
    int a[ (10) ], 
    void (*callback)(int, int)
);

/* Empty struct with just GTY markers */
struct empty GTY((user("{}"))) {
    /* intentionally empty */
};

/* Final complex test case hitting all three delimiters */
struct final_test GTY((
    chain_next("nxt"),
    chain_prev("prv"),
    length("(int)({ int x = 5; x * 2; })"),
    user("{ .type = 1, .data = {0} }"),
    param("ptr[(index + 1)]")
)) {
    struct final_test *nxt;
    struct final_test *prv;
    void *ptr GTY((length("size"), skip));
    int size;
    char tag;
};

#endif /* TEST_PARSE_H */
