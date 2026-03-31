/* test_suite.h - Comprehensive test for gengtype parser delimiter handling */

/* ========== DEFAULT CASE COVERAGE ========== */
/* Keywords, identifiers, operators, and punctuation */
static const volatile int global_counter = 42;
extern unsigned long *pointer_var;
typedef char byte_type;
#define MAX_SIZE 100
#include <stddef.h>

/* ========== PARENTHESES CASES ========== */
/* Simple function prototype */
void simple_func(int arg);

/* Function with empty parameter list */
int get_value(void);

/* Function pointer */
int (*func_ptr)(double, char);

/* Complex function pointer with nested parentheses */
void (*signal(int sig, void (*handler)(int)))(int);

/* Function returning function pointer */
int (*(*complex_func)(char *))(float);

/* ========== BRACKET CASES ========== */
/* Simple array */
extern int simple_array[10];

/* Multi-dimensional array */
double matrix[3][4];

/* Array of pointers */
const char *string_array[5];

/* Array of function pointers */
int (*func_array[3])(void);

/* Complex nested array with function pointers */
struct Result (*ops[2][3])(int, char *);

/* ========== BRACE CASES ========== */
/* Simple structure */
struct Point {
    int x;
    int y;
};

/* Nested structure */
struct Outer {
    struct Inner {
        int data;
        char flag;
    } inner;
    float value;
};

/* Union */
union Data {
    int i;
    float f;
    char str[20];
};

/* Enumeration */
enum Color {
    RED,
    GREEN,
    BLUE
};

/* ========== MIXED NESTED DELIMITERS ========== */
/* Array of structures */
struct Employee {
    char name[50];
    int (*get_id)(void);
    struct {
        int day;
        int month;
        int year;
    } hire_date;
} employees[100];

/* Function pointer returning pointer to array */
int (*(*get_matrix(void))[10])[20];

/* Complex type with all delimiters */
struct Container {
    int (*callbacks[5])(struct Container *);
    union {
        int (*int_func)(int);
        void (*void_func)(void);
    } func_union;
    char data[256];
};

/* Typedef with complex type */
typedef int (*(*ComplexType)[10])(char *);

/* ========== STRINGS AND COMMENTS WITH DELIMITERS ========== */
char *message = "Text with (parentheses) and [brackets]";
char *path = "/usr/local/include/file.h";

/* Comment with delimiters: { [ ( test ) ] } */

/* ========== EDGE CASES ========== */
/* Empty structures */
struct Empty {};

/* Zero-length array (GCC extension) */
struct Header {
    int type;
    char data[0];
};

/* Anonymous union in struct */
struct WithAnonymous {
    int tag;
    union {
        int num;
        char *str;
    };
};

/* Function with __attribute__ */
void special_func(void) __attribute__((deprecated));

/* ========== FINAL COMPLEX EXAMPLE ========== */
/* This should exercise all parser paths */
static const struct MasterType {
    enum { TYPE_A, TYPE_B } type;
    union {
        int (*int_ops[3])(int, int);
        struct {
            char *(*get_name)(void);
            void (*set_name)(const char *);
        } string_ops;
    } operations;
    int (*(*get_operator(void))[5])(int);
} *master_ptr[10];
