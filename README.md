# sbspatch
A header-only, streaming re-implementation of the bsdiff patch algorithm (bspatch), built for low-memory environments and embedded systems.

### **features**

* **Streaming API**

  * Process arbitrary input buffer sizes.
  * Call `sbsp_patch()` repeatedly as data arrives.
  * Returns:
    * `SBSP_MORE` → more bytes needed
    * `SBSP_DONE` → patching complete
  * No file I/O, no dynamic memory allocations.

* **Raw Patch Format (No Compression)**

  * Patch data is stored **uncompressed**, making it roughly the size of the original or new file.
  * The output is **highly compressible**. Users are expected to **apply their own compression** when storing or transferring patches.

* **Custom Magic Header**

  * Uses `"BSGG"` instead of the original `"ENDSLEY/BSDIFF43`, allowing safe identification of the modified format.

* **Fixed-size, small-chunk output writes**

  * Internal output buffer size can be configurable (default is 4 bytes) (`SBSP_WRITE_CHUNK`), tunable via compile-time define.
  * Designed for systems that write data in smaller blocks (e.g., flash pages, stream sinks, network packetizers).

---

### **Usage**

```c
#define SBSP_IMPLEMENTATION
#include "sbsp.h"

struct sbsp bs;

sbsp_init(&bs, oldfile, oldsize, my_write_callback, my_userdata);

while(stream_has_data()) {
  int r = sbsp_patch(&bs, incoming_buffer, incoming_len);
  if(r == SBSP_DONE) break;
  if(r < 0) {
    // handle error
    break;
  }
}
```

---

### **Write Callback Format**

```c
int my_write(void *userdata, int position, uint8_t *data, int len);
```

* `userdata`  → user pointer from init
* `position`  → offset to write to
* `data, len` → output bytes and length
* Return `< 0` for failure

---

### **License**

This code is based on the principles of **bsdiff/bspatch** by Colin Percival and modified by Matthew Endsley, rewritten for streaming and embedded use cases. This implementation is **not binary compatible** with the original format due to header and compression changes.

