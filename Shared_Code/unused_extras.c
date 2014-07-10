/* select_hook(void *cookie, uint8 event, uint32 ref, selectsync *sync)
 deselect_hook(void *cookie, uint8 event, uint32 ref, selectsync *sync)
   These hooks are reserved for future use. Set the corresponding entries in your device_hooks structure to NULL. */

static status_t CRAM_select(void *cookie, uint8 event, uint32 ref, selectsync *sync) {
 dprintf("CRAM: Select_Hook.\n");
 return B_OK; }

static status_t CRAM_deselect(void *cookie, uint8 event, selectsync *sync) {
 dprintf("CRAM: Deselect_Hook.\n");
 return B_OK; }

/* readv_hook() - status_t readv_hook(void *cookie, off_t position, const struct iovec *vec, size_t count, size_t *length) 
This hook handles the Posix readv() function for an open instance of your driver. This is a scatter/gather read function; given an array of 
iovec structures describing address/length pairs for a group of destination buffers, your implementation should fill each successive buffer 
with bytes, up to a total of length bytes. The vec array has count items in it. As with read_hook(), set length to the actual number of bytes 
read, and return an appropriate result code. */

////////static status_t my_device_readv(void *cookie, off_t position, const iovec *vec, size_t count, size_t *length) {
//////// sprintf(text,"Device_ReadV_hook\n"); debug_out(); return B_OK; }

/* writev_hook() - status_t writev_hook(void *cookie, off_t position, const struct iovec *vec, size_t count, size_t *length) 
This hook handles the Posix writev() function for an open instance of your driver. This is a scatter/gather write function; given an array 
of iovec structures describing address/length pairs for a group of source buffers, your implementation should write each successive buffer 
to disk, up to a total of length bytes. The vec array has count items in it. Before returning, set length to the actual number of bytes written, 
and return an appropriate result code. */

////////static status_t my_device_writev(void *cookie, off_t position, const iovec *vec, size_t count, size_t *length) {
//////// sprintf(text,"Device_WriteV_hook\n"); debug_out(); return B_OK; }
