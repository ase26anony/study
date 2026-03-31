/* Test input for gengtype parser coverage - targeting parentheses, brackets, and braces */

/* GTY marker for gengtype recognition */
#define GTY(x) __attribute__((gty x))

/* 1. PARENTHESES () - Function pointer declarations */
typedef int (*simple_func_ptr)(int, char);
typedef void (*(*complex_func_ptr)(void))(int);
typedef char *(*string_processor)(const char *input, int length);

/* GTY-marked function pointer */
static GTY(()) int (*marked_callback)(double, float) = NULL;

/* 2. BRACKETS [] - Array declarations */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
static char *string_array[(10 + 5)];

/* Array with nested parentheses in size expression */
int computed_size_array[(1 << 3) - 1];

/* 3. BRACES {} - Struct/union/enum definitions */
struct SimpleStruct {
    int field1;
    char field2;
};

union DataUnion {
    int int_val;
    float float_val;
    struct {
        int x;
        int y;
    } point;
};

enum Color {
    RED,
    GREEN,
    BLUE
};

/* Static initializer with braces */
int global_vector[3] = {1, 2, 3};
struct SimpleStruct global_struct = {42, 'A'};

/* 4. NESTED COMBINATIONS */

/* Array of function pointers - combines [] and () */
int (*callback_array[5])(const char *);

/* Function pointer returning pointer to array - combines (), *, and [] */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member and nested initializer - combines {} and [] */
struct Container {
    int values[2];
    int (*processor)(int);
};

struct Container container = {
    .values = {100, 200},
    .processor = NULL
};

/* Complex nested example */
typedef struct {
    int (*comparator)(const void *, const void *);
    void *data[4];
} SortContext;

/* Multi-level nesting */
int (*(*(*nested_fp)(int))[5])(void);

/* GTY-marked struct with all delimiter types */
struct GTY(()) MarkedType {
    int (*methods[3])(struct MarkedType *);
    union {
        int i;
        float f;
    } value;
    struct {
        int x;
        int y[2];
    } position;
};

/* Function-like macro with parentheses (should be skipped) */
#define MAX(a,b) ((a) > (b) ? (a) : (b))

/* Declaration using the macro */
int max_value = MAX(10, 20);

/* Final complex type combining all delimiters */
typedef struct {
    int (*(*handlers[2])(int))[3];
    struct {
        int count;
        char *items[];
    } flexible_array;
} MasterType;
