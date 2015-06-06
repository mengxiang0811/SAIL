# Summary

The Forwarding Information Base (FIB) of backbone routers has been rapidly growing in size. An ideal IP lookup algorithm should achieve constant, yet small, IP lookup time and on-chip memory usage. However, no prior IP lookup algorithm achieves both requirements at the same time. The main contributions of this paper lie in the following aspects.
First, we propose a two-dimensional splitting approach to IP lookup. The key benefit of such splitting is that we can solve the sub-problem of finding the prefix length <25 in on-chip memory of bounded small size. Second, we propose a suite of algorithms for IP lookup based on our SAIL framework. One key feature of our algorithms is that we achieve constant, yet small, IP lookup time and on-chip memory usage. Another key feature is that our algorithms are cross platform as the data structures are all arrays and only require four operations of ADD, SUBTRACTION, SHIFT, and logical AND. Note that SAIL is a general framework where different solutions to the sub-problems can be adopted. The algorithms proposed in this paper represent particular instantiations of our SAIL framework. Third, we implemented our algorithms on four platforms (namely FPGA, CPU, GPU, and many-core) and conducted extensive experiments to evaluate our algorithms using real FIBs and traffic from a major ISP in China. Our experimental results show that our SAIL algorithms are several times or even two orders of magnitude faster than the well-known IP lookup algorithms.
