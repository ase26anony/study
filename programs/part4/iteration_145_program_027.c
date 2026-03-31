Initial: a=1, b=2, c=3

1. a = b + 1    → a=3, b=2, c=3
   [MEMORY BARRIER - compiler can't reorder across this]
2. c = a * 2    → a=3, b=2, c=6
3. b = c - a    → a=3, b=3, c=6
