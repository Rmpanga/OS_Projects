#include <stdio.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <fstream>
/* IDEAS
   Scan file once and just update the freeblock 

*/



struct Inode
{
 char filename[8];
 int size;
 int blockPointers[8];
 int used;
};


class FileSystem
{
public:
 FILE *disk; 
 int fd; // File Descriptor --DO NOT NEED
 const char * diskName;	
 const int offset = 1024;
 const int maxInodes = 16;
 const int maxBlocks = 128;

FileSystem(char dName[16])
 {
    disk = fopen(dName , "r+");
    diskName = dName;
    if (disk == NULL)
     printf("Error Opening Disk\n");
      // if  (fd = open(diskName , O_RDWR) < 0)
      //   printf("There was a problem opening the disk\n"); 
  
    // open the file with the above name
    // this file will act as the "disk" for your file system
  }

void populateInodes()
{
   //disk = freopen (diskName , "r+" , disk);
  // printf("[PopulateInodes] : Disk was closed opening it now.."); 
   fseek(disk, 128, SEEK_SET); // Skipping over freeBlock list
   for (int j = 0; j< 16; j++)
   { 
    Inode newInode;
    newInode.size = j;
    fwrite( (char *) &newInode, sizeof(Inode), 1 , disk); 
   }
   
   //printf("Number of elements written %d\n" , wrote);
   //printf("Populating Inodes %lu\n" , sizeof(struct Inode));
   fseek(disk , sizeof(Inode) * 16 * -1, SEEK_CUR);
   Inode *inodes = new Inode[maxInodes];
   fread(inodes , 48, 16, disk); 
   
   for (int j = 0; j <16; j++)
   {
    printf("Inode %d size is %d\n" , j , inodes[j].size);
   }
    

 }


int getFreeBlocks()
{
 int freeBlocks = 0; 

 fseek(disk, 1, SEEK_SET); //Skipping the super block  
 for (int j = 1; j<128; j++) // Counting free blocks
  {
    int c = getc(disk);
     //printf("Int c: %d\n" , c); 
    if (c == 0)   
     {       
       freeBlocks++; 
     }
  }


  return freeBlocks;
}



  int create(char name[8], int size)
  { //create a file with this name and this size
   //fpos_t file_pos; Used for old files
   fpos_t inode_begin; //Pointer to begining of inode
   fpos_t block_ptrs;
   fpos_t freeblock_ptr;
   int blocksAllocated = 0;
   int freeBlocks;  
   int done = 0;
 
   freeBlocks = getFreeBlocks();  //Getting free blocks
    printf("[CREATE TEST FREE BLOCKS] : There are  %d\n freeblocks" , freeBlocks);

  if (size <= freeBlocks)
   {
    printf("[CREATE] : Size %d is <= freeblocks %d\n" , size , freeBlocks); 
    // We have space for the file
    fseek(disk, 128, SEEK_SET); //Moving to inodes
    printf("[CREATE] : Moving to inodes\n");
    for(int j =1; j<=16; j++)
      {
     //  fseek(disk,  (j * 48), SEEK_CUR); //Iterating through inodes                 MIGHT NOT NEED J 
       fseek(disk , 48 , SEEK_CUR);
       printf("[CREATE] : Checking inode used paramter\n");
       if (fgetpos ( disk, &inode_begin) != 0)
         printf("  [CREATE ERROR] : with fgetpos funtion\n");
 
       //Scanning to check if inode is free
       fseek(disk, 44 , SEEK_CUR);  //Seeking to used parameter --Total inode size = 48, 
      
       int c = getc(disk);
       printf("[CREATE] : Inode used parameter is %d\n" , c); 
       if ( c == 0)
        {
        // fsetpos(disk, &inode_begin);
       
         ungetc(c, disk); //Going back to the used parameter
         fputc(1, disk);
         printf("[CREATE] : Inode is free replacing with 1\n");
         //**IDEA**//
         //Maybe instead of going back to used parameter go back to the begining of inode and write from the begining
         fsetpos(disk, &inode_begin); 
         printf("[CREATE] : Moving back to the begining of Inode\n");
         fwrite(name, 1, 8, disk); //8 maybe incorrect, filename <=8
         printf("[CREATE] : Writing file name %s to disk\n" , name);                           //maynot matter
         fputc(size, disk); //writing to size
         printf("[CREATE] : Writing size to disk %d to disk\n" , size); 
         fgetpos(disk ,&block_ptrs);
         fseek(disk, 1, SEEK_SET); //Moving to freeblock list (Maybe comment out)
          
          //set up blockPointer (no idea)
          for (int j = 1; j<128; j++)
          {
           if (blocksAllocated < size)
           {
            if (getc(disk) == 0)
            {
             printf("  [CREATE] : A block is about to be allocated\n");
             fseek(disk, -1, SEEK_CUR);
             fputc(1 , disk);              
             fgetpos(disk , &freeblock_ptr);   //Store location of the current free block 
             //fseek(disk, -1, SEEK_CUR); 
/*TEST*/     fsetpos(disk , &block_ptrs + blocksAllocated);      //Move pos to blockpointers 
             fputc(1024 * j , disk);
             blocksAllocated++;
             fsetpos(disk , &freeblock_ptr);
             //fseek(disk, 1, SEEK_CUR);
            }
           }
            else //wrote out all the blocks for this file
             {
               done = 1;
               printf("[CREATE] : Successfully wrote out file\n"); 
               break;
              }
      
         }//4 loop
 
     
        }//getc   
  
      if (done == 1)
       {
        printf("[CREATE] : Breaking for loop\n");
        break;
       } 
     fsetpos(disk, &inode_begin); //Setting back to begining of inode
    } //4 loop
   
   }//if size <= blocks
    else
     { //We dont have space for the file
      printf("  [CREATE ERROR]: File can not fit on disk\n");
     }
   
    // high level pseudo code for creating a new file

    // Step 1: Check to see if we have sufficient free space on disk by
    //   reading in the free block list. To do this:
    // Move the file pointer to the start of the disk file.
    // Read the first 128 bytes (the free/in-use block information)
    // Scan the list to make sure you have sufficient free blocks to
    //   allocate a new file of this size

    // Step 2: we look for a free inode on disk
    // Read in an inode
    // Check the "used" field to see if it is free
    // If not, repeat the above two steps until you find a free inode
    // Set the "used" field to 1
    // Copy the filename to the "name" field
    // Copy the file size (in units of blocks) to the "size" field

    // Step 3: Allocate data blocks to the file
    // for(i=0;i<size;i++)
      // Scan the block list that you read in Step 1 for a free block
      // Once you find a free block, mark it as in-use (Set it to 1)
      // Set the blockPointer[i] field in the inode to this block number.
    // end for


  /**For now treat as completed **/
    // Step 4: Write out the inode and free block list to disk  fwrite
    // Move the file pointer to the start of the file 
    // Write out the 128 byte free block list                                    ??Is that already done for us or I did that with put
    // Move the file pointer to the position on disk where this inode was stored   //^^ might apply here
    // Write out the inode

fseek(disk, 1, SEEK_SET); //Skipping the super block
 int freeBlocksTest =0;
  for (int j = 1; j<128; j++) // Counting free blocks
  {
    int c = getc(disk);
     //printf("Int c: %d\n" , c); 
    if (c == 0)   
     { 
       // printf("FreeBlock!\n");      
       freeBlocksTest++; 
     }
   } //
  printf("[TEST] Free Blocks: %d\n", freeBlocksTest);
   return 0;
  } // End Create



  int remove(char name[8])
  {
    // Delete the file with this name

    // Step 1: Locate the inode for this file
    // Move the file pointer to the 1st inode (129th byte)
    // Read in an inode
    // If the inode is free, repeat above step.
    // If the inode is in use, check if the "name" field in the
    //   inode matches the file we want to delete. If not, read the next
    //   inode and repeat

    // Step 2: free blocks of the file being deleted
    // Read in the 128 byte free block list (move file pointer to start
    //   of the disk and read in 128 bytes)
    // Free each block listed in the blockPointer fields as follows:
    // for(i=0;i< inode.size; i++) 
      // freeblockList[ inode.blockPointer[i] ] = 0;

    // Step 3: mark inode as free
    // Set the "used" field to 0.

    // Step 4: Write out the inode and free block list to disk
    // Move the file pointer to the start of the file 
    // Write out the 128 byte free block list
    // Move the file pointer to the position on disk where this inode was stored
    // Write out the inode
   return 0; 
  } // End Delete


  int ls()
  { 
   char* filename;
   int * size; 
    // List names of all files on disk
   fseek(disk, 129 ,SEEK_SET);
   
    

    // Step 1: read in each inode and print
    // Move file pointer to the position of the 1st inode (129th byte)
     
     for(int i=1;i<=16;i++) //Scanning 16 inodes
     {
     fseek(disk, 44, SEEK_CUR); //Moved to used position 
     if (getc(disk)  != 0)      //If used position is not 0
      {
       fseek(disk , -44 , SEEK_CUR); //Go back to name position
       //Print out name;
       fread(filename, 1, 8, disk);
       //Print out size;
       fseek(disk, 8 , SEEK_CUR);
       fread(size, 4, 1, disk);
       printf("Filename: ");
       //DEBUGING FILENAME
       for (int j = 0; j < 8; j++)
       {
        printf("%c",filename[j]);
       }  
       
       //DEBUGING SIZE
       printf("/n%d/n",*size);
  

       fseek(disk, -12 , SEEK_CUR); //Go back to name position
      }
      fseek(disk, 48, SEEK_CUR);   
      


    }
      // Read in an inode
      // If the inode is in-use
        // print the "name" and "size" fields from the inode
    // end for
  return 0;
  } // End ls

  int read(char name[8], int blockNum, char* buf[1024])
  {
    // read this block from this file
   

    // Step 1: locate the inode for this file
    // Move file pointer to the position of the 1st inode (129th byte)
    // Read in an inode
    // If the inode is in use, compare the "name" field with the above file
    // If the file names don't match, repeat

    // Step 2: Read in the specified block
    // Check that blockNum < inode.size, else flag an error
    // Get the disk address of the specified block
    // That is, addr = inode.blockPointer[blockNum]
    // Move the file pointer to the block location (i.e., to byte #
    //   addr*1024 in the file)

    // Read in the block => Read in 1024 bytes from this location
    //   into the buffer "buf"
  return 0;
  } // End read


  int write(char name[8], int blockNum, char* buf[1024])
  {
    // write this block to this file
  fpos_t filename_ptr;
  fpos_t size_ptr; 

  /*
    fseek(disk , 128 , SEEK_SET);
    for (int j = 0; j < 16; j++)
    {
     fgetpos(disk, &filename_ptr);
     //char* fileNameptr = *filename_ptr;
      
     if (strcmp(fileNameptr , name) == 0)
      { //Filenames are equal!
        fseek(disk , 8 , SEEK_CUR);
        fgetpos(disk, &size_ptr);
          int maxSize = -44;    //ERROR = (int *) size_ptr;
         
         if (blockNum <= maxSize)
         {
         fseek(disk, 12 + blockNum, SEEK_CUR);
         fputs(buf, disk);
         printf("   [WRITE] : Successful wrote file to disk\n");
         break;  //Found the file, writing and breakng for loop
         } else
           {
            printf("[WRITE_ERROR] : BlockNum is larger than file size\n");
           break; //Cant write end for loop
           }    
      } else //Advancing to the next inode
       {
       printf("   [WRITE] : inode did not match filename..Traversing..\n");
       fseek(disk, 48 , SEEK_CUR);
       }
     



    // Step 1: locate the inode for this file
    // Move file pointer to the position of the 1st inode (129th byte)
    // Read in a inode
    // If the inode is in use, compare the "name" field with the above file
    // If the file names don't match, repeat

    // Step 2: Write to the specified block
    // Check that blockNum < inode.size, else flag an error
    // Get the disk address of the specified block
    // That is, addr = inode.blockPointer[blockNum]
    // Move the file pointer to the block location (i.e., byte # addr*1024)

    // Write the block! => Write 1024 bytes from the buffer "buff" to 
    //   this location
  return 0;
*/  } // end write
};

int main ()
{
  FileSystem FS ("disk0");
 // printf("HELLO\n");
  FS.populateInodes();
 // File.create("File" ,9 );
  printf("Freeblocksd\n",  FS.getFreeBlocks());
//  printf("Free Blocks is: %d\n" , File.checkFreeBlocks(File.disk));


}

