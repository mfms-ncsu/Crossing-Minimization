# Crossing-Minimization
Programs, scripts and other utilities related to minimizing crossings (and other objectives) in layered graphs. Instructions for compilation are in file `INSTALLATION.txt`
Currently, all programs use two input files, a `.dot` file whose format is that used by `GraphViz` and a `.ord` file that specifies the order of nodes on each layer. Translations from simpler formats are provided by two scripts in the `scripts` directory.
- `sgf2dot+ord.py` translates from `sgf` (simple graph format, documented at the beginning of the script) to `dot` and `ord`; requires Python 3 (but may work with Python 2)
- `mlcm2dot+ord` is a shell script that translates from `mlcm` to `dot` and `ord` by first translating to `sgf` and then using `mlcm2sgf.py`; documentation of the `mlcm` format is at the beginning of the latter

To run `mce` by itself, go to the `src` directory and (after `make min_crossings`), do
` ./min_crossings -p dfs -h mce -i ITERATIONS FILE.dot FILE.ord` (for a specified number of iterations, at least 10,000); or
` ./min_crossings -p dfs -h mce -r TIME FILE.dot FILE.ord` if a limit on runtime is desired.

For best results, add the option `-R SEED` to break ties by randomization (using a chosen seed).

To run a sequence of heuristics, as recommended in the 2012 JEA paper, use one of the variations of the `followHeuristic` script. The best combinations are (INPUT_DIR is a directory containg dot and ord files)
`followHeuristic_R INPUT_DIR OUTPUT_DIR mod_bary sifting` when minimizing total crossings
`followHeuristic_R INPUT_DIR OUTPUT_DIR mod_bary mce` when minimizing max crossings for any edge
Edit the scripts so that the number of iterations ot runtime for mod_bary is roughly 10-20% of the total.
