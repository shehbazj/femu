#ifndef __FEMU_MEM_BACKEND__
#define __FEMU_MEM_BACKEND__

#include <stdint.h>
//#include "nvme.h"

/* Coperd: FEMU memory backend structure */
struct femu_mbe {
    void *mem_backend;
    int64_t size; /* in bytes */
    int femu_mode;
	int computation_mode;
	uint32_t flash_read_latency;
	uint32_t flash_write_latency;
};

enum NvmeComputeDirectiveType;

void femu_init_mem_backend(struct femu_mbe *mbe, int64_t nbytes);
void femu_destroy_mem_backend(struct femu_mbe *mbe);
int femu_rw_mem_backend_bb(struct femu_mbe *mbe, QEMUSGList *qsg,
        uint64_t data_offset, bool is_write, int computational_fd_send, 
	int computational_fd_recv, int ctype_fd, enum NvmeComputeDirectiveType computetype);
int femu_rw_mem_backend_oc(struct femu_mbe *mbe, QEMUSGList *qsg,
        uint64_t *data_offset, bool is_write);

#endif
