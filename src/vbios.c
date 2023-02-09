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

#define CHIPSET_ID          0x35808086

#define VBIOS_START         0xc0000
#define CFG_SIGNATURE       "BIOS_DATA_BLOCK "

#ifndef VBIOS_FILE
#define VBIOS_FILE    "/dev/mem"
#define VBIOS_OFFSET_IN_FILE VBIOS_START
#else
#define VBIOS_OFFSET_IN_FILE 0
#endif

unsigned char *bios = 0;

static int biosfd = 0;

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
        fprintf(stderr, "BIOS should be open already!\n");
        exit(2);
    }

    munmap(bios, VBIOS_SIZE);
    close(biosfd);
}

void display_chipset(void) {
unsigned int chipset;

    chipset = get_chipset();
    printf("Chipset: ");
    switch (chipset) {
        case 0x25608086:
            printf("845G\n");
        break;

        case 0x35808086:
            printf("855GM\n");
        break;

        case 0x25708086:
            printf("865G\n");
        break;

        default:
            printf("Unknown (0x%08x)\n", chipset);
        break;
    }
}

unsigned char *get_vbios_cfg(void) {
    return memmem(bios, VBIOS_SIZE, CFG_SIGNATURE, strlen(CFG_SIGNATURE));
}

