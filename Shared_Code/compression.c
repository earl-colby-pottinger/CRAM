// Compress-0 does not compression, it is a simple place holder to confirm pointers are working properley.
long Compress0(long source_pointer, long dest_pointer) { // Compresses from VM_Buffer[] to VM_Buffer[].
 long compress_length = 0L, processed_data = 0L;

 dprintf("CRAM: Compressing %ld to %ld\n", source_pointer, dest_pointer);
 while (processed_data < t_size) { // Test if complete data track is compressed.
       VM_Buffer[dest_pointer++] = VM_Buffer[source_pointer++]; processed_data++; compress_length++; }
 dprintf("CRAM: Compressed %ld bytes to %ld bytes.\n", t_size, compress_length);
 return compress_length; }

long DeCompress0(long source_pointer, long dest_pointer) { // Decompresses from VM_Buffer[] to VM_Buffer[].
 long processed_data = 0L, temp_pointer; uint8 index, data;

 dprintf("CRAM: Decompressing %ld to %ld\n", source_pointer, dest_pointer);
 while (processed_data < t_size) { // Test if complete data track is compressed.
       VM_Buffer[dest_pointer++] = VM_Buffer[source_pointer++]; processed_data++; }
 dprintf("CRAM: End of decompressed data %ld\n", dest_pointer);
 return dest_pointer; }



// Compress-1 expands the data, this is a test that calls to Compress() and DeCompress() are being done properly.
long Compress(long source_pointer, long dest_pointer) { // Compresses from VM_Buffer[] to VM_Buffer[].
 long compress_length = 0L, processed_data = 0L;

 dprintf("CRAM: Compressing %ld to %ld\n", source_pointer, dest_pointer);
 while (processed_data < t_size) { // Test if complete data track is compressed.
       VM_Buffer[dest_pointer++] = VM_Buffer[source_pointer];
       VM_Buffer[dest_pointer++] = VM_Buffer[source_pointer++]; // Doubles each character.
       processed_data++; compress_length++; compress_length++; }
 dprintf("CRAM: Compressed %ld bytes to %ld bytes.\n", t_size, compress_length);
 return compress_length; }

long DeCompress(long source_pointer, long dest_pointer) { // Decompresses from VM_Buffer[] to VM_Buffer[].
 long processed_data = 0L, temp_pointer; uint8 index, data;

 dprintf("CRAM: Decompressing %ld to %ld\n", source_pointer, dest_pointer);
 while (processed_data < t_size) { // Test if complete data track is compressed.
       VM_Buffer[dest_pointer++] = VM_Buffer[source_pointer++]; source_pointer++; processed_data++; }
 dprintf("CRAM: End of decompressed data %ld\n", dest_pointer);
 return dest_pointer; }


void Compress_Data_Track() { // Compress a Data Track.
 long index, length_pointer, data_pointer, compress_length;

 sweeper++; sweeper = sweeper % tracks; // Steps thru the array of Data Track pointers.
 dprintf("CRAM: ++Sweeper %ld\n", sweeper);

 if (VM_Status[sweeper] == EMPTY_TRACK)  { dprintf("CRAM: Sweeper found an empty Data Track.\n"); }
 if (VM_Status[sweeper] == PACKED_TRACK) { dprintf("CRAM: Sweeper found a packed Data Track.\n"); }
 if (VM_Status[sweeper] == CMPRSS_TRACK) { dprintf("CRAM: Sweeper found a compressed Data Track.\n"); }
 if (VM_Status[sweeper] == STORED_TRACK) { dprintf("CRAM: Sweeper found a stored Data Track.\n"); }
 if (VM_Status[sweeper] == CLEAN_TRACK)  { dprintf("CRAM: Sweeper found a clean Data Track.\n"); }
 if (VM_Status[sweeper] == DIRTY_TRACK)  { dprintf("CRAM: Sweeper found a dirty Data Track.\n"); }

 if (VM_Status[sweeper] == VALID_TRACK) { // Found a Data Track to compress.
     dprintf("CRAM: Sweeper: Valid Data Track to compress %ld\n", sweeper);
     data_pointer = VM_Index[sweeper];                         // Store location of uncompressed data.

     length_pointer = Write_4Bytes(heap_pointer, sweeper);     // Start to create a compressed Data Track.
     heap_pointer = heap_pointer + HEADER_LENGTH;              // Points to start of compressed data.
     VM_Index[sweeper] = heap_pointer;                         // Record start of compressed Data Track.
     Write_4Bytes(data_pointer - HEADER_LENGTH, INVALID_DATA); // Mark old Data Track for future garbage collection. 

     compress_length = Compress(data_pointer, heap_pointer);   // Compress Data Track, get length of compressed Data Track.
     Write_4Bytes(length_pointer, compress_length);            // Store length into new Data Track structure.
     heap_pointer = heap_pointer + compress_length;            // Move heap pointer to the end of the new Datat Track.
     VM_Status[sweeper] = PACKED_TRACK; }                      // Update Status.

 return; }
