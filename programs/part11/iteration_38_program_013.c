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
#define NESTED_MACRO(x) ((x) * ((x) + 1))

/* Case 1: Parentheses - struct with GTY((length("(complex expression)"))) */
struct s1 GTY((user)) {
    int data GTY((length("(sizeof(int) * (2 + 3))")))[];
    struct s1 *next GTY((skip));
};

/* More complex parentheses nesting */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    int value;
    struct s2 *next;
    struct s2 *prev;
};

/* Function pointer with nested parentheses in argument */
typedef void (*complex_fn_ptr GTY((callback)))(
    int (*)(int [ (sizeof(int) * 2) ], 
            void (*)(int, (int))  /* nested parentheses in cast */
           )
);

/* Case 2: Brackets - Array types with complex dimension expressions */
struct array_struct GTY(()) {
    int arr1 GTY((length("N")))[ (2 * 3) + (4 / 2) ];
    char arr2 GTY((length("M")))[ NESTED_PAREN_EXPR ];
    int (*ptr_to_array GTY((skip)))[ (1 << 3) + (2 * 2) ];
};

/* Multi-dimensional array with nested brackets */
typedef int multi_array_t GTY((user))[ (2 + 3) ][ (4 * 2) ][ NESTED_MACRO(3) ];

/* Pointer to array of function pointers */
typedef int (*(*complex_array_ptr GTY((user)))
    [ (sizeof(void*) * 2) ])(int, int);

/* Case 3: Braces - Structure with nested struct definition */
struct outer_struct GTY((user)) {
    int id;
    /* Nested struct definition with braces */
    struct inner_struct GTY((tag("LANG"))) {
        int x;
        int y;
        union inner_union {
            int a;
            char b;
        } u;
    } nested;
    
    /* Union with initializer-like syntax in GTY */
    union data_union GTY((desc("(%s == 0) ? 0 : 1"))) {
        int int_val;
        char *str_val GTY((length("(strlen(%h.str_val) + 1)")));
    } data;
};

/* Another brace case: GTY annotation with nested structure */
struct s3 GTY((user, 
    nested({
        .tag = "SPECIAL",
        .param = (void*)(&global_var)  /* nested parentheses */
    })
)) {
    int value;
    struct {
        int nested_field;
    } inner;
};

/* Forward declaration with GTY containing parentheses */
struct forward_decl GTY((user, if("(FLAGS & 1)"))) ;

/* Complete definition with complex expressions */
struct forward_decl {
    int flags;
    char *name GTY((length("(strlen(%h.name) + (sizeof(char)))")));
    struct forward_decl *children GTY((length("(%h.flags & 2) ? 10 : 20")))[];
};

/* Union with array containing bracket expressions */
union u1 GTY((user)) {
    int i;
    char arr[ 10 + (5 * (2 + 1)) ];
    long matrix[ (2 + 2) ][ (3 * 3) ];
};

/* Template-like macro usage with all delimiters */
#define DEFINE_GTY_STRUCT(name, size_expr) \
    struct name##_t GTY((user, length(#size_expr))) { \
        int data[(size_expr)]; \
        struct { \
            int nested; \
        } inner; \
    }

/* Instantiate the macro with complex expression */
DEFINE_GTY_STRUCT(my_struct, (1 << 3) + (sizeof(int) * 2));

/* Variable declaration with GTY and array */
extern int global_array GTY((length("(MAX_SIZE)")))[ (100 * (2 + 3)) ];

/* Enum with GTY - though enums don't typically need GC, test parentheses */
enum my_enum GTY((user)) {
    VALUE1 = (1 << 0),
    VALUE2 = (1 << 1) + (2 * 3),
    VALUE3 = (sizeof(int) * (4 / 2))
};

/* Typedef with function type containing nested parentheses */
typedef int (complex_func_t GTY((callback)))(
    int a[ (sizeof(int) * 2) ], 
    void (*callback)(int, (int)),  /* nested parentheses */
    struct { int x; int y; } point  /* anonymous struct with braces */
);

/* One more test: GTY with skip and param containing all delimiters */
struct final_test GTY((
    skip, 
    param((void*)(&global_array[ (2 * 3) ])),
    user
)) {
    int value;
    char *str GTY((length("(strlen(%h.str) ?: 1)")));
};

#endif /* TEST_PARSE_H */
