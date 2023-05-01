#define _CRT_SECURE_NO_WARNINGS

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <sys/stat.h>

#define BLOCK_SIZE 1024
#define NUM_BLOCKS 65536
#define BLOCKS_PER_FILE 1024
#define NUM_FILES 256
#define FIRST_DATA_BLOCK 1001
#define MAX_FILE_SIZE 1048576
uint8_t data[NUM_BLOCKS][BLOCK_SIZE];

// 512 blocks just for free block map
uint8_t *free_blocks;
uint8_t *free_inodes;

// directory
struct directoryEntry
{
  char filename[64];
  short in_use;
  int32_t inode;
};

struct directoryEntry *directory;

// inode
struct inode
{
  int32_t blocks[BLOCKS_PER_FILE];
  short in_use;
  uint8_t attribute;
  uint32_t file_size;
};

struct inode *inodes;
FILE *fp;
char image_name[64];
uint8_t image_open;

#define WHITESPACE " \t\n" // We want to split our command line up into tokens
                           // so we need to define what delimits our tokens.
                           // In this case  white space
                           // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5 // Mav shell only supports four arguments

#define MAX_HIST_ARGUMENTS 15 // History can contain no more than 15 commands
#define MAX_PIDS 15           // To hold the pids for the 15 history commands

//Retrieving file data
void retrieve (char *filename , char *newfilename)
{
  FILE *fp;
  fp = fopen(newfilename, "w"); //opening newfile
  FILE *fp1;
  fp1 = fopen(filename, "r"); //opening hello.txt

  if (newfilename == NULL)
  {
 
    strncpy(directory[0].filename, filename, strlen(filename)); // copying hello.txt into the current directory
    printf("%s has been retrieved successfully\n" , filename);

  }
  else
  {
    strncpy(newfilename, filename, strlen(filename)); // Copying hello.txt into newfile helloagain.txt
    printf("%s has been retrieved successfully and placed into %s\n" , filename , newfilename);
  }

  fclose(fp); //Closing newfile
  fclose(fp1); //Closing hello.txt

}
//End of retrieve function


// Creating Encrypt and Decrypt function
void encrypt(char *filename, char *password)
{
  FILE *file;
  int block_size = strlen(password);
  unsigned char block[block_size];
  int bytes;
  printf("---HI--\n");

  file = fopen(filename, "rb+"); // Opening file hello.txt
  if (file == NULL) // Checking if the file opens
  {
    printf("ERROR: Error opening file.\n"); 
  }

  do
  {
    
    bytes = fread(block, 1, block_size, file); //Calculating bytes to encrypt
    for (int i = 0; i < bytes; i++)
    {
      block[i] ^= password[i]; // encrypting file data with cipher text
    }
    fseek(file, -bytes, SEEK_CUR); //Finding address of the file to write it back using fseek
    fwrite(block, 1, bytes, file); //Writing it back to the file

  } while (bytes == block_size);

  fclose(file); // Closing hello.txt
}


void decrypt(char *filename, char *password)
{
  FILE *file;
  int block_size = strlen(password);
  unsigned char block[block_size];
  int bytes;
  printf("---HI--\n");

  file = fopen(filename, "rb+"); // Opening file hello.txt
  if (file == NULL) // Checking if the file opens or not
  {
    printf("ERROR: Error opening file.\n");
  }

  do
  {
    
    bytes = fread(block, 1, block_size, file); //calculating bytes of data in file to be decrypted
    for (int i = 0; i < bytes; i++)
    {
      block[i] ^= password[i]; //Decrypting file data by applying the cipher text again.
    }
    fseek(file, -bytes, SEEK_CUR); //Getting address of the file to write to
    fwrite(block, 1, bytes, file); //Writing data back to the file

  } while (bytes == block_size);

  fclose(file); // Closing file hello.txt
}

// Done with encrypt and decrypt

int32_t findFreeBlock()
{

  int i;
  for (i = 0; i < NUM_BLOCKS; i++)
  {
    if (free_blocks[i])
    {
      return i + 1001;
    }
  }
  return -1;
}

int32_t findFreeInode()
{

  int i;
  for (i = 0; i < NUM_FILES; i++)
  {
    if (free_inodes[i])
    {
      return i;
    }
  }
  return -1;
}

int32_t findFreeInodeBlock(int32_t inode)
{

  int i;
  for (i = 0; i < BLOCKS_PER_FILE; i++)
  {
    if (inodes[inode].blocks[i] == -1)
    {
      return i;
    }
  }
  return -1;
}

void init()
{
  directory = (struct directoryEntry *)&data[0][0];
  inodes = (struct inode *)&data[20][0];

  free_blocks = (uint8_t *)&data[1000][0];
  free_inodes = (uint8_t *)&data[19][0];

  memset(image_name, 0, 64);
  image_open = 0;

  int i;
  for (i = 0; i < NUM_FILES; i++)
  {
    directory[i].in_use = 0;
    directory[i].inode = -1;
    free_inodes[i] = 1;

    memset(directory[i].filename, 0, 64);
    int j;
    for (j = 0; j < NUM_BLOCKS; j++)
    {
      inodes[i].blocks[j] = -1;
      inodes[i].in_use = 0;
      inodes[i].attribute = 0;
      inodes[i].file_size = 0;
    }
  }
  int j;
  for (j = 0; j < NUM_BLOCKS; j++)
  {
    free_blocks[j] = 1;
  }
}

uint32_t df()
{
  int j;
  int count = 0;
  for (j = FIRST_DATA_BLOCK; j < NUM_BLOCKS; j++)
  {
    if (free_blocks[j])
    {
      count++;
    }
  }
  return count * BLOCK_SIZE;
}

void createfs(char *filename)
{
  fp = fopen(filename, "w");
  strncpy(image_name, filename, strlen(filename));
  memset(data, 0, NUM_BLOCKS * BLOCK_SIZE);
  image_open = 1;

  int i;
  for (i = 0; i < NUM_FILES; i++)
  {
    directory[i].in_use = 0;
    directory[i].inode = -1;
    free_inodes[i] = 1;

    memset(directory[i].filename, 0, 64);
    int j;
    for (j = 0; j < NUM_BLOCKS; j++)
    {
      inodes[i].blocks[j] = -1;
      inodes[i].in_use = 0;
      inodes[i].attribute = 0;
      inodes[i].file_size = 0;
    }
  }

  int j;
  for (j = 0; j < NUM_BLOCKS; j++)
  {
    free_blocks[j] = 1;
  }

  fclose(fp);
}

void savefs()
{
  if (image_open == 0)
  {
    printf("ERROR: Disk image is not open\n");
    return;
  }
  fp = fopen(image_name, "w");

  fwrite(&data[0][0], BLOCK_SIZE, NUM_BLOCKS, fp);

  memset(image_name, 0, 64);
  fclose(fp);
}

void openfs(char *filename)
{
  fp = fopen(filename, "r");
  strncpy(image_name, filename, strlen(filename));
  fread(&data[0][0], BLOCK_SIZE, NUM_BLOCKS, fp);
  image_open = 1;
  fclose(fp);
}

void closefs()
{
  if (image_open == 0)
  {
    printf("ERROR: Disk image is not open\n");
    return;
  }

  fclose(fp);
  image_open = 0;
  memset(image_name, 0, 64);
}

void list()
{

  int i;
  int not_found = 1;
  for (i = 0; i < NUM_FILES; i++)
  {
    if (directory[i].in_use)
    {
      not_found = 0;
      char filename[65];
      memset(filename, 0, 65);
      strncpy(filename, directory[i].filename, strlen(directory[i].filename));
      printf("%s\n", filename);
    }
  }
  if (not_found)
  {
    printf("ERROR: No files found.\n");
  }
}

void delete_file(char* filename) 
{
  int i;
  for (i = 0; i < NUM_FILES; i++) 
  {
    if (strcmp(directory[i].filename, filename) == 0) 
    {
      inodes[directory[i].inode].in_use = 0; // mark the inode as not in use
      directory[i].in_use = 0; // mark the directory entry as not in use
      int j;
      for (j = 0; j < BLOCKS_PER_FILE; j++)
      {
        if (inodes[directory[i].inode].blocks[j] != -1)
        {
          free_blocks[inodes[directory[i].inode].blocks[j]] = 0; // mark the block as free
          inodes[directory[i].inode].blocks[j] = -1; // clear the block reference in the inode
        }
      }
      printf("File '%s' deleted.\n", filename);
      return;
    }
  }
  printf("File '%s' not found.\n", filename);
}

void undelete_file(char* filename) 
{
  int i;
  for (i = 0; i < NUM_FILES; i++) 
  {
    if (strcmp(directory[i].filename, filename) == 0) 
    {
      if (!inodes[directory[i].inode].in_use)
      {
        inodes[directory[i].inode].in_use = 1; // mark the inode as in use
        directory[i].in_use = 1; // mark the directory entry as in use
        int j;
        for (j = 0; j < BLOCKS_PER_FILE; j++)
        {
          if (inodes[directory[i].inode].blocks[j] != -1)
          {
            free_blocks[inodes[directory[i].inode].blocks[j]] = 1; // mark the block as in use
          }
        }
        printf("File '%s' undeleted.\n", filename);
        return;
      }
      else
      {
        printf("File '%s' is already undeleted.\n", filename);
        return;
      }
    }
  }
  printf("File '%s' not found.\n", filename);
}

//attribute
void attrib(char *attribute , char *filename)
{
  if (!image_open) // Check if a file system is currently open
  {
    printf("Error: No file system is currently open.\n");
    return;
  }
  // Find the inode index of the file in the directory
  int32_t inode_index = -1;
  for (int i = 0; i < NUM_FILES; i++)
  {
    if (strcmp(directory[i].filename, filename) == 0)
    {
      inode_index = directory[i].inode;
      break;
    }
  }
  // Check if the file exists in the directory
  if (inode_index == -1)
  {
    printf("attrib: File '%s' not found.\n", filename);
    return;
  }
  // Convert the attribute string to an integer
  int attribute_val = atoi(attribute);
  if (attribute_val < 0 || attribute_val > 255) // Check if the attribute is within the valid range
  {
    printf("Error: Attribute must be an integer between 0 and 255.\n");
    return;
  }

  inodes[inode_index].attribute = (uint8_t)attribute_val; // Set the attribute of the file to the specified value
  printf("Attribute set.\n");
}


//Reading File
//read a file with a given filename starting from a specific byte offset and print out the specified number of bytes
void Read(char *filename, int32_t start_bytes, int32_t number_bytes)
{
  fp = fopen(filename, "r"); //Opening file
    if(fp == NULL)// Checking if the file exists or not
    { 
        printf("ERROR: File not found\n");
        return;
    }
    if(fseek(fp, start_bytes, SEEK_SET) != 0) //move the file pointer to the specified start byte
    {
        printf("ERROR: Invalid start byte\n");
        fclose(fp);
        return;
    }
    //allocate memory for a buffer to hold the specified number of bytes
    char * buffer = (char *)malloc(number_bytes);
    //read the specified number of bytes into the buffer
    int32_t read_bytes = fread(buffer, 1, number_bytes, fp);
    
    if(read_bytes != number_bytes) // Checking if the bytes read is not equal to the number of bytes
    {
        printf("ERROR: Invalid number of bytes\n");
        fclose(fp);
        return;
    }
    for(read_bytes = 0; read_bytes < number_bytes; read_bytes++)
    {
        printf("%x ", buffer[read_bytes]); //print out the bytes in hexadecimal format
    }
    printf("\n");
    fclose(fp); //Closing File
}


void insert(char *filename)
{
  if (filename == NULL)
  {
    printf("ERROR:Filename is NULL.\n");
    return;
  }

  struct stat buf;
  int ret = stat(filename, &buf);

  if (ret == -1)
  {
    printf("ERROR: File does not exist. \n");
    return;
  }

  if (buf.st_size > MAX_FILE_SIZE)
  {
    printf("ERROR: File is too large. \n");
    return;
  }

  if (buf.st_size > df())
  {
    printf("ERROR: Not Enough free disk space \n");
    return;
  }

  int i;
  int directory_entry = -1;
  for (i = 0; i < NUM_FILES; i++)
  {
    if (directory[i].in_use == 0)
    {
      directory_entry = i;
      break;
    }
  }

  if (directory_entry == -1)
  {
    printf("ERROR: Could not find a free directory entry.\n");
    return;
  }

  FILE *ifp = fopen(filename, "r");
  printf("Reading %d bytes from %s\n", (int)buf.st_size, filename);

  // Save off the size of the input file since we'll use it in a couple of places and
  // also initialize our index variables to zero.
  int32_t copy_size = buf.st_size;

  // We want to copy and write in chunks of BLOCK_SIZE. So to do this
  // we are going to use fseek to move along our file stream in chunks of BLOCK_SIZE.
  // We will copy bytes, increment our file pointer by BLOCK_SIZE and repeat.
  int32_t offset = 0;

  // We are going to copy and store our file in BLOCK_SIZE chunks instead of one big
  // memory pool. Why? We are simulating the way the file system stores file data in
  // blocks of space on the disk. block_index will keep us pointing to the area of
  // the area that we will read from or write to.
  int32_t block_index = -1;

  // find a fre inode
  int32_t inode_index = findFreeInode();
  {
    if (inode_index == -1)
    {
      printf("ERROr: Can not find a free inode.\n");
      return;
    }
  }

  // place the file info in the directory
  directory[directory_entry].in_use = 1;
  directory[directory_entry].inode = inode_index;

  strncpy(directory[directory_entry].filename, filename, strlen(filename));

  inodes[inode_index].file_size = buf.st_size;

  // copy_size is initialized to the size of the input file so each loop iteration we
  // will copy BLOCK_SIZE bytes from the file then reduce our copy_size counter by
  // BLOCK_SIZE number of bytes. When copy_size is less than or equal to zero we know
  // we have copied all the data from the input file.

  while (copy_size > 0)
  {
    // Index into the input file by offset number of bytes.  Initially offset is set to
    // zero so we copy BLOCK_SIZE number of bytes from the front of the file.  We
    // then increase the offset by BLOCK_SIZE and continue the process.  This will
    // make us copy from offsets 0, BLOCK_SIZE, 2*BLOCK_SIZE, 3*BLOCK_SIZE, etc.
    fseek(ifp, offset, SEEK_SET);

    // Read BLOCK_SIZE number of bytes from the input file and store them in our
    // data array.

    // find a freee block
    block_index = findFreeBlock();

    if (block_index == -1)
    {
      printf("ERROR; Can not find a free block");
      return;
    }

    int32_t bytes = fread(data[block_index], BLOCK_SIZE, 1, ifp);

    // saving blocks
    int32_t inode_block = findFreeInodeBlock(inode_index);
    inodes[inode_index].blocks[inode_block] = block_index;

    // If bytes == 0 and we haven't reached the end of the file then something is
    // wrong. If 0 is returned and we also have the EOF flag set then that is OK.
    // It means we've reached the end of our input file.
    if (bytes == 0 && !feof(ifp))
    {
      printf("ERROR: An error occured reading from the input file.\n");
      return;
    }

    // Clear the EOF file flag.
    clearerr(ifp);

    // Reduce copy_size by the BLOCK_SIZE bytes.
    copy_size -= BLOCK_SIZE;

    // Increase the offset into our input file by BLOCK_SIZE.  This will allow
    // the fseek at the top of the loop to position us to the correct spot.
    offset += BLOCK_SIZE;

    // Increment the index into the block array
    // DO NOT just increment block index in your file system
    block_index = findFreeBlock();
  }

  // We are done copying from the input file so close it out.
  fclose(ifp);
}



int main()
{

  char *command_string = (char *)malloc(MAX_COMMAND_SIZE);

  fp = NULL;

  init();

  while (1)
  {
    // Print out the msh prompt
    printf("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while (!fgets(command_string, MAX_COMMAND_SIZE, stdin))
      ;

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    for (int i = 0; i < MAX_NUM_ARGUMENTS; i++)
    {
      token[i] = NULL;
    }

    int token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr = NULL;

    char *working_string = strdup(command_string);

    // we are going to move the working_string pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *head_ptr = working_string;
    int nf_count = 0;

    if (working_string[0] == '\n' || working_string[0] == ' ' || working_string[0] == '\t')
    {
      continue;
    }
    // Tokenize the input strings with whitespace used as the delimiter
    while (((argument_ptr = strsep(&working_string, WHITESPACE)) != NULL) &&
           (token_count < MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
      if (strlen(token[token_count]) == 0)
      {
        token[token_count] = NULL;
      }
      token_count++;
    }

    if (strcmp("createfs", token[0]) == 0)
    {
      if (token[1] == NULL)
      {
        printf("ERROR: No filename specified\n");
        continue;
      }
      createfs(token[1]);
    }

    if (strcmp("savefs", token[0]) == 0)
    {
      savefs();
    }

    if (strcmp("open", token[0]) == 0)
    {
      if (token[1] == NULL)
      {
        printf("ERROR: No filename specified\n");
        continue;
      }
      openfs(token[1]);
    }

    if (strcmp("close", token[0]) == 0)
    {
      closefs();
    }

    if (strcmp("list", token[0]) == 0)
    {
      
      if (!image_open)
      {
        printf("ERROR: Disk image is not opened.\n");
        continue;
      }
      list();
    }

    if (strcmp("df", token[0]) == 0)
    {
      if (!image_open)
      {
        printf("ERROR: Disk image is not opened.\n");
        continue;
      }
      printf("%d bytes free\n", df());
    }

    if (strcmp("insert", token[0]) == 0)
    {
      if (!image_open)
      {
        printf("ERROR: Disk image is not opened. \n");
        continue;
      }

      if (token[1] == NULL)
      {
        printf("ERROR:No filename specified.\n");
        continue;
      }
      insert(token[1]);
    }

    //if command was encrypt
    if (strcmp("encrypt", token[0]) == 0)
    {

      printf("%s %s %s ", token[0], token[1], token[2]);
      int password_length = strlen(token[2]);

      if (password_length < 8)
      {
        printf("The password needs to have at least 8 characters.\n");
        continue;
      }
      encrypt(token[1], token[2]);
    }

    //if command was decrypt
    if (strcmp("decrypt", token[0]) == 0)
    {

      printf("%s %s %s ", token[0], token[1], token[2]);
      int password_length = strlen(token[2]);

      if (password_length < 8)
      {
        printf("The password needs to have at least 8 characters.\n");
        continue;
      }
      decrypt(token[1], token[2]);
    }

    // if command was delete
    if (strcmp("delete", token[0]) == 0) 
    {
      if (!image_open) 
      {
        printf("ERROR: Disk image is not opened.\n");
        continue;
      }
      if (token[1] == NULL) 
      {
        printf("ERROR: No filename specified.\n");
        continue;
      }
      delete_file(token[1]);
    }

    // if command was undelete
    if (strcmp("undelete", token[0]) == 0) 
    {
      if (!image_open) 
      {
        printf("ERROR: Disk image is not opened.\n");
        continue;
      }
      if (token[1] == NULL) 
      {
        printf("ERROR: No filename specified.\n");
        continue;
      }
      undelete_file(token[1]);
    }

     // if command was retrieve
    if (strcmp("retrieve", token[0]) == 0)
    {

      if (token[1] == NULL)
      {

        printf("ERROR: File cannot be found.\n");
        continue;

      }
      retrieve( token[1] , token[2] );

    }

    // if the command was read

    if(strcmp("read", token[0]) == 0)
    {

      if(token[1] == NULL || token[2] == NULL || token[3] == NULL)
      {

          printf("ERROR: Improper Usage(read <filename> <starting bytes> <number of bytes>).\n");
          continue;

      }

      Read(token[1], atoi(token[2]), atoi(token[3]));

    }

    // if the command was attrib

    if(strcmp("attrib", token[0]) == 0)
    {
      if ( token[1] == NULL)
      {

        printf("ERROR: Improper Usage. attrib [+attribute] [-attribute] <filename>\n Attrib must contain an argument\n");
        continue;

      }
      attrib( token[1] , token[2] );

    }

    // // Now print the tokenized input as a debug check

    // int token_index = 0;
    // for (token_index = 0; token_index < token_count; token_index++)
    // {
    //   printf("token[%d] = %s\n", token_index, token[token_index]);
    // }

    // exit the shell if user enters quit or exit
    if (strcmp(token[0], "quit") == 0 || strcmp(token[0], "exit") == 0)
    {
      exit(0);
    }

    // Cleanup allocated memory
    for (int i = 0; i < MAX_NUM_ARGUMENTS; i++)
    {
      if (token[i] != NULL)
      {
        free(token[i]);
      }
    }

    free(head_ptr);
  }

  free(command_string);

  return 0;
  // e2520ca2-76f3-90d6-0242ac120003
}