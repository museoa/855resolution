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

 /* VBIOS as found on the Dell 510m */

#include <stdio.h>
#include <sys/io.h>

#include "../plugin.h"

struct vbios_resolution {
    unsigned char unknow1[2];
    unsigned char x1;
    unsigned char unknow2;
    unsigned char x2;
    unsigned char y1;
    unsigned char unknow3;
    unsigned char y2;
} __attribute__((packed));

static void unlock_bios(void) {
    outl(0x8000005a, 0xcf8);
    outb(0x33, 0xcfe);
}

static void relock_bios(void) {
    outl(0x8000005a, 0xcf8);
    outb(0x11, 0xcfe);
}

static int detect_vbios_type(struct vbios_mode *modes) {
short int r1, r2;
float f;

    r1 = r2 = 32000;
    while(modes->mode != 0xff) {
        if(modes->resolution <= r1) {
            r1 = modes->resolution;
        } else {
            if(modes->resolution <= r2) {
                r2 = modes->resolution;
            }
        }

        modes++;
    }

    f = ((float) (r2-r1-6)) / sizeof(struct vbios_resolution);

    return f == (int) f;
}


static unsigned char *get_vbios_version(unsigned char *vbios_cfg) {
    return vbios_cfg+31;
}

static void get_resolution(struct vbios_resolution *resolution, unsigned int *x, unsigned int *y) {
    *x = ((((unsigned int) resolution->x2) & 0xf0) << 4) | resolution->x1;
    *y = ((((unsigned int) resolution->y2) & 0xf0) << 4) | resolution->y1;
}

static void set_resolution(struct vbios_resolution *resolution, unsigned int x, unsigned int y) {
    unlock_bios();

    resolution->x2 = (resolution->x2 & 0x0f) | ((x >> 4) & 0xf0);
    resolution->x1 = (x & 0xff);

    resolution->y2 = (resolution->y2 & 0x0f) | ((y >> 4) & 0xf0);
    resolution->y1 = (y & 0xff);

    relock_bios();
}

struct plugin plugin2 = {
    detect_vbios_type,
    get_vbios_version,
    (get_resolution_type) get_resolution,
    (set_resolution_type) set_resolution
};

