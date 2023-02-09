855resolution
=============

This software changes the resolution of an available vbios mode.

I've personaly only tested this software on a Dell 510m, to get the
1400x1050 resolution. It's reported, by others, to also works on several
855 / 865 / 915 chipset based laptops.

It patches only the RAM version of the video bios so the new resolution
is loose each time you reboot. If you want to set the resolution each
time you reboot and before to launch X, use your rc.local, local.start ...
file of your Linux version.


Usage
-----

You must be root to launch it.

  Usage: 855resolution [-l] [-f bios_type] [-p] [mode X Y]
  Options:
      -l display the modes found into the vbios
      -f skip the automatic VBIOS detection by forcing a VBIOS type


Installing
----------

$ make
$ su
# make install


Setting
-------

    1. Display the available resolutions :

        # 855resolution -l
        Chipset: 855GM
        VBIOS type: 1
        BIOS Version: 3240

        Mode 30 : 640x480, 8 bits/pixel
        Mode 32 : 800x600, 8 bits/pixel
        Mode 34 : 1024x768, 8 bits/pixel
        Mode 38 : 1280x1024, 8 bits/pixel
        Mode 3a : 1600x1200, 8 bits/pixel
        Mode 3c : 1920x1440, 8 bits/pixel
        Mode 41 : 640x480, 16 bits/pixel
        Mode 43 : 800x600, 16 bits/pixel
        Mode 45 : 1024x768, 16 bits/pixel
        Mode 49 : 1280x1024, 16 bits/pixel
        Mode 4b : 1600x1200, 16 bits/pixel
        Mode 4d : 1920x1440, 16 bits/pixel
        Mode 50 : 640x480, 32 bits/pixel
        Mode 52 : 800x600, 32 bits/pixel
        Mode 54 : 1024x768, 32 bits/pixel
        Mode 58 : 1280x1024, 32 bits/pixel
        Mode 5a : 1600x1200, 32 bits/pixel
        Mode 5c : 1920x1440, 32 bits/pixel

    2. I personnaly decided to overwrite the 1920x1440 resolution (mode 3c)
       because I don't use it :

       # 855resolution 3c 1400 1050

    3. Now the bios reports a 1400x1050 resolution :

        # 855resolution -l
        Chipset: 855GM
        VBIOS type: 1
        BIOS Version: 3240

        Mode 30 : 640x480, 8 bits/pixel
        Mode 32 : 800x600, 8 bits/pixel
        Mode 34 : 1024x768, 8 bits/pixel
        Mode 38 : 1280x1024, 8 bits/pixel
        Mode 3a : 1600x1200, 8 bits/pixel
        Mode 3c : 1400x1050, 8 bits/pixel
        Mode 41 : 640x480, 16 bits/pixel
        Mode 43 : 800x600, 16 bits/pixel
        Mode 45 : 1024x768, 16 bits/pixel
        Mode 49 : 1280x1024, 16 bits/pixel
        Mode 4b : 1600x1200, 16 bits/pixel
        Mode 4d : 1400x1050, 16 bits/pixel
        Mode 50 : 640x480, 32 bits/pixel
        Mode 52 : 800x600, 32 bits/pixel
        Mode 54 : 1024x768, 32 bits/pixel
        Mode 58 : 1280x1024, 32 bits/pixel
        Mode 5a : 1600x1200, 32 bits/pixel
        Mode 5c : 1400x1050, 32 bits/pixel

    4. My XF86Config has the following screen definition :

       Section "Screen"
         Identifier  "Screen 1"
         Device      "device"
         Monitor     "LCD"
         DefaultDepth 24

         Subsection "Display"
           Depth       24
           Modes       "1400x1050"
         EndSubsection
       EndSection

     5. $ startx


Disclaimer
----------

855resolution is free to use, distribute or modify. But please mention
my name and the names of the respective contributors.
I tried to make the programs as safe as possible but obviously I can't
guarantee that they'll work for you. So don't blame me if something bad
happens.

                                          Alain Poirier
                                          alain.poirier1 at wanadoo.fr
