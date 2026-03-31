Loop A: blocks {1, 2, 3, 4, 5}
Loop B: blocks {2, 3, 4}

- They intersect (share blocks 2,3,4)
- Loop B has no blocks outside A → B is subloop of A
- Loop A has blocks outside B → not added to B's subloops
Result: A->loops includes B
