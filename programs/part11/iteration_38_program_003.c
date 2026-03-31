/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to introduce nested delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define BRACKET_EXPR [ (2 * 3) + (4 / 2) ]
#define COMPLEX_BRACKETS [10][ (5) ]
#define BRACE_INITIALIZER { .x = (1 + 2), .y = {3, 4} }

/* Forward declarations */
struct forward_decl GTY((user));

/* ========== TEST CASE 1: Parentheses in GTY options ========== */
struct s1 GTY((length("(sizeof(int) * (2 + 3))"))) {
    int data[];
};

/* Multiple parenthesized groups in GTY options */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Nested parentheses in skip expression */
struct s3 GTY((skip(("skip_func")))) {
    int a;
    double b;
};

/* ========== TEST CASE 2: Brackets in type declarations ========== */
/* Array with complex dimension expression */
struct s4 GTY((length("N"))) {
    int arr[ (2 * 3) + sizeof(int) ];
    int N;
};

/* Pointer to array with nested brackets */
typedef int (*array_ptr GTY((tag("ARRAY_PTR"))))[ (5) ][ (3 + 2) ];

/* Multi-dimensional array with macro expansion */
struct s5 GTY(()) {
    int matrix BRACKET_EXPR;
    char buffer[10][ (sizeof(double)) ];
};

/* ========== TEST CASE 3: Braces in type definitions ========== */
/* Structure with nested structure definition (contains braces) */
struct s6 GTY((user)) {
    struct inner GTY((tag("INNER"))) {
        int x;
        int y;
    } nested;
    int outer;
};

/* Union with array and initializer-like syntax in GTY */
union u1 GTY((desc("(%s.kind == 0) ? 0 : 1"))) {
    int i;
    char arr[ 10 + (5) ];
    struct {
        int kind;
        void *data;
    } s;
};

/* ========== TEST CASE 4: Function pointers with complex signatures ========== */
/* Function pointer with nested parentheses in parameter list */
typedef void (*complex_callback GTY((callback)))(
    int (*)(int [ (4) ][ (2) ]),
    struct s1 *,
    union u1 *
);

/* GTY on function pointer type with nested argument types */
struct s7 GTY(()) {
    int (*processor GTY((skip)))(
        char *buffer[ (256) ],
        int (*helper)(int, int)
    );
    int count;
};

/* ========== TEST CASE 5: Mixed delimiters ========== */
/* Structure with all delimiter types in GTY options */
struct s8 GTY((length("(sizeof(int) * [10])"),
               skip(("skip_me")),
               tag("MIXED"))) {
    /* Array declaration with brackets containing parenthesized expression */
    int mixed_array[ (2 + 3) * sizeof(int) ];
    
    /* Nested structure (braces) */
    struct {
        int x[ (5) ];  /* brackets with parentheses */
        int y;
    } nested;
    
    /* Function pointer with complex signature */
    void (*action)(int (*)(int[ (10) ]));
};

/* ========== TEST CASE 6: Conditional expressions in GTY ========== */
struct forward_decl GTY((user)) {
    int id;
    struct forward_decl *next;
};

struct s9 GTY((if("CONDITION"), length("(CONDITION ? 10 : 20)"))) {
    int data[ (sizeof(struct forward_decl)) ];
    char *name;
};

/* ========== TEST CASE 7: Template-like macro expansions ========== */
#define DECLARE_GTY_ARRAY(T, N) T arr GTY((length(#N)))[ (N) ]

struct s10 GTY(()) {
    DECLARE_GTY_ARRAY(int, (5 + 3));
    DECLARE_GTY_ARRAY(char, (10 * 2));
};

/* ========== TEST CASE 8: Nested type definitions with GTY ========== */
struct outer GTY((chain_next("next"))) {
    struct outer *next;
    
    /* Anonymous union with GTY marker */
    union GTY((desc("type"))) {
        int type;
        struct {
            int x;
            int y[ (8) ];
        } point;
        struct {
            char *name;
            int length;
        } string;
    } data;
};

/* ========== TEST CASE 9: GTY with string containing delimiters ========== */
struct s11 GTY((string_length("(strlen(data) + 1)"))) {
    char *data;
    int flags;
};

/* ========== TEST CASE 10: Complex array of structures ========== */
struct element GTY((user)) {
    int id;
    char name[ (32) ];
};

struct container GTY((length("count"))) {
    struct element *elements;
    int count;
    
    /* Array of function pointers */
    void (*callbacks[ (10) ] GTY((skip)))(struct element *);
};

#endif /* TEST_PARSE_H */
