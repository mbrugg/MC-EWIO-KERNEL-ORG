/*********************************************************************************

 Copyright MC-Technology GmbH 2009

 Autor:	Dipl.-Ing. Steffen Kutsche		

NOTE:
	Debugging macro and defines 

*********************************************************************************/
#define MCT_DEBUG_LEVEL0	(0)	/* Quiet   */
#define MCT_DEBUG_LEVEL1	(1)	/* Audible - timer infos */
#define MCT_DEBUG_LEVEL2	(2)	/* Loud    - function calls */
#define MCT_DEBUG_LEVEL3	(3)	/* Noisy   - json-level*/
#define MCT_DEBUG_LEVEL4	(4)	/* Blubber - key-level*/


#ifdef CONFIG_MCT_DEBUG
	#define DEBUG(n, args...)				\
	do {						\
		if (n <= CONFIG_MCT_DEBUG_VERBOSE)	\
			printk(KERN_INFO args);		\
	} while(0)
#else /* CONFIG_MCT_DEBUG */
	#define DEBUG(n, args...) do { } while(0)
#endif /* CONFIG_MCT_DEBUG */
