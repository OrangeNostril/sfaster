IMPORTANT: This package requires G++ to function. A (likely slower) version that doesn't require G++ is coming soon.

This program works similarly to sfinder, except that it runs considerably faster, does not run into memory issues, and finds every way to clear the space, including solutions that aren't possible in a regular game (ie: it allows floating pieces, impossible spins, etc.).

You can run the program by typing `sfaster` (or `./sfaster` in Linux) in the command line, or by running `run_sfaster.py` directly.

You can provide the starting board either as an argument in the command line, or by providing a board in `input.txt`. The board can either be provided as a Fumen, or as a string. 
Eg: 
`sfaster v115@9gilEeR4glRpDeR4wwg0RpCeBtxwi0DeBtwwJeAgH`
`sfaster LLL_____SSLOO____SSTJOO___ZZTTJJJ____ZZT`

If no argument is provided (or the program is run from `run_sfaster.py` directly), the program will read from `input.txt`. Note that the input can have line breaks in `input.txt`.

If no argument is provided and `input.txt` is blank, an empty board will be used by default.

While the program is running, every solution will be written as a string to `output.txt` as it is found. Keep in mind that the output file can get very large if a large number of solutions are found.

Lastly, note that the program currently only solves 4L PCs, and currently does not consider bags or piece restrictions, but both of those things will change soon.