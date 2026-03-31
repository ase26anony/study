/* test_parse.h - Complex GTY annotations to test delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef unsigned int size_t;

/* Macros to add nesting depth */
#define NESTED_EXPR ( (10) + (20) )
#define COMPLEX_SIZE (sizeof(int) * (2 + 3))
#define ARRAY_DIM ( (5) * ( (2) + (1) ) )
#define BRACE_BLOCK { int x = 0; }
#define CALLBACK_FUNC ((void (*)(void))0)

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int count;
    /* Flexible array member with complex length expression */
    int data GTY((length("(sizeof(int) * (2 + 3))")))[];
};

/* Additional parentheses case with multiple nested groups */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    struct s2 *next;
    struct s2 *prev;
    int value GTY((skip))[ (1 << 3) + (2 * 4) ];
};

/* Case 2: Brackets - array types with complex dimension expressions */
typedef struct {
    int arr1 GTY((length("NESTED_EXPR")))[ (2 * 3) ];
    char arr2 GTY((length("COMPLEX_SIZE")))[ sizeof(int) * (2 + 3) ];
} array_struct GTY(());

/* Pointer to array with nested brackets */
typedef int (*array_ptr GTY((callback)))[ ( (5) + (3) ) ][ (2) ];

/* Multi-dimensional array with GTY */
struct md_array GTY(()) {
    int matrix GTY((length("ARRAY_DIM")))[ ( (3) * (2) ) ][ (4) + (1) ];
};

/* Case 3: Braces - structure with nested struct definition inside GTY */
struct outer GTY((user)) {
    struct inner GTY((tag("LANG"))) {
        int x;
        int y GTY((length("(sizeof(int) + 2)")))[];
    } nested;
    
    union {
        int i;
        float f;
    } value GTY(());
};

/* Union containing array with brace initialization in type context */
union u1 GTY((desc("1"))) {
    int i;
    char arr[ 10 + (5) ];
    struct {
        int a;
        int b;
    } s;
};

/* Function pointer typedef with complex argument list containing brackets */
typedef void (*complex_fptr GTY((callback)))(
    int (*)(int [ (4) + (2) ], char *),
    struct s1 *,
    int (*callback_array[])(void)
);

/* GTY with deeply nested parentheses in options */
struct deeply_nested GTY(( 
    maybe_undef,
    chain_next("next"),
    reorder("sort_func")
)) {
    struct deeply_nested *next;
    int values GTY((length("((((5)) + ((3)) * ((2))))")))[];
};

/* Template-like macro usage with GTY */
#define DEFINE_GTY_STRUCT(name, size) \
    struct name GTY((length(#size))) { \
        int data[size]; \
    }

/* Use the macro with complex expression */
DEFINE_GTY_STRUCT(macro_struct, ( (3) * ( (2) + (1) ) ));

/* Another case: GTY on typedef with function type containing nested delimiters */
typedef int (*(*complex_func_ptr GTY((callback)))(int, char *))(
    int (*)(int [][ (2) + (3) ]),
    void (*)(struct { int x; } *)
);

/* Structure with attribute containing brace-like syntax */
struct with_attribute GTY((user)) {
    int field1;
    /* Simulating something that might look like braces to the parser */
    int (*func)() GTY((callback, reorder("sort_func { special }")));
};

/* Enum with GTY marker (though enums don't typically need GC) */
enum my_enum GTY((tag("ENUM"))) {
    VALUE1 = ( (1) << (0) ),
    VALUE2 = ( (1) << (1) ),
    VALUE3 = ( (1) << (2) )
};

/* Forward declaration with GTY annotation containing parentheses */
struct forward_decl GTY((user, reorder("((compare_func))")));

/* Complete definition */
struct forward_decl {
    int data;
    struct forward_decl *next;
};

/* Array of pointers with GTY and complex size calculation */
typedef struct {
    void *items GTY((length("(sizeof(void *) * ( (16) + (8) ))")))[];
} ptr_array GTY(());

/* Final test: Mixed delimiters in a single GTY annotation */
struct mixed_delimiters GTY((
    user,
    desc("(1)"),
    length("[ (sizeof(int)) * {N} ]"),  /* Note: This string contains all three */
    chain_next("next->link")
)) {
    struct mixed_delimiters *next;
    int data GTY((skip));
    union {
        int i;
        struct {
            char c;
            int pad;
        } s;
    } u;
};

#endif /* TEST_PARSE_H */
