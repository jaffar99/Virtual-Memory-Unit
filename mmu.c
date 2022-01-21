#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


signed char output;
int pnum = -1, poffset = -1;
int num_of_TLBhits = 0, TLBindex = 0;
int num_of_operations = 0, num_of_frames = 0, num_faults = 0;
int logadr;
int n = -1;
char adr[10];
int frame = 0;
int min_index = 0;
int physical_address = 0;
int s = 0;
int total_frames = 0, total_pages = 0;
FILE *out;                       //output file
FILE *backing_store_bin;         //backing store.bin
FILE *addresses;                 //address.txt


typedef struct // create a standard page structure for tlb
{
    int pagenum;
    int framenum;
} pages;

pages TLB[16];
int main(int argc, char *argv[]) // argv={"./mmu","256","backing_store.bin", "addresses.txt"}

{
    if (!strcmp(argv[1], "256"))
    {
        total_frames = 256;
        total_pages = 256;

        out = fopen("output256.csv", "w+");
    }
    if (!strcmp(argv[1], "128"))
    {
        total_frames = 128;
        total_pages = 128;

        out = fopen("output128.csv", "w+");
    }
    if (((strcmp(argv[1], "256")) && (strcmp(argv[1], "128"))) || (argc != 4))
    {
        printf("invalid arguments. Please enter input in correct format \n");
        return -1;
    }
    pages pagetable[total_pages];
    signed char phymem[total_frames][256];
    backing_store_bin = fopen(argv[2], "r");
    addresses = fopen(argv[3], "r");
    //initialize
    int i=0;
    for ( i = 0; i < total_pages; i++)
    {
        pagetable[i].pagenum = -1;
        pagetable[i].framenum = -1;
    }
     i=0;
    for ( i = 0; i < 16; i++)
    {
        TLB[i].pagenum = -1;
        TLB[i].framenum = -1;
    }
    i=0;
    int j=0;
    for ( i = 0; i < total_frames; i++)
    {
        for (j = 0; j < 256; j++)
        {
            phymem[i][j] = 0; //assuming physical memory always has some sort of value
        }
    }

    while (fgets(adr, 10, addresses))
    {
        n = -1;
        int found = 0;
        logadr = atoi(adr); //fetch the logical address
        num_of_operations = num_of_operations + 1;

        //masking and bit shifting
        pnum = (logadr >> 8) & 0x00FF; //page number= bits 8-16 = shift 8 bits to the right
                                       // and delete the bits 16-32 by masking
        poffset = logadr & 0x00FF;     //last 8 bits.
        //check TLB next
         i=0;
        for ( i = 0; i < 16; i++)
        {
            if (TLB[i].pagenum == pnum) //TLB hit
            {
                frame = TLB[i].framenum;

                num_of_TLBhits++;
                found = 1;

                if (total_pages == 128)
                {         j=0;
                    for ( j = 0; j < total_pages; j++) //
                    {
                        if ((pagetable[j].pagenum == pnum)) //find the index where TLB hit page actually is located inside page tab;e
                        {
                            s = j; //record the index in s
                            break;
                        }
                    }

                    frame = (num_of_frames + 1) % 128;
                    pages element = pagetable[s];
                    int k=0;
                    for ( k = s; k > 0; k--)
                    {
                        pagetable[k] = pagetable[k - 1];
                    }
                    pagetable[0] = element;
                    frame = pagetable[0].framenum;
                }
                break;
            }
        }
        if (found == 0) //TLB miss //CHECK PAGE TABLE
        {
            if ((total_pages == 256) && (pagetable[pnum].framenum >= 0)) //check if exists in page table
            {
                n = pnum;
                frame = pagetable[pnum].framenum;
            }
            if ((total_pages == 128))
            {
                  i=0;
                for ( i = 0; i < total_pages; i++)
                {
                    if ((pagetable[i].pagenum == pnum) && (pagetable[i].framenum >= 0))
                    {
                        n = i;
                        break;
                    }
                }
                if (n != -1)
                {
                    
                    pages e = pagetable[n];
                    for ( j = n; j > 0; j--)
                    {
                        pages temp = pagetable[j - 1];
                        pagetable[j] = temp;
                    }
                    pagetable[0] = e;
                    frame = pagetable[0].framenum;
                }
            }

            if (n == -1) // else it is a page fault. update memory
            {
                num_faults = num_faults + 1;
                if (total_frames == 256)
                {
                    frame = num_of_frames % total_frames;

                    num_of_frames = num_of_frames + 1;

                    pagetable[pnum].framenum = frame;
                }
                if (total_frames == 128)
                {

                    int newframe = pagetable[127].framenum;
                    for ( i = 127; i > 0; i--)
                    {

                        pages temp = pagetable[i - 1];
                        pagetable[i] = temp; //push each item down because the last element is accessed last
                    }

                    pagetable[0].pagenum = pnum;
                    if (num_of_frames > 127)
                    {
                        pagetable[0].framenum = newframe;
                        frame = pagetable[0].framenum;
                    }
                    else
                    {
                        frame = num_of_frames % 128;
                        pagetable[0].framenum = frame;
                    }
                    num_of_frames = num_of_frames + 1;

                    /* pagetable[min_index].framenum=frame;
                       frame = num_of_frames;
                       num_of_frames=num_of_frames+1;
                       num_of_frames=(num_of_frames)%total_frames; */

                    //num_of_frames=num_of_ printf("%d\n", num_of_frames);frames % total_frames;
                }

                if (fseek(backing_store_bin, pnum * 256, SEEK_SET) != 0)
                {
                    fprintf(out, "error in retrieving frame from backing_store.bin");
                    return -1; //exit main function
                }
                if (fread(phymem[frame], sizeof(signed char), 256, backing_store_bin) == 0)
                {
                    fprintf(out, "error can't read from backing store");
                    return -1;//exit main function
                }
            }
             //add new frame to tlb
            TLB[TLBindex].pagenum = pnum;
            TLB[TLBindex].framenum = frame;
            TLBindex++;
            TLBindex = TLBindex % 16;
        }

        //obtain physical address from the frame
        physical_address = (frame << 8) | poffset; //reverse. shift first 8 bits to higher position
                                                   //use bit masking/write masking to add offset bits.
        output = phymem[frame][poffset];

        fprintf(out, "%d,%d,%d\n", logadr, physical_address, output);
    }

    double pft = ((double)num_faults / (double)num_of_operations) * 100; //page fault rate
    double thr = ((double)num_of_TLBhits / (double)num_of_operations) * 100; //tlb hit rate

    fprintf(out, "Page Faults Rate, %.2f%%,\n", pft);
    fprintf(out, "TLB Hits Rate, %.2f%%,", thr);
    fclose(out);
    fclose(backing_store_bin);
    fclose(addresses);
    return 0;
}
