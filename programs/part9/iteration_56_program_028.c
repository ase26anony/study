// Forces register spilling
asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
               "r"(f), "r"(g), "r"(h), "r"(i), "r"(j),
               "r"(fa), "r"(fb), "r"(fc));
