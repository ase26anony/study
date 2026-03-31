      [Start]
         |
         v
    input > 10?
     /        \
    /          \
[true]       [false]
   |            |
   v            v
val = some    val = another
computation() computation()
   \          /
    \        /
     [Merge] ← phi(val) selects appropriate value
         |
         v
     val == 0?
