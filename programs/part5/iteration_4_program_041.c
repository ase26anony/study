struct S {
    unsigned int bf : 4;  // bitfield member
};

struct S s;
unsigned int *ptr = &s.bf;  // ERROR: Cannot take address of bitfield
