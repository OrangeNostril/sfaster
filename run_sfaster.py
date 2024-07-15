import argparse
from py_fumen.decoder import decode
import subprocess

def parseInputBoard(inputBoard:str,lines:int)->str:
    if (inputBoard==''):
        with open("input.txt") as file:#read from input.txt
            inputBoard=file.read()
        if (inputBoard==''):#needed?
            inputBoard="v115@vhAAgH"
    if ("v115" in inputBoard):#fumen to string
        inputBoard = decode(inputBoard)[0].get_field().string()[:-11]#doesn't do glued inputs (for now)
    inputBoard=inputBoard.replace("\n","")#remove \n
    if (len(inputBoard)%10!=0):
        raise args.argumentParseError("Please ensure your board has a multiple of 10 spaces in it.")

    if (len(inputBoard)<lines*10):
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
parser.add_argument("-F", "--format-solution", choices=["fumen", "string", "str"], default="fumen", help="Format of each solution")

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
print("Bitmap created\nCompiling finder...")#
if (load180Kicks!=""):
    output=subprocess.run(["g++", "v4.1_demo.cpp", f"-DmaxLines={lines}", f"-Dboard=bitmap({bitmap&0xFFFFFFFFFFFFFFFF}llu,{bitmap>>64}llu)", f"-DpatternStr=\"{pattern}\"", f"-DallowHold={hold}", f"-Dglue={glue}", load180Kicks, "-O3", "-std=c++11", "-o", "v4"],capture_output=True)
else:
    output=subprocess.run(["g++", "v4.1_demo.cpp", f"-DmaxLines={lines}", f"-Dboard=bitmap({bitmap&0xFFFFFFFFFFFFFFFF}llu,{bitmap>>64}llu)", f"-DpatternStr=\"{pattern}\"", f"-DallowHold={hold}", f"-Dglue={glue}", "-O3", "-std=c++11", "-o", "v4"],capture_output=True)
if (output.stderr!=b''):
    raise Exception("Compilation error:\n\t"+output.stderr.decode())

print("Finished compiling, running...")#
output=subprocess.run(["./v4"],capture_output=True)
#input(output.stdout.decode()+"\n")#debug
if (output.stderr!=b''):
    raise Exception("Program error:\n\t"+output.stderr.decode())

data=output.stdout.decode().split('\n')
seconds=int(data[0][:-3])/1e6
solutions=format(int(data[1].split(" ")[0]),",")
if (seconds>=60):#show minutes
    print(f"Found {solutions} solutions in {int(seconds//60)}:{round(seconds%60):02}s")
else:
    print(f"Found {solutions} solutions in {seconds%60} seconds")
print("Solutions have been written to output.txt")