// Variables used together in asm statements:
asm volatile ("" : : "r"(a), "r"(c), "r"(e), "r"(fa));  // Needs 4 registers
asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
                   "r"(f), "r"(g), "r"(h), "r"(i), "r"(j),
                   "r"(fa), "r"(fb), "r"(fc));  // Needs 13+ registers!
