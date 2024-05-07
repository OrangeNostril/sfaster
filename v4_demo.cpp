//an implementation of v4 that's hopefully a lot more human readible and stuff
/*Currently (probably) gets every glueable solution with no false positives*/

/*TODO
  [ ] elegant inputter
  [X] arrays to vectors? (or better, variablize maxLines)
  [ ] taking bags into consideration
  [ ] <4 line PCs
  [ ] >4 line PCs
  bonus:
  [ ] fumenifier option?
  [ ] varaiblize writestring template to allow for non-gray minos
*/
#include <chrono>//timer
using namespace std::chrono;
unsigned long long debugCounter=0;

#include<stdio.h>
#include<iostream>
#include<fstream>
#include<vector>
#include<array>
#include<list>
#include<cstring>

#ifndef maxLines
#define maxLines 4
#endif

#ifndef board//starting board
#define board 0
#endif

#define wall 0x80100200400801llu//6 mino vertical line
#define playfield ((bitmap)((-1ll)<<(11*maxLines)|(wall<<10)))//everything out of bounds is 1

struct bitmap{
    unsigned long long val;
    bitmap() {}
    bitmap(unsigned long long v) : val(v) {}
    operator unsigned long long() const {
        return val;
    }
    bitmap operator&(const bitmap& shape) const {
        return bitmap(val & shape.val);
    }
    bitmap operator|(const bitmap& shape) const {
        return bitmap(val | shape.val);
    }
    bitmap operator^(const bitmap& shape) const {
        return bitmap(val ^ shape.val);
    }
    void operator&=(const bitmap& shape) {
        val &= shape.val;
    }
    void operator|=(const bitmap& shape) {
        val |= shape.val;
    }
    void operator^=(const bitmap& shape) {
        val ^= shape.val;
    }
    bitmap operator~() const {
        return bitmap(~val);
    }
    template <typename anyInt> bitmap operator<<(anyInt dist) const {
        return bitmap(val<<dist);
    }
    template <typename anyInt> bitmap operator>>(anyInt dist) const {
        return bitmap(val>>dist);
    }
    template <typename anyInt> void operator<<=(anyInt dist) {
        val <<= dist;
    }
    template <typename anyInt> void operator>>=(anyInt dist) {
        val >>= dist;
    }
    template <typename anyInt> bool operator()(anyInt ind) const {
        return (val>>ind & 1);
    }
    //eventually should probably override == too
};

const bitmap shapes[19] = { bitmap(0x200C01llu),bitmap(0x801801llu),bitmap(0x3801llu),bitmap(0x600801llu),bitmap(0xE01llu),bitmap(0xC00801llu),bitmap(0x1C01llu),bitmap(0x401801llu),bitmap(0x400C01llu),bitmap(0x200400801llu),bitmap(0x3003llu),bitmap(0xC03llu),bitmap(0x400803llu),bitmap(0x801003llu),bitmap(0x1803llu),bitmap(0x2007llu),bitmap(0x807llu),bitmap(0x1007llu),bitmap(0xFllu) };
//key: {Sr1,Zr1,Lr1,Lr2,Jr3,Jr2,Tr0,Tr3,Tr1,Ir0,Sr0,Zr0,Lr0,Jr0,Or0,Lr3,Jr1,Tr2,Ir1}

struct piece{//or part of piece
    bitmap mat;
    char mino;//shape index
    char origin;//initial row (only matters for top splits)
    //maybe in the future fragments can point to pieceList or something
    piece(bitmap m = 0, char mn = '\0', char org = '\0') : mat(m), mino(mn), origin(org) {}//chatgpt solution
    //piece(bitmap ma, char mi, char ori) : mat(ma), mino(mi), origin(ori) {}
};

unsigned long long solCount = 0;//tag: countSolutions

void printMatrix(bitmap matrix, char rowN=-1) {
    if (rowN==-1){//default case
        rowN=(maxLines>5?5:maxLines);//can only cleanly print 5 rows (for now)
    }
    char tracer = 11*(rowN-1);//rowN=how many rows to prints (note: can't print more than (almost) 6 rows)
    printf("___________\n");
    while (tracer >= 0) {
        printf("%c%s", (tracer>63 ? '#' : (matrix&(bitmap(1llu)<<tracer) ? 'X' : (tracer%11==10 ? '|' : '-'))), (tracer%11==10 ? "\n" : ""));
        tracer = ((tracer + 1) % 11 ? tracer + 1 : tracer - 21);//was :tracer-19
    }//key: | = (empty) wall space, - = empty space, X = filled space, # = exceeds max size (unknown)
}
std::ofstream outFile;
void writeSolution(std::vector<piece>& pieceList){//stringifies and writes one solution to file
    //maybe make asynchronous?
    std::string solStr(10*maxLines,'X');
    for (int r=0;r<maxLines;r++){
        for (int c=0;c<10;c++){
            for (auto p:pieceList){
                if (p.mat(r*11+c)){
                    solStr[(maxLines-r-1)*10+c]=p.mino;//flip solution (fumen reads top-bottom)
                    break;//0-1 minos per square obv
                }
            }
        }
    }
    //std::ofstream outFile ("output.txt",std::ios::app);//append to file
    if (outFile.is_open()){
        outFile << solStr << '\n';
        //outFile.close();
    }
    else{
        //printf("error writing to file\n");
        throw std::runtime_error("Error writing to output file.");
    }
}

void findSolutions(bitmap matrix, int tracer, std::array<std::list<piece>,10>& fragments, std::array<int,10>& heights, std::vector<piece>& pieceList, std::array<std::array<char,maxLines>,maxLines>& clearOrder){    
    //printMatrix(matrix,6);//
    for (int i=0;i<maxLines;i++){//ensure line clear order isn't circular
        for (int j=i+1;j<maxLines;j++){
            if (clearOrder[i][j] && clearOrder[j][i]){
                return;//fail
            }
        }
    }
    if (matrix==(bitmap)0xFFFFFFFFFFFFFFFFllu){
        //solutions.push_back(pieceList);
        writeSolution(pieceList);
        solCount++;
        return;//success
    }
    while (matrix(tracer)) {
        tracer++;
    }
    //guaranteed up/right open (except gray minos)
    int col=tracer%11;
    char row=tracer/11;
    pieceList.emplace_back();
    for (auto it=fragments[col].begin();it!=fragments[col].end();it++){
        if (!(matrix>>tracer & bitmap(it->mat))){
            piece save=*it;//copying whole piece? change later
            auto saveSpot = next(it);
            //note: actually have to save the node
            std::list<piece> helper;
            //TODO: special cases move node to new list instead of just copying?
            if (save.mat==bitmap(0x1003)){//special case: top of Zr1, Tr1
                //it=fragments[col].erase(it);//it temporarily it.next()
                helper.splice(helper.begin(),fragments[col],it);
                fragments[col+1].emplace_front(bitmap(0x1),(char)(it->mino),row);
            }
            else if (save.mat==bitmap(0xC01)){//special case: top of Lr2
                helper.splice(helper.begin(),fragments[col],it);
                fragments[col-1].emplace_front(bitmap(0x3),(char)(it->mino),row);
            }
            else if (save.mat>>11){//crosses into higher rows
                it->mat>>=11;
                it->origin=row;
            }
            else{
                //it=fragments[col].erase(it);//it temporarily it.next()
                helper.splice(helper.begin(),fragments[col],it);
            }

            pieceList.back()=save;//copy whole piece (for now)
            pieceList.back().mat=(save.mat&(bitmap)0x3FFllu)<<tracer;//adjusting mat
            for (int i=save.origin+1;i<row;i++){
                //[after][before]
                clearOrder[save.origin][i]++;
                clearOrder[row][i]++;
            }
            findSolutions(matrix|(bitmap)((save.mat&(bitmap)0x3FFllu)<<tracer), tracer+1, fragments, heights, pieceList, clearOrder);
            for (int i=save.origin+1;i<row;i++){
                //[after][before]
                clearOrder[save.origin][i]--;
                clearOrder[row][i]--;
            }

            if (save.mat==bitmap(0x1003)){//special case: top of Zr1, Tr1
                //fragments[col].insert(it,save);//put it back where it was
                fragments[col].splice(saveSpot,helper,helper.begin());
                fragments[col+1].pop_front();
            }
            else if (save.mat==bitmap(0xC01)){//special case: top of Lr2
                fragments[col].splice(saveSpot,helper,helper.begin());
                fragments[col-1].pop_front();
            }
            else if (save.mat>>11){
                //*it=save;
                it->mat=save.mat;
                it->origin=save.origin;
            }
            else{
                //fragments[col].insert(it,save);//put it back where it was
                fragments[col].splice(saveSpot,helper,helper.begin());
            }
        }
    }

    if (heights[col]==maxLines){//ie: fragments have dibs
        pieceList.pop_back();
        return;
    }
    pieceList.back().origin=row;
    //1-tall: Ir1, Jr0, Sr0, Tr2, Lr3
    if (col<7 && heights[col+1]+1<=maxLines && heights[col+2]+1<=maxLines && heights[col+3]+1<=maxLines){//Ir1
        heights[col]++;
        heights[col+1]++;
        heights[col+2]++;
        heights[col+3]++;
        pieceList.back().mat=(bitmap(0xF)<<tracer);
        pieceList.back().mino='I';
        findSolutions(matrix|(bitmap(0xF)<<tracer), tracer+4, fragments, heights, pieceList, clearOrder);//4 minos
        heights[col]--;
        heights[col+1]--;
        heights[col+2]--;
        heights[col+3]--;
    }
    if (col<9 && heights[col+1]+3<=maxLines){//Jr0
        heights[col]++;
        heights[col+1]+=3;
        pieceList.back().mat=(bitmap(0x3)<<tracer);
        pieceList.back().mino='J';
        fragments[col+1].emplace_front(bitmap(0x801),'J',row);
        findSolutions(matrix|(bitmap(0x3)<<tracer), tracer+2, fragments, heights, pieceList, clearOrder);//2 minos
        fragments[col+1].pop_front();
        heights[col]--;
        heights[col+1]-=3;
    }
    if (col<8 && heights[col+1]+2<=maxLines && heights[col+2]+1<=maxLines){//Sr0, Tr2
        heights[col]++;
        heights[col+1]+=2;
        heights[col+2]++;
        pieceList.back().mat=(bitmap(0x3)<<tracer);
        pieceList.back().mino='S';
        fragments[col+1].emplace_front(bitmap(0x3),'S',row);
        findSolutions(matrix|(bitmap(0x3)<<tracer), tracer+2, fragments, heights, pieceList, clearOrder);//Sr0 - 2 minos
        pieceList.back().mat=(bitmap(0x7)<<tracer);
        pieceList.back().mino='T';
        fragments[col+1].front().mat=0x1;
        fragments[col+1].front().mino='T';
        findSolutions(matrix|(bitmap(0x7)<<tracer), tracer+3, fragments, heights, pieceList, clearOrder);//Tr2 - 3 minos
        fragments[col+1].pop_front();
        heights[col]--;
        heights[col+1]-=2;
        heights[col+2]--;
    }
    if (col<8 && heights[col+1]+1<=maxLines && heights[col+2]+2<=maxLines){//Lr3
        heights[col]++;
        heights[col+1]++;
        heights[col+2]+=2;
        pieceList.back().mat=(bitmap(0x7)<<tracer);
        pieceList.back().mino='L';
        fragments[col+2].emplace_front(bitmap(0x1),'L',row);
        findSolutions(matrix|(bitmap(0x7)<<tracer), tracer+3, fragments, heights, pieceList, clearOrder);//Lr3 - 3 minos
        fragments[col+2].pop_front();
        heights[col]--;
        heights[col+1]--;
        heights[col+2]-=2;
    }
    if (heights[col]+2<=maxLines){//Jr3, Sr1, Zr0, Tr0, Or0, Zr1, Jr1, Lr1
        if (col>=2 && heights[col-2]+1<=maxLines && heights[col-1]+1<=maxLines){//Jr3
            heights[col-2]++;
            heights[col-1]++;
            heights[col]+=2;
            pieceList.back().mat=(bitmap(0x1)<<tracer);
            pieceList.back().mino='J';
            fragments[col-2].emplace_front(bitmap(0x7),'J',row);
            findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, clearOrder);//1 mino
            fragments[col-2].pop_front();
            heights[col-2]--;
            heights[col-1]--;
            heights[col]-=2;
        }
        if (col>=1 && heights[col-1]+2<=maxLines){//Sr1
            heights[col-1]+=2;
            heights[col]+=2;
            pieceList.back().mat=(bitmap(0x1)<<tracer);
            pieceList.back().mino='S';
            fragments[col-1].emplace_front(bitmap(0x803),'S',row);
            findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, clearOrder);//1 mino
            fragments[col-1].pop_front();
            heights[col-1]-=2;
            heights[col]-=2;
        }
        if (col>=1 && heights[col-1]+1<=maxLines && heights[col+1]+1<=maxLines){//Zr0
            heights[col-1]++;
            heights[col]+=2;
            heights[col+1]++;
            pieceList.back().mat=(bitmap(0x3)<<tracer);
            pieceList.back().mino='Z';
            fragments[col-1].emplace_front(bitmap(0x3),'Z',row);
            findSolutions(matrix|(bitmap(0x3)<<tracer), tracer+2, fragments, heights, pieceList, clearOrder);//2 minos
            fragments[col-1].pop_front();
            heights[col-1]--;
            heights[col]-=2;
            heights[col+1]--;
        }
        if (col>=1 && heights[col-1]+1<=maxLines && col<9 && heights[col+1]+1<=maxLines){//Tr0
            heights[col-1]++;
            heights[col]+=2;
            heights[col+1]++;
            pieceList.back().mat=(bitmap(0x1)<<tracer);
            pieceList.back().mino='T';
            fragments[col-1].emplace_front(bitmap(0x7),'T',row);
            findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, clearOrder);//1 mino
            fragments[col-1].pop_front();
            heights[col-1]--;
            heights[col]-=2;
            heights[col+1]--;
        }
        if (col<9 && heights[col+1]+2<=maxLines){//Or0
            heights[col]+=2;
            heights[col+1]+=2;
            pieceList.back().mat=(bitmap(0x3)<<tracer);
            pieceList.back().mino='O';
            fragments[col].emplace_front(bitmap(0x3),'O',row);
            findSolutions(matrix|(bitmap(0x3)<<tracer), tracer+2, fragments, heights, pieceList, clearOrder);//2 minos
            fragments[col].pop_front();
            heights[col]-=2;
            heights[col+1]-=2;
        }
        if (col<9 && heights[col+1]+2<=maxLines){//Zr1
            heights[col+1]+=2;
            heights[col]+=2;
            pieceList.back().mat=(bitmap(0x1)<<tracer);
            pieceList.back().mino='Z';
            fragments[col].emplace_front(bitmap(0x1003),'Z',row);
            findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, clearOrder);//1 mino
            fragments[col].pop_front();
            heights[col+1]-=2;
            heights[col]-=2;
        }
        if (col<8 && heights[col+2]+1<=maxLines && heights[col+1]+1<=maxLines){//Jr1, Lr1
            heights[col+2]++;
            heights[col+1]++;
            heights[col]+=2;
            pieceList.back().mat=(bitmap(0x7)<<tracer);
            pieceList.back().mino='J';
            fragments[col].emplace_front(bitmap(0x1),'J',row);
            findSolutions(matrix|(bitmap(0x7)<<tracer), tracer+3, fragments, heights, pieceList, clearOrder);//Jr1 - 3 minos
            pieceList.back().mat=(bitmap(0x1)<<tracer);
            pieceList.back().mino='L';
            fragments[col].front().mat=0x7;
            fragments[col].front().mino='L';
            findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, clearOrder);//Lr1 - 1 mino
            fragments[col].pop_front();
            heights[col+2]--;
            heights[col+1]--;
            heights[col]-=2;
        }

        if (heights[col]+3<=maxLines){//Tr1, Lr2, Lr0, Tr3, Jr2
            if (col>=1 && heights[col-1]+1<=maxLines){//Tr1, Lr2
                heights[col-1]++;
                heights[col]+=3;
                pieceList.back().mat=(bitmap(0x1)<<tracer);
                pieceList.back().mino='T';
                fragments[col-1].emplace_front(bitmap(0x1003),'T',row);
                findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, clearOrder);//Tr1 - 1 mino
                fragments[col-1].pop_front();
                pieceList.back().mat=(bitmap(0x1)<<tracer);
                pieceList.back().mino='L';
                fragments[col].emplace_front(bitmap(0xC01),'L',row);
                findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, clearOrder);//Lr2 - 1 mino
                fragments[col].pop_front();
                heights[col-1]--;
                heights[col]-=3;
            }
            if (col<9 && heights[col+1]+1<=maxLines){//Lr0, Tr3, Jr2
                heights[col+1]++;
                heights[col]+=3;
                pieceList.back().mat=(bitmap(0x3)<<tracer);
                pieceList.back().mino='L';
                fragments[col].emplace_front(bitmap(0x801),'L',row);
                findSolutions(matrix|(bitmap(0x3)<<tracer), tracer+2, fragments, heights, pieceList, clearOrder);//Lr0 - 2 minos
                pieceList.back().mat=(bitmap(0x1)<<tracer);
                pieceList.back().mino='T';
                fragments[col].front().mat=0x803;
                fragments[col].front().mino='T';
                findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, clearOrder);//Tr3 - 1 mino
                pieceList.back().mat=(bitmap(0x1)<<tracer);
                pieceList.back().mino='J';
                fragments[col].front().mat=0x1801;
                fragments[col].front().mino='J';
                findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, clearOrder);//Jr2 - 1 mino
                fragments[col].pop_front();
                heights[col+1]--;
                heights[col]-=3;
            }

            if (heights[col]+4<=maxLines){//Ir0
                heights[col]+=4;
                pieceList.back().mat=(bitmap(0x1)<<tracer);
                pieceList.back().mino='I';
                fragments[col].emplace_front(bitmap(0x400801),'I',row);
                findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, clearOrder);//1 mino
                fragments[col].pop_front();
                heights[col]-=4;
            }
        }
    }
    pieceList.pop_back();
}

int main() {
    bitmap testMap = board;//defined at compile time
        //0xffffffe1fc3f87f0llu : 4x4
        //0xffffff81f03e07c0llu : 6x4
        //0xfe7fce01c0380700llu : 7x4
    testMap|=playfield;

    std::array<std::list<piece>,10> fragments;
    std::array<int,10> heights;
    for (int i=0;i<10;i++){
        heights[i]=__builtin_popcountll(0x200400801llu<<i & testMap);//number of gray minos in column
    }
    std::vector<piece> pieceList;
    std::vector< std::vector<piece> > solutions;
    std::array< std::array<char,maxLines> ,maxLines> clearOrder;
    for(int i=0;i<maxLines;i++){
        for(int j=0;j<maxLines;j++){
            clearOrder[i][j]=0;
        }
    }
    outFile.open("output.txt",std::ios::trunc);
    if (!outFile.is_open()){
        //printf("error opening file\n");
        //return 1;//error
        throw std::runtime_error("Error opening output file.");
    }

    //printf("0x%llxllu\n",testMap);//
    //printMatrix(testMap,6);//

    /*Timing findSolutions()*/
    auto timer1=high_resolution_clock::now();
    auto timer2=timer1;
    auto timer3=duration_cast<microseconds>(timer2-timer1);

    timer1=high_resolution_clock::now();
    findSolutions(testMap, 0, fragments, heights, pieceList, clearOrder);/* FUNCTION CALL */
    timer2=high_resolution_clock::now();
    timer3=duration_cast<microseconds>(timer2-timer1);
    printf("%llu us\n",timer3);//millionths of a second
    printf("%llu solutions\n",solCount);

    outFile.close();
    //printf("\nExiting...");//
    return 0;
}
/*pre-circular-dependency-pruning: 10 pieces => 292'909'915  4:40m (without -O3)
* post-pruning, just counting -O3: 10 pieces => 99'389'880 13:34s
* with (inefficient?) disc writing, -O3 => 12:21s (somehow)
*/

//golden number: 6L => 43606 solutions