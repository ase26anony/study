To make this code compile, you would need to either:
1. Remove `explicit` from the copy constructor
2. Use direct initialization instead: `ExplicitClass e2(ExplicitClass(42));`

If you want to preserve the explicit nature while making it compile, you could modify it to:
