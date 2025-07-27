#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>

using namespace std;

#define DISK_SIZE 512
int BS;

// Function to convert decimal to binary char
char decToBinary(int n) {
    return static_cast<char>(n);
}

// #define SYS_CALL
// ============================================================================
class fsInode {
    int fileSize;
    int block_in_use;

    int directBlock1;
    int directBlock2;
    int directBlock3;

    int singleInDirect;
    int doubleInDirect;
    int block_size;
    int final ;
    int blocksWithPointers ;


public:
    fsInode(int _block_size) {
        fileSize = 0;
        block_in_use = 0;
        block_size = _block_size;
        directBlock1 = -1;
        directBlock2 = -1;
        directBlock3 = -1;
        singleInDirect = -1;
        doubleInDirect = -1;
        final =-1;
        blocksWithPointers=0;

    }

// Getters
    int getFinal (){
        return this -> final;
    }
    void setFinal (int f ){
        this ->final = f ;
    }

    int getFileSize() const {
        return fileSize;
    }

    int getBlockInUse() const {
        return block_in_use;
    }

    int getDirectBlock1() const {
        return directBlock1;
    }

    int getDirectBlock2() const {
        return directBlock2;
    }

    int getDirectBlock3() const {
        return directBlock3;
    }

    int getSingleInDirect() const {
        return singleInDirect;
    }

    int getDoubleInDirect() const {
        return doubleInDirect;
    }
    int getBlocksWithPointers  (){
        return this -> blocksWithPointers;
    }
    //-----------------------------------------------
    // Setters
    void setFileSize(int size) {
        fileSize = size;
    }

    void setBlockInUse(int blocks) {
        block_in_use = blocks;
    }

    void setDirectBlock1(int block) {
        directBlock1 = block;
    }

    void setDirectBlock2(int block) {
        directBlock2 = block;
    }

    void setDirectBlock3(int block) {
        directBlock3 = block;
    }

    void setSingleInDirect(int block) {
        singleInDirect = block;
    }

    void setDoubleInDirect(int block) {
        doubleInDirect = block;
    }
    void setBlocksWithPointers (int num  ){
        this->blocksWithPointers= num ;
    }
};

// ============================================================================
class FileDescriptor {
    pair<string, fsInode*> file;
    bool inUse;

public:
    FileDescriptor(string FileName, fsInode* fsi) {
        file.first = FileName;
        file.second = fsi;
        inUse = true;
    }
    string getFileName() {
        return file.first;
    }
    fsInode* getInode() {

        return file.second;
    }

    int GetFileSize() {
        return file.second->getFileSize();

    }
    void setFileSiza (int f ){
        file.second->setFileSize(f );
    }

    void setFileName(string name){
        file.first=name ;
    }
    bool isInUse() {
        return (inUse);
    }
    void setInUse(bool _inUse) {
        inUse = _inUse ;
    }

};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
// ============================================================================
class fsDisk {
    FILE *sim_disk_fd;

    bool is_formated;
    int maxFS;

    int BitVectorSize;
    int *BitVector;

    map<string, fsInode *> MainDir;
    vector<FileDescriptor> OpenFileDescriptors;

public:
    //=======================================================================================================================
    fsDisk() {
        this->is_formated = false;
        this->maxFS = 0;            //max file size
        sim_disk_fd = fopen(DISK_SIM_FILE, "r+");
        assert(sim_disk_fd);
        for (int i = 0; i < DISK_SIZE; i++) {
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fwrite("\0", 1, 1, sim_disk_fd);
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);
    }

    void setMaxFS(int max) {
        this->maxFS = max;
    }

    int getMaxFS() {
        return this->maxFS;
    }


    //=======================================================================================================================
    void listAll() {
        int i = 0;
        for (auto it = begin(OpenFileDescriptors); it != end(OpenFileDescriptors); ++it) {
            cout << "index: " << i << ": FileName: " << it->getFileName() << " , isInUse: "
                 << it->isInUse() << " file Size: " << it->GetFileSize() << endl;
            i++;
        }
        char bufy;
        cout << "Disk content: '";
        for (i = 0; i < DISK_SIZE; i++) {
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fread(&bufy, 1, 1, sim_disk_fd);
            cout << bufy;
        }
        cout << "'" << endl;


    }

    //=======================================================================================================================
    void fsFormat(int blockSize = 4) {

        if (blockSize <= 1 || blockSize > DISK_SIZE) {
            perror("choose a relavent number for the block size ");
            return;
        }
        //-----------------------------------------------------------
        BS = blockSize;                                                       //Update the block size
        setMaxFS((3 + blockSize + blockSize * blockSize) * blockSize);           //Max file size
        this->is_formated = true;
        this->BitVectorSize = DISK_SIZE / blockSize;
        //-----------------------------------------------------------
        //Free the Bit vector
        if (BitVector!=NULL){
            free(BitVector);
        }
        //-----------------------------------------------------------
        //Initialize the BitVector
        this->BitVector = (int *) malloc(BitVectorSize * sizeof(int));
        for (int i = 0; i < BitVectorSize; i++) {
            BitVector[i] = 0;
        }
        //-----------------------------------------------------------
        // Iterate through MainDir and delete each fsInode
        for (auto& entry : MainDir) {
            delete entry.second; // Delete the fsInode
        }
        //-----------------------------------------------------------
        // Clear all entries in the map
        MainDir.clear();
        //-----------------------------------------------------------
        // Clear the vector after releasing resources
        OpenFileDescriptors.clear();
        //-----------------------------------------------------------
        //Put NULL in the disk
        for (int i = 0; i < DISK_SIZE; i++) {
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fwrite("\0", 1, 1, sim_disk_fd);
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);

    }
    //=======================================================================================================================
    int CreateFile(string fileName) {
        //check if the desk is formated
        if (!this->is_formated ) {
            perror("the disk has to be formated before ");
            return -1;
        }
        //-----------------------------------------------------------
        // Check if the file already exists in the MainDir
        if (MainDir.find(fileName) != MainDir.end()) {
            perror("File already exists.");
            return -1;
        }
        //-----------------------------------------------------------
        // Check if there's an available file descriptor
        int fd = -1;
        for (int i = 0; i < OpenFileDescriptors.size(); ++i) {
            if (!OpenFileDescriptors[i].isInUse()) {
                fd = i;
                break;
            }
        }
        //-----------------------------------------------------------
        //Push the vector for a new fd
        if (fd == -1 ){
            fd = OpenFileDescriptors.size();
            OpenFileDescriptors.push_back(FileDescriptor("", nullptr)); }
        //-----------------------------------------------------------
        // Create a new inode for the file
        fsInode *newInode = new fsInode(BS);
        //-----------------------------------------------------------
        //Add to the maindir map
        MainDir[fileName] = newInode;
        //-----------------------------------------------------------
        // Update the file descriptor entry with the new information
        OpenFileDescriptors[fd] = FileDescriptor(fileName, newInode);
        //-----------------------------------------------------------
        OpenFileDescriptors[fd].setInUse(true);

        return fd;  // Return the file descriptor
    }

    //=======================================================================================================================
    int OpenFile(string FileName) {
        //check if the desk is formated
        if (!this->is_formated) {
            perror("the disk has to be formated before ");
            return -1;
        }
        //-----------------------------------------------------------
        //Check if the file exists in the maindir
        if (!(MainDir.find(FileName) != MainDir.end())) {
            perror(" you have to create the file ,the file is not existed in the disk");
            return -1;
        }
        //-----------------------------------------------------------
        //Check if the file has already opened
        for (int  i=0 ; i < OpenFileDescriptors.size();i++){
            if (OpenFileDescriptors[i].getFileName()==FileName){
                if (OpenFileDescriptors[i].isInUse()){
                    perror ("The file is already opened ");
                    return -1 ;
                }
            }
        }
        //-----------------------------------------------------------
        //Check if the file exists in the fd
        int fd = -1;
        for (size_t i = 0; i < OpenFileDescriptors.size(); ++i) {
            if (!OpenFileDescriptors[i].isInUse()) {
                // Replace the closed file descriptor with the new one
                FileDescriptor newFileDesc(FileName, MainDir[FileName]);
                OpenFileDescriptors[i] = newFileDesc;
                OpenFileDescriptors[i].setInUse(true);
                fd = i;
                break;
            }
        }
        //-----------------------------------------------------------
        //Push the vector
        if (fd ==-1 ){
            fd = (int)OpenFileDescriptors.size();
            FileDescriptor newFileDesc(FileName, MainDir[FileName]);
            OpenFileDescriptors.push_back(newFileDesc);
        }
        return fd;
    }
    //=======================================================================================================================
    string CloseFile(int fd) {
        //check if the desk is formated
        if (!this->is_formated) {
            perror("the disk has to be formated before ");
            return "-1";
        }
        //-----------------------------------------------------------
        // Check if the file descriptor is valid
        if (fd < 0 || fd >= OpenFileDescriptors.size()) {
            cout << "Invalid file descriptor." << endl;
            return "-1";
        }
        //-----------------------------------------------------------
        // Check if the file descriptor is in use
        if (!OpenFileDescriptors[fd].isInUse() ) {
            cout << "File descriptor already has closed or deleted " << endl;
            return "-1";
        }
        //-----------------------------------------------------------
        // Get the file name from the fd
        string fileName = OpenFileDescriptors[fd].getFileName();
        //-----------------------------------------------------------
        // Set the file descriptor entry as not in use
        OpenFileDescriptors[fd].setInUse(false);
        //-----------------------------------------------------------
        return fileName;

    }

    //=======================================================================================================================
    int WriteToFile(int fd, char *buf, int len) {
        //check if the disk is formated
        if (!this->is_formated ) {
            perror("the disk has to be formated before ");
            return -1;
        }
        //-----------------------------------------------------------
        string str = buf;
        if (fd < 0 || fd >= OpenFileDescriptors.size()) {
            perror("invalid fd ");
            return -1;
        }
        //-----------------------------------------------------------
        //Check if the file is open
        if (!OpenFileDescriptors[fd].isInUse()) {
            perror("You have to open the file ");
            return -1;
        }
        //-----------------------------------------------------------
        //Check that we have not exceeded the max file size
        if (len + OpenFileDescriptors[fd].GetFileSize() > getMaxFS()) {
            len = getMaxFS() - OpenFileDescriptors[fd].GetFileSize();
            if (len == 0) {
                perror("You have arrived to the max of the file ");
                return -1;
            }
        }
        str = str.substr(0, len);       //Cut the string to match it to the max file size
        //-----------------------------------------------------------
        // Check if the disk is full
        if (findAnEmptyBlock() == -1) {
            perror("The disk is full");
            return -1;
        }
        //-----------------------------------------------------------
        //Check if there is a place in the last block that we have written into it
        size_t temp =(OpenFileDescriptors[fd].getInode()->getBlockInUse() * BS) - OpenFileDescriptors[fd].GetFileSize();
        //temp = 0 ---> the blocks is full
        //temp >0  ---> there is still space in the last block we wrote to
        if (temp > 0) {
            size_t temp2 = temp;
            if (temp > strlen(str.c_str()))
                temp = strlen(str.c_str());
            string cutStr = str.substr(0, temp);            //The beginning of the string to complete in the last block
            //-----------------------------------------------------------
            int cursor = (OpenFileDescriptors[fd].getInode()->getFinal() * BS) + (BS - (int) temp2);
            if (fseek(sim_disk_fd, cursor, SEEK_SET) != 0) {
                perror("Error setting the write position!");
                return -1;
            }
            //-----------------------------------------------------------
            // Write data to the file at the specified index
            size_t dataSize = strlen(cutStr.c_str());
            if (fwrite(cutStr.c_str(), sizeof(char), dataSize, sim_disk_fd) != dataSize) {
                perror("Error writing data!");
                return -1;
            }
            fflush(sim_disk_fd);
            //-----------------------------------------------------------
            //Edit the string after we've written to the file
            str = str.substr(temp, strlen(str.c_str()) - temp);
            len = strlen(str.c_str());
            OpenFileDescriptors[fd].getInode()->setFileSize(OpenFileDescriptors[fd].getInode()->getFileSize() + temp);
        }
        //-----------------------------------------------------------
        // Calculate  the needed blocks
        int neededBlocks = ceil((double) len / BS);
        string strTemp;
        //Loop to write into blocks
        while (1) {
            if (neededBlocks == 0) {
                break;
            }
            int empty;
            //-----------------------------------------------------------
            //Direct blocks
            if (OpenFileDescriptors[fd].getInode()->getBlockInUse() < 3) {
                //DirectBlock1
                if (OpenFileDescriptors[fd].getInode()->getDirectBlock1() == -1) {
                    empty = findAnEmptyBlock();
                    writeForBlocks(empty, str, fd);
                    OpenFileDescriptors[fd].getInode()->setDirectBlock1(empty);
                    len = strlen(str.c_str());
                    OpenFileDescriptors[fd].getInode()->setBlocksWithPointers(OpenFileDescriptors[fd].getInode()->getBlocksWithPointers()+1);
                //DirectBlock2
                } else if (OpenFileDescriptors[fd].getInode()->getDirectBlock2() == -1) {
                    empty = findAnEmptyBlock();
                    writeForBlocks(empty, str, fd);
                    OpenFileDescriptors[fd].getInode()->setDirectBlock2(empty);
                    len = (int) strlen(str.c_str());
                    OpenFileDescriptors[fd].getInode()->setBlocksWithPointers(OpenFileDescriptors[fd].getInode()->getBlocksWithPointers()+1);
                //DirectBlock3
                } else if (OpenFileDescriptors[fd].getInode()->getDirectBlock3() == -1) {
                    empty = findAnEmptyBlock();
                    writeForBlocks(empty, str, fd);
                    OpenFileDescriptors[fd].getInode()->setDirectBlock3(empty);
                    len = (int) strlen(str.c_str());
                    OpenFileDescriptors[fd].getInode()->setBlocksWithPointers(OpenFileDescriptors[fd].getInode()->getBlocksWithPointers()+1);
                }
            }
            //-----------------------------------------------------------
            //Single region
            else if (OpenFileDescriptors[fd].getInode()->getBlockInUse() >= 3 &&
                     OpenFileDescriptors[fd].getInode()->getBlockInUse() < 3 + BS) {
                //-----------------------------------------------------------
                // check if it is the first time we use a single block
                if (OpenFileDescriptors[fd].getInode()->getSingleInDirect() == -1) {
                    int single = findAnEmptyBlock();
                    if (single == -1) {
                        perror("The disk is full ");
                        return -1;
                    }
                    BitVector[single] = 1;
                    empty = findAnEmptyBlock();
                    if (empty == -1) {
                        perror("The disk is full ");
                        BitVector[single] = 0;
                        return -1;
                    }
                    //Update the single block
                    OpenFileDescriptors[fd].getInode()->setSingleInDirect(single);
                    write(single * BS, string(1, decToBinary(empty)), fd);
                    OpenFileDescriptors[fd].getInode()->setBlocksWithPointers(OpenFileDescriptors[fd].getInode()->getBlocksWithPointers()+1);

                }
                //-----------------------------------------------------------
                empty = findAnEmptyBlock();
                //Check if there is an empty block to write
                if (empty == -1) {
                    perror("The disk is full ");
                    return -1;
                }
                //-----------------------------------------------------------
                //Write into blocks
                write(OpenFileDescriptors[fd].getInode()->getSingleInDirect() * BS +
                      (OpenFileDescriptors[fd].getInode()->getBlockInUse() - 3), string(1, decToBinary(empty)), fd);

                writeForBlocks(empty, str, fd);
                len = strlen(str.c_str());

                OpenFileDescriptors[fd].getInode()->setBlocksWithPointers(OpenFileDescriptors[fd].getInode()->getBlocksWithPointers()+1);

            }
                //-----------------------------------------------------------
                //Double region
            else if (OpenFileDescriptors[fd].getInode()->getBlockInUse() >= 3 + BS &&
                     OpenFileDescriptors[fd].getInode()->getBlockInUse() < 3 + BS + BS * BS) {
                int emptyDouble2;
                //-----------------------------------------------------------
                //First time in double
                //We have to save three empty blocks
                if (OpenFileDescriptors[fd].getInode()->getDoubleInDirect() == -1) {
                    int emptyDouble1 = findAnEmptyBlock();
                    if (emptyDouble1 == -1) {
                        perror("The disk is full ");
                        return -1;
                    }
                    BitVector[emptyDouble1] = 1;
                    emptyDouble2 = findAnEmptyBlock();
                    if (emptyDouble2 == -1) {
                        BitVector[emptyDouble1] = 0;
                        perror("The disk is full ");
                        return -1;
                    }
                    BitVector[emptyDouble2] = 1;
                    empty = findAnEmptyBlock();
                    if (empty == -1) {
                        BitVector[emptyDouble1] = 0;
                        BitVector[emptyDouble2] = 0;
                        perror("The disk is full ");
                        return -1;
                    }
                    //Write and update
                    BitVector[emptyDouble2] = 0;
                    OpenFileDescriptors[fd].getInode()->setDoubleInDirect(emptyDouble1);
                    write(emptyDouble1 * BS, string(1, decToBinary(emptyDouble2)), fd);
                    write(emptyDouble2 * BS, string(1, decToBinary(empty)), fd);
                    OpenFileDescriptors[fd].getInode()->setBlocksWithPointers(OpenFileDescriptors[fd].getInode()->getBlocksWithPointers()+1);

                }
                //-----------------------------------------------------------
                if ((OpenFileDescriptors[fd].getInode()->getBlockInUse() - 3 - BS) % BS == 0) {         // Check if we have to use a new pointer
                   //We have to save 2 empty blocks
                    emptyDouble2 = findAnEmptyBlock();
                    if (emptyDouble2 == -1) {
                        perror("The disk is full ");
                        return -1;
                    }
                    BitVector[emptyDouble2] = 1;

                    empty = findAnEmptyBlock();
                    if (empty == -1) {
                        BitVector[emptyDouble2] = 0;
                        perror("The disk is full ");
                        return -1;
                    }
                    //-----------------------------------------------------------
                    //write and update
                    write(emptyDouble2 * BS, string(1, decToBinary(empty)), fd);
                    write((OpenFileDescriptors[fd].getInode()->getDoubleInDirect() * BS) +
                          ((OpenFileDescriptors[fd].getInode()->getBlockInUse() - 3 - BS) / BS),
                          string(1, decToBinary(emptyDouble2)), fd);
                    writeForBlocks(empty, str, fd);
                    len = strlen(str.c_str());
                    OpenFileDescriptors[fd].getInode()->setBlocksWithPointers(OpenFileDescriptors[fd].getInode()->getBlocksWithPointers()+2);
                    //-----------------------------------------------------------
                } else {
                    //We have to open one block to write data
                    empty = findAnEmptyBlock();
                    if (empty == -1) {
                        perror("The disk is full ");
                        return -1;
                    }
                    //-----------------------------------------------------------
                    //Put the cursor
                    seekInFile(OpenFileDescriptors[fd].getInode()->getDoubleInDirect() * BS +
                               ((OpenFileDescriptors[fd].getInode()->getBlockInUse() - 3 - BS) / BS));
                    //-----------------------------------------------------------
                    // Read the character at the 5th position
                    int ch = fgetc(sim_disk_fd);
                    //-----------------------------------------------------------
                    //writing to the empty double 2
                    write((ch * BS) + ((OpenFileDescriptors[fd].getInode()->getBlockInUse() - 3 - BS) % BS),
                          string(1, decToBinary(empty)), fd);
                    writeForBlocks(empty, str, fd);
                    len = strlen(str.c_str());
                    OpenFileDescriptors[fd].getInode()->setBlocksWithPointers(OpenFileDescriptors[fd].getInode()->getBlocksWithPointers()+1);

                }
            }
            neededBlocks--;
        }
        return 1;
    }

    //=======================================================================================================================
    int DelFile(string FileName) {
        // Check if the file exists in the directory
        if (MainDir.find(FileName) == MainDir.end()) {
            perror("The file is not existed ");
            return -1; // Return an error code
        }
        //-----------------------------------------------------------
        //Check that the file is closed ,
        for (int i =0; i< OpenFileDescriptors.size();i++){
            if (OpenFileDescriptors[i].getFileName()==FileName){
                if (OpenFileDescriptors[i].isInUse()){
                    perror ("The file is opened , you have to close it before deleting it");
                    return -1 ;
                }
                else {
                    OpenFileDescriptors[i].setInUse(false);
                    OpenFileDescriptors[i].setFileName("");     //Empty name
                }
            }
        }
        //-----------------------------------------------------------
        fsInode* tempInode =MainDir[FileName];
        //-----------------------------------------------------------
        //Free the blocks in the Bit vector
        freeBlocks(tempInode, tempInode->getBlocksWithPointers());
        //-----------------------------------------------------------
        // Delete the file's inode and remove it from the directory
        delete MainDir[FileName];
        //-----------------------------------------------------------
        MainDir.erase(FileName);

        return 1;

    }

    //=======================================================================================================================
    int ReadFromFile(int fd, char *buf, int len) {

        memset(buf, 0, len); // Clear the buffer to an empty string
        //----------------------------------------------------------
        //check if the desk is formated
        if (!this->is_formated) {
            perror("The disk has to be formated before ");
            return -1;
        }
        //-----------------------------------------------------------
        if (fd < 0 || fd >= OpenFileDescriptors.size()) {
            perror("Invalid fd ");
            return -1;
        }
        //-----------------------------------------------------------
        if (!OpenFileDescriptors[fd].isInUse()) {
            perror("You have to open the file ");
            return -1;
        }
        //-----------------------------------------------------------
        if (len > OpenFileDescriptors[fd].getInode()->getFileSize()){
            len =OpenFileDescriptors[fd].getInode()->getFileSize();
        }
        //not reading any thing from the file
        if (len ==0){
            strcpy(buf,"");
            return 1 ;
        }
        //-----------------------------------------------------------
        int tempBlock = ceil((double) len / BS);  //Blocks we have to read
        int i = 1;
        int numCharsToRead=BS;                          //Num chars to read in one rotation
        while (1) {
            //-----------------------------------------------------------
            //stop condition
            if (i == (tempBlock + 1)){
                break;}
            //-----------------------------------------------------------
            //Check the fragmentation in the last block
            if (i==tempBlock){
                if (len %BS!=0)
                    numCharsToRead=len%BS;
            }
            //-----------------------------------------------------------
            //Direct blocks
            if (i <= 3) {
                if (i==1){
                    read (OpenFileDescriptors[fd ].getInode()->getDirectBlock1()*BS,&buf,numCharsToRead);
                }
                else if (i==2){
                    read(OpenFileDescriptors[fd].getInode()->getDirectBlock2()*BS,&buf,numCharsToRead);
                }
                else if (i==3){
                    read(OpenFileDescriptors[fd].getInode()->getDirectBlock3()*BS,&buf,numCharsToRead);

                }
            }
            //-----------------------------------------------------------
            //Single blocks
            if (i >3&&i <= 3+BS){
                seekInFile((OpenFileDescriptors[fd].getInode()->getSingleInDirect()*BS)+(i-4));
                int ch =fgetc(sim_disk_fd);             // The num of the block that the single pointer point
                //Read the data
                read(ch*BS,&buf,numCharsToRead);
            }
            //-----------------------------------------------------------
            //Double blocks
            if (i >3+BS && i<=3+BS+BS*BS){
                //The main block in double
                seekInFile((OpenFileDescriptors[fd].getInode()->getDoubleInDirect()*BS)+((i-4-BS)/BS) );
                int chInMainBlock= fgetc(sim_disk_fd);      //the num of the pointer in the second level
                seekInFile((chInMainBlock *BS)+((i-4-BS)%BS));
                int chIn2Level = fgetc(sim_disk_fd);        //num of the block that saves the data
                //Reading the data
                read(chIn2Level*BS,&buf,numCharsToRead);

            }
            i++;
        }

        return 1;
    }
    //=======================================================================================================================
    int GetFileSize(int fd) {
        return OpenFileDescriptors[fd].getInode()->getFileSize();
    }
    //=======================================================================================================================
    int CopyFile(string srcFileName, string destFileName) {
        //check if the desk is formated
        if (!this->is_formated ) {
            perror("The disk has to be formated before ");
            return -1;
        }
        //-----------------------------------------------------------
        // Check if the file exists in the directory
        if (MainDir.find(srcFileName) == MainDir.end()) {
            perror("The srcFileName is not existed ");
            return -1; // Return an error code
        }
        //-----------------------------------------------------------
        // Check that the file is close
        for (int i=0 ;i< OpenFileDescriptors.size();i++){
            if (OpenFileDescriptors[i].getFileName()==srcFileName){
                if (OpenFileDescriptors[i].isInUse()){
                    perror ("you have to close the srcFile to can copy it ");
                    return -1;
                }
            }
        }
        //-----------------------------------------------------------
        fsInode* srcInode = MainDir[srcFileName];
        //-----------------------------------------------------------
        // Check if the file exists in the directory
        int fdDest;
        bool check= false; // Check if we have to create a new file
        if (MainDir.find(destFileName) == MainDir.end()) {
            // we have to create a new file
            fdDest =CreateFile(destFileName);
            check=true;
        }
        //-----------------------------------------------------------
        fsInode* destInode = MainDir[destFileName];
        //-----------------------------------------------------------
        //The file is already exist
        //we have to free the blocks in bit vector ,and update all of the attributes to the default
        if (!check){
        freeBlocks(destInode,destInode->getBlocksWithPointers());
        destInode->setBlocksWithPointers(0);
        destInode->setBlockInUse(0);
        destInode->setFileSize(0);
        destInode->setDoubleInDirect(-1);
        destInode->setSingleInDirect(-1);
        destInode->setDirectBlock3(-1);
        destInode->setDirectBlock2(-1);
        destInode->setDirectBlock1(-1);
        destInode->setFinal(-1);
        fdDest= OpenFile(destFileName);

        }
        //-----------------------------------------------------------
        // Check if we have enough place in the disk
        int count =0;
        for (int i = 0; i <BitVectorSize;i++ ){

            if (count==srcInode->getBlocksWithPointers())
                break ;
            if (BitVector[i]==0)
                count++;
        }
        if (count!=srcInode->getBlocksWithPointers()){
            perror("There is no place in the disk");
            return -1;
        }
        //-----------------------------------------------------------
        int fdSrc= OpenFile(srcFileName);
        if (fdSrc==-1){
            perror ("Error in opening the src file ");
            return  -1;
        }
        //-----------------------------------------------------------
        //Reading the data from the srcfile
        char readFromSrc[DISK_SIZE];
         ReadFromFile(fdSrc,readFromSrc,OpenFileDescriptors[fdSrc].getInode()->getFileSize());
        WriteToFile(fdDest,readFromSrc, strlen(readFromSrc));
        //-----------------------------------------------------------
        //Closing the files
        CloseFile(fdSrc);
        CloseFile(fdDest);

        return 1;
    }

    //=======================================================================================================================
    int RenameFile(string oldFileName, string newFileName) {
        // Check if the old file exists in MainDir
        if (MainDir.find(oldFileName) == MainDir.end()) {
            perror("The old file is not exist ");
            return -1;
        }
        //-----------------------------------------------------------
        // Check if the new file name is already in use
        if (MainDir.find(newFileName) != MainDir.end()) {
            perror ("The new file name is already in use");
            return -1;
        }
        //-----------------------------------------------------------
        // Find the file descriptor in OpenFileDescriptors (if it's open) and set isInUse to false
        for (FileDescriptor& fileDesc : OpenFileDescriptors) {
            if (fileDesc.getFileName() == oldFileName) {
                // Check if the file is open
                if (fileDesc.isInUse()) {
                    perror ("The file is opened ,you have to close it ");
                    return -1;
                }
            }
        }
        //-----------------------------------------------------------
        // Rename the file in MainDir
        MainDir[newFileName] = MainDir[oldFileName];
        MainDir.erase(oldFileName);
        return 1;

    }
    //=======================================================================================================================
    //=======================================================================================================================
private:
    //Put the cursor
    void seekInFile(int seek){
        if (fseek(sim_disk_fd, seek, SEEK_SET) != 0) {
            perror("Error setting the write position!");
            exit(0);
        }
    }
    //=======================================================================================================================
    int findAnEmptyBlock() {
        for (int i = 0; i < BitVectorSize; i++) {
            if (BitVector[i] == 0)
                return i;
        }
        return -1;
    }
    //=======================================================================================================================
    void write(int seek, string strToWrite, int fd) {
        //putting the cursor
        seekInFile(seek);
        //-----------------------------------------------------------
        //write into the file (disk)
        size_t dataSize = strlen(strToWrite.c_str());
        if (fwrite(strToWrite.c_str(), sizeof(char), dataSize, sim_disk_fd) != dataSize) {
            perror("Error writing data!");
            exit(0);
        }
        fflush(sim_disk_fd);
    }
    //=======================================================================================================================

    void writeForBlocks(int empty, string &str, int fd) {
        string strTemp;
        if (empty == -1) {
            perror("There is no empty block for DirectBlock");
            exit(0);
        }
        //-----------------------------------------------------------
        if (strlen(str.c_str()) < BS) {
            strTemp = str.substr(0, strlen(str.c_str()));
        } else {
            strTemp = str.substr(0, BS);
        }
        //-----------------------------------------------------------
        write(empty * BS, strTemp, fd);
        BitVector[empty] = 1;
        str = str.substr(strlen(strTemp.c_str()), strlen(str.c_str()) - strlen(strTemp.c_str()));
        OpenFileDescriptors[fd].getInode()->setFileSize(OpenFileDescriptors[fd].getInode()->getFileSize() +
                                                        strlen(strTemp.c_str()));
        OpenFileDescriptors[fd].getInode()->setFinal(empty);
        OpenFileDescriptors[fd].getInode()->setBlockInUse(OpenFileDescriptors[fd].getInode()->getBlockInUse() + 1);
    }

    //=======================================================================================================================
    //function that read from the file
    void read(int seek, char **buff, int numCharsToRead) {
        if (fseek(sim_disk_fd, seek, SEEK_SET) != 0) {
            perror("Error setting the read position!");
            exit(1); // Exit with an error code indicating failure
        }
        //-----------------------------------------------------------
        // Allocate memory for the buffer to read into
        char *buffer = (char *)malloc(numCharsToRead + 1); // +1 for null terminator
        if (buffer == NULL) {
            perror("Memory allocation error");
            exit(1); // Exit with an error code indicating failure
        }
        //-----------------------------------------------------------
        size_t num_chars_read = fread(buffer, sizeof(char), numCharsToRead, sim_disk_fd);

        if (num_chars_read > 0) {
            // Null-terminate the string
            buffer[numCharsToRead] = '\0';

            // Check if *buff is NULL (first call) or contains a concatenated string
            if (*buff == NULL) {
                *buff = strdup(buffer); // First call, just copy the buffer
                if (*buff == NULL) {
                    perror("Memory allocation error");
                    exit(1); // Exit with an error code indicating failure
                }
            } else {
                // Allocate memory for the concatenated result
                char *concatenated = (char *)malloc(strlen(*buff) + num_chars_read + 1);
                if (concatenated == NULL) {
                    perror("Memory allocation error");
                    exit(1); // Exit with an error code indicating failure
                }

                // Copy the current *buff and the newly read buffer into concatenated
                strcpy(concatenated, *buff);
                strcat(concatenated, buffer);
                strcpy(*buff, concatenated);
                free(concatenated);

            }

        }
            //-----------------------------------------------------------
        else if (num_chars_read == 0) {
            printf("End of file reached.\n");
        }
            //-----------------------------------------------------------
        else {
            printf("Error reading from file.\n");
        }

        free(buffer); // Free the buffer
    }
    //=======================================================================================================================
    void freeBlocks(fsInode* fs, int BlocksWithPointers ){
        int help = BlocksWithPointers;
        //-----------------------------------------------------------
        //Direct blocks
        if (fs->getDirectBlock1()!=-1 ){
            BitVector[fs->getDirectBlock1()]=0;
            fs->setDirectBlock1(-1);
            help--;
        }
        if (fs->getDirectBlock2()!=-1 ){
            BitVector[fs->getDirectBlock2()]=0;
            fs->setDirectBlock2(-1);
            help--;
        }
        if (fs->getDirectBlock3()!=-1 ){
            BitVector[fs->getDirectBlock3()]=0;
            fs->setDirectBlock3(-1);
            help--;
        }
        //Check the stop condition
        if (help == 0)
            return;
        //-----------------------------------------------------------
        //Single blocks
        for (int i = 0 ;; i ++){
            if (i ==BS||i ==fs->getBlockInUse()-3)
                break;
            seekInFile(fs->getSingleInDirect()*BS+i);
            int ch = fgetc(sim_disk_fd);
            BitVector[ch]=0;
            help--;
        }
        BitVector[fs->getSingleInDirect()]=0;
        fs->setSingleInDirect(-1);
        help--;
        //Check the stop condition
        if (help == 0 )
            return;
        //-----------------------------------------------------------
        //Double blocks
        //free the data
        for (int i = 0 ; ; i++) {
            if (i == BS*BS || i==fs->getBlockInUse()-3-BS)
                break;
            seekInFile((fs->getDoubleInDirect() * BS) + ((i) / BS));
            int chInMainBlock = fgetc(sim_disk_fd);
            seekInFile((chInMainBlock * BS) + ((i) % BS));
            int chIn2Level = fgetc(sim_disk_fd);
            BitVector[chIn2Level] = 0;
            help--;
        }
        // free the help blocks
        for (int i = 0 ; ; i++) {
            if ( i ==BS|| i ==help-1)
                break;
            seekInFile((fs->getDoubleInDirect() * BS) + ((i) / BS));
            int chInMainBlock = fgetc(sim_disk_fd);
            BitVector[chInMainBlock]=0;
        }
        BitVector[fs->getDoubleInDirect()]=0;
        fs->setDoubleInDirect(-1);
    }


};
//=======================================================================================================================

int main() {
    int blockSize;
    int direct_entries;
    string fileName;
    string fileName2;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while (1) {
        cin >> cmd_;

        switch (cmd_) {
            case 0:   // exit
                delete fs;
                exit(0);
                break;

            case 1:  // list-file
                fs->listAll();
                break;

            case 2:    // format
                cin >> blockSize;
                fs->fsFormat(blockSize);
                break;

            case 3:    // creat-file
                cin >> fileName;
                _fd = fs->CreateFile(fileName);
                cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 4:  // open-file
                cin >> fileName;
                _fd = fs->OpenFile(fileName);
                cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 5:  // close-file
                cin >> _fd;
                fileName = fs->CloseFile(_fd);
                cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 6:   // write-file
                cin >> _fd;
                cin >> str_to_write;
                fs->WriteToFile(_fd, str_to_write, strlen(str_to_write));
                break;

            case 7:    // read-file
                cin >> _fd;
                cin >> size_to_read;
                fs->ReadFromFile(_fd, str_to_read, size_to_read);
                cout << "ReadFromFile: " << str_to_read << endl;

                break;

            case 8:   // delete file
                cin >> fileName;
                _fd = fs->DelFile(fileName);
                cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 9:   // copy file
                cin >> fileName;
                cin >> fileName2;
                fs->CopyFile(fileName, fileName2);
                break;

            case 10:  // rename file
                cin >> fileName;
                cin >> fileName2;
                fs->RenameFile(fileName, fileName2);
                break;

            default:
                break;
        }
    }

};
