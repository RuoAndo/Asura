grep physical.id /proc/cpuinfo | sort -u | wc -l
grep cpu.cores /proc/cpuinfo | sort -u
grep processor /proc/cpuinfo | wc -l
