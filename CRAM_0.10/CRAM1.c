/* CRAM 0.10 --- A basic 1008MB RAMDRIVE.  No compression, no storage.  Just very, very fast! */

#include <KernelExport.h>
#include <Drivers.h>
#include <Errors.h>
#include <OS.h>
#include <scsi.h>
#include <Mime.h> // Drive icon flags.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define RAM_SECTOR_SIZE 4096LL // Bytes per sector.  Valid sizes are 2048, 4096 or 8192 bytes only.
#define RAM_SECTORS       64LL // Number of sectors per track.
#define RAM_TRACKS        64LL // Number of cylinders per platter.
#define RAM_HEADS         63LL // Number of surfaces.  Valid ranges are 1 to 255 for backward compatibility reasons.

#define RAM_DRIVE_SIZE  ( RAM_SECTORS * RAM_TRACKS * RAM_HEADS * RAM_SECTOR_SIZE ) // Size of the RAMDrive in bytes.
#define RAM_BUFFER_SIZE ( RAM_DRIVE_SIZE + RAM_SECTOR_SIZE ) // Size of entire in-memory VM buffer.

static int64 ramdrive_size  = RAM_DRIVE_SIZE; // Size of ram drive declared to the OS.
static int64 bios_sectors   = RAM_SECTOR_SIZE / 512LL; // How many 512 bytes bios sectors fit inside a ram sector.
static int64 bios_size      = ( RAM_SECTORS * RAM_TRACKS * RAM_HEADS ) / 1024LL; // How many tracks per bios drive.
//static int64 buffer_size  = BUFFER_SIZE; // Size of buffer used to hold virtual memory track/pages.
//static int64 track_size   = RAM_SECTOR_SIZE * RAM_SECTORS; // Size of a data track in bytes.
//static int64 track_count  = RAM_TRACKS * RAM_HEADS; // Total number of data tracks in CRAM.
//static int64 heap_pointer = 0LL; // Pointer to allocating next track (RAM_SECTORS*RAM_BLOCK_SIZE) in memory.

/* From Carlos Hasan: (note: this quote has been modified for Haiku-OS as to which location to use, but the original ideas and
pointers came from Carlos).  To test the driver, copy the binary to

~/config/add-ons/kernel/drivers/bin/cram1, and make a link in
~/config/add-ons/kernel/drivers/dev/disk/cram1.

The above was for the original Haiku-OS, in the new Package Managed Haiku-OS the paths for adding drivers has been changed to:

/boot/home/config/non-packaged/add-ons/kernel/drivers/bin/cram1, and make a link in
/boot/home/config/non-packaged/add-ons/kernel/drivers/dev/disk/cram1

Btw, if you don't have another PC to debug the driver through the serial line and dprintf(), do the following:

- Press F1 when you boot BeOS, and enable the output console (I think this is only for Beos, on Haiku I don't press F1).
- Open a Terminal window and type:

  tail -f /var/log/syslog

You will see the messages printed with dprintf() in your Terminal. */

/* api_version - This variable defines the API version to which the driver was written, and should be set to
B_CUR_DRIVER_API_VERSION at compile time. The value of this variable will be changed with every revision to the driver API;
the value with which your driver was compiled will tell devfs how it can communicate with the driver. */

int32 api_version = B_CUR_DRIVER_API_VERSION;

/* Author's Note: (ioctl)B_GET_MEDIA_STATUS is regularly polled by the Haiku-OS, this makes it's useful place to add calls to
functions that need to do periodic runs. (ie disk cache flushing, garbage collection ... ) */

static long DEBUG = 0L; // 0 ==  Error messages only. 1 == Tracking & Error messages. 2 == Adds Flush functions.
static bool media_flag = false; // Used to suppress polling debug messages.
static long media_flush = 0L;   // Tells the timer in media_status to how often to call the Timer_Flush_All_Data() function.
                                // Zero equals no calls.
static long media_count = 0L;   // Counts how many times the media status has been polled since the last flush().

static uint8 VM_Buffer[RAM_BUFFER_SIZE]; // Holds the in-memory VM pages.

static const char *sCRAMNames[] = { "disk/cram1/raw",NULL }; // Thanks to Axel Dorfleir and Ingo Weinhold for pointing out
                                                             // the need for 'raw' in the path needed by Haiku.
#include <../Shared_Code/icon_bitmap.c> // Creates an array containing a bitmap icon.
                                        // This is the old BeOS method, it get overridden if a vector icon exists.
#include <../Shared_Code/icon_vector.c> // Creates an array containing a Haiku vector icon.
                                        // This is the preferred method in Haiku-OS.
#include <../Shared_Code/device_driver_hooks.c> // Creates the internal functions hooks that all Haiku drivers needs.

/* Author Note: The following flush functions are dummy functions that exist to allow one set of shared code for all versions
for the driver. */

void Flush_All_Data() { // Save all internal data to the storage media.  To be used in later versions.
 if ( DEBUG > 1 ) { dprintf("CRAM_0.10: (Dummy Function) Flush the entire drive image to storage.\n"); }
 return; }

void Timer_Flush_All_Data() { // Save all internal data to the storage media.  To be used in later versions.
 if ( DEBUG > 1 ) { dprintf("CRAM_0.10: (Dummy Function) Flushing requested by the driver timer function.\n"); }
 Flush_All_Data(); return; }

void OS_Flush_All_Data() { // Save all internal data to the storage media.  To be used in later versions.
 if ( DEBUG > 1 ) { dprintf("CRAM_0.10: (Dummy Function) Flushing requested by the OS io_control.\n"); }
 Flush_All_Data(); return; }

void Internal_Flush_All_Data() { // Save all internal data to the storage media.  To be used in later versions.
 if ( DEBUG > 1 ) { dprintf("CRAM_0.10: (Dummy Function) Flushing requested internally by the driver.\n"); }
 Flush_All_Data(); return; }

/* init_hardware - This function is called when the system is first booted, which then lets the driver detect and reset the
hardware it controls. The function should return B_OK if the initialization is successful; otherwise, an appropriate error
code should be returned. If this function returns an error, the driver won't be used.

Author's notes: Clears the VM_Buffer[] to ensures no garbage from previous memory use exists.  */

status_t init_hardware(void) {

 if ( DEBUG > 0 ) {
    dprintf("CRAM_0.10: Init_Hardware.\n");
    dprintf("CRAM_0.10: Reset Buffer called.\n");
    dprintf("CRAM_0.10: Buffers and Pointers are being reset.\n");
  /*dprintf("CRAM_0.10: Buffer Size=%lld  Track Size=%lld  Total Tracks=%lld\n", buffer_size, track_size, track_count);*/ }

 user_memset(&VM_Buffer[0LL], 0, ramdrive_size); return B_OK; } // Clears the in-memory VM_buffer[].

/* init_driver - optional function - called every time the driver is loaded. Drivers are loaded and unloaded on an as-needed
basis. When a driver is loaded by devfs, this function is called to let the driver allocate memory and other needed system
resources. Return B_OK if initialization succeeds, otherwise return an appropriate error code.
<<<what happens if this returns an error?>>>

Author's notes: In later versions with file I/O, the loading the drive image will be done here. */

status_t init_driver(void) {
 if ( DEBUG > 0 ) { dprintf("CRAM_0.10: Init_Driver.\n"); }
 return B_OK; }

/* uninit_driver - This function is called by devfs just before the driver is unloaded from memory. This lets the driver
clean up after itself, freeing any resources it allocated.

Author's notes: Flush all pages to storage here in later versions. */

void uninit_driver(void) {
 if ( DEBUG > 0 ) { dprintf("CRAM_0.10: Uninit_Driver.\n"); }
 Internal_Flush_All_Data(); return; }
 
/* publish_devices - return a null-terminated array of devices supported by this driver.  Devfs calls publish_devices() to
learn the names, relative to /dev, of the devices the driver supports. The driver should return a NULL-terminated array of
strings indicating all the installed devices the driver supports. For example, an ethernet device driver might return: 

 static char *devices[] = { "net/ether", NULL };

In this case, devfs will then create the pseudo-file /dev/net/ether, through which all user applications can access the driver.
Since only one instance of the driver will be loaded, if support for multiple devices of the same type is desired, the driver
must be capable of supporting them. If the driver senses (and supports) two ethernet cards, it might return: 

 static char *devices[] = { "net/ether1", "net/ether2", NULL };

Author notes: Notice the examples above do not end with '/raw'?  I had to add that to device name to have Haiku-OS properly
load and init the driver during boot.  Other drivers that are just written/read as input/output devices do not need this.
Just look at my FIFO driver or Haiku's ZERO, NULL drivers for examples. */

const char ** publish_devices(void) {
 if ( DEBUG > 0 ) { dprintf("CRAM_0.10: Publish Device Names.\n"); }
 return sCRAMNames; }

/* find_device - return ptr to device hooks structure for a	given device name. When a device published by the driver is
accessed, devfs communicates with it through a series of hook functions that handle the requests.The find_device() function
is called to obtain a list of these hook functions, so that devfs can call them. The device_hooks structure returned lists
out the hook functions. The device_hooks structure, and what each hook does, is described in the next section. */

device_hooks * find_device(const char* name) {
 int i;

 if ( DEBUG > 0 ) { dprintf("CRAM_0.10: Find_Driver named: %s.\n", name); }

 for (i = 0; sCRAMNames[i] != NULL; i++) {
     if ( DEBUG > 0 ) { dprintf("CRAM_0.10: Testing device name: %s\n", sCRAMNames[i]); }
     if (strcmp(name, sCRAMNames[i]) == 0) {
        if ( DEBUG > 0 ) { dprintf("CRAM_0.10: Found Driver.\n"); }
        return &sCRAMHooks; } }
 return NULL; }

/* read_hook() - status_t read_hook(void *cookie, off_t position, void *data, size_t *length) 
This hook handles the Posix read() function for an open instance of your driver. Implement it to read length bytes of data
starting at the specified byte position on the device, storing the read bytes at data. Exactly what this does is device
specific (disk devices would read from the specified offset on the disk, but a graphics driver might have some other
interpretation of this request). Before returning, you should set length to the actual number of bytes read into the buffer
by the driver. Return B_OK if data was read (even if the number of returned bytes is less than requested), otherwise return
an appropriate error.

Author's notes: I am still doing experiments with the diffirent versions of Haiku-OS and BFS, please read my notes for the
write() function for now.  At present I have noted that during the boot process the reads come in many diffirent sizes,
many of them being smaller than the 2048 bytes default sector size. After booting BFS writes to the drive are being buffered
and so I can't see the reads to the drive, only the writes.  I hope to change that in the future versions. */

static status_t CRAM_read(void *cookie, off_t position, void *read_buffer, size_t *read_numBytes) {
 int64 read_count; uint8 *readbuffer = (uint8 *)read_buffer; // Cast pointer to a byte array.

 if ( position >= ramdrive_size ) { // Does not allow read()s outside of assigned drive space.
    dprintf("CRAM_0.10 ERROR: Read() is out of range. %lld\n", position); *read_numBytes = 0LL; return B_ERROR; }

 read_count = *read_numBytes; // Track how many bytes are to be written to the OS read_buffer.
 
 if ( ( position + read_count ) > ramdrive_size ) { // Does not allow read()s extend outside of assigned drive space.
    dprintf("CRAM_0.10 ERROR: Read() extends out of range. %lld  %lld\n", position, read_count);
    read_count = position - ramdrive_size; *read_numBytes = read_count; }

 if ( DEBUG > 0 ) { // Report how many bytes to read from device.
    dprintf("CRAM_0.10 READ: position = %lld  read bytes = %lld\n", position, read_count); }

 user_memcpy(readbuffer, &VM_Buffer[position], read_count); return B_OK; } // Copy from VM_Buffer[] to OS data buffer.

/* write_hook() - status_t write_hook(void *cookie, off_t position, void *data, size_t length) 
This hook handles the Posix write() function for an open instance of your driver. Implement it to write length bytes of
data starting at the specified byte position on the device, from the buffer pointed to by data.  Exactly what this does
is device-specific (disk devices would write to the specified offset on the disk, but a graphics driver might have some
other interpretation of this request). Return B_OK if data was read (even if the number of returned bytes is less than
requested), otherwise return an appropriate error.

Author's notes: A number of writes turn out to be larger than the default sector size, additionallly a number of writes
will cross the boundaries between tracks, this is important information for later versions that instead of using a single
large block of memory will instead allocate smaller blocks of memory on demand.  This is also important info when
developing any drivers of any storage device that in reality is not an one continous block device in reality as writting
across internal boundaries causes a performance hit.  IE: hard drives, flash drives and SSDs. */

static status_t CRAM_write(void *cookie, off_t position, const void *write_buffer, size_t *write_numBytes) {
 int64 write_count; uint8 *writebuffer = (uint8 *)write_buffer; // Cast pointer to a byte array.

 if ( position >= ramdrive_size ) { // Does not allow write()s outside of assigned drive space.
    dprintf("CRAM_0.10 ERROR: Write() is out of range. %lld\n", position); *write_numBytes = 0LL; return B_ERROR; }

 write_count = *write_numBytes; // Track how many bytes are read from the OS write_buffer.

 if ( ( position + write_count ) > ramdrive_size ) { // Does not allow write()s extend outside of assigned drive space.
    dprintf("CRAM_0.10 ERROR: Write() extends out of range. %lld  %lld\n", position, write_count);
    write_count = position - ramdrive_size; *write_numBytes = write_count; }

 if ( DEBUG > 0 ) { // Report how many bytes to write to device.
    dprintf("CRAM_0.10 WRITE: position = %lld  written bytes = %lld\n", position, write_count); }

 user_memcpy(&VM_Buffer[position], writebuffer, write_count); return B_OK; } // Copy from OS data buffer to VM_Buffer[].

/* open_hook() - status_t open_hook(const char *name, uint32 flags, void **cookie) 
This hook function is called when a program opens one of the devices supported by the driver. The name of the device (as
returned by publish_devices()) is passed in name, along with the flags passed to the Posix open() function. cookie points
to space large enough for you to store a single pointer. You can use this to store state information specific to the open()
instance. If you need to track information on a per-open() basis, allocate the memory you need and store a pointer to it
in *cookie.

Author's notes: Will have to test if this is a better location to load drive image in future versions. */

static status_t CRAM_open(const char *name, uint32 flags, void **cookie) {
 if ( DEBUG > 0 ) { dprintf("CRAM_0.10: Open_Hook: %s.\n", name); }
 return B_OK; }

/* close_hook() - status_t close_hook(void *cookie) 
This hook is called when an open instance of the driver is closed using the close() Posix function. Note that because of
the multithreaded nature of BeOS, it's possible there may still be transactions pending, and you may receive more calls on
the device. For that reason, you shouldn't free instance-wide system resources here.  Instead, you should do this in
free_hook(). However, if there are any blocked transactions pending, you should unblock them here. */

static status_t CRAM_close(void *cookie) {
 if ( DEBUG > 0 ) { dprintf("CRAM_0.10: Close_Hook.\n"); }
 return B_OK; }

/* free_hook() - status_t free_hook(void *cookie) 
This hook is called once all pending transactions on an open (but closing) instance of your driver are completed, ie: called
after the last device is closed, and after all i/o is complete. This is where your driver should release instancewide system
resources. free_hook() doesn't correspond to any Posix function. */

static status_t CRAM_free(void *cookie) {
 if ( DEBUG > 0 ) { dprintf("CRAM_0.10: Free_Hook.\n"); }
 return B_OK; }

#include <../Shared_Code/io_control_codes.c>
#include <../Shared_Code/unused_extras.c>
