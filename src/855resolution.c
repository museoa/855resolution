/* 855resolution by Alain Poirier
 *
 * Currently only tested on a Dell 510m with BIOS A04
 * *VERY* likely that this won't work yet on any
 * other versions or chipsets!!!
 *
 * This code is based on the techniques used in :
 *
 *   - 855patch.  Many thanks to Christian Zietz (czietz gmx net)
 *     for demonstrating how to shadow the VBIOS into system RAM
 *     and then modify it.
 *
 *   - 1280patch by Andrew Tipton (andrewtipton null li).
 *
 * This source code is in the public domain.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <unistd.h>
#define __USE_GNU
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>

#define CHIPSET_ID          0x35808086
#define VBIOS               0xc0000
#define VBIOS_SIZE          0x10000
#define CFG_SIGNATURE 	    "BIOS_DATA_BLOCK "
#define CFG_VERSION         29+2

unsigned char *bios = 0;
int biosfd = 0;

static unsigned int get_chipset(void) {
    outl(0x80000000, 0xcf8);
    return inl(0xcfc);
}

static int unlock_bios(void) {
    outl(0x8000005a, 0xcf8);
    outb(0x33, 0xcfe);

    return 1;
}

static int relock_bios(void) {
    outl(0x8000005a, 0xcf8);
    outb(0x11, 0xcfe);

    return 1;
}

static void open_bios(void) {
    biosfd = open("/dev/mem", O_RDWR);
    if(biosfd < 0) {
        perror("Unable to open /dev/mem");
        exit(2);
    }

    bios = mmap((void *)VBIOS, VBIOS_SIZE,
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
        biosfd, VBIOS);
    if(bios == NULL) {
        fprintf(stderr, "Cannot mmap() the video BIOS\n");
        close(biosfd);
        exit(2);
    }
}

static void close_bios(void) {
    if(bios == NULL) {
        fprintf(stderr, "BIOS should be open already!\n");
        exit(2);
    }

    munmap(bios, VBIOS_SIZE);
    close(biosfd);
}

static void display_chipset(void) {
unsigned int chipset;

    chipset = get_chipset();
    printf("Chipset: ");
    switch (chipset) {
        case 0x35808086:
            printf("855GM\n");
            break;
        default:
            printf("Unknown (0x%08x)\n", chipset);
            break;
    }
}

static int check_bios(void) {
unsigned char *bioscfg;

    /* Find the configuration area of the BIOS */
    bioscfg = memmem(bios, VBIOS_SIZE, CFG_SIGNATURE, strlen(CFG_SIGNATURE));
    if(bioscfg == NULL) {
        fprintf(stderr, "Couldn't find the configuration area in the VBIOS!\n");
        close_bios();
        return 0;
    }

    printf("BIOS Configuration area offset: 0x%04x bytes\n", bioscfg - bios);
    printf("BIOS Version: %.4s\n\n", bioscfg + CFG_VERSION);

    return 1;
}

/* Finds the EMode table in the BIOS */
unsigned char *find_emode_table() {
int i;
unsigned char *p = bios;

    for(i=0; i<(VBIOS_SIZE-10); i++) { 
        if((p[i] == 0x30) && (p[i+5] == 0x32) && (p[i+10] == 0x34)) return &p[i];
    }

    return NULL;
}

/* Lists the available modes */
void list_modes(unsigned char *table) {
unsigned int x, y;
unsigned char *p;

    while(*table != 0xff) {
	p = bios + * (short *) (table+2);

	x = ((((unsigned int) p[4]) & 0xf0) << 4) | p[2];
	y = ((((unsigned int) p[7]) & 0xf0) << 4) | p[5];

	if((x != 0) && (y != 0)) {
            printf("Mode %02x : %dx%d, %d bits/pixel\n", table[0], x, y, table[1]);
	}

        table += 5;
    }
}

/* Returns a pointer to the parameter block for a mode */
unsigned char *get_emode_params(unsigned char *table, int mode) {
    unsigned short offset;

    while (*table != 0xff) {
        if (*table == mode) {
            offset = *(unsigned short *)(table+2);
            return bios + offset;
        }
        table += 5;         /* next record */
    }

    return 0;
}

void set_resolution(unsigned char *p, int x, int y) {
    p[4] = (p[4] & 0x0f) | ((x >> 4) & 0xf0);
    p[2] = (x & 0xff);

    p[7] = (p[7] & 0x0f) | ((y >> 4) & 0xf0);
    p[5] = (y & 0xff);
}

int parse_args(int argc, char *argv[], int *list, int *mode, int *x, int *y) {
int index = 1;

    *list = *mode = *x = *y = 0;

    if(argc!=2 && argc!=4 && argc!=5) {
        return -1;
    }

    if(!strcmp(argv[index], "-l")) {
        *list = 1;
	index++;

	if(argc<=index) {
	    return 0;
	}
    }

    if(argc-index != 3) {
        return -1;
    }

    *mode = (int) strtol(argv[index], NULL, 16);
    *x = atoi(argv[index+1]);
    *y = atoi(argv[index+2]);

    return 0;
}

void usage(char *name) {
    printf("Usage: %s [-l] [mode X Y]\n", name);
    printf("  Set the resolution to XxY for mode\n");
    printf("  Option -l displays the modes found into the vbios\n");
}

int main (int argc, char *argv[]) {
unsigned char *emode_table;
unsigned char *mode_params;
int list, mode, x, y;

    if(parse_args(argc, argv, &list, &mode, &x, &y) == -1)
    {
        usage(argv[0]);
	return 2;
    }

    if(iopl(3) < 0) {
        perror("Unable to obtain the proper IO permissions");
        return 2;
    }

    display_chipset();

    open_bios();
    if(!check_bios()) {
        close_bios();
        return 2;
    }

    emode_table = find_emode_table();
    if(emode_table == NULL)
    {
        fprintf(stderr, "Couldn't find the modes table in the VBIOS!\n");
	close_bios();
	return 2;
    }

    if(list) {
        list_modes(emode_table);
    }

    if(mode!=0 && x!=0 && y!=0)
    {
        mode_params = get_emode_params(emode_table, mode);
	if(mode_params == NULL)
	{
	   fprintf(stderr, "Couldn't find the mode %02x into the modes table\n", mode);
	   close_bios();
	   return 2;
	}

        unlock_bios();
        set_resolution(mode_params, x, y);
        relock_bios();

        printf("** Patch mode %02x to resolution %dx%d complete\n", mode, x, y);

	if(list) {
	    list_modes(emode_table);
	}
    }

    close_bios();

    return 0;
}

