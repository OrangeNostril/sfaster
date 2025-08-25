import argparse
import subprocess

def parseInputBoard(inputBoard:str,lines:int)->str:
    if (inputBoard==''):
        with open("input.txt") as file:#read from input.txt
            inputBoard=file.read()
    if ("v115" in inputBoard):#fumen to string
        if (args.page<=0):
            if (args.page==0):
                raise Exception("Page out of range (`--page 1` is the first page)")
            raise Exception("Page out of range")
        try:
            try:#try using py_fumen_py first
                from py_fumen_py import decode
                inputBoard = decode(inputBoard)[args.page-1].field.string()[:-11]
            except ModuleNotFoundError:
                try:#try using py_fumen as a backup
                    print("Can't find py_fumen_py, trying py_fumen instead")#
                    from py_fumen.decoder import decode
                    inputBoard = decode(inputBoard)[args.page-1].get_field().string()[:-11]
                except ModuleNotFoundError:
                    raise Exception("Please install either py_fumen or py_fumen_py before inputting fumens")
        except IndexError:#except from either py_fumen_py or py_fumen
            raise Exception("Page out of range")
    inputBoard=inputBoard.replace("\n","")#remove line breaks
    if (len(inputBoard)%10!=0):
        raise args.argumentParseError("Please ensure your board has a multiple of 10 spaces in it")

    if (len(inputBoard)<lines*10):#(probably?) needed because inputs are vertically flipped for v4
        inputBoard="_"*(10*lines-len(inputBoard))+inputBoard
    return inputBoard

excludeMap = {"none":"0", "holes":"1", "strict-holes":"2"}
#b2bMap = {"none":"0", "tetris":"1", "tspin":"2", "b2b":"3"}#later


parser = argparse.ArgumentParser(description="Sfaster flags")

#everything commands
parser.add_argument("inputBoard", nargs="?", default="", help="Input board string")
parser.add_argument("-t", "--tetfu", help="Input board string")
parser.add_argument("-M", "--mode", choices=["path","setup"], default="path", help="path (PCs) or setup (everything else)")
parser.add_argument("-c", "--clear-line", type=int, choices=[-1]+list(range(1,11)), default=-1, help="Number of lines to clear (1-10)")
parser.add_argument("-H", "--hold", choices=["avoid", "use"], default="use",help="Hold piece preference")
parser.add_argument("-p", "--patterns", type=str, metavar="eg: [SZT]p2,*p1,*!", help="Pattern string to parse")
parser.add_argument("-s", "--split", choices=["yes", "no"], default="yes", help="Split preference")
parser.add_argument("-d", "--drop", choices=["jstris180", "tetrio180","soft","softdrop"], default="soft", help="Specify movement abilities")
parser.add_argument("-o", "--output-base", default="output.txt", help="Specify program output destination")
parser.add_argument("-F", "--format-solution", choices=["fumen", "string", "str"], default="fumen", help="Format of each solution")
parser.add_argument("-B", "--big-input", action="store_true", help="Have the program custom-compiled to run slightly faster for larger inputs")
parser.add_argument("-T", "--turbo", action="store_true", help="Turbo mode (uses all cores, try to free up CPU space beforehand)")
parser.add_argument("-b", "--b2b", choices=["none","tetris","tspin","b2b"], default="none", help="Clear requirements (no requirements, only tspins, only tetrises, or maintain b2b)")
parser.add_argument("-P", "--page", type=int, default=1, help="Which page of the fumen input to use")
#setup commands
parser.add_argument("-f", "--fill", choices=["s","z","l","j","i","t","o","S","Z","L","J","I","T","O","cyan","cy","blue","bl","orange","or","yellow","ye","green","gr","red","re","purple","pu","none","F"], default="F", help="What minos to fill (color or \'F\')")
parser.add_argument("-m", "--margin", choices=["s","z","l","j","i","t","o","S","Z","L","J","I","T","O","cyan","cy","blue","bl","orange","or","yellow","ye","green","gr","red","re","purple","pu","none","M"], default="M", help="What minos are optional (color or \'M\')")
parser.add_argument("-np", "--n-pieces", type=str, default="", help="Number of pieces to place ([min,max] for a range)")
parser.add_argument("-g", "--gaps", type=str, default="", help="Alternative to \"--n-pieces\", specifying number of gaps (unfilled minos) instead. ([min,max] for a range)")
parser.add_argument("-e", "--exclude", choices=excludeMap.keys(), default="none", metavar="test", help="\"holes\" for no overhangs at all, \"strict-holes\" for no gaps with left+right blocked too, \"none\" for no restrictions")

args = parser.parse_args()
args.exclude = excludeMap[args.exclude]

if (args.mode=="path"):
    setupFlags=False
    if (args.fill!="F"):
        print("Ignoring --fill")
        setupFlags=True
    if (args.margin!="M"):
        print("Ignoring --margin")
        setupFlags=True
    if (args.n_pieces!=""):
        print("Ignoring --n-pieces")
        setupFlags=True
    if (args.gaps!=""):
        print("Ignoring --gaps")
        setupFlags=True
    if (args.exclude!="0"):
        print("Ignoring --exclude")
        setupFlags=True
    if setupFlags:
        print("(Include \"-M setup\" to use setup mode!)\n")

lines = args.clear_line
if (lines==-1 and args.mode=="path"):
    lines=4

if args.tetfu:
    inputBoard = args.tetfu
else:
    inputBoard = args.inputBoard
inputBoard = parseInputBoard(inputBoard,lines)

if (lines==-1):#(and not path)
    lines=len(inputBoard)//10

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

if (args.mode=="setup"):
    print("NOTE: setup mode is new and still being tested")
    if (args.fill==args.margin):
        raise Exception("Your --margin mino can't be the same as your --fill mino")
    if (args.turbo):#temporary (don't have it yet)
        print("Turbo mode coming soon (ignoring -T for now)")
        args.turbo=False
    startingGaps=bitmap^0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF#the gaps mean something different for setup mode
    bitmap=0
    gapBitmap=0
    if (len(args.fill)==1):
        fillMino=args.fill.upper()
    else:
        fillMap={"cy":"I","bl":"J","or":"L","ye":"O","gr":"S","re":"Z","pu":"T","no":"F"}
        fillMino=fillMap[args.fill[:2]]
    if (len(args.margin)==1):
        marginMino=args.margin.upper()
    else:
        marginMap={"cy":"I","bl":"J","or":"L","ye":"O","gr":"S","re":"Z","pu":"T","no":"M"}
        marginMino=marginMap[args.margin[:2]]

    for i in range(len(inputBoard)):
        if (inputBoard[i]!=fillMino and inputBoard[i]!=marginMino):
            bitmap|=1<<i<<(i//10)
        elif (inputBoard[i]==marginMino):#aka, gappable!
            gapBitmap|=1<<i<<(i//10)
    emptyMinos=inputBoard.count(fillMino)+inputBoard.count(marginMino)
    gapMinMax=[0,emptyMinos]
    if (args.n_pieces!=""):
        if (args.n_pieces[0]!='['):
            minPieces=int(args.n_pieces)
            maxPieces=minPieces
        else:
            (minPieces,maxPieces)=(int(n) for n in args.n_pieces[1:-1].split(',',1))
        gapMinMax[0]=emptyMinos-maxPieces*4
        gapMinMax[1]=emptyMinos-minPieces*4
    if (args.gaps!=""):
        if (args.gaps[0]!='['):
            minGaps=int(args.gaps)
            maxGaps=minGaps
        else:
            (minGaps,maxGaps)=(int(n) for n in args.gaps[1:-1].split(',',1))
        if (args.n_pieces!=""):
            if (gapMinMax[0]!=minGaps or gapMinMax[1]!=maxGaps):#if the arguments don't match
                raise Exception("The --n-pieces and --gaps values are contradictory")
        else:
            gapMinMax[0]=minGaps
            gapMinMax[1]=maxGaps
    gapMinMaxStr="{"+str(gapMinMax[0])+","+str(gapMinMax[1])+"}"




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
        if (args.mode=="path"):
            output=subprocess.run([compiler, "v4.1_demo.cpp", f"-DmaxLines={lines}", f"-Dboard=bitmap({bitmap&0xFFFFFFFFFFFFFFFF}llu,{bitmap>>64}llu)", f"-DpatternStr=\"{pattern}\"", f"-DallowHold={hold}", f"-Dglue={glue}", f"-DconvertToFumen={convertToFumen}", f"-Db2bReq={b2bReq}", f"-DoutPath=\"{args.output_base}\"", "-O3", "-std=c++11", "-o", "v4"],capture_output=True)
        elif (args.mode=="setup"):
            #print("testing (uncompiled) setup")#
            output=subprocess.run([compiler, "v4.1_setup.cpp", f"-DmaxLines={lines}", f"-Dboard=bitmap({bitmap&0xFFFFFFFFFFFFFFFF}llu,{bitmap>>64}llu)", f"-DpatternStr=\"{pattern}\"", f"-DallowHold={hold}", f"-Dglue={glue}", f"-DconvertToFumen={convertToFumen}", f"-Db2bReq={b2bReq}", f"-DoutPath=\"{args.output_base}\"", f"-DgappableBoard=bitmap({gapBitmap&0xFFFFFFFFFFFFFFFF}llu,{gapBitmap>>64}llu)", "-DgapsInterval="+gapMinMaxStr, f"-DstartingGaps=bitmap({startingGaps&0xFFFFFFFFFFFFFFFF}llu,{startingGaps>>64}llu)", "-Dexclude="+args.exclude, "-O3", "-std=c++11", "-o", "v4"],capture_output=True)
    if (output.stderr!=b''):
        raise Exception("Compilation error:\n\t"+output.stderr.decode())

    print("Finished compiling, running...")#
    output=subprocess.run(["./v4"],capture_output=True)
else:#not-custom compiled
    print("Bitmap created")#
    exeName="v4_setup_precompiled" if args.mode=="setup" else "v4_precompiled"
    try:
        subprocess.run(["./"+exeName])#returns immediately if exists, throws error if does not exist
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
        cppName=exeName[:2]+".1"+exeName[2:]+".cpp"#.1 until new version
        subprocess.run([compiler, cppName, "-O3", "-std=c++11", "-o", exeName])
    print("Running finder...")#
    command = ["./v4_precompiled", f"{bitmap&0xFFFFFFFFFFFFFFFF},{bitmap>>64}", f'{pattern}', str(lines), hold, glue, convertToFumen, b2bReq, args.output_base]
    if (args.mode=="path"):
        pass
    elif (args.mode=="setup"):
        #print("testing precompiled setup")#
        command[0]="./v4_setup_precompiled"
        command.extend([f"{gapBitmap&0xFFFFFFFFFFFFFFFF},{gapBitmap>>64}", gapMinMaxStr[1:-1], f"{startingGaps&0xFFFFFFFFFFFFFFFF},{startingGaps>>64}", args.exclude])

    if (load180Kicks!=""):#v4.1_compiled.exe board, pattern, maxLines, allowHold, glue, convertToFumen, load180Kicks
        command.append(load180Kicks)
    output=subprocess.run(command,capture_output=True)
if (output.stderr!=b''):
    raise Exception("Program error:\n\t"+output.stderr.decode())
#print(output.stdout.decode()+"\n")#debug

data=output.stdout.decode().split('\n')
seconds=int(data[0][:-3])/1e6
solutions=format(int(data[1].split(" ")[0]),",")
if (seconds>=3600):#show hours
    print(f"\nFound {solutions} solutions in {int(seconds//3600)}:{int(round(seconds%3600)//60)}:{round(seconds%60):02}s")
elif (seconds>=60):#show minutes
    print(f"\nFound {solutions} solutions in {int(seconds//60)}:{round(seconds%60):02}s")
else:#just seconds (might be in scienfitic if <1e-4)
    print(f"\nFound {solutions} solutions in {seconds%60} seconds")
print("Solutions have been written to",args.output_base)