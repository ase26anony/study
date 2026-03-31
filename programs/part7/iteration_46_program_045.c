/* simple.h - Basic delimiter coverage */

/* Default case triggers: keywords, identifiers, operators */
#define SIMPLE_DEFINE 100
static const volatile int global_counter = 0;

/* '(' case: function prototypes */
extern void simple_func(int arg);
int calculate_sum(int a, int b, int c);

/* '[' case: array declarations */
extern int simple_array[10];
char message_buffer[256];

/* '{' case: structure definitions */
struct SimpleStruct {
    int member1;
    char member2;
    float member3;
};

union SimpleUnion {
    int as_int;
    float as_float;
};
