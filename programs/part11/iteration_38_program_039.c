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
#define BRACE_INITIALIZER { .x = 1, .y = 2 }
#define COMPLEX_PAREN ((a) * ((b) + (c)))

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((length("(sizeof(my_int) * (2 + 3))"))) {
    int data[];
};

/* More parentheses - multiple parenthesized groups */
struct s2 GTY((chain_next("next"), chain_prev("prev"), user)) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Case 2: Brackets - array type with complex dimension expressions */
struct s3 GTY((length("N"))) {
    int arr[ (2 * 3) + sizeof(my_int) ];
};

/* Nested brackets in pointer to array */
typedef int (*array_ptr GTY((tag("ARRAY_PTR"))))[ (sizeof(int) > 4) ? 8 : 16 ];

/* Multi-dimensional array with brackets */
struct s4 GTY((user)) {
    int matrix[ (1 << 2) ][ (3 + 2) ];
};

/* Case 3: Braces - structure with nested struct definition containing braces */
struct outer GTY((user)) {
    struct inner GTY((tag("INNER"))) {
        int x;
        int y;
    } nested;
    int value;
};

/* Union with array and braces in initializer position (in GTY comment) */
union u1 GTY((desc("$1"), param_is(typeof( ((union u1 *)0)->type )))) {
    int type;
    char arr[ 10 + (5) ];
    struct {
        int a;
        int b;
    } GTY((skip)) s;
};

/* Function pointer with complex parentheses in argument list */
typedef void (*complex_fn_ptr GTY((callback)))(
    int (*)(int ARRAY_SIZE_EXPR),
    void (*)(struct s1 *)
);

/* Struct with skip field containing nested parentheses in expression */
struct s5 GTY((user)) {
    int * GTY((skip(" (*(int **) &_val) "))) skipped_ptr;
    int regular_field;
};

/* Template-like macro expansion with parentheses */
#define GTY_SPECIAL(tag_val) GTY((tag(tag_val), user))

struct s6 GTY_SPECIAL("(TAG_VALUE)") {
    int special_field;
};

/* Array of pointers with nested dimension calculations */
struct s7 GTY((length("(count + 7) & ~7"))) {
    void * GTY((tag("PTR_ARRAY"))) pointers[];
};

/* Nested structure with array of structures */
struct s8 GTY((user)) {
    struct element GTY((length("elem_count"))) {
        int id;
        char name[ (32 + 7) & ~7 ];
    } elements[];
};

/* Union with conditional in array size */
union u2 GTY((desc("$1.type"))) {
    struct {
        int type;
        int data[ (sizeof(int) == 4) ? 10 : 20 ];
    } s;
    long long ll;
};

/* Function type with nested parentheses in parameter */
typedef int (*math_func GTY((callback)))(
    int a,
    int b,
    int (*callback)(int, int)
);

/* Struct with multiple GTY options containing all delimiters */
struct s9 
    GTY((chain_next("nxt"),
         chain_prev("prv"),
         length("(size + 3) & ~3"),
         tag("COMPLEX"))) 
{
    struct s9 *nxt;
    struct s9 *prv;
    unsigned size;
    unsigned char data[ (256) ];  /* Brackets */
};

/* Macro that expands to contain braces (in comment form) */
#define BRACE_COMMENT /* { int hidden; } */

struct s10 GTY((user BRACE_COMMENT)) {
    int visible;
};

/* Typedef with function pointer containing array parameter */
typedef void (*processor GTY((callback)))(
    int buffer[],
    int size
);

/* Forward declaration with GTY annotation containing parentheses */
struct forward_decl GTY((user));

/* Actual definition */
struct forward_decl GTY((user)) {
    int value;
    struct forward_decl *next;
};

/* Enum with GTY - though enums don't usually need GTY, test parentheses */
enum my_enum GTY((tag("ENUM"))) {
    VALUE_A = (1 << 0),
    VALUE_B = (1 << 1),
    VALUE_C = (1 << 2) | (1 << 3)
};

/* Complex nested type definition */
typedef struct container GTY((user)) {
    union {
        int i;
        float f;
        char str[ (16) ];
    } GTY((desc("$1.type"))) data;
    int type;
} container_t;

/* Final test: all delimiters in one (in comments/strings) */
struct all_delimiters GTY((
    user,
    tag("({[test]})"),  /* String containing all delimiters */
    length("(1 + 2) * 3"),
    skip(" /* { int x; } */ ")
)) {
    int value;
};

#endif /* TEST_PARSE_H */
