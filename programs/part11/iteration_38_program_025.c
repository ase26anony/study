/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to add nesting depth */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define COMPLEX_BRACKET_EXPR [ (2 * 3) + (4 / 2) ]
#define BRACE_INITIALIZER { .x = 1, .y = (2 + 3) }
#define NESTED_MACRO(x) ( (x) + ( (x) * 2 ) )

/* Case 1: Parentheses - struct with GTY((length("(complex expression)"))) */
struct s1 GTY((length("(1 << 5) + sizeof(int)"))) {
    int data[];
};

/* More complex parentheses nesting */
struct s2 GTY((chain_next("next"), chain_prev("prev"), 
               user, desc("test"), length("NESTED_PAREN_EXPR"))) {
    struct s2 *next;
    struct s2 *prev;
    int count;
    char buffer[ (sizeof(int) * 2) ];
};

/* Case 2: Brackets - array types with complex dimension expressions */
typedef int array_type GTY((length("10")))[ (2 * 3) + 1 ];

/* Multi-dimensional array with nested brackets */
struct s3 GTY((user)) {
    int matrix GTY((length("(N * M)")))[ (2 + 3) ][ (4 * 2) ];
    int *ptr_array GTY((length("sizeof(int) * 8")))[];
};

/* Pointer to array with GTY annotation */
typedef int (*array_ptr GTY((callback)))[ (sizeof(int) > 4) ? 8 : 16 ];

/* Case 3: Braces - structure with nested struct definition inside GTY */
struct outer GTY((user)) {
    struct inner GTY((tag("LANG"))) {
        int x;
        int y;
    } nested;
    int value;
};

/* Union with brace-enclosed initializer in macro expansion */
#define DEFAULT_VALUES { .i = 0, .f = 0.0, .arr = {1, 2, 3} }

union u1 GTY((desc("union_test"))) {
    int i;
    float f;
    int arr[3];
};

/* Function pointer type with complex argument list containing parentheses */
typedef void (*complex_callback GTY((callback)))(
    int (*)(int [ (sizeof(int) * 2) ], 
            char *(*)(void)),
    struct s1 *
);

/* Structure with attribute containing nested braces */
struct s4 GTY((user, desc("nested_braces"))) {
    struct {
        int a;
        int b;
    } GTY((tag("INNER"))) inner;
    
    union {
        int x;
        char y;
    } GTY((tag("UNION"))) u;
};

/* GTY with skip directive containing parentheses */
struct skip_test GTY((skip(" (skip_condition) "))) {
    int should_skip;
    int value;
};

/* Array with GTY and nested bracket expressions */
int global_array GTY((length("(1 + 2) * 3")))[ (1 << 2) + (1 << 3) ];

/* Nested structure with multiple GTY annotations */
struct container GTY((chain_next("next_container"))) {
    struct contained GTY((user)) {
        int id;
        char *name GTY((length("strlen(name) + 1")));
    } item;
    struct container *next_container;
};

/* Function pointer with deeply nested parentheses */
typedef int (*(*nested_func_ptr GTY((callback)))(int))(int, int);

/* Structure with conditional in array dimension */
struct conditional_array GTY((user)) {
    int data GTY((length("condition ? 10 : 20")))[ 
        (sizeof(void*) == 8) ? 64 : 32 
    ];
};

/* Union with anonymous struct containing braces */
union anonymous_union GTY((desc("test"))) {
    struct {
        int x;
        int y;
    };
    struct {
        char a;
        char b;
    } chars;
};

/* Macro that expands to brace-enclosed list within GTY context */
#define GTY_SPECIAL ((tag("SPECIAL")), (user), (desc("macro_test")))

struct macro_test GTY(GTY_SPECIAL) {
    int value;
};

/* Test all three delimiters in one annotation */
struct all_delimiters GTY((length("( (2) + [test] )"), 
                          user,
                          desc("{test}"))) {
    int complex_field;
    char *string GTY((length("strlen(string) + (1)")));
};

/* Forward declaration with GTY annotation containing parentheses */
struct forward_decl GTY((user, desc("(forward declared struct)")));

/* Now define it */
struct forward_decl {
    int value;
    struct forward_decl *next GTY((chain_next("next")));
};

/* Typedef with function type containing nested parentheses */
typedef int (*(*complex_type GTY((callback)))(int (*)(int)))(int, int, int);

/* Structure with bitfield and GTY */
struct bitfield_test GTY((user)) {
    unsigned int flag:1;
    unsigned int value: (sizeof(int) * 8 - 1);
    int array GTY((length("(flag ? 10 : 20)")))[];
};

/* Final test: multiple nested delimiters */
struct ultimate_test GTY((length("( { [ (1) ] } )"),  /* Invalid but tests parsing */
                         skip(" (x) && (y) "),
                         desc("{{test}}"),
                         user)) {
    int (*func_ptr GTY((callback)))(int (*)(int [ (2) ]));
    struct inner_most {
        int x;
    } GTY((tag("INNERMOST"))) inner;
};

#endif /* TEST_PARSE_H */
