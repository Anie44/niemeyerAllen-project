#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGE_SIZE 256
#define TLB_SIZE 16
#define FRAME_SIZE 256
#define PAGE_TABLE_SIZE 256
#define MEMORY_SIZE 65536

typedef struct {
    int page_number;
    int frame_number;
} TLBEntry;

int readFromBackingStore(int pageNumber, FILE* backingStoreFile, signed char* destination, size_t elementSize);
int getPageTableEntry(int pageNumber, int* pageTable);
int searchTLB(int pageNumber, TLBEntry* tlb);
void updateTLB(int pageNumber, int frameNumber, TLBEntry* tlb);

int main() {
    FILE* addressesFile = fopen("addresses.txt", "r");
    FILE* backingStoreFile = fopen("BACKING_STORE.bin", "rb");

    if (!addressesFile || !backingStoreFile) {
        fprintf(stderr, "Error opening files.\n");
        return 1;
    }

    TLBEntry tlb[TLB_SIZE];
    int pageTable[PAGE_TABLE_SIZE];

    for (int i = 0; i < TLB_SIZE; ++i) {
        tlb[i].page_number = -1;
        tlb[i].frame_number = -1;
    }

    for (int i = 0; i < PAGE_TABLE_SIZE; ++i) {
        pageTable[i] = -1;
    }

    int logicalAddress;
    signed char destinationArray[PAGE_SIZE];

    int pageFaultCount = 0;
    int tlbHitCount = 0;

    while (fscanf(addressesFile, "%d", &logicalAddress) != EOF) {
        int pageNumber = (logicalAddress >> 8) & 0xFF;
        int offset = logicalAddress & 0xFF;

        int frameNumber = searchTLB(pageNumber, tlb);

        if (frameNumber == -1) {
            pageFaultCount++;

            frameNumber = getPageTableEntry(pageNumber, pageTable);

            if (frameNumber == -1) {
                frameNumber = readFromBackingStore(pageNumber, backingStoreFile, destinationArray, sizeof(signed char));
                pageTable[pageNumber] = frameNumber;
            }

            updateTLB(pageNumber, frameNumber, tlb);
        } else {
            tlbHitCount++;
        }

        int physicalAddress = (frameNumber * PAGE_SIZE) + offset;

        fseek(backingStoreFile, physicalAddress, SEEK_SET);
        signed char value;
        fread(&value, sizeof(signed char), 1, backingStoreFile);

        printf("%d, %d, %d\n", logicalAddress, physicalAddress, value);
    }

    float totalReferences = (float)(pageFaultCount + tlbHitCount);
    float pageFaultRate = (float)pageFaultCount / totalReferences * 100.0;
    float tlbHitRate = (float)tlbHitCount / totalReferences * 100.0;

    printf("Page-fault rate: %.2f%%\n", pageFaultRate);
    printf("TLB hit rate: %.2f%%\n", tlbHitRate);

    fclose(addressesFile);
    fclose(backingStoreFile);

    return 0;
}

int readFromBackingStore(int pageNumber, FILE* backingStoreFile, signed char* destination, size_t elementSize) {
    fseek(backingStoreFile, pageNumber * PAGE_SIZE, SEEK_SET);
    fread(destination, elementSize, PAGE_SIZE, backingStoreFile);
    return pageNumber % FRAME_SIZE;
}

int getPageTableEntry(int pageNumber, int* pageTable) {
    return pageTable[pageNumber];
}

int searchTLB(int pageNumber, TLBEntry* tlb) {
    for (int i = 0; i < TLB_SIZE; ++i) {
        if (tlb[i].page_number == pageNumber) {
            return tlb[i].frame_number;
        }
    }
    return -1;
}

void updateTLB(int pageNumber, int frameNumber, TLBEntry* tlb) {
    for (int i = TLB_SIZE - 1; i > 0; --i) {
        tlb[i] = tlb[i - 1];
    }
    tlb[0].page_number = pageNumber;
    tlb[0].frame_number = frameNumber;
}





// I was sick and drugged up on NyQuil for most of the time I worked on this 
// code so I dont remember half of why I did what I did. 
// Does it produce an outcome? yes
// Is it the most desirable? probably not
// 




//⠀⠀⠀⠀⠀⠀⢀⣤⡿⠟⠛⠉⠉⠉⠀⠀⠁⠉⠉⠚⠫⢷⢤⡀⠀⠀⠀⠀⠀⠀
//⠀⠀⠀⠀⣠⠞⠋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠋⢷⣄⠀⠀⠀⠀
//⠀⠀⣠⠾⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠷⡀⠀⠀
//⢀⡴⠅⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢙⣄⠀
//⣽⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠊⢆
//⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠸
//⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⡤⠖⠀⠀⠀⠀⠀⠙⠦⢤⣀⠀⠀⠀⠀⠀⢸
//⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠚⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸
//⡇⠀⠀⠀⠀⠀⠀⢀⣀⣀⠤⠤⠤⠤⠒⠒⠒⢦⠀⣤⠦⠤⠤⣀⣀⠀⠀⠀⠀⣸
//⣿⠀⠀⢠⠖⠊⠉⠁⠀⠀⠀⢀⣤⣤⣤⣄⠀⡸⠀⠘⣄⢠⣴⣾⣿⣿⣍⠁⢲⣿
//⢿⣧⠀⠈⢷⣀⠀⠀⠀⠀⣼⣿⣿⣿⣿⣿⡟⠁⠀⠀⠘⠿⣿⣿⣿⣿⣿⣷⣾⠋
//⠀⠿⣧⡀⠀⠙⠳⣦⣤⣀⣿⣿⣿⣿⠟⠋⠀⠀⠀⠀⠀⠀⠈⠉⠉⠉⠁⣤⠋⠀
//⠀⠀⠈⢯⣄⡀⠀⠀⠉⠉⠉⠉⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣖⠟⠁⠀⠀
//⠀⠀⠀⠀⠉⠻⣦⢄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣴⠟⠁⠀⠀⠀⠀
//⠀⠀⠀⠀⠀⠀⠀⢉⠛⠷⣢⡤⢄⢄⣠⢀⡄⠤⢤⣶⠮⢛⣋⠀⠀⠀⠀⠀⠀⠀