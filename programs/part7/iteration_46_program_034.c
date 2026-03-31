/* simple.h - Basic delimiter cases for coverage testing */

/* Default case triggers: keywords, identifiers, operators */
#define SIMPLE_DEFINE 100
typedef unsigned long ulong;

/* Case '(': Function prototypes */
extern void simple_func(int param);
static int calculate_sum(int a, int b, int c);

/* Case '[': Array declarations */
extern int simple_array[10];
char message_buffer[256];
double matrix[3][4];

/* Case '{': Structure definitions */
struct SimpleStruct {
    int id;
    char name[32];
    float value;
};

union SimpleUnion {
    int int_val;
    float float_val;
    char char_val;
};

/* Mixed default characters */
const volatile int special_counter = 0;
