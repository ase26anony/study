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

/* Enum with trailing semicolon */
enum SimpleEnum { A, B, C };
