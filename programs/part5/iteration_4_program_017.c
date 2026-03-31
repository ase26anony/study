struct S {
    unsigned int bf : 4;  // bitfield
};

struct S s;
unsigned int* p = &s.bf;  // ERROR: cannot take address of bitfield
