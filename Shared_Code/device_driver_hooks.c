// Creates the internal functions hooks that all Haiku drivers needs.

static status_t CRAM_open(const char *name, uint32 flags, void **cookie);
static status_t CRAM_read(void *cookie, off_t position, void *read_buffer, size_t *read_numBytes);
static status_t CRAM_write(void *cookie, off_t position, const void *write_buffer, size_t *write_numBytes);
static status_t CRAM_control(void *cookie, uint32 ioctl_op_code, void *arg, size_t length);
static status_t CRAM_close(void *cookie);
static status_t CRAM_free(void *cookie);
static status_t CRAM_select(void *cookie, uint8 event, uint32 ref, selectsync *sync);
static status_t CRAM_deselect(void *cookie, uint8 event, selectsync *sync);

/* Device Hooks - The hook functions specified in the device_hooks function returned by the driver's find_device() function handle requests 
made by devfs (and through devfs, from user applications). These are described in this section. 
The structure itself looks like this: 
      typedef struct { 
            device_open_hook open; 
            device_close_hook close; 
            device_free_hook free; 
            device_control_hook control; 
            device_read_hook read; 
            device_write_hook write; 
            device_select_hook select; 
            device_deselect_hook deselect; 
            device_readv_hook readv; 
            device_writev_hook writev; 
      } device_hooks;
In all cases, return B_OK if the operation is successfully completed, or an appropriate error code if not. */

/* function pointers for the device hooks entry points */
static device_hooks sCRAMHooks = {
   CRAM_open,     // -> open entry point.
   CRAM_close,    // -> close entry point.
   CRAM_free,     // -> free cookie.
   CRAM_control,  // -> control entry point.
   CRAM_read,     // -> read entry point.
   CRAM_write,    // -> write entry point.
   CRAM_select,   // -> select entry point.
   CRAM_deselect, // -> deselect entry point.
   NULL,          // -> posix read entry point.
   NULL } ;       // -> posix write entry point.
