/* simple.h - Basic delimiter cases for gengtype parser coverage */

/* Default case triggers: keywords, identifiers, operators */
#define SIMPLE_DEFINE 100
typedef unsigned long size_t;

/* Case '(': Function prototypes */
extern void simple_func(int arg);
static int calculate_sum(int a, int b, int c);
const char* get_message(void);

/* Case '[': Array declarations */
int global_array[100];
extern char string_buffer[256];
static double matrix[3][3];

/* Case '{': Structure definitions */
struct SimpleStruct {
    int id;
    char name[50];
    float value;
};

union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

/* Mixed default characters: * & ; , = */
int* pointer_var;
int& reference_var = *pointer_var;
volatile const int read_only = 42;
