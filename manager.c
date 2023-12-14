#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define PAGE_SIZE 256
#define TLB_SIZE 16
#define FRAME_SIZE 256
#define PAGE_TABLE_SIZE 256
#define MEMORY_SIZE 65536

// tlb structure
typedef struct {
    int page_number;
    int frame_number;
} TLBEntry;

// Declarations
int readFromBackingStore(int pageNumber, char* backingStore, void* destination, size_t elementSize);
int getPageTableEntry(int pageNumber, int* pageTable);
int searchTLB(int pageNumber, TLBEntry* tlb);
void updateTLB(int pageNumber, int frameNumber, TLBEntry* tlb);

int main() {
    FILE* addressesFile = fopen("addresses.txt", "r");
    FILE* correctFile = fopen("correct.txt", "w");
    FILE* backingStoreFile = fopen("BACKING_STORE.bin", "rb");

    if (!addressesFile || !correctFile || !backingStoreFile) {
        fprintf(stderr, "Error opening files.\n");
        return 1;
    }

    // Initialize TLB and page table
    TLBEntry tlb[TLB_SIZE];
    int pageTable[PAGE_TABLE_SIZE];

    // Initialize TLB and page table entries
    for (int i = 0; i < TLB_SIZE; ++i) {
        tlb[i].page_number = -1;
        tlb[i].frame_number = -1;
    }

    for (int i = 0; i < PAGE_TABLE_SIZE; ++i) {
        pageTable[i] = -1;
    }

    // Buffer to store the logical addresses
    int logicalAddress;
    int destinationArray[PAGE_SIZE];  

    // page fault and tbl trackers
    int pageFaultCount = 0;
    int tlbHitCount = 0;

    // read logical addresses from file
    while (fscanf(addressesFile, "%d", &logicalAddress) != EOF) {
        // Extract page number and offset
        int pageNumber = (logicalAddress >> 8) & 0xFF;
        int offset = logicalAddress & 0xFF;

        // search tlb
        int frameNumber = searchTLB(pageNumber, tlb);

        // if tlb miss consult page table
        if (frameNumber == -1) {
            pageFaultCount++; 

            frameNumber = getPageTableEntry(pageNumber, pageTable);

            // if page table miss read from backing store
            if (frameNumber == -1) {
                readFromBackingStore(pageNumber, "BACKING_STORE.bin", destinationArray, sizeof(int));
                frameNumber = pageNumber % FRAME_SIZE;
                pageTable[pageNumber] = frameNumber;
            }

            // update tlb
            updateTLB(pageNumber, frameNumber, tlb);
        } else {
            tlbHitCount++; 
        }

        // calculate physical address
        int physicalAddress = (frameNumber << 8) | offset;

        // read byte from physical memory
        fseek(backingStoreFile, physicalAddress, SEEK_SET);
        char value;
        fread(&value, sizeof(char), 1, backingStoreFile);

        // write result
        fprintf(correctFile, "%d\n", value);
    }

    // display page-fault rate and tlb hit rate -_-
    float pageFaultRate = (float)pageFaultCount / (float)(pageFaultCount + tlbHitCount) * 100.0;
    float tlbHitRate = (float)tlbHitCount / (float)(pageFaultCount + tlbHitCount) * 100.0;

    printf("Page-fault rate: %.2f%%\n", pageFaultRate);
    printf("TLB hit rate: %.2f%%\n", tlbHitRate);

    // Close Files
    fclose(addressesFile);
    fclose(correctFile);
    fclose(backingStoreFile);

    return 0;
}

int readFromBackingStore(int pageNumber, char* backingStore, void* destination, size_t elementSize) {
    FILE* file = fopen(backingStore, "rb");
    if (!file) {
        fprintf(stderr, "Error opening backing store file.\n");
        exit(1);
    }

    fseek(file, pageNumber * PAGE_SIZE, SEEK_SET);

    char* page = (char*)malloc(PAGE_SIZE);
    if (!page) {
        fprintf(stderr, "Memory allocation error.\n");
        exit(1);
    }

    fread(page, sizeof(char), PAGE_SIZE, file);

    fclose(file);

    // Determine frame num
    int frameNumber = pageNumber % FRAME_SIZE;

    // Copy page to physical 
    memcpy(destination, page, PAGE_SIZE * elementSize);

    // Free allocated
    free(page);

    return frameNumber;
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
    return -1; // tlb miss    :'(
} 

void updateTLB(int pageNumber, int frameNumber, TLBEntry* tlb) {
    for (int i = TLB_SIZE - 1; i > 0; --i) {
        tlb[i] = tlb[i - 1];
    }

    // Insert new entry at the beginning :)
    tlb[0].page_number = pageNumber;
    tlb[0].frame_number = frameNumber;
}





// I was sick and hyped up on NyQuil when I made this code and as such I dont remember why 
// exactly I did certain things they way I did. 