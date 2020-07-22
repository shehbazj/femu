gzip compression application.

Important functions -

	`deflate()` function called by `zip()`.
	after compression of a block (using no-compression, variable or static Huffmann), the compressed block is written
	to the underlying device.
	functions of interest - `FLUSH_BLOCK(1)` - macro that resolves into `flush_block()`
	which then calls `copy_block(buf, len, header)`, which then calls `put_byte()`

	trace put_byte() and all other calls...
	
