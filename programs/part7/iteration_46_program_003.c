/* simple.h - Basic delimiter patterns to trigger all switch cases */

/* Default case triggers: keywords, identifiers, operators */
#define DEFAULT_CASE 1
typedef unsigned long size_t;

/* Case '(': Function prototypes */
extern void simple_func(int param);
int calculate_sum(int a, int b, int c);
static void internal_func(void);

/* Case '[': Array declarations */
int global_array[100];
extern char string_buffer[256];
static double matrix[3][3];

/* Case '{': Structure definitions */
struct Point {
    int x;
    int y;
    int z;
};

union Data {
    int i;
    float f;
    char str[20];
};

/* Mixed default characters: ; , * & */
int *pointer;
const char *const_string;
volatile unsigned int *vol_ptr;
