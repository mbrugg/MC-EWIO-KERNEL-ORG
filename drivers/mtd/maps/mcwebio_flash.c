/*
 * MTD map driver for flash on the MCWEBIO
 *
 * (C) 2009 Karl-Heinz Weber MC-Technology <kweber@metz-connect.com>
 * This code is GPL
 *
 */
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>
#include <asm/mach-types.h>

#include <mach/gpio.h>

//select Adr25->adr24 (max 32 MB) or adr 24 as pioport PC8 (max 64 MB)
//#define MCWEBIO_FLASH_CS
#ifndef MCWEBIO_FLASH_CS
	#define MCWEBIO_FLASH_PIO
#endif

static struct mtd_info *mcwebio_mtd;
#ifdef MCWEBIO_FLASH_CS
static unsigned char* virt_memory2_vector;
#endif
static unsigned char* virt_memory1_vector;

#define MCWEBIO_FLASH_PHYS_START 0x10000000 
#ifdef MCWEBIO_FLASH_PIO
	#define MCWEBIO_FLASH_LEN 	128*1024*1024
#else
	#define MCWEBIO_FLASH_LEN 	32*1024*1024
#endif

 
#define VIRT_SEC1_OFS 	0x00000000
#define VIRT_SEC2_OFS 	0x01000000
#define VIRT_SEC3_OFS 	0x02000000
#define VIRT_SEC4_OFS	0x03000000
#define VIRT_SEC5_OFS	0x04000000
#define VIRT_SEC6_OFS	0x05000000
#define VIRT_SEC7_OFS	0x06000000
#define VIRT_SEC8_OFS	0x07000000


#ifdef MCWEBIO_FLASH_CS
/* if 64 MB Flash mounted */
#define PHYS_SEC3_START	0x30000000
#define MCWEBIO_FLASH_PHYS_START2 0x30000000


//inline uint16_t* mcwebio_ofs_to_virt_addr(struct map_info *map, unsigned long ofs)
inline uint16_t* mcwebio_ofs_to_virt(struct map_info *map, unsigned long ofs)
{
	uint16_t* ret;
	int i;
	static int iS;
	
	if (ofs >= VIRT_SEC4_OFS)
		{

		ret = (uint16_t*)(virt_memory2_vector + ofs + 0x1000000);
		}
	else if (ofs >= VIRT_SEC3_OFS)
		{
		ret = (uint16_t*)(virt_memory2_vector + ofs );
		}
	else if (ofs >= VIRT_SEC2_OFS)
		{
		ret = (uint16_t*)(map->virt + ofs + 0x1000000);
		}
	else
		{
		ret = (uint16_t*)(map->virt + ofs);
		}
return ret;
}
#endif

#ifdef MCWEBIO_FLASH_PIO
//inline uint16_t* mcwebio_ofs_to_virt_pio(struct map_info *map, unsigned long ofs)
inline uint16_t* mcwebio_ofs_to_virt(struct map_info *map, unsigned long ofs)
{
	uint16_t* ret;	
	if (ofs >= VIRT_SEC8_OFS)
		{
		at91_set_gpio_value(AT91_PIN_PC8, 1);
		at91_set_gpio_value(AT91_PIN_PC10, 0);
		ret = (uint16_t*)(map->virt + ofs);
		}	
	else if (ofs >= VIRT_SEC7_OFS)
		{
		at91_set_gpio_value(AT91_PIN_PC8, 0);
		at91_set_gpio_value(AT91_PIN_PC10, 0);
		ret = (uint16_t*)(map->virt + ofs);
		}
	else if (ofs >= VIRT_SEC6_OFS)
		{
		at91_set_gpio_value(AT91_PIN_PC8, 1);
		at91_set_gpio_value(AT91_PIN_PC10, 0);
		ret = (uint16_t*)(map->virt + ofs);
		}
	else if (ofs >= VIRT_SEC5_OFS)
		{
		at91_set_gpio_value(AT91_PIN_PC8, 0);
		at91_set_gpio_value(AT91_PIN_PC10, 0);
		ret = (uint16_t*)(map->virt + ofs);
		}
	else if (ofs >= VIRT_SEC4_OFS)
		{
		at91_set_gpio_value(AT91_PIN_PC8, 1);
		at91_set_gpio_value(AT91_PIN_PC10, 1);
		ret = (uint16_t*)(map->virt + ofs);		
		}
	else if (ofs >= VIRT_SEC3_OFS)
		{
		at91_set_gpio_value(AT91_PIN_PC8, 0);
		at91_set_gpio_value(AT91_PIN_PC10, 1);
		ret = (uint16_t*)(map->virt + ofs);		
		}
	else if (ofs >= VIRT_SEC2_OFS)
		{
		at91_set_gpio_value(AT91_PIN_PC8, 1);
		at91_set_gpio_value(AT91_PIN_PC10, 1);
		ret = (uint16_t*)(map->virt + ofs);		
		}
	else
		{
		at91_set_gpio_value(AT91_PIN_PC8, 0);
		at91_set_gpio_value(AT91_PIN_PC10, 1);
		ret = (uint16_t*)(map->virt + ofs);
		}
return ret;
}
#endif


static map_word mcwebio_read(struct map_info *map, unsigned long ofs)
{
	map_word val;	/* map_word is an array of long */
	val.x[0] = __raw_readw(mcwebio_ofs_to_virt(map, ofs));
	//val.x[0] = *mcwebio_ofs_to_virt(map, ofs);	
	return val;
}

static void mcwebio_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
	memcpy(to, (void*)mcwebio_ofs_to_virt(map, from), len);
/*	while (len > 0) {
		map_word d;
		d=mcwebio_read(map, from);
		*((uint16_t*)to)=d.x[0];
		from += 2;
		to += 2;
		len -= 2;
	}*/
}

static void mcwebio_write(struct map_info *map, const map_word d, unsigned long adr)
{
	 __raw_writew(d.x[0],mcwebio_ofs_to_virt(map, adr));
	//*mcwebio_ofs_to_virt(map, adr) = d.x[0];
}

static void mcwebio_copy_to(struct map_info *map, unsigned long to, const void *from, ssize_t len)
{
	while (len > 0) {
		map_word d;
		d.x[0] = *((uint16_t*)from);
		mcwebio_write(map, d, to);
		from += 2;
		to += 2;
		len -= 2;
	}
}

static struct map_info mcwebio_map = {
	.name = "mcwebio-flash",
	.bankwidth = 2,
	.phys = MCWEBIO_FLASH_PHYS_START,
	.size = MCWEBIO_FLASH_LEN,
	.copy_from = mcwebio_copy_from,
	.read = mcwebio_read,
	.write = mcwebio_write,
	.copy_to = mcwebio_copy_to
};


/* Partition stuff */
#ifdef CONFIG_MTD_PARTITIONS
static struct mtd_partition *mcwebio_parts;
static const char *part_probe_types[] = { "cmdlinepart", NULL };
#endif

static int __init init_mcwebio_flash(void)
{

#ifdef CONFIG_MTD_PARTITIONS
	int nrparts;
#endif

	printk (KERN_NOTICE "mcwebio flash support (%d-bit bankwidth)\n",
		mcwebio_map.bankwidth*8);

	mcwebio_map.virt = ioremap(MCWEBIO_FLASH_PHYS_START, 128*1024*1024);
	virt_memory1_vector=mcwebio_map.virt;
	if (!mcwebio_map.virt) {
		printk("Failed to ioremap\n");
		return -EIO;
	}
	printk("virt memory1 %p\n",mcwebio_map.virt);

	at91_set_gpio_output(AT91_PIN_PC8, 0);

#ifdef MCWEBIO_FLASH_CS
	virt_memory2_vector = ioremap(MCWEBIO_FLASH_PHYS_START2, 128*1024*1024);
	if ((!mcwebio_map.virt)||(!virt_memory2_vector)) {
		printk("Failed to ioremap\n");
		return -EIO;
	}
	printk("virt memory2 %x\n",virt_memory2_vector);

#endif

	mcwebio_mtd = do_map_probe("cfi_probe", &mcwebio_map);
	if (!mcwebio_mtd) {
		iounmap(mcwebio_map.virt);
		return -ENXIO;
	}
	mcwebio_mtd->owner = THIS_MODULE;

#ifdef CONFIG_MTD_PARTITIONS
	nrparts = parse_mtd_partitions(mcwebio_mtd, part_probe_types, &mcwebio_parts, 0);
	printk("MTD-name: %s parts %d\n", mcwebio_mtd->name,nrparts);
	if (nrparts > 0) {
		add_mtd_partitions(mcwebio_mtd, mcwebio_parts, nrparts);
		return 0;
	}
#endif

	add_mtd_device(mcwebio_mtd);
	return 0;
}

static void __exit cleanup_mcwebio_flash(void)
{
#ifdef CONFIG_MTD_PARTITIONS
	if (mcwebio_parts) {
		del_mtd_partitions(mcwebio_mtd);
		kfree(mcwebio_parts);
	} else
#endif
		del_mtd_device(mcwebio_mtd);

	map_destroy(mcwebio_mtd);
	iounmap(mcwebio_map.virt);
#ifdef MCWEBIO_FLASH_CS
	iounmap(virt_memory2_vector);
#endif
	
}

module_init(init_mcwebio_flash);
module_exit(cleanup_mcwebio_flash);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Karl-Heinz Weber MC-Technology <kweber@metz-connect.com>");
MODULE_DESCRIPTION("MTD map driver for MCWEBIO board");

