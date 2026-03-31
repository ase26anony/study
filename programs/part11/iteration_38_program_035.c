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
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define COMPLEX_MACRO(x) ((x) + ( (x) * (x) ))

/* Forward declarations */
struct forward_decl GTY((user));

/* ========== TEST CASE 1: Parentheses in GTY options ========== */
/* This will trigger consume_balanced for '(' ')' */
struct test_struct_1 GTY((user)) {
    int data GTY((length("(sizeof(int) * (2 + 3))"))) [];
    /* Multiple parenthesized groups */
    struct test_struct_1 *next GTY((chain_next("next"), chain_prev("prev")));
    struct test_struct_1 *prev;
    
    /* Nested parentheses in callback specification */
    void (*callback GTY((callback)))(
        int (*)(int, int), 
        void (*)(char *(*(*)(void))[5])
    );
};

/* ========== TEST CASE 2: Brackets in array dimensions ========== */
/* This will trigger consume_balanced for '[' ']' */
union test_union_1 GTY((user)) {
    int i;
    /* Array with complex dimension expression containing parentheses */
    char arr GTY((length("N"))) [ (2 * 3) + sizeof(int) ];
    
    /* Pointer to array with nested brackets */
    int (*ptr_to_arr GTY((skip))) [ (4) ][ (5) ];
    
    /* Multi-dimensional array with nested brackets */
    double matrix GTY((length("ROWS"), param_is("COLS"))) 
        [ (1 << 3) ]  /* 8 */
        [ (sizeof(double) > 4 ? 10 : 20) ];
};

/* ========== TEST CASE 3: Braces in nested structures ========== */
/* This will trigger consume_balanced for '{' '}' */
struct test_struct_2 GTY((user)) {
    /* Nested structure definition with braces inside GTY context */
    struct inner_struct GTY((tag("LANG"))) {
        int x;
        int y GTY((length("(2+2)"))) [];
    } nested;
    
    /* Union with initializer-like syntax in GTY option */
    union {
        int a;
        float b;
    } value GTY((desc("%0.a ? 1 : 2")));
};

/* ========== TEST CASE 4: Mixed delimiters ========== */
/* Complex type with all delimiter types */
typedef struct test_complex GTY((user)) {
    /* Function pointer with complex signature */
    int (*complex_func GTY((callback)))(
        int arg1[ (sizeof(int) == 4) ? 10 : 20 ],
        struct { int x; int y; } point,
        void (*callback)(int, char *[])
    );
    
    /* Array of pointers to functions */
    void (*func_array GTY((length("SIZE")))[ (5) ])(
        int, 
        char *(*)(char *[])
    );
    
    /* Nested structure with array */
    struct {
        int count;
        char *items GTY((length("%h.count"))) [];
    } container;
} complex_t;

/* ========== TEST CASE 5: GTY with macro expansions ========== */
/* Macros that expand to delimiter-heavy expressions */
#define ARRAY_SIZE_EXPR [ NESTED_PAREN_EXPR ]
#define CALLBACK_TYPE void (*)(int, char *[])

struct test_macro GTY((user)) {
    /* Using macro that expands to bracketed expression */
    int data GTY((length("10"))) ARRAY_SIZE_EXPR;
    
    /* Macro that expands to type with parentheses */
    CALLBACK_TYPE handler GTY((callback));
    
    /* Complex expression in array dimension */
    int matrix GTY((length("ROWS * COLS"))) 
        [ (1 << 2) + (3 * 4) ]  /* 4 + 12 = 16 */
        [ sizeof(struct { char a; int b; }) ];
};

/* ========== TEST CASE 6: Template-like patterns ========== */
/* GCC extensions that might appear in headers */
typedef struct test_template GTY((user)) {
    /* __attribute__ with nested parentheses */
    int field GTY((skip)) __attribute__((
        aligned(
            (sizeof(long) == 8) ? 8 : 4
        )
    ));
    
    /* Array with computed size */
    unsigned char buffer GTY((length(
        "((sizeof(int) + 7) & ~7) * 1024"  /* Complex paren expression */
    ))) [];
} template_t;

/* ========== TEST CASE 7: Forward declaration with GTY ========== */
struct forward_decl GTY((user)) {
    /* Self-referential pointer */
    struct forward_decl *self;
    
    /* Conditional expression in array dimension */
    int conditional_array GTY((length(
        "(FLAG ? (SIZE_A) : (SIZE_B))"  /* Nested parentheses */
    ))) [];
};

/* ========== TEST CASE 8: Union with anonymous struct ========== */
union test_anonymous GTY((user)) {
    struct GTY((tag("1"))) {
        int type;
        union {
            int i;
            float f;
            char *s GTY((length("strlen(%h.s) + 1")));
        } data;
    } variant;
    
    /* Byte array with size expression */
    unsigned char raw GTY((length("sizeof(struct {int x; double y;})"))) [];
};

/* ========== TEST CASE 9: Function pointer madness ========== */
/* Extremely complex function pointer type */
typedef void (*(*complex_fp_type GTY((callback)))(
    int (*)(char *[], int),
    void (*)(struct { int a; int b; } *)
))(
    int,
    char *(*)(void),
    int (*[ (2) + (3) ])(int, int)
);

/* ========== TEST CASE 10: Bitfield and weird combinations ========== */
struct test_bitfield GTY((user)) {
    unsigned int flags: (sizeof(int) * 8 - 1);
    
    /* Array dimension with ternary operator */
    int values GTY((length("COUNT"))) 
        [ (FLAG_A && FLAG_B) ? (10) : (20) ];
    
    /* Pointer to array of function pointers */
    int (*(*func_table GTY((skip)))[ (5) ])(
        int, 
        char *(*)(char *[])
    );
};

#endif /* TEST_PARSE_H */
