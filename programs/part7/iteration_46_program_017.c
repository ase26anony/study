/* simple.h - Basic delimiter coverage */

/* Function prototype (triggers '(' case) */
void simple_function(int arg);

/* Array declaration (triggers '[' case) */
extern int simple_array[10];

/* Structure definition (triggers '{' case) */
struct SimpleStruct {
    int member1;
    char member2;
};

/* Union definition */
union SimpleUnion {
    int i;
    float f;
};

/* Enum with default case coverage */
enum SimpleEnum {
    VALUE1,    /* default case: identifiers and commas */
    VALUE2,
    VALUE3
};
