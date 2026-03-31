/* test_parse.h - Complex GTY annotations to exercise balanced delimiter parsing */

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void *ptr_t;

/* Macros to add nesting depth */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define BRACKET_EXPR [ (2 * 3) + (4 - 1) ]
#define BRACE_BLOCK { int x = (1 + 2); }
#define COMPLEX_MACRO(x) ((x) * (2 + (3)))

/* Forward declarations */
struct forward_decl GTY((user));

/* 1. Parentheses case: struct with GTY((length("(complex expression)"))) */
struct s1 GTY((length("(sizeof(int) * (2 + 3))"))) {
    int data[];
};

/* More parentheses nesting */
struct s2 GTY((chain_next("next"), chain_prev("prev"), user)) {
    struct s2 *next;
    struct s2 *prev;
    int value GTY((skip));
};

/* Even more complex parentheses */
struct s3 GTY((length("NESTED_PAREN_EXPR"))) {
    char buffer[];
};

/* 2. Brackets case: array types with complex dimension expressions */
struct s4 GTY(()) {
    int arr1 GTY((length("(2 * 3)"))) [ (2 * 3) ];
    int arr2 GTY((length("N"))) BRACKET_EXPR;
};

/* Pointer to array with brackets */
typedef int (*array_ptr_t GTY((callback))) [ (4 + 1) ];

/* Multi-dimensional array with nested brackets */
struct s5 GTY(()) {
    int matrix GTY((length("ROWS * COLS"))) [ (2 + 3) ][ (4 * 2) ];
};

/* 3. Braces case: nested structure definitions within GTY */
struct s6 GTY((user)) {
    struct inner GTY((tag("LANG"))) {
        int x;
        int y;
    } nested;
    
    union inner_union GTY((desc("$1 & 1"))) {
        int i;
        char c BRACKET_EXPR;
    } u;
};

/* Union with brace-enclosed initializer-like syntax in comment */
union u1 GTY((desc("(int)$1"))) {
    int i;
    char arr[ 10 + (5) ];
    struct {
        float f;
        double d;
    } nested;
};

/* 4. Function pointer with complex argument list containing all delimiters */
typedef void (*complex_callback GTY((callback)))(
    int (*)(int [ (4) ]), 
    struct s1 *,
    union u1 *
);

/* 5. GTY with skip containing nested expressions */
struct s7 GTY((skip(("(&" "var)")))) {
    int var;
    char *name GTY((length("strlen(name) + (1)")));
};

/* 6. Conditional expression in GTY argument */
struct s8 GTY((user, if("(FLAGS & 1) || (FLAGS & 2)"))) {
    int flags;
    void *data GTY((length("(flags & 1) ? 10 : 20")));
};

/* 7. Nested structure with array of structures */
struct outer GTY((user)) {
    struct inner_arr GTY((length("COUNT"))) {
        int id;
        char name[ (16 + 4) ];
    } items[];
    
    int count;
};

/* 8. Template-like macro expansion with delimiters */
#define DEFINE_GTY_STRUCT(name, size) \
    struct name##_t GTY((length(#size))) { \
        my_##name data[(size)]; \
    }

typedef int my_data;
DEFINE_GTY_STRUCT(data, (2 * 8));

/* 9. Complex chain_next with nested parentheses */
struct node GTY((chain_next("((struct node*)next)"), chain_prev("prev"))) {
    struct node *next;
    struct node *prev;
    int value;
    
    /* Array within node */
    int scores GTY((length("value * (2)"))) [];
};

/* 10. Union with desc containing switch expression */
union desc_union GTY((desc("($1.type == 1) ? (0) : (1)"))) {
    struct {
        int type;
        int data;
    } s;
    
    struct {
        int type;
        float data[ (2) + (2) ];
    } f;
};

/* 11. Structure containing all delimiter types in one */
struct all_delimiters GTY((user, 
    if("(defined(FLAG1) && (FLAG2 || FLAG3))"),
    skip("(&((struct all_delimiters*)0)->array[0])")
)) {
    /* Parentheses in array size */
    int array[ (sizeof(int) * (8 / 2)) ];
    
    /* Nested structure with braces */
    struct {
        int x;
        int y[ (2) + (2) ];
    } point;
    
    /* Union with initializer-like comment */
    union {
        int i;
        char c[ (4) ];
    } value;
};

/* 12. Typedef with function pointer containing nested parentheses */
typedef int (*nested_func_ptr GTY((callback)))(
    int (*)(char *[ (2) ], void *), 
    struct all_delimiters *
);

/* 13. Variable length array in structure */
struct vla_struct GTY((length("len * (sizeof(int))"))) {
    int len;
    int data[];
};

/* 14. GTY with param_is option containing parentheses */
struct param_struct GTY((param_is("PARAM"))) {
    int param;
    void *extra GTY((skip, length("(param > 0) ? param : 1")));
};

/* 15. Final test case: deeply nested parentheses */
struct deep_nest GTY((user,
    if("((A && B) || (C && D)) && (E || F)"),
    skip("(&deep_nest_var)"),
    length("(sizeof(int) * ((A) ? (B) : (C)))")
)) {
    int A, B, C, D, E, F;
    char buffer[];
};
