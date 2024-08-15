IMPORTANT: If you are a Mac or linux user, this package requires either Clang++ or G++ to function.

Sfaster works similarly to the sfinder `path` tool, except that it runs considerably faster, and does not run into memory issues. Sfaster is capable of quickly handling many smaller tasks, as well as taking on large tasks that sfinder would fail to complete.

You can run the program by typing `sfaster` (or `./sfaster` in Linux) in the command line, or by running `run_sfaster.py` directly.

You can provide the starting board either as an argument in the command line, or by providing a board in `input.txt`. In the command line, the board can be the first argument provided, or paired with the `-t` flag. The board can be a fumen or a string, though in order to input fumens you need to have `py_fumen_py` installed (run `pip install py_fumen_py` in the console). If no argument is provided (or the program is run from `run_sfaster.py` directly), the program will read from `input.txt`. Note that the input can have line breaks in `input.txt`. If the input is still blank, an empty board will be used.

Examples:
`sfaster v115@9gilEeR4glRpDeR4wwg0RpCeBtxwi0DeBtwwJeAgH`
`sfaster -t v115@9gilEeR4glRpDeR4wwg0RpCeBtxwi0DeBtwwJeAgH`
`sfaster LLL_____SSLOO____SSTJOO___ZZTTJJJ____ZZT`
`sfaster -t LLL_____SSLOO____SSTJOO___ZZTTJJJ____ZZT`
`sfaster` (No board provided, will read from `input.txt`.)

While the program is running, every solution will be written to `output.txt` as it is found. By default, the solutions will be written as fumens, but to have solutions printed as strings, add `-F string` to your command. Keep in mind that the output file can get very large if a large number of solutions are found.

Sfaster doesn't have some of the more niche flags in sfinder yet, but it currently implements the most common ones, with some changes:
* `-p`, `--patterns` Piece order restrictions. If no pattern is specified, piece restrictions will not be considered.
* `-c`, `--clear-line` Number of lines to clear. Defaults to 4.
* `-H`, `--hold` Whether to allow hold to be used. Specify `use` to allow, or `avoid` to disallow. Defaults to `use`.
* `-d`, `--drop` Specify movement abilities (ie: enable 180 spins). Currently, the options are `soft` (or `softdrop`), `jstris180`, or `tetris180`. To enable 180 spins with the 180 kick table for either jstris or tetr.io, choose `jstris180` or `tetrio180`, respectively.
    * Warning: the 180 kicktables use the same logic as the other kicktables, but they haven't been thoroughly tested yet.
* `-o`, `--output-base` Specify where the solutions should be written. Defaults to `"output.txt"`.
Here are the flags unique to Sfinder:
* `-F`, `--format-solution` Specify whether solutions should be written as fumens or strings. Choose `fumen` or `string`, defaults to `fumen`.
* `-B`, `--big-input` Add this flag if you're processing a very large input (ie: many pieces). It should typically run faster, but will take a few seconds to recompile before it starts. Clang++/G++ is required to use this flag. Due to how compilers work, there's a chance this version could have a significantly different (faster or even slower) runtime than the default version.
* `-T`, `--turbo` Run in Turbo mode: uses all the cores of your computer to run large inputs several times faster than normal. Works best if you don't have other programs open.
    * G++ (not Clang++) is required to use both `-B` and `-T` simultaneously. No compiler is needed to run `-T` alone.

### NEW AND UPCOMING FEATURES
#### New features:
* Prune impossible solutions
* Piece restrictions
* Variable-height PCs (not just 4L)
* Allow hold (`--hold use`)
* Option to output fumens instead of strings
* Command line flags (like `-t`, `-c`)
* Option to enable 180 spins with either the jstris or tetrio kicktables
* Version for people without Clang++/G++ (or version that doesn't need to recompile every time)
* Faster fumenify (or option to output directly as fumens)
* Multithreading (should increase the speed significantly)
* Custom output destination (`-o`)
#### Coming soon:
* Version to run many boards back to back (eg: every page in fumen)
#### Coming later:
* Faster setup (ie: new non-python version of run_sfaster)
* Specify kicktables (beyond 180 toggle)
* Option to keep minos in initial board from turning gray in outputs
* Glued solutions (`-s`)
* Specify input fumen page (`-P`)
* Specify multiple patterns (`;` operator in `-p`)