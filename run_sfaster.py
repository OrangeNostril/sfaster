import argparse
from py_fumen.decoder import decode
from py_fumen.field import Field, create_inner_field
from py_fumen.page import Page
import subprocess

def parsePattern(pattern:str) -> str:#DEFINITELY not optimal, though it's a good template for when I eventually redo this in C++
    pieceBits = {c: 1<<i for i, c in enumerate("IJLOSTZ")}  #each character is given a unique bit
    pattern = pattern.replace("!","p7").replace("*","[IJLOSTZ]").replace(",","")
    patternNodes = []
    nodeMap=0
    inBrackets = False
    i=0
    while i<len(pattern):
        c=pattern[i]
        if c in "IJLOSTZ":
            if not inBrackets:
                patternNodes.append((pieceBits[c],1))
            elif ((nodeMap&pieceBits[c])):
                raise Exception("Error: Duplicate pieces in brackets in pattern input")
            else:
                nodeMap|=pieceBits[c]
        elif c=="[":
            if inBrackets:#no nested brackets allowed
                raise Exception("Error: Nested brackets in pattern input")
            inBrackets=True
        elif c=="]":
            if not inBrackets:#unmatched right bracket
                raise Exception("Error: Unmatched right bracket in pattern input")
            if (i+1>=len(pattern) or pattern[i+1]!='p'):
                patternNodes.append((nodeMap,1))#no p# defaults to 1
            elif i+2>=len(pattern) or pattern[i+1]!='p' or pattern[i+2] not in "01234567" or (i+3<len(pattern) and pattern[i+3] in "0123456789"):#correct format eg: [TLOZ]p4 (number after p must be 0-7)
                raise Exception("Error: Missing or invalid pick number in pattern input")
            else:
                if (pattern[i+2]!='0'):
                    patternNodes.append((nodeMap,int(pattern[i+2])))
                i+=2
            nodeMap=0#reset for next time its used
            inBrackets=False
        else:
            raise Exception("Error: Unexpected or out of place character in pattern input")
        i+=1
    if inBrackets:#unmatched left bracket
        raise Exception("Error: unmatched left bracket in pattern input")
    return "{"+",".join(f"patternNode({node[0]},{node[1]})" for node in patternNodes)+"}"

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

parser = argparse.ArgumentParser(description='Process some arguments.')

parser.add_argument('inputBoard', nargs='?', default='', help='Input board string')
parser.add_argument('-t', '--tetfu', help='Input board string')
parser.add_argument('-c', '--clear-line', type=int, choices=range(1,11), default=4, help='Number of lines to clear (1-10)')
parser.add_argument('-H', '--hold', choices=['avoid', 'use'], default="use",help='Hold piece preference')
parser.add_argument('-p', '--patterns', type=parsePattern, help='Pattern string to parse')
parser.add_argument('-s', '--split', choices=['yes', 'no'], default="yes", help='Split preference')
parser.add_argument('-d', '--drop', choices=['jstris180', 'tetrio180','soft','softdrop'], default="soft", help='Specify movement abilities')

args = parser.parse_args()

lines = args.clear_line

if args.tetfu:
    inputBoard = args.tetfu
else:
    inputBoard = args.inputBoard
inputBoard = parseInputBoard(inputBoard,lines)

if not args.patterns:
    pattern=",".join(["patternNode(127,1)"]*(5*lines//2 + 3))#aka *p1*p1*p1*p1...
else:
    pattern = args.patterns

hold = "false" if args.hold=="avoid" else "true"
glue = "true" if args.split=="yes" else "false"

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
#print("g++", "v4.1_demo.cpp", f"-DmaxLines={lines}", f"-Dboard=bitmap({bitmap&0xFFFFFFFFFFFFFFFF}llu,{bitmap>>64}llu)", "-O3", "-o", "v4")
if (load180Kicks!=""):
    output=subprocess.run(["g++", "v4.1_demo.cpp", f"-DmaxLines={lines}", f"-Dboard=bitmap({bitmap&0xFFFFFFFFFFFFFFFF}llu,{bitmap>>64}llu)", f"-DinputPattern={pattern}", f"-DallowHold={hold}", f"-Dglue={glue}", load180Kicks, "-O3", "-o", "v4"],shell=True,capture_output=True)
else:
    output=subprocess.run(["g++", "v4.1_demo.cpp", f"-DmaxLines={lines}", f"-Dboard=bitmap({bitmap&0xFFFFFFFFFFFFFFFF}llu,{bitmap>>64}llu)", f"-DinputPattern={pattern}", f"-DallowHold={hold}", f"-Dglue={glue}", "-O3", "-o", "v4"],shell=True,capture_output=True)
if (output.stderr!=b''):
    raise Exception("Compilation error:\n\t"+output.stderr.decode())

print("Finished compiling, running...")#
output=subprocess.run(["v4"],shell=True,capture_output=True)
#input(output.stdout.decode()+"\n")#debug
if (output.stderr!=b''):
    raise Exception("Program error:\n\t"+output.stderr.decode())

data=str(output.stdout).split('\\r\\n')
seconds=int(data[0][2:-3])/1e6
solutions=format(int(data[1].split(" ")[0]),",")
if (seconds//60):#show minutes
    print(f"Found {solutions} solutions in {int(seconds//60)}:{round(seconds%60):02}s")
else:
    print(f"Found {solutions} solutions in {seconds%60} seconds")
print("Solutions have been written to output.txt")