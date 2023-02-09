/* 855resolution by Alain Poirier
 *
 * This code is based on the techniques used in :
 *
 *   - 855patch.  Many thanks to Christian Zietz (czietz gmx net)
 *     for demonstrating how to shadow the VBIOS into system RAM
 *     and then modify it.
 *
 *   - 1280patch by Andrew Tipton (andrewtipton null li).
 *
 * This source code is into the public domain.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define __USE_GNU
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/io.h>

#include "vbios.h"

#define VBIOS_START 0xc0000
#define CFG_SIGNATURE "BIOS_DATA_BLOCK "

#define CHIPSET_855 0x35808086

#ifndef VBIOS_FILE
#define VBIOS_FILE "/dev/mem"
#define VBIOS_OFFSET_IN_FILE VBIOS_START
#else
#define VBIOS_OFFSET_IN_FILE 0
#endif

unsigned char *bios = 0;

static int biosfd = 0;
static unsigned char b1, b2;

static unsigned int get_chipset(void) {
    outl(0x80000000, 0xcf8);
    return inl(0xcfc);
}

void open_bios(void) {
    biosfd = open(VBIOS_FILE, O_RDWR);
    if(biosfd < 0) {
        perror("Unable to open the BIOS file");
        exit(2);
    }

    bios = mmap((void *)VBIOS_START, VBIOS_SIZE,
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
        biosfd, VBIOS_OFFSET_IN_FILE);

    if(bios == NULL) {
        fprintf(stderr, "Cannot mmap() the video BIOS\n");
        close(biosfd);
        exit(2);
    }
}

void close_bios(void) {
    if(bios == NULL) {
        fprintf(stderr, "BIOS not opened !\n");
        exit(2);
    }

    munmap(bios, VBIOS_SIZE);
    close(biosfd);
}

void unlock_bios(void) {
    if(get_chipset() == CHIPSET_855)
    {
        outl(0x8000005a, 0xcf8);
		b1 = inb(0xcfe);

        outl(0x8000005a, 0xcf8);
        outb(0x33, 0xcfe);
    }
    else
    {
        outl(0x80000090, 0xcf8);
        b1 = inb(0xcfd);
        b2 = inb(0xcfe);

        outl(0x80000090, 0xcf8);
        outb(0x33, 0xcfd);
        outb(0x33, 0xcfe);
    }
}

void relock_bios(void) {
    if(get_chipset() == CHIPSET_855)
    {
        outl(0x8000005a, 0xcf8);
        outb(b1, 0xcfe);
    }
    else
    {
        outl(0x80000090, 0xcf8);
        outb(b1, 0xcfd);
        outb(b2, 0xcfe);
    }
}

void display_chipset(void) {
unsigned int chipset;

    chipset = get_chipset();
    printf("Chipset: ");
    switch (chipset) {
        case 0x25608086:
            printf("845G");
        break;

        case CHIPSET_855:
            printf("855GM");
        break;

        case 0x25708086:
            printf("865G");
        break;

        case 0x25808086:
            printf("915G");
        break;

        default:
            printf("Unknown");
        break;
    }
    printf(" (id=0x%08x)\n", chipset);
}

unsigned char *get_vbios_cfg(void) {
    return memmem(bios, VBIOS_SIZE, CFG_SIGNATURE, strlen(CFG_SIGNATURE));
}

