┌─────────────────────────────────────────┐
│ Host CPU                                │
│   ↓ Offload to target device            │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│ Target Device (e.g., GPU)               │
│                                         │
│ Team 0: j = 0..k      Team 1: j = k+1.. │
│   ↓ SIMD vectorization   ↓ SIMD         │
│   i = 0..M-1 (vector)   i = 0..M-1      │
│   [16 elements at once]  [16 elements]  │
└─────────────────────────────────────────┘
