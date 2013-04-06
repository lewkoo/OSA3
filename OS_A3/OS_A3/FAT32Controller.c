//
//  FAT32Controller.c
//  OS_A3
//
//  Created by Levko Ivanchuk on 2013-04-03.
//  Copyright (c) 2013 Levko Ivanchuk. All rights reserved.
//

#include <stdio.h>
#include "FAT32Controller.h"
#include "BSParser.h"
#include "dir.h"
#include "Util.h"

extern void* memset(); //to suppress the memset warninig.

enum FATType{
    FAT16,
    FAT32
}FATType;

enum FATType fatType;
char *diskImageLocaiton = NULL;
fat32BS *boot_sector = NULL;


void startCL(){
    locateRootDir();
}

uint8_t validateBS(fat32BS *boot_sector){
    
    uint8_t result = checkFAT16Descriptors(boot_sector);
    result = checkSignatureBytes(boot_sector);
    
    //perform calculations on P14
    //instanciate variables
    uint16_t FATSz;
    uint32_t TotSec;
    uint32_t DataSec;
    uint32_t CountofClusters;
    
    uint16_t RootDirSectors = ((boot_sector->BPB_RootEntCnt *32) +(boot_sector->BPB_BytesPerSec -1)) / boot_sector->BPB_BytesPerSec;
    
    
        FATSz = boot_sector->BPB_FATSz32;

        TotSec = boot_sector->BPB_TotSec32;

    
    DataSec = TotSec - (boot_sector->BPB_RsvdSecCnt + (boot_sector->BPB_NumFATs * FATSz) + RootDirSectors );
    
    CountofClusters = DataSec / boot_sector->BPB_SecPerClus;
    
    if(CountofClusters < 4085){
        //volume is FAT12
        result = 0;
    }else if(CountofClusters < 65525){
        //volume is FAT16
        fatType = FAT16;
        result = 0;
    }else{
        //volume is FAT32
        fatType = FAT32;
        //fprintf(stderr,"%s\n\n", "Count of clusters >= 65525. It is FAT32.");
        result = 1;
    }
    
    return result;
}

void locateRootDir(){
    //root dir is pointed to by BPB_RootClus
    //but that is just a cluster
    //either way, we have the cluster number (usually 2)
    //use calculateFATEntry to get the data region of Nth cluster
    
    bool testResult = FALSE;
    
    testResult = validateBootSector();
    
    if(testResult == FALSE){
        fprintf(stderr, "Failed to validate Boot Sector. Exit.");
        //break;
    }
    
    testResult = validateDiskImageLocation();
    
    if(testResult == FALSE){
        fprintf(stderr, "Failed to validate Disk Image Location. Exit.");
        //break;
    }
    
    long currPos; //for testing
    size_t n = 0; // number of bytes read
    FILE *source;
    size_t BUFFER_SIZE = boot_sector->BPB_BytesPerSec; //size of the sector (in bytes)
    unsigned char buffer[BUFFER_SIZE];
    memset(&buffer,0,sizeof(buffer));
    
    //The White paper Way
     
    uint32_t rootDirCLusterNum = boot_sector->BPB_RootClus;
    uint64_t FirstSectorofCluster = getDataOnClusterNum(rootDirCLusterNum, boot_sector);
    
     
    source = fopen(diskImageLocaiton, "r");
    
    int result = fseek(source, (long)FirstSectorofCluster*boot_sector->BPB_BytesPerSec, SEEK_SET);
    
    if(result == 0){
        //success
    }else{
        fprintf(stderr, "Failed to seek.");
    }
    
    
    if(source){
        n = fread(buffer, 1, BUFFER_SIZE, source);
    }
    
    if(n == BUFFER_SIZE){
    //processing
    fatDir *tstStruct = (fatDir*)&buffer;
    }else{
        fprintf(stderr, "Error in read.");
    }
        
}



void setDiskImageLocation(char *diskImageLoc){
    diskImageLocaiton = diskImageLoc;
}

void setBootSector(fat32BS *boot_sec){
    boot_sector = boot_sec;
}

bool validateDiskImageLocation(){
    if(diskImageLocaiton != NULL){
        return TRUE;
    }else{
        return FALSE;
    }
}
bool validateBootSector(){
    if(boot_sector != NULL){
        return TRUE;
    }else{
        return FALSE;
    }
}



