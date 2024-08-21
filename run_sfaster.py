import argparse
import subprocess

def parseInputBoard(inputBoard:str,lines:int)->str:
    if (inputBoard==''):
        with open("input.txt") as file:#read from input.txt
            inputBoard=file.read()
    if ("v115" in inputBoard):#fumen to string
        try:#try using py_fumen_py first
            from py_fumen_py import decode
            inputBoard = decode(inputBoard)[0].field.string()[:-11]#only gets first page (for now)
        except ModuleNotFoundError:
            try:#try using py_fumen as a backup
                from py_fumen.decoder import decode
                inputBoard = decode(inputBoard)[0].get_field().string()[:-11]
            except ModuleNotFoundError:
                raise Exception("Please install either py_fumen or py_fumen_py before inputting fumens")
    inputBoard=inputBoard.replace("\n","")#remove line breaks
    if (len(inputBoard)%10!=0):
        raise args.argumentParseError("Please ensure your board has a multiple of 10 spaces in it")

    if (len(inputBoard)<lines*10):#(probably?) needed because inputs are vertically flipped for v4
        inputBoard="".join(["_"]*(10*lines-len(inputBoard)))+inputBoard
    return inputBoard

parser = argparse.ArgumentParser(description="Sfaster flags")

parser.add_argument("inputBoard", nargs="?", default="", help="Input board string")
parser.add_argument("-t", "--tetfu", help="Input board string")
parser.add_argument("-c", "--clear-line", type=int, choices=range(1,11), default=4, help="Number of lines to clear (1-10)")
parser.add_argument("-H", "--hold", choices=["avoid", "use"], default="use",help="Hold piece preference")
parser.add_argument("-p", "--patterns", type=str, help="Pattern string to parse")
parser.add_argument("-s", "--split", choices=["yes", "no"], default="yes", help="Split preference")
parser.add_argument("-d", "--drop", choices=["jstris180", "tetrio180","soft","softdrop"], default="soft", help="Specify movement abilities")
parser.add_argument("-o", "--output-base", default="output.txt", help="Specify program output destination")
parser.add_argument("-F", "--format-solution", choices=["fumen", "string", "str"], default="fumen", help="Format of each solution")
parser.add_argument("-B", "--big-input", action="store_true", help="Have the program custom-compiled to run slightly faster for larger inputs")
parser.add_argument("-T", "--turbo", action="store_true", help="Turbo mode (uses all cores, try to free up CPU space beforehand)")
parser.add_argument("-b", "--b2b", choices=["none","tetris","tspin","b2b"], default="none", help="Clear requirements (no requirements, only tspins, only tetrises, or maintain b2b)")

args = parser.parse_args()

lines = args.clear_line

if args.tetfu:
    inputBoard = args.tetfu
else:
    inputBoard = args.inputBoard
inputBoard = parseInputBoard(inputBoard,lines)

if not args.patterns:
    pattern="*p1"*(5*lines//2 + 3)
else:
    pattern = args.patterns

hold = "false" if args.hold=="avoid" else "true"
glue = "true" if args.split=="yes" else "false"
convertToFumen = "true" if args.format_solution=="fumen" else "false"

load180Kicks = "" if args.drop[:4]=="soft" else "-Dload180Kicks="+args.drop#leaving it out entirely if not specified

print("Board:")
for i in range(0,len(inputBoard),10):
    print(inputBoard[i:i+10])
print("Creating bitmap...")#

inputBoard="".join([inputBoard[i:i+10] for i in range(len(inputBoard)-10,-10,-10)])#flip rows (v4.1 reads bottom to top)
bitmap=0
for i in range(len(inputBoard)):
    if (inputBoard[i]!='_'):
        bitmap|=1<<i<<(i//10)

b2bReq = {"none":"0", "tetris":"1", "tspin":"2", "b2b":"3"}[args.b2b]

if (args.turbo and not args.big_input):#turbo_precompiled
    print("Bitmap created\nTurbo mode")
    try:
        subprocess.run(["./v4_precompiled_turbo"])#returns immediately if exists, throws error if does not exist
    except FileNotFoundError:
        print("Executable not found, compiling...")
        try:
            subprocess.run(["g++"],capture_output=True)
            compiler = "g++"
            print("Using G++")#
        except FileNotFoundError:
            raise Exception("Couldn't find G++ or Clang++\nCouldn't find or compile executable")
        subprocess.run([compiler, "-fopenmp", "v4.1_precompiled_turbo.cpp", "-O3", "-std=c++11", "-o", "v4_precompiled_turbo"])
    print("Running finder...")#
    if (load180Kicks!=""):#v4.1_compiled.exe board, pattern, maxLines, allowHold, glue, convertToFumen, load180Kicks
        output=subprocess.run(["./v4_precompiled_turbo", f"{bitmap&0xFFFFFFFFFFFFFFFF},{bitmap>>64}", f'{pattern}', str(lines), hold, glue, convertToFumen, b2bReq, args.output_base, load180Kicks],capture_output=True)
    else:
        output=subprocess.run(["./v4_precompiled_turbo", f"{bitmap&0xFFFFFFFFFFFFFFFF},{bitmap>>64}", f'{pattern}', str(lines), hold, glue, convertToFumen, b2bReq, args.output_base],capture_output=True)
elif (args.turbo):#turbo mode (custom compiled)
    print("Bitmap created\nTurbo mode\nCompiling finder...")#
    try:#only trying g++ because clang++ isn't automatically compatible with OpenMP
        subprocess.run(["g++"],capture_output=True)
        compiler = "g++"
        print("Using G++")#
    except FileNotFoundError:
        raise Exception("Please install G++ before using the -T flag")
    if (load180Kicks!=""):
        output=subprocess.run([compiler, "-fopenmp", "v4.1_turbo.cpp", f"-DmaxLines={lines}", f"-Dboard=bitmap({bitmap&0xFFFFFFFFFFFFFFFF}llu,{bitmap>>64}llu)", f"-DpatternStr=\"{pattern}\"", f"-DallowHold={hold}", f"-Dglue={glue}", f"-DconvertToFumen={convertToFumen}", f"-Db2bReq={b2bReq}", f"-DoutPath=\"{args.output_base}\"", load180Kicks, "-O3", "-std=c++11", "-o", "v4"],capture_output=True)
    else:
        output=subprocess.run([compiler, "-fopenmp", "v4.1_turbo.cpp", f"-DmaxLines={lines}", f"-Dboard=bitmap({bitmap&0xFFFFFFFFFFFFFFFF}llu,{bitmap>>64}llu)", f"-DpatternStr=\"{pattern}\"", f"-DallowHold={hold}", f"-Dglue={glue}", f"-DconvertToFumen={convertToFumen}", f"-Db2bReq={b2bReq}", f"-DoutPath=\"{args.output_base}\"", "-O3", "-std=c++11", "-o", "v4"],capture_output=True)
    if (output.stderr!=b''):
        raise Exception("Compilation error:\n\t"+output.stderr.decode())

    print("Finished compiling, running...")#
    output=subprocess.run(["./v4"],capture_output=True)
elif (args.big_input):#custom-compiled
    print("Bitmap created\nCompiling finder...")#
    try:#trying clang first because it's generally faster (at least for me)
        subprocess.run(["clang++"],capture_output=True)
        compiler = "clang++"
        print("Using Clang++")#
    except FileNotFoundError:#if g++ not installed
        try:
            subprocess.run(["g++"],capture_output=True)
            compiler = "g++"
            print("Using G++")#
        except FileNotFoundError:
            raise Exception("Please install either Clang++ or G++ before using the -B flag")
    if (load180Kicks!=""):
        output=subprocess.run([compiler, "v4.1_demo.cpp", f"-DmaxLines={lines}", f"-Dboard=bitmap({bitmap&0xFFFFFFFFFFFFFFFF}llu,{bitmap>>64}llu)", f"-DpatternStr=\"{pattern}\"", f"-DallowHold={hold}", f"-Dglue={glue}", f"-DconvertToFumen={convertToFumen}", f"-Db2bReq={b2bReq}", f"-DoutPath=\"{args.output_base}\"", load180Kicks, "-O3", "-std=c++11", "-o", "v4"],capture_output=True)
    else:
        output=subprocess.run([compiler, "v4.1_demo.cpp", f"-DmaxLines={lines}", f"-Dboard=bitmap({bitmap&0xFFFFFFFFFFFFFFFF}llu,{bitmap>>64}llu)", f"-DpatternStr=\"{pattern}\"", f"-DallowHold={hold}", f"-Dglue={glue}", f"-DconvertToFumen={convertToFumen}", f"-Db2bReq={b2bReq}", f"-DoutPath=\"{args.output_base}\"", "-O3", "-std=c++11", "-o", "v4"],capture_output=True)
    if (output.stderr!=b''):
        raise Exception("Compilation error:\n\t"+output.stderr.decode())

    print("Finished compiling, running...")#
    output=subprocess.run(["./v4"],capture_output=True)
else:#not-custom compiled
    print("Bitmap created")#
    try:
        subprocess.run(["./v4_precompiled"])#returns immediately if exists, throws error if does not exist
    except FileNotFoundError:
        print("Executable not found, compiling...")
        try:#trying clang first because it's generally faster (at least for me)
            subprocess.run(["clang++"],capture_output=True)
            compiler = "clang++"
            print("Using Clang++")#
        except FileNotFoundError:#if g++ not installed
            try:
                subprocess.run(["g++"],capture_output=True)
                compiler = "g++"
                print("Using G++")#
            except FileNotFoundError:
                raise Exception("Couldn't find G++ or Clang++\nCouldn't find or compile executable")
        subprocess.run([compiler, "v4.1_precompiled.cpp", "-O3", "-std=c++11", "-o", "v4_precompiled"])
    print("Running finder...")#
    if (load180Kicks!=""):#v4.1_compiled.exe board, pattern, maxLines, allowHold, glue, convertToFumen, load180Kicks
        output=subprocess.run(["./v4_precompiled", f"{bitmap&0xFFFFFFFFFFFFFFFF},{bitmap>>64}", f'{pattern}', str(lines), hold, glue, convertToFumen, b2bReq, args.output_base, load180Kicks],capture_output=True)
    else:
        output=subprocess.run(["./v4_precompiled", f"{bitmap&0xFFFFFFFFFFFFFFFF},{bitmap>>64}", f'{pattern}', str(lines), hold, glue, convertToFumen, b2bReq, args.output_base],capture_output=True)
if (output.stderr!=b''):
    raise Exception("Program error:\n\t"+output.stderr.decode())
#print(output.stdout.decode()+"\n")#debug
data=output.stdout.decode().split('\n')
seconds=int(data[0][:-3])/1e6
solutions=format(int(data[1].split(" ")[0]),",")
if (seconds>=60):#show minutes
    print(f"Found {solutions} solutions in {int(seconds//60)}:{round(seconds%60):02}s")
else:
    print(f"Found {solutions} solutions in {seconds%60} seconds")
print("Solutions have been written to",args.output_base)