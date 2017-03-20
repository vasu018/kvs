
HUGEPGSZ=`cat /proc/meminfo  | grep Hugepagesize | cut -d : -f 2 | tr -d ' '`
#
# Loads new igb_uio.ko (and uio module if needed).
#
load_igb_uio_module()
{
	if [ ! -f $RTE_SDK/$RTE_TARGET/kmod/igb_uio.ko ];then
		echo "## ERROR: Target does not have the DPDK UIO Kernel Module."
		echo "       To fix, please try to rebuild target."
		return
	fi

	remove_igb_uio_module

	/sbin/lsmod | grep -s uio > /dev/null
	if [ $? -ne 0 ] ; then
		modinfo uio > /dev/null
		if [ $? -eq 0 ]; then
			echo "Loading uio module"
			sudo /sbin/modprobe uio
		fi
	fi

	# UIO may be compiled into kernel, so it may not be an error if it can't
	# be loaded.

	echo "Loading DPDK UIO module"
	sudo /sbin/insmod $RTE_SDK/$RTE_TARGET/kmod/igb_uio.ko
	if [ $? -ne 0 ] ; then
		echo "## ERROR: Could not load kmod/igb_uio.ko."
		quit
	fi
}

#
# Creates hugepage filesystem.
#
create_mnt_huge()
{
	echo "Creating /mnt/huge and mounting as hugetlbfs"
	sudo mkdir -p /mnt/huge

	grep -s '/mnt/huge' /proc/mounts > /dev/null
	if [ $? -ne 0 ] ; then
		sudo mount -t hugetlbfs nodev /mnt/huge
	fi
}
set_non_numa_pages()
{
	clear_huge_pages

	echo ""
	echo "  Input the number of ${HUGEPGSZ} hugepages"
	echo "  Example: to have 128MB of hugepages available in a 2MB huge page system,"
	echo "  enter '64' to reserve 64 * 2MB pages"
	echo -n "Number of pages: "
	read Pages

	echo "echo $Pages > /sys/kernel/mm/hugepages/hugepages-${HUGEPGSZ}/nr_hugepages" > .echo_tmp

	echo "Reserving hugepages"
	sudo sh .echo_tmp
	rm -f .echo_tmp

	create_mnt_huge
}

load_igb_uio_module
set_non_numa_pages
