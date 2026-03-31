/* simple.h - Basic delimiter cases for gengtype parser coverage */

/* Default case triggers: keywords, identifiers, operators */
#define SIMPLE_DEFINE 100
typedef unsigned long ulong;

/* '(' case: function prototypes */
extern void simple_func(int param);
static int calculate_sum(int a, int b, int c);

/* '[' case: array declarations */
extern int global_array[100];
static char buffer[256];
volatile double measurements[10];

/* '{' case: structure definitions */
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

/* Mixed default characters: ; , * & */
const int *ptr;
int &ref = *ptr;
unsigned counter, index;
