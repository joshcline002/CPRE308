/* bsdump.c 
 * 
 * Reads the boot sector of an MSDOS floppy disk
 *
 * Starting bytes and length of bytes gotten from http://www.eit.lth.se/fileadmin/eit/courses/eitn50/Projekt1/FAT12Description.pdf
 */
 
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#define SIZE 32  /* size of the read buffer */
//define PRINT_HEX // un-comment this to print the values in hex for debugging

struct BootSector
{
    unsigned char  sName[9];            // The name of the volume
    unsigned short iBytesSector;        // Bytes per Sector
    
    unsigned char  iSectorsCluster;     // Sectors per Cluster
    unsigned short iReservedSectors;    // Reserved sectors
    unsigned char  iNumberFATs;         // Number of FATs
    
    unsigned short iRootEntries;        // Number of Root Directory entries
    unsigned short iLogicalSectors;     // Number of logical sectors
    unsigned char  xMediumDescriptor;   // Medium descriptor
    
    unsigned short iSectorsFAT;         // Sectors per FAT
    unsigned short iSectorsTrack;       // Sectors per Track
    unsigned short iHeads;              // Number of heads
    
    unsigned short iHiddenSectors;      // Number of hidden sectors
};


unsigned short endianSwap(unsigned char one, unsigned char two);
// Pre: Two initialized characters
// Post: The characters are swapped (two, one) and returned as a short

void decodeBootSector(struct BootSector * pBootS, unsigned char buffer[]);
// Pre: An initialized BootSector struct and a pointer to an array
//      of characters read from a BootSector
// Post: The BootSector struct is filled out from the buffer data

void printBootSector(struct BootSector * pBootS);
// Pre: A filled BootSector struct
// Post: The information about the boot sector prints to the console


// entry point:
int main(int argc, char * argv[])
{
	int pBootSector = 0;
	unsigned char buffer[SIZE];
	struct BootSector sector;
    
	// Check for argument
	if (argc < 2) {
		printf("Specify boot sector\n");
		exit(1);
	}
    
	// Open the file and read the boot sector
	pBootSector = open(argv[1], O_RDONLY);
	read(pBootSector, buffer, SIZE);
    close(pBootSector);
    
	// Decode the boot Sector
	decodeBootSector(&sector, buffer);
    
	// Print Boot Sector information
	printBootSector(&sector);
    
    return 0;
}  // end main()


// Converts two characters to an unsigned short with two, one
unsigned short endianSwap(unsigned char one, unsigned char two)
{
    //MSB is on the right so two contains the MSB
    short MSB = two;
    
    //Move MSB to left to make Big Endian
    MSB = MSB << 8;
    
    // LSB is on the left so one contains the LSB
    short LSB = one;
    
    //Or the two together to get return short.
    short ret = MSB | LSB;
    
    // This is stub code!
	return ret;
}


// Fills out the BootSector Struct from the buffer
void decodeBootSector(struct BootSector * pBootS, unsigned char buffer[])
{
	int i = 3;  // Skip the first 3 bytes
    
	// Pull the name and put it in the struct (remember to null-terminate)
	while (i<11){
	    pBootS->sName[i-3] = buffer[i]; //get 3-10 in buffer to put in 0 - 7 for pBoots->NAME. 
	    if(i ==10){
		//Null terminate name.
		pBootS->sName[8] = 0;
	    }
	    i++;	    
	}
    
	// Read bytes/sector and convert to big endian
	
	    //Bytes/sector starting byte = 11 length in bytes = 2.
	unsigned char one = buffer[i]; // i = 11 which is the starting byte of Bytes per sector
	i++;
	unsigned char two = buffer[i];//i = 12 because bytes/sector is two bytes long.
	i++;
	
	pBootS->iBytesSector = endianSwap(one, two);
	// Read sectors/cluster, Reserved sectors and Number of Fats
	
	    //Sectors/Cluster Starting Byte = 13 Length in Bytes = 1.
	one = buffer[i]; // i =13 because sectors/cluster starts at 13.
	two = 0; //Sectors/Cluster is only 1 byte long so two = 0.
	i++;
	
	pBootS->iSectorsCluster = endianSwap(one,two);
	
	    //Reserved Sectors starting byte = 14 length in bytes = 2.
	one = buffer[i];
	i++;
	two = buffer[i];
	i++;
	
	pBootS->iReservedSectors = endianSwap(one,two);
	
	    //Number of Fats starting byte = 16 length in bytes = 1.
	one = buffer[i];
	two = 0;
	i++;
	
	pBootS->iNumberFATs = endianSwap(one,two);
	
	// Read root entries, logicical sectors and medium descriptor
	
	    //Root entries Starting byte 17 length of bytes 2.
	one = buffer[i];
	i++;
	two = buffer[i];
	i++;
	
	pBootS->iRootEntries = endianSwap(one,two);
	
	    //Logical Sectors Starting byte 19 length of bytes 2.
	one = buffer[i];
	i++;
	two = buffer[i];
	i++;
	
	pBootS->iLogicalSectors = endianSwap(one,two);
	
	    //Medium Descriptor Starting byte 21 length of bytes 1.
	one = buffer[i];
	two = 0;
	i++;
	
	pBootS->xMediumDescriptor = endianSwap(one,two);
	
	// Read and convert sectors/fat, sectors/track, and number of heads
	    //Sectors/fat starting byte = 22 length of bytes 2.
	one = buffer[i];
	i++;
	two = buffer[i];
	i++;
	
	pBootS->iSectorsFAT = endianSwap(one,two);
	 
	    //Sectors/track starting byte = 24 length of bytes 2.
	one = buffer[i];
	i++;
	two = buffer[i];
	i++;
	
	pBootS->iSectorsTrack = endianSwap(one, two);
	
	    //Number of heads starting byte = 26 length of bytes 2.
	one = buffer[i];
	i++;
	two = buffer[i];
	i++;
	
	pBootS->iHeads = endianSwap(one,two);
	// Read hidden sectors
	    //Hidden Sectors starting byte = 28 length of bytes 2.
	one = buffer[i];
	i++;
	two = buffer[i];
	
	pBootS->iHiddenSectors = endianSwap(one,two);
    
	return;
}


// Displays the BootSector information to the console
void printBootSector(struct BootSector * pBootS)
{
#ifndef PRINT_HEX
	printf("                    Name:   %s\n", pBootS->sName);
	printf("            Bytes/Sector:   %i\n", pBootS->iBytesSector);
	printf("         Sectors/Cluster:   %i\n", pBootS->iSectorsCluster);
	printf("        Reserved Sectors:   %i\n", pBootS->iReservedSectors);
	printf("          Number of FATs:   %i\n", pBootS->iNumberFATs);
	printf("  Root Directory entries:   %i\n", pBootS->iRootEntries);
	printf("         Logical sectors:   %i\n", pBootS->iLogicalSectors);
	printf("       Medium descriptor:   0x%04x\n", pBootS->xMediumDescriptor);
	printf("             Sectors/FAT:   %i\n", pBootS->iSectorsFAT);
	printf("           Sectors/Track:   %i\n", pBootS->iSectorsTrack);
	printf("         Number of heads:   %i\n", pBootS->iHeads);
	printf("Number of Hidden Sectors:   %i\n", pBootS->iHiddenSectors);
#else
	printf("                    Name:   %s\n",     pBootS->sName);
	printf("            Bytes/Sector:   0x%04x\n", pBootS->iBytesSector);
	printf("         Sectors/Cluster:   0x%02x\n", pBootS->iSectorsCluster);
	printf("        Reserved Sectors:   0x%04x\n", pBootS->iReservedSectors);
	printf("          Number of FATs:   0x%02x\n", pBootS->iNumberFATs);
	printf("  Root Directory entries:   0x%04x\n", pBootS->iRootEntries);
	printf("         Logical sectors:   0x%04x\n", pBootS->iLogicalSectors);
	printf("       Medium descriptor:   0x%02x\n", pBootS->xMediumDescriptor);
	printf("             Sectors/FAT:   0x%04x\n", pBootS->iSectorsFAT);
	printf("           Sectors/Track:   0x%04x\n", pBootS->iSectorsTrack);
	printf("         Number of heads:   0x%04x\n", pBootS->iHeads);
	printf("Number of Hidden Sectors:   0x%04x\n", pBootS->iHiddenSectors);
#endif
	return;
}
