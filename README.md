LBoot
=====

x86 bootloader for floppy disks

Current Support
---------------

 - Filesystems
   - FAT12 - Also limited to the boot device
 - Kernel Format
   - ELF
 - Multiboot 2 (optional)
   - `BOOTLOADER_NAME`
   - `BASIC_MEMINFO`
   - `MMAP`
 - Outputs
   - VGA
   - Serial

Future Work
-----------

 - Loading kernel via XModem
 - Multiboot module loading support
 - Flat binary support
 - Kernel relocation support (maybe)
 - Look into adding minimal FAT12 support to stage 1 loader
   - Currently this is leveraging an additional tool (`sector_mapper`) to
     generate a list of sectors to load for stage 2. This makes writing/updating
     the bootloader to the floppy more complicated than is probably necessary.
   - Simply giving the stage 1 loader the first sector of the stage 2 file
     would probably be sufficient.

Building
--------

Requirements:
 - `make`
 - `gcc` or `clang`
 - `mtools` for generating floppy image

To build, simply run `make` in the main directory. This will generate a `boot.img`
file containing the bootloader and configuration file. A kernel file will need to
be provided.

Testing
-------
This has been tested both within QEMU, and on a Compaq Armada 1700, booting the
[Lambda Kernel](https://github.com/farlepet/lambda-kern)

