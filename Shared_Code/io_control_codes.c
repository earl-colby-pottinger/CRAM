/* control_hook() - status_t control_hook(void *cookie, uint32 ioctl_op_code, void *data, size_t length) 
This hook handles the ioctl() function for an open instance of your driver. The control hook provides a means to perform operations that 
don't map directly to either read() or write(). It receives the cookie for the open instance, plus the command code ioctl_op_code and the data
and length arguments specified by ioctl()'s caller. These arguments have no inherent relationship; they're simply arguments to ioctl() that
are forwarded to your hook function. Their definitions are defined by the driver.

Author's notes:
Common command codes found in be/drivers/Drivers.h.  
enum {
 B_GET_DEVICE_SIZE        =  1 get # bytes - returns size_t in *data
 B_SET_DEVICE_SIZE        =  2 set # bytes - passes size_t in *data
 B_SET_NONBLOCKING_IO     =  3 set to non-blocking i/o
 B_SET_BLOCKING_IO        =  4 set to blocking i/o
 B_GET_READ_STATUS        =  5 check if can read w/o blocking, returns bool in *data
 B_GET_WRITE_STATUS       =  6 check if can write w/o blocking, returns bool in *data
 B_GET_GEOMETRY,          =  7 get info about device geometry, returns struct geometry in *data
 B_GET_DRIVER_FOR_DEVICE  =  8 get the path of the executable serving that device
 B_GET_PARTITION_INFO     =  9 get info about a device partition returns struct partition_info in *data
 B_SET_PARTITION          = 10 obsolete, will be removed
 B_FORMAT_DEVICE          = 11 low-level device format
 B_EJECT_DEVICE           = 12 eject the media if supported
 B_GET_ICON               = 13 return device icon (see struct)
 B_GET_BIOS_GEOMETRY      = 14 get info about device geometry as reported by the bios returns struct geometry in *data
 B_GET_MEDIA_STATUS       = 15  get status of media. return status_t in *data:
       B_NO_ERROR: media ready
       B_DEV_NO_MEDIA: no media
       B_DEV_NOT_READY: device not ready
       B_DEV_MEDIA_CHANGED: media changed since open or last B_GET_MEDIA_STATUS
       B_DEV_MEDIA_CHANGE_REQUESTED: user pressed button on drive
       B_DEV_DOOR_OPEN: door open
 B_LOAD_MEDIA             = 16 load the media if supported
 B_GET_BIOS_DRIVE_ID      = 17 Bios id for this device
 B_SET_UNINTERRUPTABLE_IO = 18 prevent cntl-C from interrupting i/o
 B_SET_INTERRUPTABLE_IO   = 19 allow cntl-C to interrupt i/o 
 B_FLUSH_DRIVE_CACHE      = 20 flush drive cache
 B_GET_PATH_FOR_DEVICE    = 21 get the absolute path of the device
 B_GET_ICON_NAME,         = 22 get an icon name identifier
 B_GET_VECTOR_ICON,       = 23 retrieves the device's vector icon
 B_GET_DEVICE_NAME,       = 24 get name, string buffer

 B_GET_NEXT_OPEN_DEVICE = 1000, obsolete, will be removed
 B_ADD_FIXED_DRIVER,            obsolete, will be removed
 B_REMOVE_FIXED_DRIVER,			obsolete, will be removed

 B_AUDIO_DRIVER_BASE = 8000,    base for codes in audio_driver.h
 B_MIDI_DRIVER_BASE = 8100,		base for codes in midi_driver.h
 B_JOYSTICK_DRIVER_BASE = 8200,	base for codes in joystick.h
 B_GRAPHIC_DRIVER_BASE = 8300,	base for codes in graphic_driver.h

 B_DEVICE_OP_CODES_END = 9999,

 IOCTL_FILE_UNCACHED_IO = 10000,

 B_SCSI_SCAN_FOR_DEVICES = B_DEVICE_OP_CODES_END + 1,
 B_SCSI_ENABLE_PROFILING,

 B_SCSI_INQUIRY = B_DEVICE_OP_CODES_END + 100,
 B_SCSI_EJECT,
 B_SCSI_PREVENT_ALLOW,
 B_RAW_DEVICE_COMMAND };

The length argument is only valid when ioctl() is called from user space; the kernel always sets it to 0. */ 

static status_t CRAM_control(void *cookie, uint32 ioctl_op_code, void *arg1, size_t length) {
 device_geometry *dinfo; device_icon *dicon;

  switch (ioctl_op_code) { // generic mass storage device IO control codes.

  case B_GET_GEOMETRY: // Fills out the specified device_geometry structure to describe the device.
       dinfo = (device_geometry *) arg1;
       dinfo->bytes_per_sector = RAM_SECTOR_SIZE; // sector size in bytes.
       dinfo->sectors_per_track = RAM_SECTORS;    // # sectors per track.
       dinfo->cylinder_count = RAM_TRACKS;        // # cylinders
       dinfo->head_count = RAM_HEADS;             // # heads
       dinfo->device_type = B_DISK;               // type 
       dinfo->removable = FALSE;                  //  non-zero if removable
       dinfo->read_only = FALSE;                  //  non-zero if read only
       dinfo->write_once = FALSE;                 //  non-zero if write-once.
       return B_NO_ERROR;

  case B_GET_BIOS_GEOMETRY:
       dinfo = (device_geometry *) arg1;
       dinfo->bytes_per_sector = 512L; 
       dinfo->sectors_per_track = bios_sectors * bios_size; 
       dinfo->cylinder_count = 1024L; 
       dinfo->head_count = 1L; 
       dinfo->device_type = B_DISK;
       dinfo->removable = FALSE; 
       dinfo->read_only = FALSE; 
       dinfo->write_once = FALSE;
       return B_NO_ERROR;

/* Author's notes: In testing the driver I found that this call is repeatly done.  The debugging dprintf was filling up the syslog file!
So I added the flag to only print the debugging message once.  If media_status can be change in future versions I would rewrite the code
to only output a message when said status change takes place. */

  case B_GET_MEDIA_STATUS: // Gets the status of the media in the device by placing a status_t at the location pointed to by data.
       if ( media_flag == true ) { // This ioctl() gets polled. If flag is false then suppress any status messages.
          dprintf("CRAM: <<<< (Media Status) >>>>\n"); }
       media_count++; // Count number of times called.
       //dprintf("CRAM: Media Count %ld\n", media_count);
       if ( media_flush > 0L ) { // Test if this drive requires regular flushes to storage.
          if ( media_count > media_flush ) { // Every couple of seconds (15?) do a flush().
             Timer_Flush_All_Data(); media_count = 0L; } } // Saves all internal data to the selected storage media.
       *(status_t*)arg1 = B_NO_ERROR; return B_OK;

  case B_GET_DEVICE_SIZE: /* Returns a size_t indicating the device size in bytes. */
       *(size_t*)arg1 = (size_t)(RAM_DRIVE_SIZE); return B_NO_ERROR;

  case B_SET_DEVICE_SIZE: /* Returns a size_t indicating the device size in bytes. */
       //(size_t)(RAM_DRIVE_SIZE)= *(size_t*)arg1;
       return EINVAL;

  case B_GET_READ_STATUS: // Returns true if the device can read without blocking, otherwise false.
       *(bool*)arg1 = false; return B_OK;

  case B_GET_WRITE_STATUS: // Returns true if the device can write without blocking, otherwise false.
       *(bool*)arg1 = false; return B_OK;

  case B_GET_ICON: // Old standard of a bitmapped icon.
       dicon = (device_icon *) arg1;
       switch (dicon->icon_size) {
              case B_LARGE_ICON:
                   memcpy(dicon->icon_data, icon_disk, B_LARGE_ICON * B_LARGE_ICON); return B_NO_ERROR;
              case B_MINI_ICON:
                   memcpy(dicon->icon_data, icon_disk_mini, B_MINI_ICON * B_MINI_ICON); return B_NO_ERROR;
       default:
            memcpy(dicon->icon_data, icon_disk_mini, B_MINI_ICON * B_MINI_ICON); return B_NO_ERROR; }

  case B_GET_ICON_NAME: // New standard, get a text name.
       user_strlcpy(arg1, "devices/CRAM", B_FILE_NAME_LENGTH); return B_OK;

  case B_GET_VECTOR_ICON: // New standard, gets a vector icon.
       return B_ERROR;                     

//  case B_GET_DEVICE_NAME: // Get name, string buffer
//       dprintf("CRAM: <<<< B_GET_DEVICE_NAME >>>>\n");
//       user_strlcpy(arg1, "CRAM", B_FILE_NAME_LENGTH);
//       return B_OK;
//     dicon->icon_size = (int32)sizeof(vector_icon);
//     dicon->icon_data = &vector_icon[0];   // Why does this not work?
//     memcpy(dicon->icon_data, &vector_icon, sizeof(vector_icon)); // Why does this not work?
//     return B_NO_ERROR;                           // It use to, all of a sudden it does not!

/* Author notes: In some versions of the ram drive the entire contents are all sent to storage from this IOCTRL call. */

  case B_FLUSH_DRIVE_CACHE: /* Flushes the drive's cache. */
       //media_count = -999L; 
       OS_Flush_All_Data(); 
       //media_count = 0L;
       return B_OK; // Saves all internal data to the selected storage media.

  case B_GET_DEVICE_NAME: /* Flushes the drive's cache. */
       return B_ERROR;                     

  case B_SCSI_SCAN_FOR_DEVICES:
       dprintf("CRAM: <<<< B_SCSI_SCAN_FOR_DEVICES >>>>\n");
       return B_ERROR;                     

  case B_SCSI_ENABLE_PROFILING:
       dprintf("CRAM: <<<< B_SCSI_ENABLE_PROFILING >>>>\n");
       return B_ERROR;                     

  case B_SCSI_INQUIRY:
       dprintf("CRAM: <<<< B_SCSI_INQUIRY >>>>\n");
       return B_ERROR;                     

  case B_SCSI_EJECT:
       dprintf("CRAM: <<<< B_SCSI_EJECT >>>>\n");
       return B_ERROR;                     

  case B_SCSI_PREVENT_ALLOW:
       dprintf("CRAM: <<<< B_SCSI_PREVENT_ALLOW >>>>\n");
       return B_ERROR;                     

  case B_RAW_DEVICE_COMMAND:
       dprintf("CRAM: <<<< B_RAW_DEVICE_COMMAND >>>>\n");
       return B_ERROR;                     

  default:
       dprintf("CRAM ERROR: <<<< Driver Control(%ld) unknown >>>>\n", ioctl_op_code);
       return B_ERROR; }

 return B_ERROR; }
