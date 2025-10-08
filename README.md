Metro Map Planning — SAT assignment scaffolding

This repository contains scaffolding for the Metro Map Planning SAT assignment described in the prompt. It's intended to help you implement the SAT encoding in C++ and integrate with MiniSat.

What I added

- `src/run1.cpp` — C++ skeleton for the encoder. Reads `<name>.city`, writes `<name>.satinput` (DIMACS CNF) and a helper file `vars.txt` that describes variable numbering. Currently writes a trivial UNSAT or placeholder CNF; replace with your encoding.
- `src/run2.cpp` — C++ skeleton for the decoder. Reads `<name>.city`, `<name>.satinput` (optional), and `<name>.satoutput` (MiniSat output) and writes `<name>.metromap`. Currently produces placeholder output.
- `compile.sh` — compiles the C++ files into `run1` and `run2` (Unix-like). On Windows PowerShell use `.uild.bat` or run the g++ command shown.
- `run1.sh`, `run2.sh` — wrapper scripts that call the compiled binaries (they do not call MiniSat). They assume the current directory contains `<name>.city`.
- `build.bat` — Windows build script that compiles sources with g++.

How to use (Unix / Linux / macOS)

1. ./compile.sh
2. ./run1.sh test # reads test.city and writes test.satinput and test.vars
3. Run MiniSat externally (TA will run this): minisat test.satinput test.satoutput
4. ./run2.sh test # reads test.city and test.satoutput and writes test.metromap

Notes and next steps

- The important part is implementing the CNF encoding in `src/run1.cpp`. The README contains an encoding guide below to help you design variable maps and common encodings (flow, turn counting, and at-most-one / cardinality constraints).
- Don't call MiniSat from `run1` or `run2` — the TA will call it externally.

Encoding guide (short)

- Use integer variable IDs starting at 1 and map every boolean concept ("cell (x,y) used by line k", "edge (x,y)->(x+1,y) used by line k", "turn at vertex") to a variable.
- Output DIMACS CNF: first line `p cnf NUM_VARS NUM_CLAUSES` then clauses.
- For 'at most one' constraints across a set, use pairwise clauses for small sets or Sinz sequential counter for larger sets.
- For path constraints, enforce degree constraints per line and flow constraints or reachability with ordering variables.
- For counting turns (<= J), encode turn detection variables and apply a cardinality constraint over them.

If you'd like, I can now:

- compile the skeletons and run a quick smoke test using the provided Python `testcase_gen.py` to generate sample `.city` files, or
- open `src/run1.cpp` and `src/run2.cpp` so you can inspect and I can extend them with more detailed comments and scaffolding for variable mapping.

Tell me which next step you prefer.
