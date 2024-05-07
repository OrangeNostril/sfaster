import sys
from py_fumen.decoder import decode
from py_fumen.field import Field, create_inner_field
from py_fumen.page import Page
import subprocess

try:#set board
    inputBoard=sys.argv[1]
except IndexError:#if no board provided
    with open("input.txt") as file:#read from input.txt
        inputBoard=file.read()
    if (inputBoard==''):#if nothing provided in input.txt
        inputBoard="v115@vhAAgH"#blank board

if ("v115" in inputBoard):#fumen to string
    inputBoard = decode(inputBoard)[0].get_field().string()[:-11]#doesn't do glued inputs (for now)
inputBoard=inputBoard.replace("\n","")#remove \n
if (len(inputBoard)%10!=0):
    print("Please ensure your board has a multiple of 10 spaces in it.")
    exit(1)

lines=len(inputBoard)//10#determine clear height
if (lines>4):
    print("Warning: this program currently only solves 4L PCs. Only the bottom four rows will be considered.")
    inputBoard=inputBoard[-40:]
elif (lines<4):
    inputBoard="".join(["_"]*(40-10*lines))+inputBoard
lines=4#for now
print("Board:\n"+inputBoard[:10]+"\n"+inputBoard[10:20]+"\n"+inputBoard[20:30]+"\n"+inputBoard[30:]+"\nCreating bitmap...",flush=True)#

inputBoard="".join([inputBoard[i:i+10] for i in range(len(inputBoard)-10,-10,-10)])#flip rows (v4 reads bottom to top)
bitmap=0
for i in range(len(inputBoard)):
    if (inputBoard[i]!='_'):
        bitmap|=1<<i<<(i//10)

print("Bitmap created\nCompiling finder...",flush=True)#
output=subprocess.run(["g++", "v4_demo.cpp", f"-DmaxLines={lines}", f"-Dboard={bitmap}llu", "-O3", "-o", "v4"],shell=True,capture_output=True)
if (output.stderr!=b''):
    print(str(output.stderr)[2:-1])
    exit(2)#error
print("Finished compiling, running...",flush=True)#
output=subprocess.run(["v4"],shell=True,capture_output=True)
if (output.stderr!=b''):
    print(str(output.stderr)[2:-1])
    exit(2)#error
data=str(output.stdout).split('\\r\\n')
seconds=int(data[0][2:-3])/1e6
solutions=format(int(data[1].split(" ")[0]),",")
if (seconds//60):#show minutes
    print(f"Found {solutions} solutions in {int(seconds//60)}:{round(seconds%60):02}s")
else:
    print(f"Found {solutions} solutions in {seconds%60} seconds")
print("Solutions have been written to output.txt")