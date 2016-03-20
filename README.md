# Summary

The Forwarding Information Base (FIB) of backbone routers has been rapidly growing in size. An ideal IP lookup algorithm should achieve constant, yet small, IP lookup time and on-chip memory usage. However, no prior IP lookup algorithm achieves both requirements at the same time. The main contributions of this paper lie in the following aspects.
First, we propose a two-dimensional splitting approach to IP lookup. The key benefit of such splitting is that we can solve the sub-problem of finding the prefix length <25 in on-chip memory of bounded small size. Second, we propose a suite of algorithms for IP lookup based on our SAIL framework. One key feature of our algorithms is that we achieve constant, yet small, IP lookup time and on-chip memory usage. Another key feature is that our algorithms are cross platform as the data structures are all arrays and only require four operations of ADD, SUBTRACTION, SHIFT, and logical AND. Note that SAIL is a general framework where different solutions to the sub-problems can be adopted. The algorithms proposed in this paper represent particular instantiations of our SAIL framework. Third, we implemented our algorithms on four platforms (namely FPGA, CPU, GPU, and many-core) and conducted extensive experiments to evaluate our algorithms using real FIBs and traffic from a major ISP in China. Our experimental results show that our SAIL algorithms are several times or even two orders of magnitude faster than the well-known IP lookup algorithms.

# Publications

Guarantee IP Lookup Performance with FIB Explosion

Tong Yang, Gaogang Xie, Yanbiao Li, Qiaobin Fu, Alex Liu, Qi Li, Laurent Mathy

ACM SIGCOMM 2014

# Releases

**Platform**: Windows/Visual C++ 2010

**Lulea (Release 0.1)**

Lulea algorithm is proposed by sigcomm 97 Best paper. One of the most sophisticated algorithms, Lulea algorithm, constructing very small lookup tables mainly using bitmap mechanism, and thus can perform fast lookup, is also chosen to compare with our algorithm, although it cannot perform incremental update. When the lookup most occurs in on-chip memory, the determinant of lookup speed is no longer the number of memory accesses, but the instructions and CPU cycles. Because the operations of Lulea is complicated, thus the lookup speed is limited.

M. Degermark and e. a. Brodnik. Small forwarding tables for fast routing lookups. In ACM SIGCOMM '97, SIGCOMM '97 Best paper, pages 3{14, New York, NY, USA, 1997. ACM.

**LC-trie (Release 0.1)**

LC-trie is the classic algorithm, which is adopted by Linux Kernel. LC-trie combines path compression and level-compression, thus can significantly reduce the memory accesses for one lookup. Unfortunately, due to the large memory requirement of trie structure, LC-trie has a relatively large memory requirement, thus the cache behavior is not so good, incurring not so fast lookup speed.

S. Nilsson and G. Karlsson. IP-address lookup using LC-tries. Selected Areas in Communications, IEEE Journal on, 17(6):1083-1092, 1999.

**Tree Bitmap (Release 0.1)**

An important variant of Lulea, tree bitmap, is adopted by Cisco routers. It also uses the bitmap method to compress lookup table, and adds the ability of update at the cost of the degradation of lookup speed. Tree bitmap algorithm is chosen to compare with our algorithm as well.

W. Eatherton, G. Varghese, and Z. Dittia. Tree bitmap: hardware/software ip lookups with incremental updates. ACM SIGCOMM Computer Communication Review, 34(2):97{122, 2004.

**SAIL_L (Release 0.2)**

We propose a two-dimensional splitting method which is the prerequisite for achieving constant lookup time and constant on-chip memory simultaneously.

Towards different goals, we propose three algorithms (SAIL_B, SAIL_U, and SAIL_L) that can achieve both constant lookup time and constant on-chip memory based on this splitting method.

SAIL_L algorithm is the version of speed oriented performance optimization based on SAIL algorithm on CPU platform.

For more information, please refer to http://fi.ict.ac.cn/firg.php?n=PublicationsAmpTalks.OpenSource
