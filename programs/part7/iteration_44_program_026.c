/* Test input for gengtype parser coverage.
   This file is only meant to be parsed by gengtype, not compiled normally. */

/* --- Parentheses cases --- */
/* Function pointer typedef */
typedef int (*func_ptr_type)(int, char);

/* Complex function pointer declaration */
void (*(*complex_fp)(void))(int);

/* Function pointer in GTY context */
typedef void (*GTY((skip)) gty_callback)(void*);

/* --- Brackets cases --- */
/* Simple array */
int simple_array[10];

/* Array with size expression in brackets */
extern int sized_array[(sizeof(int) * 2)];

/* Multi-dimensional array */
int matrix[5][10];

/* --- Braces cases --- */
/* Struct definition */
struct simple_struct {
    int field1;
    char field2;
};

/* Union with nested struct */
union data_union {
    int i;
    float f;
    struct { int x; } inner;
};

/* Array initializer */
int initialized_array[3] = {1, 2, 3};

/* --- Nested/Combined delimiter cases --- */
/* Array of function pointers ([] and ()) */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array ((), *, []) */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member initialized in-line ({}, []) */
struct container {
    int values[2];
} global_container = { .values = {10, 20} };

/* Complex GTY-marked structure */
struct GTY((tag("TREE"))) tree_node {
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    int values[3];
};

/* Enum with last element (braces) */
enum colors { RED, GREEN, BLUE };

/* Function pointer inside struct */
struct operations {
    int (*compare)(const void*, const void*);
    void (*free)(void*);
};

/* Typedef combining all delimiters */
typedef struct {
    int (*methods[2])(int);
    char name[20];
} object_type;

/* End of test input */
