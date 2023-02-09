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
#include <sys/io.h>

#include "../plugin.h"

struct vbios_modeline {
    unsigned long clock;

    unsigned short x1;
    unsigned short htotal;
    unsigned short x2;
    unsigned short hblank;
    unsigned short hsyncstart;
    unsigned short hsyncend;

    unsigned short y1;
    unsigned short vtotal;
    unsigned short y2;
    unsigned short vblank;
    unsigned short vsyncstart;
    unsigned short vsyncend;
} __attribute__((packed));

struct vbios_resolution {
    unsigned char unknown[6];

    struct vbios_modeline modelines[];
} __attribute__((packed));

#define TIMING_60 0
#define TIMING_75 1
#define TIMING_85 2

static int freqs[] = { 60, 75, 85 };

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

    f = ((float) (r2-r1-6)) / sizeof(struct vbios_modeline);

    return f == (int) f;
}

static unsigned char *get_vbios_version(unsigned char *vbios_cfg) {
    return vbios_cfg+29;
}

static void get_resolution(struct vbios_resolution *resolution, unsigned int *x, unsigned int *y) {
struct vbios_modeline *modeline;
int i;

    *x = resolution->modelines[0].x1+1;
    *y = resolution->modelines[0].y1+1;

    for(i=0; i<3; i++)
    {
        modeline = &resolution->modelines[i];

    if(modeline->x1 == resolution->modelines[0].x1) {
            printf("  (Modeline : %ld %d %d %d %d %d %d %d %d)\n",
                modeline->clock,
                modeline->x1+1, modeline->hsyncstart, modeline->hsyncend, modeline->htotal,
                modeline->y1+1, modeline->vsyncstart, modeline->vsyncend, modeline->vtotal);
    }
    }
}

static void set_resolution(struct vbios_resolution *resolution, unsigned int x, unsigned int y) {
struct vbios_modeline *modeline;
int i;

    unlock_bios();

    for(i=0; i<3; i++) {
        modeline = &resolution->modelines[i];

        if(modeline->x1 == resolution->modelines[0].x1) {
            modeline->x1 = modeline->x2 = x-1;
            modeline->y1 = modeline->y2 = y-1;

                /* WARNING: I don't know at all what are the formulas
            to calculate these values !
            The following lines are only an example.
            */
            modeline->clock = freqs[i] * x;

            modeline->hsyncstart = x+10;
            modeline->hsyncend   = x+20;
            modeline->htotal     = x+30;

            modeline->vsyncstart = y+10;
            modeline->vsyncend   = y+20;
            modeline->vtotal     = y+30;
        }
    }

    relock_bios();
}

struct plugin plugin1 = {
    detect_vbios_type,
    get_vbios_version,
    (get_resolution_type) get_resolution,
    (set_resolution_type) set_resolution
};

