IMPORTANT: If you are a macOS or Linux user, this package requires either Clang++ or G++ to function.

Sfaster works similarly to the sfinder `path` and `setup` tools, except that it runs considerably faster, and does not run into memory issues. Sfaster is capable of quickly handling many smaller tasks, as well as taking on large tasks that sfinder would fail to complete.

You can run the program by typing `sfaster` (or `./sfaster`) in the command line, or by running `run_sfaster.py` directly.

You can provide the starting board either as an argument in the command line, or by providing a board in `input.txt`. In the command line, the board can be the first argument provided, or paired with the `-t` flag. The board can be a fumen or a string, though in order to input fumens you need to have `py_fumen_py` installed (run `pip install py_fumen_py` in the console). If no argument is provided (or the program is run from `run_sfaster.py` directly), the program will read from `input.txt`. Note that the input can have line breaks in `input.txt`. If the input is still blank, an empty board will be used.

Examples:
`sfaster v115@9gilEeR4glRpDeR4wwg0RpCeBtxwi0DeBtwwJeAgH`
`sfaster -t v115@9gilEeR4glRpDeR4wwg0RpCeBtxwi0DeBtwwJeAgH`
`sfaster LLL_____SSLOO____SSTJOO___ZZTTJJJ____ZZT`
`sfaster -t LLL_____SSLOO____SSTJOO___ZZTTJJJ____ZZT`
`sfaster` (No board provided, will read from `input.txt`.)

You can specify the mode with the `-M` (or `--mode`) flag, specifying either `path` or `setup`. If no mode is specified, the default mode is `path`.

While the program is running, every solution will be written to `output.txt` as it is found. By default, the solutions will be written as fumens, but to have solutions printed as strings, add `-F string` to your command. Keep in mind that the output file can get very large if a large number of solutions are found.

Sfaster doesn't have some of the more niche flags in sfinder yet, but it currently implements the most common ones, with some changes:
* `-p`, `--patterns` Piece order restrictions. If no pattern is specified, piece restrictions will not be considered.
* `-c`, `--clear-line` Number of lines to clear. For `setup` mode, may give an interval (eg: `[2,4]`). Defaults to 4 in `path` mode, or no restrictions in `setup` mode.
* `-H`, `--hold` Whether to allow hold to be used. Specify `use` to allow, or `avoid` to disallow. Defaults to `use`.
* `-d`, `--drop` Specify movement abilities (eg: enable 180 spins). Currently, the options are `soft` (or `softdrop`), `hard` (or `harddrop`), `jstris180`, or `tetris180`. To enable 180 spins with the 180 kick table for either jstris or tetr.io, choose `jstris180` or `tetrio180`, respectively.
    * Warning: the 180 kicktables use the same logic as the other kicktables, but they haven't been thoroughly tested yet.
* `-P`, `--page` Which fumen page to use as the input. Ignored for string input. 
* `-o`, `--output-base`, `--output-file`, `--output` Specify where the solutions should be written. Defaults to `"output.txt"`.
Here are the flags unique to Sfinder:
* `-F`, `--format-solution`, `--format-output`, Specify whether solutions should be written as fumens or strings. Choose `fumen` or `string`, defaults to `fumen`.
* `-B`, `--big-input` Add this flag if you're processing a very large input (ie: many pieces). It should typically run faster, but will take a few seconds to recompile before it starts. Clang++/G++ is required to use this flag. Due to how compilers work, there's a chance this version could have a significantly different (faster or even slower) runtime than the default version.
* `-T`, `--turbo` Run in Turbo mode: uses all the cores of your computer to run large inputs several times faster than normal. Works best if you don't have other programs open.
    * G++ (not Clang++) is required to use both `-B` and `-T` simultaneously. No compiler is needed to run `-T` alone.
* `-b`, `--b2b` Set b2b restrictions. Choose `"tetris"` to return solutions where the only clears are tetrises, `"tspin"` to return solutions where the only clears are tspins, `"b2b"` to return solutions where b2b is maintained (either tspins or tetrises), or `"none"` for no restrictions. Defaults to `"none"`.
    * Note that setting it to `"tspin"` guarantees no solutions in `path` mode since you can't tspin a PC.

Here are flags that only apply to `setup` mode:
* `-f`, `--fill` Which mino must be filled. You can specify the mino by letter (eg: `Z`), by color (eg: `"cyan"` or `"cy"`), or you can choose `F` (for string inputs). Defaults to `F`.
    * Unlike in sfinder, you can run sfaster `setup` mode with no fill minos on the board at all!
* `-m`, `--margin` Which mino may be filled. You can specify the mino by letter (eg: `Z`), by color (eg: `"cyan"` or `"cy"`), or you can choose `M` (for string inputs). Defaults to `M`.
`-np`, `--n-pieces` How many pieces should be placed in the solution. You can specify a number, or you can give an interval (eg: `[0,3]`) to specify the minimum and maximum number of pieces allowed. If not specified, ANY number of pieces are allowed (this will probably be changed).
`-g`, `--gaps` Alternative to `--n-pieces`, specifying number of gaps (unfilled minos) instead. You can specify a number, or you can give an interval (eg: `[12,20]`) to specify the minimum and maximum number of pieces allowed.
`-e`, `--exclude` Select `"holes"` for no overhangs at all, `"strict-holes"` for no gaps with left+right blocked too, `"none"` for no restrictions. Defaults to `"none"`.

### NEW AND UPCOMING FEATURES
#### New features:
* Can specify min/max line clears for `setup` mode now. Between this and the preexisting functionality, it can essentially do everything sfinder's `spin` mode can do now as well
* `hard`/`harddrop` drop option (about 10-20% faster than `soft`/`softdrop` at the moment, will speed it up more later) 
* Specify input fumen page (`-P`)
* `setup` mode, with flexible piece/gap counts
#### Coming soon:
* Glued solutions (`-s`)
* Option to get %s on solutions as they're found (like a built-in `cover`)
* Also a regular `cover` mode
* Version to run many boards back to back (eg: every page in fumen)
#### Coming later:
* Faster setup (ie: new non-python version of run_sfaster)
* Specify kicktables (beyond 180 toggle)
* Option to keep minos in initial board from turning gray in outputs
* Specify multiple patterns (`;` operator in `-p`)