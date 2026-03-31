struct S {
    unsigned int bf : 4;  // bitfield of 4 bits
};

struct S s;
unsigned int *ptr = &s.bf;  // ERROR: cannot take address of bitfield
