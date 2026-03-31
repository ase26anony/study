struct S {
    unsigned int bf : 4;  // bitfield
};

struct S s;
unsigned int* ptr = &s.bf;  // ERROR: cannot take address of bitfield
