IMPORTANT: This package requires G++ to function. A (probably slightly slower) version that doesn't require G++ is coming soon.

Sfaster works similarly to the sfinder `path` tool, except that it runs considerably faster, and does not run into memory issues. Currently, sfaster works best for large tasks, the type of inputs that would take sfinder more than a few seconds, but a version for smaller inputs (slightly slower, but won't have the few seconds of startup) will be added soon.

You can run the program by typing `sfaster` (or `./sfaster` in Linux) in the command line, or by running `run_sfaster.py` directly.

You can provide the starting board either as an argument in the command line, or by providing a board in `input.txt`. In the command line, the board can be the first argument provided, or paired with the `-t` flag.
Eg: 
`sfaster v115@9gilEeR4glRpDeR4wwg0RpCeBtxwi0DeBtwwJeAgH`
`sfaster LLL_____SSLOO____SSTJOO___ZZTTJJJ____ZZT`
`sfaster -t LLL_____SSLOO____SSTJOO___ZZTTJJJ____ZZT`

If no argument is provided (or the program is run from `run_sfaster.py` directly), the program will read from `input.txt`. Note that the input can have line breaks in `input.txt`.

If no argument is provided and `input.txt` is blank, an empty board will be used by default.

While the program is running, every solution will be written to `output.txt` as it is found. By default, the solutions will be written as fumens, but to have solutions printed as strings, add `-F string` to your command. Keep in mind that the output file can get very large if a large number of solutions are found.

Sfaster doesn't have some of the more niche flags in sfinder yet, but it currently implements the most common ones, with some changes:
* `-p`, `--patterns` Piece order restrictions. If no pattern is specified, piece restrictions will not be considered.
* `-c`, `--clear-line` Number of lines to clear. Defaults to 4.
* `-H`, `--hold` Whether to allow hold to be used. Specify `use` to allow, or `avoid` to disallow. Defaults to `use`.
* `-d`, `--drop` Specify movement abilities (ie: enable 180 spins). Currently, the options are `soft` (or `softdrop`), `jstris180`, or `tetris180`. To enable 180 spins with the 180 kick table for either jstris or tetr.io, choose `jstris180` or `tetrio180`, respectively.
    * Warning: the 180 kicktables use the same logic as the other kicktables, but they haven't been thoroughly tested yet.
* `-F`, `--format-solution` Unique to sfaster, specify whether solutions should be written as fumens or strings. Choose `fumen` or `string`, defaults to `fumen`.

### NEW AND UPCOMING FEATURES
#### New features:
* Prune impossible solutions
* Piece restrictions
* Variable-height PCs (not just 4L)
* Allow hold (--hold use)
* Option to output fumens instead of strings
* Command line flags (like `-t`, `-c`)
* Option to enable 180 spins with either the jstris or tetrio kicktables
#### Coming soon:
* Multithreading (should increase the speed significantly)
* Version for people without G++ (or version that doesn't need to recompile every time)
* Version to run many boards back to back (eg: every page in fumen)
* Custom output destination (`-o`)
#### Coming later:
* Faster setup (ie: new non-python version of run_sfaster)
* Specify kicktables (beyond 180 toggle)
* Option to keep minos in initial board from turning gray in outputs
* Glued solutions (`-s`)
* Faster fumenify (or option to output directly as fumens)
* Specify input fumen page (`-P`)
* Specify multiple patterns (`;` operator in `-p`)