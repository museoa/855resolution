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
#ifndef _VBIOS_H_
#define _VBIOS_H_

#define VBIOS_SIZE 0x10000

struct vbios_mode
{
    unsigned char mode;
    unsigned char bits_per_pixel;
    unsigned short resolution;
    unsigned char unknow;
} __attribute__((packed));

extern unsigned char *bios;
#define VBIOS_POINTER(x) ((void *) ((x)+bios))

extern void display_chipset(void);
extern void open_bios(void);
extern void close_bios(void);
extern unsigned char *get_vbios_cfg(void);

#endif

