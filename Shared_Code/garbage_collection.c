long Garbage_Collect() { // Removes invalid Data Tracks from VM_Buffer[] and updates the points to the valid Data Tracks, returns new Heap_Pointer. */
 long  scan_pointer = 0L, write_pointer = 0L, back_pointer, length, index;

 dprintf("CRAM: Garbage Collection scan started, Heap = %ld\n",heap_pointer);
 while (scan_pointer < heap_pointer) { // Scan until end of heap is reached.
       dprintf("CRAM: Position: %ld   Index: %ld   Length: %ld\n", scan_pointer, Read_4Bytes(scan_pointer), Read_4Bytes(scan_pointer + 4L));
       scan_pointer = scan_pointer + HEADER_LENGTH + Read_4Bytes(scan_pointer + 4L); } // Skip length of entire structure.
 dprintf("CRAM: Garbage Collection scan ended, Heap=%ld   Scan=%ld\n",heap_pointer, scan_pointer);


 
 return heap_pointer; }






/* dprintf("CRAM: Garbage Collection running.\n");
 scan_pointer = 0L;
 while (scan_pointer < heap_pointer) { // Scan until end of heap is reached.
       back_pointer = Read_4Bytes(scan_pointer); // Find VM_Index[] to this Data Track.
       if (back_pointer == INVALID_DATA) {       // Need to skip over this invalid Data Track Structure.
          dprintf("CRAM: Back pointer marks as invalid, scan=%ld\n",scan_pointer);
          scan_pointer = scan_pointer + HEADER_LENGTH + Read_4Bytes(scan_pointer + 4L); } // Skip length of entire structure.
       scan_pointer = heap_pointer; }

 dprintf("CRAM: Garbage Collection processed, Heap = %ld\n",heap_pointer);
 scan_pointer = 0L;
 while (scan_pointer < heap_pointer) { // Scan until end of heap is reached.
       dprintf("CRAM: Position: %ld   Index: %ld   Length: %ld\n", scan_pointer, Read_4Bytes(scan_pointer), Read_4Bytes(scan_pointer + 4L));
       scan_pointer = scan_pointer + HEADER_LENGTH + Read_4Bytes(scan_pointer + 4L); } // Skip length of entire structure.
 dprintf("CRAM: Garbage Collection process scan ended, Heap=%ld   Scan=%ld\n\n",heap_pointer, scan_pointer);
      } else {
       	  if (scan_pointer == write_pointer) { // No invalid Data Tracks so far.
       	     write_pointer = write_pointer + HEADER_LENGTH + Read_4Bytes(scan_pointer + 4L); // Skip length of entire structure.
             scan_pointer  = scan_pointer  + HEADER_LENGTH + Read_4Bytes(scan_pointer + 4L); // Update scan pointer also.
          } else {
             VM_Index[back_pointer] = write_pointer + HEADER_LENGTH;  // Update pointer to Data Track.
       	     length = Read_4Bytes(scan_pointer + 4L) + HEADER_LENGTH; // Get length of entire Data Track.
       	     for (index = 0L; index < length; index++); {             // Copy all the data.
       	         VM_Buffer[write_pointer++] = VM_Buffer[scan_pointer++]; } } } } */
 // heap_pointer = write_pointer;
