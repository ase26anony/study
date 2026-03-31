/* Test file for gengtype parser coverage */
/* Parentheses in function pointers */
typedef int (*func_ptr_type)(int, char);
typedef void (*(*complex_fp_type)(void))(int);

/* Brackets in arrays */
GTY(()) int global_array[10];
GTY(()) extern int matrix[5][(sizeof(int)*2)];

/* Braces in struct definition */
struct GTY(()) Node {
    int value;
    struct Node * GTY((skip)) next;
};

/* Combined: array of function pointers */
GTY(()) int (*callbacks[5])(const char*);

/* Nested: function pointer returning pointer to array */
typedef int (*(*get_array_ptr_type(void))[10]);

/* Struct with array member */
struct GTY(()) Data {
    int vals[2];
    struct Data * GTY((skip)) child;
};

/* Initializer with braces */
GTY(()) struct Data root = { .vals = {10, 20}, .child = NULL };

/* Union with nested struct */
union GTY(()) U {
    int i;
    float f;
    struct {
        int x;
        int y;
    } GTY((skip)) point;
};

/* Complex nested example */
typedef struct GTY(()) Container {
    int (*processor)(int (*)(int), int);
    struct Item {
        char name[(10+2)];
        int (*methods[3])(void);
    } GTY((skip)) items[5];
} Container;
