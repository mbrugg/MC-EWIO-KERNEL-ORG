/*********************************************************************************

 	Copyright MC-Technology GmbH 2009,2010

 	$Autor: $	Dipl.-Ing. Steffen Kutsche		
	$Id: $
	$Date:$

	30.07.2010	- #define LBUS_USER_FS entfernt (nutze user Filesystem)
			und damit auch die struct t_fs  wobei alle wichtigen relevanten
			Speicherbereiche nach struct t_attr  verlagert wurden 
			buf_name, buf_in, buf_out ...)


*********************************************************************************/
#ifndef MCT_LBUS_
	#define MCT_LBUS_
/// LBUS-HARDWARE	Infos
/*
	Standardeinstellung:
			Baudrate 	115.200k 
			Daten		8 Byte
			Parität		keine
			Stop-Bits	1
*/
/* ------------------------------------------------------------------------	
	Das L(ower)-BUS-Protokoll ist ein halbduplex Master-Slave-Protokoll.
	Der LBUS verwendet die NETWORK-BYTE-ORDER, d.h. BIG-ENDIAN für alle 
	Multi-Bytes  wie z.B. INT,LONG,FLOAT.
	
	Der Master initiiert und kontrolliert den Protokollablauf. Zur Zeit
	werden LBUS_DEF_SLOTS_MAX Slaves mit jeweils einem Lese- und einem 
	Schreib-Container mit maximal 32 Byte Länge unterstützt.
	Jeder Slave muss mit einer eindeutigen Adresse im Bereich 
	(0...LBUS_DEF_SLOTS_MAX) parametriert sein und die Broadcast-Adresse 255 
	unterstützen.
	
	*** LBUS-UP (Adresse 255)
	Zu Begin der Initialisierung sendet der Master an alle Slaves
	eine gemeinsame LBUS-UP-Message mit der Adresse 255 - es erfolgt
	keine Antwort der Slaves! Slaves können sich nun intern 
	Initialisieren ...

	*** LBUS-DOWN (Adresse 255)
	Der Master sendet an alle Slaves eine gemeinsame LBUS-DOWN-Message 
	mit der Adresse 255 - es erfolgt keine Antwort der Slaves!

	*** RESET (Adresse 0-LBUS_DEF_SLOTS_MAX)
	Ist ein Slave mit der entsprechenden Adresse am LBUS vorhanden, 
	so erfolgt eine RSP_RESET-Antwort. Der Slave kann nun ein Softwarereset 
	ausführen. 
	 
 	*** WHO (Adresse 0-LBUS_DEF_SLOTS_MAX)
	Ist ein Slave mit der entsprechenden Adresse am LBUS vorhanden, so 
	erfolgt eine RSP_WHO-Antwort.Durch die RSP_WHO Antwort eines Slaves 
	kann der Master feststellen, ob der Slave Inputs und/oder Outputs hat,
	eine Konfiguration benötigt und wie viele Bytes zur Konfiguration und
	für den Datentransport notwendig sind. 

	*** PING (Adresse 0-LBUS_DEF_SLOTS_MAX)
	Eine PING-Message des Masters dient der Prüfung - "Lebt das Modul?"
	Ist ein Slave mit der entsprechenden Adresse am  LBUS vorhanden, 
	so erfolgt eine RSP_PING-Antwort. Die Ping-Message wird verwendet,
	um ein Modul abzufragen, dass noch nicht konfiguriert wurde und/oder 
	um ein Modul abzufragen das initialisiert werden muss.
	1) Input/Output-Module können eine Konfiguration erfordern um die
	Hardware einzustellen!
	2) Output-Module sollten immer initialisiert werden um definierte
	Ausgangssignale zu erzeugen! 
	3) Sind Konfiguration und Initialisierung notwendig so ist die Abfolge
	->erst Konfiguration, dann Initialisierung!
	Die interne Statusmaschine muss diese beiden PING-Zustände unterscheiden,
	wobei extern in jedem Fall nur REQ_PING/RSP_PING kommuniziert wird.
	Durch die festgelegte Reihenfolge (erst Konfiguration, dann Initialisierung)
	geht das also.
	
	*** CONFIG (Adresse 0-LBUS_DEF_SLOTS_MAX)
	Eine CONFIG-Message des Masters dient der Configuration des Modules
	Ist ein Slave mit der entsprechenden Adresse am  LBUS vorhanden, 
	so erfolgt eine RSP_CONFIG-Antwort. Eine Konfiguration ist immer gerätespezifisch!
	Die Konfiguration wird durch spezifische  Gerätetreiber "geliefert".

	*** DATA (Adresse 0-LBUS_DEF_SLOTS_MAX)
	REQ_DATA-Message sendet der Master an den Slave und erhält eine RSP_DATA-
	Message als Antwort.
	Der Aufbau der DATA-Message variiert in der Länge und ist vom jeweiligen Modul
	abhängig. Maximal können jedoch nur LBUS_DEF_DATA_LEN Zeichen versendet werden.

	------------------------------------------
	INPUT-Module (asymmetrischer Datenverkehr)
	------------------------------------------
 	z.B DI4, DI10, AI8 welche ausschließlich Eingänge haben
	- RSP_WHO_DESCR_INPUT_BIT

	Per RSP_WHO/REQ_WHO Telegrammsequenz lieferte der LBUS-Slave die Variablen
 	cfg_in	= Anzahl Eingangsbytes
	cfg_out	= 0
	cfg_fg 	= 0	(keine Konfiguration notwendig)

	Der LBUS-Master sendet REQ_DATA-Message mit Längenangabe cfg_out (0) an den
	LBUS-Slave. 
	Der LBUS-Slave antwortet mit RSP_DATA-Message mit der Längenangabe von cfg_in.
	
 	---------------------------------------------------------
	OUTPUT-Module ohne Handmaske (symmetrischer Datenverkehr)
	---------------------------------------------------------
	z.B. AO4 welche ausschließlich Ausgänge haben
	- RSP_WHO_DESCR_OUTPUT_BIT

	Per RSP_WHO/REQ_WHO Telegrammsequenz lieferte der LBUS-Slave die Variablen
 	cfg_in	= 0
	cfg_out	= Anzahl Ausgangsbytes
	cfg_cfg = 0	(keine Konfiguration notwendig)

	Der LBUS-Master sendet REQ_DATA-Message mit Längenangabe cfg_out an den
	LBUS-Slave. 
	Der LBUS-Slave antwortet mit RSP_DATA-Message mit der Längenangabe von cfg_out.
	In der Antwort des LBUS-Slaves wird der aktuelle Zustand der Ausgangsbytes
	zurückgeliefert!
	
 	---------------------------------------------------------
	MIX-Module (asymmetrischer Datenverkehr)
	---------------------------------------------------------
 	z.B. DO4, TO4, DIO4/2 welche Ausgänge und Eingänge haben
	- RSP_WHO_DESCR_OUTPUT_BIT und RSP_WHO_DESCR_OUTPUT_BIT
	Die Anordnung der Bytes, speziell der einzelnen Gruppen:
	Output, Input, Mask (Handschaltung) ist fest vorgegeben.
	Fehlt eine Gruppe so "rutschen" die nachfolgenden Gruppen
	auf.
	Eine "Handschaltung" wird als Quasi-Eingang behandelt bildet
	die Gruppe Hand-Bytes.
	
	Beispiel (DIO 4/2) für die eine vollständige Anordnung eines MIX-Moduls
	mit Output, Input und Handschaltung.
	
	vom Master				zum Master
	(REQ_DATA)				(RSP_DATA)
	---------------------			---------------------	
	| Output-Bytes	    |			| Output-Bytes      |
	---------------------			---------------------
						| Input-Bytes	    |
						---------------------
						| Hand-Bytes	    |
						---------------------
		
	Per RSP_WHO/REQ_WHO Telegrammsequenz lieferte der LBUS-Slave die Variablen
 	cfg_in	= Anzahl Eingangsbytes
	cfg_out	= Anzahl Ausgangsbytes
	cfg_cfg = 0	(keine Konfiguration notwendig)

	************
	Error-Codes
	************
	

	***********
	Checksummen:
	***********
	Alle Bytes außer START, STOP und das Checksummenbyte werden kumuliert
	und anschließend das 2er Kompliment des Ergebinsses gebildet. Die niederwertigen
	8 bits dieses Ergebnisses werden als Checksumme verwendet.
	Um die Checksumme zu prüfen sind alle Bytes (incl. Checksummenbyte) außer 
	START und STOP zu addieren. Falls das Ergebniss der niederwertigen 8 bits
	= 0 ist, ist die Checksumme gültig.	

	LBUS-Frame			Master-Side			Slave-Side
	-------------
	START				= 0x10h
*	Adress-Feld	
*	Command-Feld
*	Fehler-Feld
*	Längen-Feld			= Datenlänge 0 
*	Data[0...23]
	Checksum-Feld
	STOP				= 0x16h
*/

/// Local Debug
// *** developer
//#define db_lbus(lbus_mode_str)	printk("%s\n",lbus_mode_str);	
//#define db_lbus_rsp(msg,slot)		printk("rec: %s[%d]!\n",msg,slot );	
//#define db_lbus_req(msg,slot)		printk("snd: %s[%d]!\n",msg,slot );	

// *** production
#define db_lbus(lbus_mode_str)
#define db_lbus_rsp(msg,slot)
#define db_lbus_req(msg,slot)

/// Globale Infos	
#define LBUS_MASTER			// Arbeite als Master
//#define LBUS_SLAVES			// Arbeite als Slave(s)		

// SlotsMin / SlotsMax
#ifdef CONFIG_MCT_PAA_SLOTS_MAX		// Overload by Kconfig File
	#define	LBUS_DEF_SLOTS_MAX	CONFIG_MCT_PAA_SLOTS_MAX
#else
	#ifdef LBUS_MASTER
		#define LBUS_DEF_SLOTS_MAX	6 	// maximal unterstützte Geräte 6
	#endif
	#ifdef LBUS_SLAVES
		#define LBUS_DEF_SLOTS_MAX	6	// maximal unterstützte Geräte 4
	#endif
#endif

#define	LBUS_DEF_SLOTS_MIN	1

#define LBUS_DEF_TLGS		5			// z.Zeit 5 verschiedene Kommandos 
#define LBUS_DEF_TLG_LEN	7			// Standardtelegrammlänge
#define LBUS_DEF_DATA_LEN	32			// maximale Datenlänge

/// LBUS-Modi
#define LBUS_MODE_IDLE		0
#define LBUS_MODE_SND		1
#define LBUS_MODE_REC		2
#ifdef LBUS_MASTER
	#define LBUS_MODE_UP		3		// Telegramm Bus "hochziehen"
	#define LBUS_MODE_UP_WAIT	4		// timeout abwarten ...
	#define LBUS_MODE_DOWN		5		// Telegramm Bus "runterfahren"
	#define LBUS_MODE_DOWN_WAIT	6		// timeout abwarten ...
	#define LBUS_MODE_SLEEP		7		// Bus ist abgeschaltet
	#define LBUS_MODE_IDLE_STR	"LBUS_MODE_IDLE"	
	#define LBUS_MODE_UP_STR	"LBUS_MODE_UP"
	#define LBUS_MODE_UP_WAIT_STR	"LBUS_MODE_UP_WAIT"
	#define LBUS_MODE_DOWN_STR	"LBUS_MODE_DOWN"
	#define LBUS_MODE_DOWN_WAIT_STR "LBUS_MODE_DOWN_WAIT"
	#define LBUS_MODE_SLEEP_STR 	"LBUS_MODE_SLEEP"	
#endif

/// LBUS-Index-Zugriff mit 0 beginnend
#define LBUS_IDX_STA		0		// Startzeichen
#define LBUS_IDX_ADD		1		// Addresse
#define LBUS_IDX_CMD		2		// Commando
#define LBUS_IDX_ERR		3		// Error
#define LBUS_IDX_LEN		4		// Länge
// wenn Länge = 0 können die Indizes verwendet werden, sonst
// muss dynamisch ermittelt werden!
#define LBUS_IDX_CRC		5		// Checksumme
#define LBUS_IDX_STP		6		// Stopzeichen
#define LBUS_IDX_DATA		5		// Daten

/// Zeichendefinition
#define LBUS_SIGN_START		0x10		// alternativ STX=0x02
#define LBUS_SIGN_STOP		0x16		// alternativ ETX=0x03

/// *** ADDRESS-FIELD ***
#define LBUS_ADD_0		0x00
#define LBUS_ADD_1		0x01
#define LBUS_ADD_2		0x02
#define LBUS_ADD_3		0x03
#define LBUS_ADD_4		0x04
#define LBUS_ADD_5		0x05
#define LBUS_ADD_MAX		LBUS_ADD_5
#define LBUS_ADD_FF		0xff		// Broadcast

/// *** COMMAND-FIELD ***
// Command-Bit-Identifier
#define LBUS_REQ_BIT		0<<7		// Master-> Slave
#define LBUS_RSP_BIT		1<<7		// Slave -> Master
#define LBUS_UP_BIT		1<<5		// Master -> Slaves
#define LBUS_DOWN_BIT		1<<4		// Master -> Slaves

// single Device
#define LBUS_RES		0x00		// (reset slave) 
#define LBUS_WHO		0x01		// (ask node name, containersupport)
#define LBUS_PING		0x02		// (test if node is present, link unconfigured)
#define LBUS_CFG		0x03		// (test if node is present, link configure)
#define LBUS_DATA		0x04		// (data packets)	

// Bus-Nachrichten für alle Geräte ohne Antwort
#define LBUS_BUS_UP		(LBUS_REQ_BIT | LBUS_UP_BIT)	
#define LBUS_BUS_DOWN		(LBUS_REQ_BIT | LBUS_DOWN_BIT)

// Geräte-Nachrichten für jedes einzelne Gerät
// Host --> Slaves
#define LBUS_REQ_RES		(LBUS_REQ_BIT | LBUS_RES)
#define LBUS_REQ_WHO		(LBUS_REQ_BIT | LBUS_WHO)
#define LBUS_REQ_PING		(LBUS_REQ_BIT | LBUS_PING)	
#define LBUS_REQ_CFG		(LBUS_REQ_BIT | LBUS_CFG)	
#define LBUS_REQ_DATA		(LBUS_REQ_BIT | LBUS_DATA)
// Slaves --> Host
#define LBUS_RSP_RES		(LBUS_RSP_BIT | LBUS_RES)
#define LBUS_RSP_WHO		(LBUS_RSP_BIT | LBUS_WHO)
#define LBUS_RSP_PING		(LBUS_RSP_BIT | LBUS_PING)	
#define LBUS_RSP_CFG		(LBUS_RSP_BIT | LBUS_CFG)	
#define LBUS_RSP_DATA		(LBUS_RSP_BIT | LBUS_DATA )

/// *** ERROR-FIELD ***
// Codes
#define LBUS_ERR_NONE		0x00		// Fehlerfrei
#define LBUS_ERR_ADD		0x01		// Adresse 
#define LBUS_ERR_CMD		0x02		// unbekanntes/nicht unterstüztes Kommando!
#define LBUS_ERR_SIZE		0x03		// Containergröße fehlerhaft
#define LBUS_ERR_CRC		0x04		// CRC fehlerhaft
#define LBUS_ERR_END		0x05		// Endekennung
#define LBUS_ERR_LEN		0x06		// fehlerhafte Telegrammlänge
#define LBUS_ERR_TIME		0x07		// Zeitüberschreitung
#define LBUS_ERR_ERROR		0x08		// unbekannter Fehlercode			
#define LBUS_ERR_LAST		LBUS_ERR_ERROR	// Maximal erlaubte Fehlerkennung

/// RSP_WHO Container-Aufbau
//	maximal 24 Zeichen 
//----------------------------------------------
// Byte 0: bit7: Inputs  	Ja/Nein
// Byte 0: bit6: Outputs 	Ja/Nein
// Byte 0: bit5: Konfiguriere  	Ja=1/Nein=0
// Byte 0: bit4: SlowDevice  	Ja=1/Nein=0

// Byte 1: bit3...bit0 	Input:  Containersize-Reservierung (32Byte mehr geht nicht)
// Byte 2: bit3...bit0 	Output: Containersize-Reservierung (32Byte mehr geht nicht)

// Byte 3: 'V' = Versionskenner
// Byte 4: 2*BCD bit7-bit4 = Mainversion bit3-bit0=Subversion
// Byte 5: ... Byte 31	Maximal 26Byte + '\0' für den Namen des Treibers!

// Beispiel:
// CFG			CFG_IN 	CFG_OUT		Version				Name
// 1100 0000  	0x01 	0x02 		'V'	0000 0001		MCT_DIO-4-2\0

#define RSP_WHO_CFG_IDX			0
#define RSP_WHO_CFG_IN_IDX		1
#define RSP_WHO_CFG_OUT_IDX		2

#define RSP_WHO_CFG_INPUT_BIT 		1<<7	// Eingänge
#define RSP_WHO_CFG_OUTPUT_BIT 		1<<6	// Ausgänge
#define RSP_WHO_CFG_CONFIG_BIT 		1<<5	// Konfiguration erforderlich
#define RSP_WHO_CFG_SLOWDEV_BIT 	1<<4	// langsames Gerät, (Sheduler Hinweis)	
#define RSP_WHO_CFG_SIZE_MASK		7<<1	// 2,4,6,8 Byte zugelassen
#define RSP_WHO_CFG_HANDMASK_BIT 	1<<0	// Handsteuerung, (Sheduler Hinweis)	

#define RSP_WHO_VERSION_IDX		3
#define RSP_WHO_VERSION_SIZE		RSP_WHO_NAME_IDX - RSP_WHO_VERSION_IDX
#define RSP_WHO_NAME_IDX		5

/// Receiver States
// intern 
#define LBUS_REC_STATE_IDLE  		0	//
#define LBUS_REC_STATE_START		1	// Startzeichen
#define LBUS_REC_STATE_A_FLD		2	// Adresse gekommen
#define	LBUS_REC_STATE_C_FLD		3	// Kommando gekommen
#define	LBUS_REC_STATE_E_FLD		4	// Fehler gekommen
#define	LBUS_REC_STATE_L_FLD		5	// Längenfeld gekommen
#define LBUS_REC_STATE_DATA		6	// Daten kommen
#define LBUS_REC_STATE_CRC		7	// CRC gekommen
#define	LBUS_REC_STATE_STOP		8	// Stopzeichen
#define LBUS_REC_STATE_RDY		9

/// Sender States
// intern
#define SND_IDLE			0
#define SND_ACTIVE			1
#define SND_READY			2


struct t_snd{
	unsigned char   cmd;						// aktuelles Sende-Commando
	unsigned char * msgs[LBUS_DEF_TLGS]; 				// telegramm-links
	unsigned char 	msg_reset[LBUS_DEF_TLG_LEN];			// Standard: Reset
	unsigned char 	msg_who[LBUS_DEF_TLG_LEN + LBUS_DEF_DATA_LEN];	// Informationen
	unsigned char 	msg_ping[LBUS_DEF_TLG_LEN];			// Standard: Ping
	unsigned char 	msg_cfg[LBUS_DEF_TLG_LEN + LBUS_DEF_DATA_LEN];	// Konfiguration
	unsigned char 	msg_data[LBUS_DEF_TLG_LEN + LBUS_DEF_DATA_LEN];	// Daten
};

struct t_rec{		
	unsigned char	state;			// interner Status
	unsigned char	pos;			// Position im Telegramm
	unsigned char	adr;			// aktuelle Adresse
	unsigned char	cmd;			// aktuelles Kommando
	unsigned char	err;			// aktueller Fehlercode
	unsigned char	err_slv;		// Fehlercode Slave
	unsigned char 	dat_len;		// Datenlängenangabe
	unsigned char 	dat_idx;		// temporäre Datenposition
	unsigned char   dat[LBUS_DEF_DATA_LEN];	// Datenpuffer	
	unsigned char	crc;			// aktueller crc
};

struct t_attr{		
	unsigned char	offline;			// Gerät erkannt (nach WHO)
	unsigned char	cfg;				// Gerät Konfiguration 	(nach WHO)
	unsigned char	cfg_in;				// IN-Längenangabe
	unsigned char	cfg_out;			// OUT-Längenangabe
	unsigned char	cfg_msk;			// MASK-Längenangabe
	unsigned char	cfg_cfg;			// Configuration-Längenangabe
	// from FS
	unsigned char 	buf_nam	[LBUS_DEF_DATA_LEN];
	unsigned char 	buf_ver	[LBUS_DEF_DATA_LEN];
	unsigned char 	buf_in	[LBUS_DEF_DATA_LEN];	
	unsigned char 	buf_out	[LBUS_DEF_DATA_LEN];
	unsigned char 	buf_cfg	[LBUS_DEF_DATA_LEN];

	unsigned char	ext_cfg;			// 1= Gerätekonfiguration extern angelegt!
	unsigned char	ext_ini;			// 1= Initialisierung extern angelegt!
	unsigned char	int_cfg;			// 1= Gerätekonfiguration beendet!
	unsigned char	int_ini;			// 1= Initialisierung beendet!
};


#define LBUS_EXTERNAL_PING_TEST 3

/// LOWER-BUS-CONTROL
struct lower_bus{	
	unsigned char	mode;				// aktueller Status
	unsigned char 	slot;				// aktueller Slot
	unsigned char   time;				// aktueller Rundenzähler
	struct t_attr	ctrl[LBUS_DEF_SLOTS_MAX];	// Control
	struct t_snd	snd	[LBUS_DEF_SLOTS_MAX];
	struct t_rec	rec;
#ifdef LBUS_MASTER
	unsigned char 	msg_up	[LBUS_DEF_TLG_LEN];	// LBus "hochfahren"	
	unsigned char 	msg_down[LBUS_DEF_TLG_LEN];	// LBus "runterfahren"
#endif
};

#endif

