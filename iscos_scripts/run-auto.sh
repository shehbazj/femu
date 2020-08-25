#!/bin/bash
# Huaicheng Li <huaicheng@cs.uchicago.edu>
# Run VM with FEMU support: FEMU as a black-box SSD (FTL managed by the device)

# image directory

source $1

IMGDIR=$HOME/lighthost/iskos_images
# virtual machine disk image
OSIMGF=$IMGDIR/40G

FEMU_BUILDDIR=$HOME/femu/build_femu

if [[ ! -e "$OSIMGF" ]]; then
	echo ""
	echo "VM disk image couldn't be found ..."
	echo "Please prepare a usable VM image and place it as $OSIMGF"
	echo "Once VM disk image is ready, please rerun this script again"
	echo ""
	exit
fi

num_cpu=${CPU:-12}
memory=${MEMORY:-2G}
dev_size=${DEVSIZE:-1024}
computation_mode=${COMPUTEMODE:-1}
flash_read_latency=${FLASH_READ_LATENCY:-0}
flash_write_latency=${FLASH_WRITE_LATENCY:-0}

echo $num_cpu
echo $memory
echo $dev_size
echo $computation_mode
echo $flash_read_latency
echo $flash_write_latency

sudo $FEMU_BUILDDIR/x86_64-softmmu/qemu-system-x86_64 \
    -name "FEMU-blackbox-SSD" \
    -enable-kvm \
    -cpu host \
    -smp $num_cpu \
    -m $memory \
    -device virtio-scsi-pci,id=scsi0 \
    -device scsi-hd,drive=hd0 \
    -drive file=$OSIMGF,if=none,aio=native,cache=none,format=raw,id=hd0 \
    -device femu,devsz_mb=$dev_size,femu_mode=1,computation_mode=$computation_mode,flash_read_latency=$flash_read_latency,flash_write_latency=$flash_write_latency \
    -net user,hostfwd=tcp::8080-:22 \
    -net nic,model=virtio #\
#    -qmp unix:./qmp-sock,server,nowait 2>&1 | tee log
#    -drive file=$OSIMGF,if=none,aio=native,cache=none,format=qcow2,id=hd0 \

# access VM :
# ssh vm@localhost -p 8080
