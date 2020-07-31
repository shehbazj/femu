#include "qemu/osdep.h"
#include "hw/block/block.h"
#include "hw/pci/pci.h"
#include "qemu/error-report.h"

#include "mem-backend.h"
#include "computation.h"
#include "nvme.h"
#include <poll.h>

enum NvmeComputeDirectiveType;

uint64_t do_pointer_chase(int computational_fd_send, int computational_fd_recv, int ctype_fd, void *mb, 
		uint64_t mb_oft, dma_addr_t cur_len, AddressSpace *as, dma_addr_t *cur_addr, uint32_t read_delay);

uint64_t do_count(int computational_fd_send, int computational_fd_recv, int ctype_fd, void *mb , uint64_t mb_oft, dma_addr_t cur_len);

uint64_t do_compression(int computational_fd_send, int computational_fd_recv, int ctype_fd, void *mb, uint64_t mb_oft, 
	dma_addr_t cur_len, AddressSpace *as, dma_addr_t *cur_addr, uint32_t flash_write_delay, uint32_t flash_read_delay, enum NvmeComputeDirectiveType);
/* Coperd: FEMU Memory Backend (mbe) for emulated SSD */

void femu_init_mem_backend(struct femu_mbe *mbe, int64_t nbytes)
{
    assert(!mbe->mem_backend);

    mbe->size = nbytes;
    mbe->mem_backend = g_malloc0(nbytes);
    if (mbe->mem_backend == NULL) {
        error_report("FEMU: cannot allocate %" PRId64 " bytes for emulating SSD,"
                "make sure you have enough free DRAM in your host\n", nbytes);
        abort();
    }

    if (mlock(mbe->mem_backend, nbytes) == -1) {
        error_report("FEMU: failed to pin %" PRId64 " bytes ...\n", nbytes);
        g_free(mbe->mem_backend);
        abort();
    }
}

void femu_destroy_mem_backend(struct femu_mbe *mbe)
{
    if (mbe->mem_backend) {
        munlock(mbe->mem_backend, mbe->size);
        g_free(mbe->mem_backend);
    }
}

inline static void add_delay(uint32_t micro_seconds) {
	unsigned long long current_time, req_time;
	current_time = cpu_get_host_ticks();
	req_time = current_time + (micro_seconds);
	while( cpu_get_host_ticks()  < req_time);
}

uint64_t do_pointer_chase(int computational_fd_send, int computational_fd_recv, int ctype_fd, void *mb, uint64_t mb_oft, 
	dma_addr_t cur_len, AddressSpace *as, dma_addr_t *cur_addr, uint32_t read_delay)
{
	uint64_t new_offset = mb_oft;
	uint64_t c;
	int ret;
	DMADirection dir = DMA_DIRECTION_FROM_DEVICE;

	enum NvmeComputeDirectiveType computetype = NVME_DIR_COMPUTE_POINTER_CHASE;
	ret = write(ctype_fd, &computetype , sizeof(uint8_t));
	if (ret < 0) {
		printf("write on pipe failed %s\n", strerror(errno));
		exit(1);
	}

	printf("new_offset = %lu\n", new_offset);
	ret = write(computational_fd_send, mb + new_offset, 4096);
	if ( ret < 0) {
		printf("write on pipe failed %s\n", strerror(errno));
		exit(1);
	}
	c = 0;
	ret = read(computational_fd_recv, &c, sizeof(c));
	printf("received new block number = %lu\n", new_offset);
	if (ret < 0) {
		printf("read from pipe failed %s\n", strerror(errno));	
		exit(1);
	}

	while (c != 0 && c!= END_BLOCK_MAGIC)
	{
		if (mb + new_offset != NULL) {
			new_offset = c * BLOCK_SIZE;
			add_delay(read_delay);
			if (dma_memory_rw(as, *cur_addr, mb + new_offset, cur_len, dir)) {
				error_report("FEMU: dma_memory_rw error");
			}
			ret = write(ctype_fd, &computetype , 1);
			if (ret < 0) {
				printf("write on pipe failed %s\n", strerror(errno));
				exit(1);
			}
			ret = write(computational_fd_send, mb + new_offset, 4096);
			if ( ret < 0) {
				printf("write on pipe failed %s\n", strerror(errno));
			}
			c = 0;

			ret = read(computational_fd_recv, &c, sizeof(c));
			if (ret < 0) {
				printf("read from pipe failed %s\n", strerror(errno));
			}

			printf("next block_pointer %lu\n", c);
		}else {
			return c;
		}
	}
	return c;
}


//uint64_t do_pointer_chase(int computational_fd_send, int computational_fd_recv, int ctype_fd, void *mb, uint64_t mb_oft, 
//	dma_addr_t cur_len, AddressSpace *as, dma_addr_t *cur_addr, uint32_t read_delay)

uint64_t do_compression(int computational_fd_send, int computational_fd_recv, int ctype_fd, void *mb, uint64_t mb_oft, 
	dma_addr_t cur_len, AddressSpace *as, dma_addr_t *cur_addr, uint32_t flash_write_delay, uint32_t flash_read_delay, enum NvmeComputeDirectiveType computetype)
{
	int ret;

	struct pollfd fds[1];
	memset(fds, 0 , sizeof(fds));

	fds[0].fd = computational_fd_recv;
        fds[0].events = POLLIN;
	uint8_t compress_buf[BLOCK_SIZE];

	DMADirection dir;

        struct timespec t;
        t.tv_sec = 0;
        t.tv_nsec = 1;
        sigset_t origmask;

        int rc;
//      int total_read_bytes = 0;

	printf("computation mode = %d\n", computetype);
	ret = write(ctype_fd, &computetype , sizeof(enum NvmeComputeDirectiveType));
	if (ret < 0) {
		printf("write on pipe failed %s\n", strerror(errno));
		exit(1);
	}

	if (computetype == NVME_DIR_COMPUTE_COMPRESSION) {
		dir = DMA_DIRECTION_TO_DEVICE;
		add_delay(flash_write_delay);
	} else {
		dir = DMA_DIRECTION_FROM_DEVICE;
		add_delay(flash_read_delay);
	}

	// first send the write to disk.... remove this eventually.
	if (dma_memory_rw(as, *cur_addr, mb + mb_oft, cur_len, dir)) {
		error_report("FEMU: dma_memory_rw error");
	}

	// then send the write to the computational media
	ret = write(computational_fd_send, mb + mb_oft, 4096);

	rc = ppoll (fds, 1, &t, &origmask);
        if (rc < 0) {
                perror( "poll failed");
                exit(1);
        }

        if (rc == 0) {
                printf(("poll timed out\n"));
        }else {
		// optionally read back the data.
        	if (fds[0].revents == POLLIN) {
        	        if (fds[0].fd == computational_fd_recv) {
        	                ret = read (computational_fd_recv, compress_buf, BLOCK_SIZE);
        	                if (ret < 0) {
        	                        printf("read from pipe failed %s\n", strerror(errno));
        	                        exit(1);
        	                }
        	                printf("returned %d bytes\n", ret);
        	    //            total_read_bytes += ret;
        	    //            ret = write (output_fd, compress_buf, ret);
        	    //            if (ret < 0) {
        	    //                    printf("write to outfile failed %s\n", strerror(errno));
        	    //                    exit(1);
        	    //            }
        	        }
        	}
	}

	// TODO write only if data has been read back from the thread.
	// TODO if data has not been read back from the thread, return 1 so that dont_increment is false.
	return 0;
}

uint64_t do_count(int computational_fd_send, int computational_fd_recv, int ctype_fd, void *mb , uint64_t mb_oft, dma_addr_t cur_len)
{	
	uint8_t computetype = NVME_DIR_COMPUTE_COUNTER;
	int ret = write(ctype_fd, &computetype , 1);
	if (ret < 0) {
		printf("write on pipe failed %s\n", strerror(errno));
		exit(1);
	}

	ret = write(computational_fd_send, mb + mb_oft , cur_len);
	if (ret < 0 ) {
		printf("write on pipe failed %s\n", strerror(errno));
		exit(1);
	}
	uint64_t c=0;
	ret = read(computational_fd_recv, &c, sizeof(c));
	if (ret < 0) {
		printf("read from pipe failed %s\n", strerror(errno));
		exit(1);
	}
	printf("number of bytes in current block %lu\n", c);
	return c;
}

bool opTypeMismatch(enum NvmeComputeDirectiveType c, bool is_write)
{
	if (c == NVME_DIR_COMPUTE_COUNTER || c == NVME_DIR_COMPUTE_POINTER_CHASE || c == NVME_DIR_COMPUTE_DECOMPRESSION) {
		if(is_write) {
			return true;
		} else {
			return false;
		}
	}else {
		if (is_write) {
			return false;
		} else {
			return true;
		}
	}
}

/* Coperd: directly read/write to memory backend from blackbox mode */
int femu_rw_mem_backend_bb(struct femu_mbe *mbe, QEMUSGList *qsg,
        uint64_t data_offset, bool is_write, int computational_fd_send, int computational_fd_recv, int ctype_fd, enum NvmeComputeDirectiveType computetype)
{
    int sg_cur_index = 0;
    dma_addr_t sg_cur_byte = 0;
    dma_addr_t cur_addr, cur_len;
    uint64_t mb_oft = data_offset;
    void *mb = mbe->mem_backend;
	bool dont_increment = false;

	uint32_t flash_read_delay = mbe->flash_read_latency;
	uint32_t flash_write_delay = mbe->flash_write_latency;

	int ret;
    DMADirection dir = DMA_DIRECTION_FROM_DEVICE;

    if (is_write) {
        dir = DMA_DIRECTION_TO_DEVICE;
    }

    while (sg_cur_index < qsg->nsg) {
        cur_addr = qsg->sg[sg_cur_index].base + sg_cur_byte;
        cur_len = qsg->sg[sg_cur_index].len - sg_cur_byte;

	// make first I/O irrespective of compute mode.
	if (is_write) {
		add_delay(flash_write_delay);
	} else {
		add_delay(flash_read_delay);
	}
	// for compression and decompression, do not do the first I/O.
        if (!isCompression(mbe->computation_mode)) {
		printf("do dma\n");
		if(dma_memory_rw(qsg->as, cur_addr, mb + mb_oft, cur_len, dir)) {
			error_report("FEMU: dma_memory_rw error");
		}
        }else {
		printf("skip DMA\n");
	}

	if (mbe->computation_mode) {
		
		// if (is_write) : Write Based computation, eg. compression. else:
		if ((mb + mb_oft) != NULL) {
			if (opTypeMismatch(computetype, is_write)) {
				continue;
			}
			switch (computetype) {
				case NVME_DIR_COMPUTE_NONE:
					printf("no compute\n");
					// TODO debug spurious reads..
					break;

				case NVME_DIR_COMPUTE_COUNTER:
					printf("counter io\n");
					ret = do_count(computational_fd_send, computational_fd_recv, ctype_fd, mb, mb_oft, cur_len);
					if (ret < 0) {
						printf("Error occured while counting %s\n", strerror(ret));
					}
					break;

				case NVME_DIR_COMPUTE_POINTER_CHASE:
					printf("pointer io\n");
					ret = do_pointer_chase(computational_fd_send, computational_fd_recv, ctype_fd, mb, mb_oft, cur_len, qsg->as, &cur_addr, flash_read_delay);
					if (ret < 0) {
						printf("Error occured while counting %s\n", strerror(ret));
					}
					break;

				case NVME_DIR_COMPUTE_COMPRESSION:
				case NVME_DIR_COMPUTE_DECOMPRESSION:
					printf("compress io\n");
					ret = do_compression(computational_fd_send, computational_fd_recv, ctype_fd, mb, mb_oft, 
							cur_len, qsg->as, &cur_addr, flash_write_delay, flash_read_delay, mbe->computation_mode);
					if (ret) dont_increment = true;
					break;

				default:
					printf("warning: could not find compute type\n");
			}
		}
	}

	if(!dont_increment) {
	        mb_oft += cur_len;
	}else {
		dont_increment = false;
	}

        sg_cur_byte += cur_len;
        if (sg_cur_byte == qsg->sg[sg_cur_index].len) {
            sg_cur_byte = 0;
            ++sg_cur_index;
        }
    }

    qemu_sglist_destroy(qsg);

    return 0;
}

/* Coperd: directly read/write to memory backend from whitebox mode */
int femu_rw_mem_backend_oc(struct femu_mbe *mbe, QEMUSGList *qsg,
        uint64_t *data_offset, bool is_write)
{
    int sg_cur_index = 0;
    dma_addr_t sg_cur_byte = 0;
    dma_addr_t cur_addr, cur_len;
    uint64_t mb_oft = data_offset[0];
    void *mb = mbe->mem_backend;

    DMADirection dir = DMA_DIRECTION_FROM_DEVICE;

    if (is_write) {
        dir = DMA_DIRECTION_TO_DEVICE;
    }

    while (sg_cur_index < qsg->nsg) {
        cur_addr = qsg->sg[sg_cur_index].base + sg_cur_byte;
        cur_len = qsg->sg[sg_cur_index].len - sg_cur_byte;
        if (dma_memory_rw(qsg->as, cur_addr, mb + mb_oft, cur_len, dir)) {
            error_report("FEMU: dma_memory_rw error");
        }

        sg_cur_byte += cur_len;
        if (sg_cur_byte == qsg->sg[sg_cur_index].len) {
            sg_cur_byte = 0;
            ++sg_cur_index;
        }

        /*
         * Coperd: update drive LBA to r/w next time
         * for OC: all LBAs are in data_offset[]
         * for BB: LBAs are continuous
         */
        mb_oft = data_offset[sg_cur_index];
	printf("mb_oft %lu sg_cur_index %d\n", mb_oft, sg_cur_index);
    }

    qemu_sglist_destroy(qsg);

    return 0;
}
