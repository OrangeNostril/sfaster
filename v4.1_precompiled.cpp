//the code is fairly messy right now because it's going through a lot of changes, but everything works how it should

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
#include<map>
#include<set>

//#define wall 0x80100200400801llu//6 mino vertical line
#define wall bitmap(0x80100200400801llu,0x80100200400801llu<<2)//12 mino vertical line

//global variables (defined at start of main)
int maxLines;
//bitmap board; (declared after struct bitmap)
//bitmap playfield; (declared after struct bitmap)
//std::vector<patternNode> inputPattern; (declared after struct patternNode)
bool allowHold;
bool glue;//not yet used
bool convertToFumen;
bool enable180=false;//only one not guaranteed to be initialized
//std::array<std::vector<char>,4>& kickTable180; (declared and initialized after jstris180 and tetrio180 are)

struct bitmap{
    unsigned long long val[2];
    bitmap() {}
    bitmap(unsigned long long v){
        val[0] = v;
        val[1] = 0;
    }
    bitmap(unsigned long long x, unsigned long long y) {//lower bits in lower indices
        val[0] = x;
        val[1] = y;
    }
    unsigned long long operator[](int i) const {
        return val[i];
    }
    bitmap operator&(const bitmap& shape) const {
        return bitmap(val[0] & shape.val[0] , val[1] & shape.val[1]);
    }
    bitmap operator|(const bitmap& shape) const {
        return bitmap(val[0] | shape.val[0] , val[1] | shape.val[1]);
    }
    bitmap operator^(const bitmap& shape) const {
        return bitmap(val[0] ^ shape.val[0] , val[1] ^ shape.val[1]);
    }
    void operator&=(const bitmap& shape) {
        val[0] &= shape.val[0];
        val[1] &= shape.val[1];
    }
    void operator|=(const bitmap& shape) {
        val[0] |= shape.val[0];
        val[1] |= shape.val[1];
    }
    void operator^=(const bitmap& shape) {
        val[0] ^= shape.val[0];
        val[1] ^= shape.val[1];
    }
    bitmap operator~() const {
        return bitmap(~val[0] , ~val[1]);
    }
    template <typename anyInt> bitmap operator<<(anyInt dist) const {//the tricky ones
        //bitshifts over 128 is undefined behavior
        if (dist==0){//0 would cause undefined behavior
            return *this;//bitmap(val[0],val[1]);
        }
        if (dist>=64){
            return bitmap(0 , val[0]<<(dist-64));
        }
        return bitmap(val[0]<<dist , val[1]<<dist | val[0]>>(64-dist));
    }
    template <typename anyInt> bitmap operator>>(anyInt dist) const {
        //bitshifts over 128 is undefined behavior
        if (dist==0){//0 would cause undefined behavior
            return *this;//bitmap(val[0],val[1]);
        }
        if (dist>=64){
            return bitmap(val[1]>>(dist-64) , 0);
        }
        return bitmap(val[0]>>dist | val[1]<<(64-dist) , val[1]>>dist);
    }
    template <typename anyInt> void operator<<=(anyInt dist) {
        if (dist>=64){
            val[1]=val[0]<<(dist-64);
            val[0]=0;
        }
        else if (dist!=0){//0 would cause undefined behavior
            val[1]=val[1]<<dist | val[0]>>(64-dist);
            val[0]<<=dist;
        }
    }
    template <typename anyInt> void operator>>=(anyInt dist) {
        if (dist>=64){
            val[0]=val[1]>>(dist-64);
            val[1]=0;
        }
        else if (dist!=0){//0 would cause undefined behavior
            val[0]=val[0]>>dist | val[1]<<(64-dist);
            val[1]>>=dist;
        }
    }
    template <typename anyInt> bool operator()(anyInt ind) const {//quick-check one bit
        return (val[ind>>6]>>(ind&0x3F) & 1);
    }
    bool operator==(const bitmap& shape) const {
        return (val[0]==shape.val[0] && val[1]==shape.val[1]);
    }
    bool operator!() const {
        return (val[0]==0 && val[1]==0);
    }
};
bitmap board;
bitmap playfield;

struct piece{//or part of piece
    bitmap mat;
    int id;//stores info on piece type, srs rotation, place in matrix
    int filledMap;//touched rows
    piece() : mat(), id(), filledMap() {}
    piece(bitmap m, int i, int fM) : mat(m), id(i), filledMap(fM) {}
};

unsigned long long solCount = 0;//tag: countSolutions

void printMatrix(bitmap matrix, char rowN=maxLines) {
    int tracer = 11*(rowN-1);//rowN=how many rows to prints
    printf("___________\n");
    while (tracer >= 0) {
        //printf("%c%s", (tracer>63 ? '#' : (matrix&(bitmap(1llu)<<tracer) ? 'X' : (tracer%11==10 ? '|' : '-'))), (tracer%11==10 ? "\n" : ""));
        printf("%c%s", (tracer>=128 ? '#' : (matrix(tracer) ? 'X' : (tracer%11==10 ? '|' : '-'))), (tracer%11==10 ? "\n" : ""));
        tracer = ((tracer + 1) % 11 ? tracer + 1 : tracer - 21);//was :tracer-19
    }//key: | = (empty) wall space, - = empty space, X = filled space, # = exceeds max size (unknown)
}
std::ofstream outFile;
void writeFumen(std::string solStr){//converts str to fumen and writes to file
    static const std::map<char,char> pieceNum = {{'_',8},{'I',9},{'L',10},{'O',11},{'Z',12},{'T',13},{'J',14},{'S',15},{'X',16}};//unique to fumen
    //static const std::map<char,char> pieceNum = {{'_',8},{'Z',9},{'L',10},{'O',11},{'S',12},{'I',13},{'J',14},{'T',15},{'X',16}};//unique to fumen
    //block number, number in a row -1
    std::vector<std::array<int,2>> data;
    data.push_back(std::array<int,2>{8,229-(int)solStr.size()});//empty rows before solution
    for (char c:solStr){
        int mapped=pieceNum.at(c);
        if (mapped==data.back()[0]) data.back()[1]++;
        else data.push_back(std::array<int,2>{mapped,0});
    }
    if (data.back()[0]==8) data.back()[1]+=10;//extra empty row at end
    else data.push_back(std::array<int,2>{8,9});

    static const std::array<std::string,64> code = {"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z","a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z","0","1","2","3","4","5","6","7","8","9","+","/"}; 
    std::string outFumen = "v115@";
    int count=5;//need to insert a ? every 47 chars i guess
    for (auto& d : data){
        int num = 240*d[0]+d[1];
        outFumen.append(code[num&0x3F]);//%64
        if (outFumen.size()%47==0) outFumen.append("?");
        outFumen.append(code[num>>6]);// /64
        if (outFumen.size()%47==0) outFumen.append("?");
    }
    //outFumen.append("AgH");//using the right piece color system
    outFumen.append("A");
    if (outFumen.size()%47==0) outFumen.append("?");
    outFumen.append("g");
    if (outFumen.size()%47==0) outFumen.append("?");
    outFumen.append("H");
    if (outFumen.size()%47==0) outFumen.append("?");//fumen website does this

    if (outFile.is_open()){
        outFile << outFumen << '\n';
    }
    else{
        throw std::runtime_error("Error writing to output file.");
    }
}
void writeSolution(std::vector<piece>& pieceList){//stringifies and writes one solution to file
    std::string solStr(10*maxLines,'X');
    constexpr char key[7] = {'I','J','L','O','S','T','Z'};
    for (int r=0;r<maxLines;r++){
        for (int c=0;c<10;c++){
            for (auto p:pieceList){
                if (p.mat(r*11+c)){
                    solStr[(maxLines-r-1)*10+c]=key[p.id&0xFF];//flip solution (fumen reads top-bottom)
                    break;//0-1 minos per square obv
                }
            }
        }
    }
    if (convertToFumen){
        writeFumen(solStr);
    }
    else{
        if (outFile.is_open()){
            outFile << solStr << '\n';
        }
        else{
            throw std::runtime_error("Error writing to output file.");
        }
    }
}
bitmap rotations[7][4]={{0x3c00000llu,0x801002004llu,0x7800llu,0x400801002llu},{0x403800llu,0x1801002llu,0x3804llu,0x801003llu},{0x1003800llu,0x801006llu,0x3801llu,0xc01002llu},{0x1803000llu,0x1803000llu,0x1803000llu,0x1803000llu},{0x1801800llu,0x803004llu,0x3003llu,0x401802llu},{0x803800llu,0x803002llu,0x3802llu,0x801802llu},{0xc03000llu,0x1003002llu,0x1806llu,0x801801llu}};
//IJLOSTZ
//https://static.wikia.nocookie.net/tetrisconcept/images/3/3d/SRS-pieces.png/revision/latest?cb=20060626173148
constexpr char wallKicks[4][5]={//srs shifts
    {0,-1,-1+1*11,-2*11,-1-2*11},//0>>1
    {0,1,1-1*11,2*11,1+2*11},//1>>2
    {0,1,1+1*11,-2*11,1-2*11},//2>>3
    {0,-1,-1-1*11,2*11,-1+2*11}//3>>0
};//going the other way is just *=-1
constexpr char I_wallKicks[4][5]={//I pieces have a unique table
    {0,-2,1,-2-1*11,1+2*11},//0>>1
    {0,-1,2,-1+2*11,2-1*11},//1>>2
    {0,2,-1,2+1*11,-1-2*11},//2>>3
    {0,1,-2,1-2*11,-2+1*11}//3>>0
};
//https://tetris.fandom.com/wiki/SRS#Wall_Kicks

std::array<std::vector<char>,4> jstris180={
    std::vector<char>{0,1*11},//0>>2
    std::vector<char>{0,1},//1>>3
    std::vector<char>{0,-1*11},//2>>0
    std::vector<char>{0,-1}//3>>1
};
std::array<std::vector<char>,4> tetrio180={
    std::vector<char>{0,1*11,1+1*11,-1+1*11,1,-1},//0>>2
    std::vector<char>{0,-1*11,-1-1*11,1-1*11,-1,1},//2>>0
    std::vector<char>{0,1,1+2*11,1+1*11,2*11,1*11},//1>>3
    std::vector<char>{0,-1,-1+2*11,-1+1*11,2*11,1*11}//3>>1
};
std::array<std::vector<char>,4>& kickTable180=jstris180;

bitmap quickTest(char piece, char rot, int pos, bitmap matrix){//debug tool
    bitmap adjusted = rotations[piece][rot];
    if (pos>=0) return adjusted<<pos;
    else return adjusted>>-pos;
}

bool unplace(char piece, char rot, int pos, bitmap matrix, std::set<int>& dp){//returns whether the piece can be placed in the current matrix
    bitmap adjusted = rotations[piece][rot];

    if (pos>=0) adjusted<<=pos;
    else{
        if (pos>=-11){//ensure not going below the matrix
            if ((rotations[piece][rot]<<(pos+11))[0] & 0x7FF) return false;
        }
        else if ((rotations[piece][rot]>>(-pos-11))[0] & 0x7FF) return false;
        adjusted>>=-pos;
    }
    if (!!(adjusted&matrix)) return false;//if overlapping other pieces

    //if (!(adjusted&((1ll<<(11*maxLines))-1))) return true;//if entirely above maxLines
    if ((11*maxLines<64 && !(adjusted&((1llu<<(11*maxLines))-1)))
        || (11*maxLines>=64 && !adjusted[0] && !(adjusted[1]&((1llu<<(11*maxLines-64))-1)))
    ) return true;//if entirely above maxLines
    //if (pos>=11*maxLines) return true;
    if (dp.find(pos<<2|rot)!=dp.end()) return false;//already been here
    dp.insert(pos<<2|rot);

    //printf("0x%llxllu,",adjusted|matrix);//
    //printMatrix(adjusted|matrix,12);//

    if (unplace(piece,rot,pos+11,matrix,dp)) return true;//up
    if (!(adjusted&1) && unplace(piece,rot,pos-1,matrix,dp)) return true;//left
    if (unplace(piece,rot,pos+1,matrix,dp)) return true;//right
    
    //rotations (needs some fixing?)
    if (piece==3) return false;//O doesn't need to rotate
    const char (*kickTable)[5]=(piece==0?I_wallKicks:wallKicks);//I pieces have a unique kick table
    bitmap rotCW = rotations[piece][(rot+1)&3];
    bitmap rotCCW = rotations[piece][(rot-1)&3]; 
    if (pos>=0){
        rotCW<<=pos;
        rotCCW<<=pos;
    }
    else{
        rotCW>>=-pos;
        rotCCW>>=-pos;
    }

    for (int k=0;k<5;k++){//eg 1>>2 backwards (unrotating from 2 to 1)
        //int kick = kickTable[rot][k];
        bitmap kicked = rotCW;
        if (kickTable[rot][k]>=0) kicked<<=kickTable[rot][k];//double neg: 2>>1 table, reversed
        else kicked>>=(-kickTable[rot][k]);
        //if ((kick>=0 && !(rotCW<<kick & matrix)) || (kick<0 && !(rotCW>>(-kick) & matrix))){
        if (!(kicked & matrix)){
            //printf("\n0x%llxllu\n",kicked);//
            bool flipsBack=true;
            for (int k2=0;k2<k;k2++){//make sure it would rotate forwards to here
                //int kick2 = -kickTable[rot][k2];
                bitmap kicked2=rotations[piece][rot];
                if (pos+kickTable[rot][k]-kickTable[rot][k2]>=0) kicked2<<=pos+kickTable[rot][k]-kickTable[rot][k2];
                else{
                    if (pos+kickTable[rot][k]-kickTable[rot][k2]>=-11){//ensure not un-unkicking back below playfield
                        if (!!(rotations[piece][rot]<<(pos+kickTable[rot][k]-kickTable[rot][k2]+11) & 0x7FF)) continue;
                    }
                    else if (!!(rotations[piece][rot]>>(-pos-kickTable[rot][k]+kickTable[rot][k2]-11) & 0x7FF)) continue;
                    kicked2>>=-pos-kickTable[rot][k]+kickTable[rot][k2];
                }
                if (!(kicked2 & matrix)){//if it would've kicked here first
                    flipsBack=false;
                    break;
                }
            }
            if (flipsBack && unplace(piece,(rot+1)&3,pos+kickTable[rot][k],matrix,dp)) return true;
        }
    }
    for (int k=0;k<5;k++){//eg 1>>0 backwards (unrotating from 0 to 1)
        bitmap kicked = rotCCW;
        int kick = -kickTable[(rot-1)&3][k];
        if (kick>=0) kicked<<=kick;//double neg: 2>>1 table, reversed
        else kicked>>=(-kick);
        //if ((kick>=0 && !(rotCCW<<kick & matrix)) || (kick<0 && !(rotCCW>>(-kick) & matrix))){
        if (!(kicked & matrix)){
            //printf("\n0x%llxllu\n",kicked);//
            /*int kick2 = kickTable[(rot-1)&3][k];//
            unsigned long long kicked2=rotations[piece][rot];
            if (pos+kick+kick2>=0) kicked2<<=pos+kick+kick2;
            else kicked2>>=-pos-kick-kick2;
            printf("\nkick:%d,kick2:%d, 0x%llxllu == 0x%llxllu => %d",kick,kick2,adjusted,kicked2,adjusted==kicked2);//*/

            bool flipsBack=true;
            for (int k2=0;k2<k;k2++){//make sure it would rotate forwards to here
                int kick2 = kickTable[(rot-1)&3][k2];
                bitmap kicked2=rotations[piece][rot];
                if (pos+kick+kick2>=0) kicked2<<=pos+kick+kick2;
                else{
                    if (pos+kick+kick2>=-11){//ensure not going below the matrix
                        if (!!(rotations[piece][rot]<<(pos+kick+kick2+11) & 0x7FF)) continue;
                    }
                    else if (!!(rotations[piece][rot]>>(-pos-kick-kick2-11) & 0x7FF)) continue;
                    kicked2>>=-pos-kick-kick2;
                }
                if (!(kicked2 & matrix)){//if it would've kicked here first
                    flipsBack=false;
                    break;
                }
            }
            if (flipsBack && unplace(piece,(rot-1)&3,pos+kick,matrix,dp)) return true;
        }
    }

    //#define load180Kicks tetrio180//for testing
    if (!enable180) return false;
    //load180Kicks was #defined as jstris180 or tetrio180
    //std::array<std::vector<char>,4>& kickTable180 = load180Kicks;
    bitmap rot180 = rotations[piece][(rot+2)&3];
    if (pos>=0) rot180<<=pos;
    else rot180>>=-pos;
    for (int k=0;k<kickTable180[0].size();k++){
        bitmap kicked = rot180;
        int kick = -kickTable180[rot][k];//negative because going backwards
        if (kick>=0) kicked<<kick;
        else kicked>>-kick;
        if (!(kicked&matrix)){
            //if (piece==2) printf("i'm in\n");//

            bool flipsBack=true;
            for (int k2=0;k2<k;k2++){
                bitmap kicked2 = rotations[piece][rot];//forwards because un-going backwards B)
                int kick2 = kickTable180[(rot+2)&3][k2];//don't have to worry about CW/CCW here
                if (pos+kick+kick2>=0) kicked2<<=pos+kick+kick2;
                else{
                    if (pos+kick+kick2>=-11){//ensure not going below the matrix
                        if (!!(rotations[piece][rot]<<(pos+kick+kick2+11) & 0x7FF)) continue;
                    }
                    else if (!!(rotations[piece][rot]>>(-pos-kick-kick2-11) & 0x7FF)) continue;
                    kicked2>>=-pos-kick-kick2;
                }
                if (!(kicked2&matrix)){//if it would've kicked here first
                    flipsBack=false;
                    break;
                }
            }
            if (flipsBack && unplace(piece,(rot+2)&3,pos+kick,matrix,dp)) return true;
        }
    }

    return false;
}
bool unplace(char piece, char rot, int pos, bitmap matrix){//overloaded function
    std::set<int> dp;
    return (unplace(piece,rot,pos,matrix,dp));
}

std::map<int,std::vector<int>> startShifts = {//converts v4 coordinate system to guideline coordinate system
    {0,{-22,-11}}//Ir0 (srs rots)
    ,{0|1<<8,{-2,-1}}//Ir1
    ,{1,{-11}}//Jr0
    ,{1|1<<8,{-1}}//Jr1
    ,{1|2<<8,{-2}}//Jr2
    ,{1|3<<8,{0}}//Jr3
    ,{2,{-11}}//Lr0
    ,{2|1<<8,{-1}}//Lr1
    ,{2|2<<8,{0}}//Lr2
    ,{2|3<<8,{-1}}//Lr3
    ,{3,{-12}}//Or0
    ,{4,{-11,0}}//Sr0
    ,{4|1<<8,{-2,-1}}//Sr1
    ,{5,{-11}}//Tr0
    ,{5|1<<8,{-1}}//Tr1
    ,{5|2<<8,{-1}}//Tr2
    ,{5|3<<8,{-1}}//Tr3
    ,{6,{-12,-1}}//Zr0
    ,{6|1<<8,{-1,0}}//Zr1
};
struct patternNode{
    int from;//bitmap
    int pick;
    patternNode(int f, int p) : from(f), pick(p) {};
};
std::vector<patternNode> inputPattern;
//note: maybe in the future, it->second.mat can be a map of mats for different combinations of line clears
//bigger note: rn placedMapDP is interfering with bags (leaving out some solutions)
bool findPath(std::map<int,piece>& solution, bitmap matrix, int clearedRows, unsigned long long placedMap, std::set<unsigned long long>& placedMapDP, const std::vector<patternNode>::iterator& pattern,const std::vector<patternNode>::iterator& hold){
    //if (placedMapDP.find(placedMap)!=placedMapDP.end()) return false;//already tried combination
    if (placedMap==(1llu<<solution.size())-1) return true;//if all pieces placed
    //while (pattern->pick==0){pattern=next(pattern);}//should allow p0 (remove "const" from parameter)


    /*if (!(matrix==wall<<10)){
    //if (__builtin_popcountll(matrix[0])>6+4){//6 for empty (just wall)
        printf("pick %d from 0x%x, hold: 0x%x ",pattern->pick,pattern->from,hold->from);//debug
        printMatrix(matrix);//
        printf("");//
    }*/

    bitmap rowMask=0x3FF;
    int clearRowsPassed=0;//to help keep clearedRows consistent
    for (int i=0;i<maxLines;i++){//clear filled lines
        if (clearedRows>>i&1){
            clearRowsPassed++;
        }
        else if ((rowMask&matrix)==rowMask){//if only gray minos remaining on row
            bitmap lowerMask;
            if ((i-clearRowsPassed)*11>=64){
                lowerMask=bitmap((unsigned long long)-1llu,(1llu<<(11*(i-clearRowsPassed)-64))-1);
            }
            else{
                lowerMask=bitmap((1llu<<11*(i-clearRowsPassed))-1,0);
            }
            matrix&=~rowMask;
            matrix=(matrix&lowerMask)|(matrix&~lowerMask)>>11|wall<<10;
            clearedRows|=1<<i;
            clearRowsPassed++;
            //printMatrix(matrix);//debug
        }
        else{//if current line was not (just nor ever) cleared
            rowMask<<=11;
        }
    }
    
    int counter=0;
    for (auto it=solution.begin();it!=solution.end();it++,counter++){
        if (!((1<<(it->second.id&0xFF))&(pattern->from|hold->from))) continue;//if breaks piece order requirements
        if (placedMap>>counter&1) continue;//piece already placed
        //if (placedMapDP.find(placedMap|1ll<<counter)!=placedMapDP.end()) continue;//already tried combination
        if ((it->second.filledMap&clearedRows)!=it->second.filledMap) continue;//if it skips any lines that haven't been cleared yet

        char piece=it->second.id&0xFF;
        char srsRot=it->second.id>>8&3;//[0-3]
        int pos=it->second.id>>10;//tracer

        int tempRow=pos/11;
        int rowsToLower = __builtin_popcount((1<<tempRow)-1&clearedRows);
        pos-=11*rowsToLower;

        int srsPos=pos+startShifts[it->second.id&0x3FF][0];
        bitmap adjusted = rotations[piece][srsRot];//just to see if it's floating
        if (srsPos>=0) adjusted<<=srsPos;
        else adjusted>>=-srsPos;
        if (!(adjusted&(matrix<<11|0x3FF))) continue;//would be placing a floating piece
        if (unplace(piece,srsRot,pos+startShifts[it->second.id&0x3FF][0],matrix)
            || (startShifts[it->second.id&0x3FF].size()==2 && unplace(piece,srsRot+2,pos+startShifts[it->second.id&0x3FF][1],matrix))
        ){//if can be placed
            pattern->pick--;
            if (pattern!=hold && (1<<piece&hold->from)){//if in hold (and hold isn't from same patternNode)
                //hold->from^=1<<piece;//this could also remove from current pattern node
                if (pattern->pick==0){//taking last from patternNode
                    if (findPath(solution,matrix|adjusted,clearedRows,placedMap|1llu<<counter,placedMapDP,next(pattern),pattern)) return true;
                }
                else if (findPath(solution,matrix|adjusted,clearedRows,placedMap|1llu<<counter,placedMapDP,pattern,pattern)) return true;
                //hold->from^=1<<piece;
            }
            if (1<<piece&pattern->from){//if in pattern
                pattern->from^=1<<piece;//also removes from hold if hold is same patternNode
                if (pattern->pick==0){//used last from node
                    if (findPath(solution,matrix|adjusted,clearedRows,placedMap|1llu<<counter,placedMapDP,next(pattern),hold)) return true;
                }
                else if (findPath(solution,matrix|adjusted,clearedRows,placedMap|1llu<<counter,placedMapDP,pattern,hold)) return true;
                pattern->from^=1<<piece;
            }
            pattern->pick++;
        }
    }

    placedMapDP.insert(placedMap);
    return false;
}

bool checkSolution(std::vector<piece>& pieceList){//placing pieces FORWARD
    std::map<int,piece> solution;//later on this section will probably be part of pruneImpossible()
    for (auto it=pieceList.begin();it!=pieceList.end();it++){        
        if (solution.find(it->id)==solution.end()){
            solution[it->id].mat=0;//the rest are auto-initialized
        }
        solution[it->id].mat|=it->mat;
        solution[it->id].filledMap|=it->filledMap;//just = should also work
        solution[it->id].id|=it->id;//redundant but w/e
    }

    /*for (auto i:solution){//debug
        constexpr char key[7] = {'I','J','L','O','S','T','Z'};
        printf("%c %x ",key[i.first&0xFF],i.second.filledMap);
        printMatrix(i.second.mat);
    }*/

    for (auto it=solution.begin();it!=solution.end();it++){//change filledMaps to skippedMaps
        int skippedMap = -1<<__builtin_ctz(it->second.filledMap)&-1u>>__builtin_clz(it->second.filledMap)^it->second.filledMap;//bitmap of all skipped rows
        //groupedBySkipped[skippedMap].push_back(it->);
        it->second.filledMap=skippedMap;
    }

    bitmap matrix = board|wall<<10;
    std::set<unsigned long long> placedMapDP;//maps work for up to 25 lines
    std::vector<patternNode> pattern=inputPattern;
    pattern.push_back(patternNode(0,0));//prevent errors when last PatternNode is only in hold
    if (allowHold){
        if (--(pattern[0].pick)==0){
            return (findPath(solution,matrix,0,0,placedMapDP,next(pattern.begin()),pattern.begin()));//starting with first piece in hold instead of empty hold, same difference
        }
        else{
            return (findPath(solution,matrix,0,0,placedMapDP,pattern.begin(),pattern.begin()));//starting with first piece in hold instead of empty hold, same difference
        }
    }
    else{
        return (findPath(solution,matrix,0,0,placedMapDP,pattern.begin(),prev(pattern.end())));//making hold the (0,0) node to ensure it's never used
    }
}

void findSolutions(bitmap matrix, int tracer, std::array<std::list<piece>,10>& fragments, std::array<int,10>& heights, std::vector<piece>& pieceList, std::vector<int>& dependencyMap, std::vector<int>& dependencyMapFlipped, std::array<char,7>& pieceLimits){
    //printMatrix(matrix,12);//debug

    for (int i=0;i<maxLines;i++){
        //if A before B and B before A, circular dependency (impossible solution)
        if (dependencyMap[i]&dependencyMapFlipped[i]){//any(dependencyMap[after]>>before&1 && dependencyMapFlipped[before]<<after&1)
            return;//fail
        }
    }

    if (matrix==bitmap(0xFFFFFFFFFFFFFFFFllu,0xFFFFFFFFFFFFFFFFllu)){//can't do matrix[1]==0xFFFFFFFFFFFFFFFFllu if maxLines<=4
        //solutions.push_back(pieceList);
        //printf("new solution\n");//
        if (checkSolution(pieceList)){
            writeSolution(pieceList);//success
            solCount++;
        }
        return;
    }
    while (matrix(tracer)) {
        tracer++;
    }
    //guaranteed up/right open (except gray minos)
    int col=tracer%11;
    char row=tracer/11;
    pieceList.emplace_back();
    std::vector<int> dependencyMapBackup = dependencyMap;
    std::vector<int> dependencyMapFlippedBackup = dependencyMapFlipped;
    for (auto it=fragments[col].begin();it!=fragments[col].end();it++){
        if (!(matrix>>tracer & bitmap(it->mat))){
            it->filledMap|=1<<row;//easy to undo
            piece save=*it;//copying whole piece (for now?)
            auto saveSpot = next(it);
            //note: actually have to save the node
            std::list<piece> helper;
            //TODO: special cases move node to new list instead of just copying?
            if (save.mat==bitmap(0x1003)){//special case: top of Zr1, Tr1
                //it=fragments[col].erase(it);//it temporarily it.next()
                helper.splice(helper.begin(),fragments[col],it);
                fragments[col+1].emplace_front(bitmap(0x1),it->id,it->filledMap);
            }
            else if (save.mat==bitmap(0xC01)){//special case: top of Lr2
                helper.splice(helper.begin(),fragments[col],it);
                fragments[col-1].emplace_front(bitmap(0x3),it->id,it->filledMap);
            }
            else if (!!(save.mat>>11)){//crosses into higher rows
                it->mat>>=11;
                //it->filledMap=row;
            }
            else{//all of piece now placed
                //it=fragments[col].erase(it);//it temporarily it.next()
                helper.splice(helper.begin(),fragments[col],it);
            }

            pieceList.back()=save;//copy whole piece (for now?)
            pieceList.back().mat=(save.mat&(bitmap)0x3FFllu)<<tracer;//adjusting mat
            int skippedMap = -1<<__builtin_ctz(it->filledMap) ^ -2<<row ^ it->filledMap;//bitmap of all skipped rows
            for (int i=__builtin_ctz(it->filledMap);i<=row;i++){//all rows between first and latest row
                //dependencyMap[after]<<before
                //dependencyMapFlipped[before]<<after
                if (it->filledMap>>i&1){//if i is filled row
                    //i must be cleared after any skipped
                    dependencyMap[i]|=skippedMap;
                }
                else{//if i is skipped row
                    //i must be cleared before any filled
                    dependencyMapFlipped[i]|=it->filledMap;
                }
            }
            findSolutions(matrix|(bitmap)((save.mat&(bitmap)0x3FFllu)<<tracer), tracer+1, fragments, heights, pieceList, dependencyMap,dependencyMapFlipped,pieceLimits);
            dependencyMap=dependencyMapBackup;
            dependencyMapFlipped=dependencyMapFlippedBackup;
            it->filledMap^=1<<row;//told you
            if (save.mat==bitmap(0x1003)){//special case: top of Zr1, Tr1
                //fragments[col].insert(it,save);//put it back where it was
                fragments[col].splice(saveSpot,helper,helper.begin());
                fragments[col+1].pop_front();
            }
            else if (save.mat==bitmap(0xC01)){//special case: top of Lr2
                fragments[col].splice(saveSpot,helper,helper.begin());
                fragments[col-1].pop_front();
            }
            else if (!!(save.mat>>11)){
                //*it=save;
                it->mat=save.mat;
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
    
    pieceList.back().filledMap=1<<row;//origin=std::array<char,maxLines>{};

    //piece ID guide: piece | SRSrot<<8 | tracer<<10, eg: Sr1 placed on 15=> 4|1<<8|15<<10 (SRSrot matches r here)
        //if multiple SRSrot values possible, the lowest is chosen

    //1-tall: Ir1, Jr0, Sr0, Tr2, Lr3
    if (pieceLimits[0] && col<7 && heights[col+1]+1<=maxLines && heights[col+2]+1<=maxLines && heights[col+3]+1<=maxLines){//Ir1
        heights[col]++;
        heights[col+1]++;
        heights[col+2]++;
        heights[col+3]++;
        pieceList.back().mat=(bitmap(0xF)<<tracer);
        pieceList.back().id=tracer<<10;
        pieceLimits[0]--;
        findSolutions(matrix|(bitmap(0xF)<<tracer), tracer+4, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//4 minos
        pieceLimits[0]++;
        heights[col]--;
        heights[col+1]--;
        heights[col+2]--;
        heights[col+3]--;
    }
    if (pieceLimits[1] && col<9 && heights[col+1]+3<=maxLines){//Jr0
        heights[col]++;
        heights[col+1]+=3;
        pieceList.back().mat=(bitmap(0x3)<<tracer);
        pieceList.back().id=1|3<<8|tracer<<10;
        fragments[col+1].emplace_front(bitmap(0x801),1|3<<8|tracer<<10,1<<row);
        pieceLimits[1]--;
        findSolutions(matrix|(bitmap(0x3)<<tracer), tracer+2, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//2 minos
        pieceLimits[1]++;
        fragments[col+1].pop_front();
        heights[col]--;
        heights[col+1]-=3;
    }
    if (col<8 && heights[col+1]+2<=maxLines && heights[col+2]+1<=maxLines){//Sr0, Tr2
        heights[col]++;
        heights[col+1]+=2;
        heights[col+2]++;
        pieceList.back().mat=(bitmap(0x3)<<tracer);
        pieceList.back().id=4|tracer<<10;
        fragments[col+1].emplace_front(bitmap(0x3),4|tracer<<10,1<<row);
        if (pieceLimits[4]){
            pieceLimits[4]--;
            findSolutions(matrix|(bitmap(0x3)<<tracer), tracer+2, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//Sr0 - 2 minos
            pieceLimits[4]++;
        }
        pieceList.back().mat=(bitmap(0x7)<<tracer);
        pieceList.back().id=5|tracer<<10;
        fragments[col+1].front().mat=0x1;
        fragments[col+1].front().id=5|tracer<<10;
        if (pieceLimits[5]){
            pieceLimits[5]--;
            findSolutions(matrix|(bitmap(0x7)<<tracer), tracer+3, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//Tr2 - 3 minos
            pieceLimits[5]++;
        }
        fragments[col+1].pop_front();
        heights[col]--;
        heights[col+1]-=2;
        heights[col+2]--;
    }
    if (pieceLimits[2] && col<8 && heights[col+1]+1<=maxLines && heights[col+2]+2<=maxLines){//Lr3
        heights[col]++;
        heights[col+1]++;
        heights[col+2]+=2;
        pieceList.back().mat=(bitmap(0x7)<<tracer);
        pieceList.back().id=2|tracer<<10;
        fragments[col+2].emplace_front(bitmap(0x1),2|tracer<<10,1<<row);
        pieceLimits[2]--;
        findSolutions(matrix|(bitmap(0x7)<<tracer), tracer+3, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//Lr3 - 3 minos
        pieceLimits[2]++;
        fragments[col+2].pop_front();
        heights[col]--;
        heights[col+1]--;
        heights[col+2]-=2;
    }
    if (heights[col]+2<=maxLines){//Jr3, Sr1, Zr0, Tr0, Or0, Zr1, Jr1, Lr1
        if (pieceLimits[1] && col>=2 && heights[col-2]+1<=maxLines && heights[col-1]+1<=maxLines){//Jr3
            heights[col-2]++;
            heights[col-1]++;
            heights[col]+=2;
            pieceList.back().mat=(bitmap(0x1)<<tracer);
            pieceList.back().id=1|2<<8|tracer<<10;
            fragments[col-2].emplace_front(bitmap(0x7),1|2<<8|tracer<<10,1<<row);
            pieceLimits[1]--;
            findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//1 mino
            pieceLimits[1]++;
            fragments[col-2].pop_front();
            heights[col-2]--;
            heights[col-1]--;
            heights[col]-=2;
        }
        if (pieceLimits[4] && col>=1 && heights[col-1]+2<=maxLines){//Sr1
            heights[col-1]+=2;
            heights[col]+=2;
            pieceList.back().mat=(bitmap(0x1)<<tracer);
            pieceList.back().id=4|1<<8|tracer<<10;
            fragments[col-1].emplace_front(bitmap(0x803),4|1<<8|tracer<<10,1<<row);
            pieceLimits[4]--;
            findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//1 mino
            pieceLimits[4]++;
            fragments[col-1].pop_front();
            heights[col-1]-=2;
            heights[col]-=2;
        }
        if (pieceLimits[6] && col>=1 && heights[col-1]+1<=maxLines && heights[col+1]+1<=maxLines){//Zr0
            heights[col-1]++;
            heights[col]+=2;
            heights[col+1]++;
            pieceList.back().mat=(bitmap(0x3)<<tracer);
            pieceList.back().id=6|tracer<<10;
            fragments[col-1].emplace_front(bitmap(0x3),6|tracer<<10,1<<row);
            pieceLimits[6]--;
            findSolutions(matrix|(bitmap(0x3)<<tracer), tracer+2, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//2 minos
            pieceLimits[6]++;
            fragments[col-1].pop_front();
            heights[col-1]--;
            heights[col]-=2;
            heights[col+1]--;
        }
        if (pieceLimits[5] && col>=1 && heights[col-1]+1<=maxLines && col<9 && heights[col+1]+1<=maxLines){//Tr0
            heights[col-1]++;
            heights[col]+=2;
            heights[col+1]++;
            pieceList.back().mat=(bitmap(0x1)<<tracer);
            pieceList.back().id=5|2<<8|tracer<<10;
            fragments[col-1].emplace_front(bitmap(0x7),5|2<<8|tracer<<10,1<<row);
            pieceLimits[5]--;
            findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//1 mino
            pieceLimits[5]++;
            fragments[col-1].pop_front();
            heights[col-1]--;
            heights[col]-=2;
            heights[col+1]--;
        }
        if (pieceLimits[3] && col<9 && heights[col+1]+2<=maxLines){//Or0
            heights[col]+=2;
            heights[col+1]+=2;
            pieceList.back().mat=(bitmap(0x3)<<tracer);
            pieceList.back().id=3|tracer<<10;
            fragments[col].emplace_front(bitmap(0x3),3|tracer<<10,1<<row);
            pieceLimits[3]--;
            findSolutions(matrix|(bitmap(0x3)<<tracer), tracer+2, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//2 minos
            pieceLimits[3]++;
            fragments[col].pop_front();
            heights[col]-=2;
            heights[col+1]-=2;
        }
        if (pieceLimits[6] && col<9 && heights[col+1]+2<=maxLines){//Zr1
            heights[col+1]+=2;
            heights[col]+=2;
            pieceList.back().mat=(bitmap(0x1)<<tracer);
            pieceList.back().id=6|1<<8|tracer<<10;
            fragments[col].emplace_front(bitmap(0x1003),6|1<<8|tracer<<10,1<<row);
            pieceLimits[6]--;
            findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//1 mino
            pieceLimits[6]++;
            fragments[col].pop_front();
            heights[col+1]-=2;
            heights[col]-=2;
        }
        if (col<8 && heights[col+2]+1<=maxLines && heights[col+1]+1<=maxLines){//Jr1, Lr1
            heights[col+2]++;
            heights[col+1]++;
            heights[col]+=2;
            pieceList.back().mat=(bitmap(0x7)<<tracer);
            pieceList.back().id=1|tracer<<10;
            fragments[col].emplace_front(bitmap(0x1),1|tracer<<10,1<<row);
            if (pieceLimits[1]){
                pieceLimits[1]--;
                findSolutions(matrix|(bitmap(0x7)<<tracer), tracer+3, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//Jr1 - 3 minos
                pieceLimits[1]++;
            }
            pieceList.back().mat=(bitmap(0x1)<<tracer);
            pieceList.back().id=2|2<<8|tracer<<10;
            fragments[col].front().mat=0x7;
            fragments[col].front().id=2|2<<8|tracer<<10;
            if (pieceLimits[2]){
                pieceLimits[2]--;
                findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//Lr1 - 1 mino
                pieceLimits[2]++;
            }
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
                pieceList.back().id=5|3<<8|tracer<<10;
                fragments[col-1].emplace_front(bitmap(0x1003),5|3<<8|tracer<<10,1<<row);
                if (pieceLimits[5]){
                    pieceLimits[5]--;
                    findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//Tr1 - 1 mino
                    pieceLimits[5]++;
                }
                fragments[col-1].pop_front();
                pieceList.back().mat=(bitmap(0x1)<<tracer);
                pieceList.back().id=2|3<<8|tracer<<10;
                fragments[col].emplace_front(bitmap(0xC01),2|3<<8|tracer<<10,1<<row);
                if (pieceLimits[2]){
                    pieceLimits[2]--;
                    findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//Lr2 - 1 mino
                    pieceLimits[2]++;
                }
                fragments[col].pop_front();
                heights[col-1]--;
                heights[col]-=3;
            }
            if (col<9 && heights[col+1]+1<=maxLines){//Lr0, Tr3, Jr2
                heights[col+1]++;
                heights[col]+=3;
                pieceList.back().mat=(bitmap(0x3)<<tracer);
                pieceList.back().id=2|1<<8|tracer<<10;
                fragments[col].emplace_front(bitmap(0x801),2|1<<8|tracer<<10,1<<row);
                if (pieceLimits[2]){
                    pieceLimits[2]--;
                    findSolutions(matrix|(bitmap(0x3)<<tracer), tracer+2, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//Lr0 - 2 minos
                    pieceLimits[2]++;
                }
                pieceList.back().mat=(bitmap(0x1)<<tracer);
                pieceList.back().id=5|1<<8|tracer<<10;
                fragments[col].front().mat=0x803;
                fragments[col].front().id=5|1<<8|tracer<<10;
                if (pieceLimits[5]){
                    pieceLimits[5]--;
                    findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//Tr3 - 1 mino
                    pieceLimits[5]++;
                }
                pieceList.back().mat=(bitmap(0x1)<<tracer);
                pieceList.back().id=1|1<<8|tracer<<10;
                fragments[col].front().mat=0x1801;
                fragments[col].front().id=1|1<<8|tracer<<10;
                if (pieceLimits[1]){
                    pieceLimits[1]--;
                    findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//Jr2 - 1 mino
                    pieceLimits[1]++;
                }
                fragments[col].pop_front();
                heights[col+1]--;
                heights[col]-=3;
            }

            if (pieceLimits[0] && heights[col]+4<=maxLines){//Ir0
                heights[col]+=4;
                pieceList.back().mat=(bitmap(0x1)<<tracer);
                pieceList.back().id=1<<8|tracer<<10;
                fragments[col].emplace_front(bitmap(0x400801),1<<8|tracer<<10,1<<row);
                pieceLimits[0]--;
                findSolutions(matrix|(bitmap(0x1)<<tracer), tracer+1, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);//1 mino
                pieceLimits[0]++;
                fragments[col].pop_front();
                heights[col]-=4;
            }
        }
    }
    pieceList.pop_back();
}

void parsePattern(std::string pattern){
    const std::map<char,int> pieceBits = {{'I',1<<0},{'J',1<<1},{'L',1<<2},{'O',1<<3},{'S',1<<4},{'T',1<<5},{'Z',1<<6}};
    //pattern = pattern.replace("!","p7").replace("*","[IJLOSTZ]").replace(",","")
    size_t pos;
    while ((pos = pattern.find("!")) != std::string::npos) {//lazy solution (losing precious nanoseconds)
        pattern.replace(pos, 1, "p7");
    }
    while ((pos = pattern.find("*")) != std::string::npos) {
        pattern.replace(pos, 1, "[IJLOSTZ]");
    }
    std::vector<patternNode> patternNodes;
    int nodeMap=0;
    bool inBrackets = false;
    for (int i=0;i<pattern.size();i++){
        char c=pattern[i];
        if (c==',') continue;//bad inputs like *p,7 still accounted for
        auto charBit = pieceBits.find(c);
        if (charBit!=pieceBits.end()){
            if (!inBrackets) patternNodes.push_back(patternNode(charBit->second,1));
            else if (nodeMap&charBit->second) throw std::runtime_error("Error: Duplicate pieces in brackets in pattern input");
            else nodeMap|=charBit->second;
        }
        else if (c=='['){
            if (inBrackets) throw std::runtime_error("Error: Nested brackets in pattern input");
            inBrackets=true;
        }
        else if (c==']'){
            if (!inBrackets) throw std::runtime_error("Error: Unmatched right bracket in pattern input");
            if (i+1>=pattern.size() || pattern[i+1]!='p'){
                patternNodes.push_back(patternNode(nodeMap,1));//no p# defaults to 1
            }
            //else if (i+2>=len(pattern) or pattern[i+1]!='p' or pattern[i+2] not in "01234567" or (i+3<len(pattern) and pattern[i+3] in "0123456789")){//correct format eg: [TLOZ]p4 (number after p must be 0-7)
            else if (i+2>=pattern.size()
                || pattern[i+1]!='p'
                || pattern[i+2]>'7' || pattern[i+2]<'1'
                || (i+3<pattern.size() && pattern[i+3]<='9' && pattern[i+3]>='0')
            ){//correct format eg: [TLOZ]p4 (number after p must be 1-7)
                throw std::runtime_error("Error: Missing or invalid pick number in pattern input");
            }
            else{
                if (pattern[i+2]!='0') patternNodes.push_back(patternNode(nodeMap,pattern[i+2]-'0'));
                i+=2;
            }
            nodeMap=0;//reset for next time its used
            inBrackets=false;
        }
        else{
            throw std::runtime_error("Error: Unexpected or out of place character in pattern input");
        }
    }
    if (inBrackets) throw std::runtime_error("Error: unmatched left bracket in pattern input");

    inputPattern = patternNodes;
}
int main(int argc, char* argv[]) {//v4.1_compiled.exe board, pattern, maxLines, allowHold, glue, convertToFumen, load180Kicks
    if (argc<7) return 1;//for me
    int comma=0;//setting board
    while(argv[1][++comma]!=',');
    argv[1][comma++]=0;
    board = bitmap(strtoull(argv[1],nullptr,0),strtoull(argv[1]+comma,nullptr,0));
    parsePattern((std::string)argv[2]);//setting inputPattern
    maxLines = (argv[3][1]?10:argv[3][0]-'0');//setting maxLines
    if (maxLines>=6){//setting playfield (flipping all out-of-matrix bits)
        playfield=bitmap(0x80100200400801llu<<10 , ((unsigned long long)-1ll)<<(11*maxLines-64)|(0x80100200400801llu<<1));
    }
    else{
        playfield=bitmap(((unsigned long long)-1ll)<<(11*maxLines)|(0x80100200400801llu<<10) , (unsigned long long)-1ll);
    }
    allowHold = (argv[4][0]=='t');//setting allowHold
    glue = (argv[5][0]=='t');//setting glue
    convertToFumen = (argv[6][0]=='t');//setting convertToFumen
    if (argc==8){//setting kickTable180
        enable180 = true;
        if (argv[7][0]=='t') kickTable180 = tetrio180;//jstris180 by default
    }

    bitmap testMap = board;//defined at compile time

    std::array<int,10> heights;
    for (int i=0;i<10;i++){
        //heights[i]=__builtin_popcountll(wall<<i & testMap);//number of gray minos in column
        bitmap overlap=wall<<i & testMap;
        heights[i]=__builtin_popcountll(overlap[0])+__builtin_popcountll(overlap[1]);//number of gray minos in column
    }

    //printf("bitmap(0x%llxllu,0x%llxllu)\n",testMap[0],testMap[1]);//
    //printMatrix(testMap,12);//
    testMap|=playfield;//this means no more extra out of bounds minos

    std::array<std::list<piece>,10> fragments;
    std::vector<piece> pieceList;
    std::vector<int> dependencyMap(maxLines);
    std::vector<int> dependencyMapFlipped(maxLines);
    std::array<char,7> pieceLimits={};
    std::vector<patternNode> pattern = inputPattern;
    for (patternNode& node:pattern){//replace pattern with inputPattern?
        //printf("patternNode(%d,%d),",node.from,node.pick);//debug
        if (node.pick==1 && __builtin_popcount(node.from)==1){
            pieceLimits[__builtin_ctz(node.from)]++;
        }
        else if (node.pick==7){//hopefully unneeded when this gets the much-needed upgrade
            pieceLimits[0]++;
            pieceLimits[1]++;
            pieceLimits[2]++;
            pieceLimits[3]++;
            pieceLimits[4]++;
            pieceLimits[5]++;
            pieceLimits[6]++;
        }
        else{//suboptimal but working solution
            for (int i=0;i<7;i++){
                pieceLimits[i]+=node.from>>i&1;
            }
        }
    }

    /*printf("\n\n");//
    for (int i=0;i<7;i++){//debug
        printf("%c:%d, ","IJLOSTZ"[i],pieceLimits[i]);
    }printf("\n");*/

    outFile.open("output.txt",std::ios::trunc);
    if (!outFile.is_open()){
        throw std::runtime_error("Error opening output file.");
    }

    /*Timing findSolutions()*/
    auto timer1=high_resolution_clock::now();
    auto timer2=timer1;
    auto timer3=duration_cast<microseconds>(timer2-timer1);

    timer1=high_resolution_clock::now();
    findSolutions(testMap, 0, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);/* FUNCTION CALL */
    //std::thread thread_obj(findSolutions, testMap, 0, fragments, heights, pieceList, dependencyMap, dependencyMapFlipped, pieceLimits);
    timer2=high_resolution_clock::now();
    timer3=duration_cast<microseconds>(timer2-timer1);
    printf("%llu us\n",timer3.count());//millionths of a second
    printf("%llu solutions\n",solCount);

    outFile.close();
    //printf("Exiting...\n");//
    return 0;
}