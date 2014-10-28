/*********************************************************************************
	Copyright MCQ TECH GmbH 2011

 	$Autor:$	Dipl.-Ing. Steffen Kutsche		
	$Id:$
	$Date:$		18.11.2011
 	
 	Description:  Intermodul-Kommunikation
	
 *******************************************************************************/
#include <linux/fs.h>			//sys und udev filesystem support
#include <linux/module.h>
#include <linux/platform_device.h>
#include "mct_spi_dio_objs.h"

///*********************************************
/// ClientX setzt im Server das Link-Bit
///*********************************************
// Achtung: Keine Überwachung der Clientanzahl!
void icom_cli_set_svr_link(unsigned char client, unsigned char link) {
	//printk("%s(%d) link:%d\n",__FUNCTION__, client, link);
	if(link == 1) {
		OBJS_ODEV_SET_LINK(&dio_O_Outputs,client);
	}
	else {
		OBJS_ODEV_CLR_LINK(&dio_O_Outputs,client);
	}
}

///*********************************************
/// ClientX setzt im Server das Init-Bit
///*********************************************
// Achtung: Keine Überwachung der Clientanzahl!
void icom_cli_set_svr_init(unsigned char client, unsigned char init) {
	unsigned char init_akt = 0;
	//printk("%s(%d) init:%d\n",__FUNCTION__, client,init);
	OBJS_ODEV_RD_INIT(&dio_O_Outputs, init_akt);
	if(init)	
		init_akt |=1<<client;	
	else
		init_akt &=~(1<<client);
	OBJS_ODEV_WR_INIT_4Bit(&dio_O_Outputs, init_akt);
}

///*********************************************
/// Server setzt im ClientX das Init-Bit
///*********************************************
extern void icom_svr_set_cli0_init	(unsigned char init);	// Client0-Side implemented
extern void icom_svr_set_cli1_init	(unsigned char init);	// Client1-Side implemented
extern void icom_svr_set_cli2_init	(unsigned char init);	// Client2-Side implemented
extern void icom_svr_set_cli3_init	(unsigned char init);	// Client3-Side implemented
static void	 (* icom_fkt_0)  		(unsigned char init);	// lokal, temporär

// (Wenn ein gültiger Tastendruck erfolgte)
void icom_svr_set_cli_init(unsigned char client, unsigned char init) {
		init = (init >> client) & 0x01;	// Nur 0 oder 1	
		//printk("%s(%d) init:%d\n",__FUNCTION__, client,init);
		switch(client) {
			case 0: 
				icom_fkt_0 = symbol_get( icom_svr_set_cli0_init );
				if(icom_fkt_0) {
					icom_fkt_0(init);
					symbol_put( icom_svr_set_cli0_init );
				}
				break;

			case 1:
				icom_fkt_0 = symbol_get( icom_svr_set_cli1_init );
				if(icom_fkt_0) {
					icom_fkt_0(init);
					symbol_put( icom_svr_set_cli1_init );
				}
				break;

			case 2: 
				icom_fkt_0 = symbol_get( icom_svr_set_cli2_init );
				if(icom_fkt_0) {
					icom_fkt_0(init);
					symbol_put( icom_svr_set_cli2_init );
				}
				break;

			case 3:
				icom_fkt_0 = symbol_get( icom_svr_set_cli3_init );
				if(icom_fkt_0) {
					icom_fkt_0(init);
					symbol_put( icom_svr_set_cli3_init );
				}
				break;

			default:
				break;
		}
}

///*********************************************
/// Server holt vom ClientX das Init-Bit
///*********************************************
extern int 	icom_svr_get_cli0_init	(void);	// Client0-Side implemented
extern int 	icom_svr_get_cli1_init	(void);	// Client1-Side implemented
extern int 	icom_svr_get_cli2_init	(void);	// Client2-Side implemented
extern int 	icom_svr_get_cli3_init	(void);	// Client3-Side implemented
static int  (* icom_fkt_1)  		(void);

// Dies erfolgt immer unmittelbar nach dem Start der  des Servers (DIO)
// Wenn ClientX mit -1 antwortet, ist ClientX nicht im S0-Mode
// Wenn ClientX  nicht antwortet, dann ist der Treiber gar nicht gestartet
void icom_svr_get_cli_info(void) {
	int flag;
	int inits = 0;
	unsigned char links = 0;
	//printk("%s()\n",__FUNCTION__);
		
	// Client 0: init-bit
	icom_fkt_1 = symbol_get( icom_svr_get_cli0_init );
	if(icom_fkt_1) {
		flag = icom_fkt_1();	// 0,1,-1
		//printk("%s flag:%d\n","client 0",flag);
		switch(flag) {
			case 0:	inits&=~(1<<0);	
					links |=1<<0;
					break;
			case 1: inits|=1<<0;
					links |=1<<0;
					break;
			default: 
					break;
		}	
		symbol_put( icom_svr_get_cli0_init );
	}

	// Client 1: init-bit
	icom_fkt_1 = symbol_get( icom_svr_get_cli1_init );
	if(icom_fkt_1) {
		flag = icom_fkt_1();	// 0,1,-1
		//printk("%s flag:%d\n","client 1",flag);
		switch(flag) {
			case 0:	inits&=~(1<<1);	
					links |=1<<1;
					break;
			case 1: inits|=1<<1;
					links |=1<<1;
					break;
			default: 
					break;
		}
		symbol_put( icom_svr_get_cli1_init );
	}

	// Client 2: init-bit
	icom_fkt_1 = symbol_get( icom_svr_get_cli2_init );
	if(icom_fkt_1) {
		flag = icom_fkt_1();	// 0,1,-1
		//printk("%s flag:%d\n","client 2",flag);
		switch(flag) {
			case 0:	inits&=~(1<<2);
					links |=1<<2;
					break;
			case 1: inits|=1<<2;
					links |=1<<2;
					break;
			default: 
					break;
		}	
		symbol_put( icom_svr_get_cli2_init );
	}

	// Client 3: init-bit
	icom_fkt_1 = symbol_get( icom_svr_get_cli3_init );
	if(icom_fkt_1) {
		flag = icom_fkt_1();	// 0,1,-1
		//printk("%s flag:%d\n","client 3",flag);
		switch(flag) {
			case 0:	inits&=~(1<<3);
					links |=1<<3;
					break;
			case 1: inits|=1<<3;
					links |=1<<3;
					break;
			default: 
					break;
		}	
		symbol_put( icom_svr_get_cli3_init );
	}

	OBJS_ODEV_WR_INIT_4Bit(&dio_O_Outputs,inits);	// Alle Client-Init-Zustände
	OBJS_ODEV_WR_LINK_S(&dio_O_Outputs,links); 	// Alle	Client-Links, die aktiv sind
};

EXPORT_SYMBOL_GPL( icom_cli_set_svr_init );
EXPORT_SYMBOL_GPL( icom_cli_set_svr_link );

