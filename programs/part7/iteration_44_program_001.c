/* Test file for gengtype parser coverage */
/* Parentheses case: function pointer declarations */
typedef int (*func_ptr_type)(int, char);
typedef void (*(*complex_func_ptr)(void))(int);

/* Brackets case: array declarations */
extern int global_array[10];
extern int multi_dim_array[5][(sizeof(int)*2)];

/* Braces case: struct definition with GTY marker */
struct GTY(()) Node {
    int value;
    struct Node* GTY((skip)) next;
};

/* Combined: array of function pointers */
int (* GTY((tag("CALLBACK"))) callbacks[5])(const char*);

/* Nested: function pointer returning pointer to array */
int (*(* GTY((skip)) get_array_ptr(void))[10]);

/* Initializer with braces */
static int GTY(()) initialized_array[3] = {1, 2, 3};
