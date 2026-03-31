/* Test input for gengtype parser coverage */
/* GTY markers ensure gengtype processes these declarations */

/* Parentheses in function pointer declarations */
typedef int (*func_ptr_type)(int, char);
GTY(()) func_ptr_type global_func_ptr;

/* Complex function pointer with parentheses */
void (*(*complex_fp)(void))(int);

/* Brackets in array declarations */
GTY(()) int global_array[10];
extern int matrix[5][(sizeof(int)*2)];

/* Braces in struct definition */
struct GTY(()) Node {
    int data;
    struct Node* next;
};

/* Array of function pointers (combines [] and ()) */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array (combines (), *, []) */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member initialized in-line (combines {} and []) */
struct GTY(()) Data {
    int vals[2];
} GTY(()) data_instance = { .vals = {10, 20} };

/* Nested braces in union */
union GTY(()) U {
    int i;
    float f;
    struct {
        int x;
        int y;
    } GTY(()) point;
};

/* Multi-dimensional array with parenthesized size */
int GTY(()) three_d[(2+3)][4][5];

/* Function pointer with array parameter */
void (*signal_handler)(int sig, const char* msg[(10)]);
