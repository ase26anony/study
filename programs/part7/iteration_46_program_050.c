/* simple.h - Basic delimiter cases to trigger all switch branches */

/* Default case triggers: keywords, identifiers, operators */
#define SIMPLE_DEFINE 100
typedef unsigned long size_t;

/* Case '(': Function prototypes */
extern void simple_func(int param);
int calculate_sum(int a, int b);
static void internal_func(void);

/* Case '[': Array declarations */
extern int global_array[100];
float matrix[10][20];
char message[] = "Hello (with parens in string)";

/* Case '{': Structure definitions */
struct SimpleStruct {
    int id;
    char name[50];
    float values[10];
};

union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

/* Mixed default characters: ; , * & */
const volatile int *ptr;
int &ref = *ptr;
