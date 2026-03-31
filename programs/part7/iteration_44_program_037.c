/* Test file for gengtype parser coverage */
/* This file contains constructs that trigger consume_balanced() calls */

/* GTY marker for gengtype recognition */
#define GTY(x) __attribute__((gty x))

/* 1. Parentheses () - Function pointer declarations */
typedef int (*func_ptr_type)(int, char);
void (*(*complex_func_ptr)(void))(int);

/* GTY-marked function pointer */
GTY(()) int (*gty_func_ptr)(double);

/* 2. Brackets [] - Array declarations */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];

/* Array with GTY marker */
GTY(()) char gty_buffer[256];

/* 3. Braces {} - Struct/union definitions and initializers */
struct SimpleStruct {
    int a;
    char b;
};

/* Struct with GTY marker */
struct GTY(()) TaggedStruct {
    int *data;
    struct TaggedStruct *next;
};

/* Union with nested struct */
union ComplexUnion {
    int i;
    float f;
    struct {
        int x;
        char y;
    } nested;
};

/* Static initializer with braces */
int global_init[3] = {1, 2, 3};
struct SimpleStruct global_struct = { .a = 42, .b = 'X' };

/* 4. Nested combinations for recursive consume_balanced() calls */

/* Array of function pointers - combines [] and () */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array - combines (), *, and [] */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member initialized in-line - combines {} and [] */
struct DataContainer {
    int values[2];
    char *name;
} data_instance = { .values = {10, 20}, .name = "test" };

/* Complex nested example */
typedef struct GTY(()) Node {
    struct Node * GTY((skip)) children[4];
    int (* GTY((tag("NODE_FUNC"))) processor)(struct Node *);
    union {
        int ival;
        struct {
            float x, y;
        } point;
    } data;
} NodeType;

/* Multi-dimensional array of function pointers */
void (*(*signal_handlers[3][2])(int))(void);

/* Typedef with all three delimiters */
typedef struct {
    int (*compare)(const void *, const void *);
    void *items[100];
    struct {
        int count;
        int max;
    } stats;
} Container;

/* Enum definition (uses braces) */
enum Flags {
    FLAG_A = 1,
    FLAG_B = 2,
    FLAG_C = 4
};

/* Forward declaration with function pointer parameter */
extern void register_callback(int (*cb)(int, void *), void *userdata);

/* Const array with initializer */
const int lookup_table[] = { [0] = 0, [1] = 1, [2] = 4, [3] = 9 };
