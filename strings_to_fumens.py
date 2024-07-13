from py_fumen.encoder import encode
from py_fumen.field import Field, create_inner_field
from py_fumen.page import Page

print("Reading from output.txt...",flush=True)#
with open("output.txt") as inFile:#generatedBoards.txt
    inputStrings=inFile.readlines()
print("Converting to fumens...")

outputStrings=[]
for i in inputStrings:
    pages = []
    pages.append(
        Page(field=create_inner_field(Field.create(
            i,
            '__________',
        )),
        comment=''))
    outputStrings.append(str(encode(pages)+"\n"))
print("Writing...")#
with open("output_fumens.txt",'w') as outFile:#generatedBoards_fumens.txt
    outFile.writelines(outputStrings)
print("Finishing writing, exiting...")#

#outputStrings=[str(encode([Page(field=create_inner_field(Field.create(sol,'__________',)),comment=''))])+"\n") for sol in inputStrings]