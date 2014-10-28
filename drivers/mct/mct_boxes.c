/*********************************************************************************

 Copyright MC-Technology GmbH
 	- 	Listenbox-Funktionen, zum Umgang mit Referenz-Listen.
	- 	Die Anzahl der zugelassenen Elemente ist PROP_BOX_ELEMENTS, wobei das letzte
		Element der Liste immer <NULL> sein muss!
  
 Autor:	Dipl.-Ing. Steffen Kutsche		

	MCT_BOXES_TRACE - siehe Makefile
	Fehlertrace ist immer aktiv und nicht abschaltbar!

*********************************************************************************/
//#include <linux/module.h>
#include <linux/string.h>
#include "mct_boxes.h"

#ifdef MCT_BOXES_TRACE
	#define DEBUG_BOX_CALL printk("%s()\n", __FUNCTION__)
	#define DEBUG_BOX_ADJUST(elem,sel,id)	printk("elem: (%s) sel:(%d) id:(%d)\n", elem, sel, id)
#else
	#define DEBUG_BOX_CALL 
	#define DEBUG_BOX_ADJUST(elem,sel,id)
#endif
// Diese Funktion besetzt die interne _ref-Liste mit den Werten aus einer ref[]-Liste
// Es wird erwartet, dass die ref[]-Liste mit <NULL> Eintrag terminiert und maximal 
// PROP_BOX_ELEMENTS-1 Elemente haben darf. 
bool box_setup(struct prop_box * this, unsigned int idx, char * const ref[])	{
	unsigned char _elem;
	this->_idx = 0;							// Basis-Multiplikator löschen.
	DEBUG_BOX_CALL;
	for(_elem = 0;  ref[_elem] != NULL ;_elem++)	{
		this->_ref[_elem] = ref[_elem];  
	if(_elem == PROP_BOX_ELEMENTS-1) { 	// Zu viele Elemente Fehler!
			this->_ref[_elem] = NULL;
			printk(KERN_ERR "%s: %s\n", __FUNCTION__, "Max box-elements reached!"); 
			return false;
		}
	}
	this->_ref[_elem] = NULL;
	if(_elem == 0)
	{
		printk(KERN_ERR "%s: %s\n", __FUNCTION__, "0 box-elements!"); 
		return false;						// Kein Element auch Fehler!
	}
	this->_idx = idx;						// Multiplikator intern merken!
	this->_sel = 0;							// Element 0 ist ausgewählt!
	this->_id = this->_idx;					// aktuelle ID intern merken!	
	DEBUG_BOX_ADJUST(this->_ref[0], this->_sel, this->_id );
	return true;
};
//--------------------------------------------------------------------------------------------------
// Diese Funktion sucht nach einem Element in der internen _ref[] -Liste.
// Wird das Element gefunden, wird es intern per _sel markiert und der
// index des Elementes berechnet!
// Falls das Element nicht gefunden werden kann, bleib alles so wie
// bisher!
//--------------------------------------------------------------------------------------------------
bool box_select(struct prop_box * this, char * pattern)	{
	unsigned int _elem = 0;
	DEBUG_BOX_CALL;
	while(this->_ref[_elem]!=NULL){			// Referenzen zur Prüfung des ELEMENTS verwenden ... bis NULL
		if( strncmp(pattern, this->_ref[_elem], strlen(this->_ref[_elem]))) {
			_elem++;
			continue;
		}	
		this->_sel = _elem;
		this->_id = (_elem * this->_idx) + this->_idx;		// ID berechnen =  (Index * Multiplikator) + 1* Multiplikator
		DEBUG_BOX_ADJUST(this->_ref[_elem], this->_sel, this->_id );
		return true;
	}
	printk(KERN_ERR "%s: %s %s\n", __FUNCTION__, pattern, " not found in box-elements!"); 	
	return false;
};
//--------------------------------------------------------------------------------------------------
// Diese Funktion druckt die _ref[]-Elemente aus, wobei das _sel= Element an der 1. Stelle steht
//--------------------------------------------------------------------------------------------------
int box_sprintf(struct prop_box * this, char * buf)	{
	unsigned int _elem = 0;
	int _written = 0;
	DEBUG_BOX_CALL;
	_written += sprintf(buf,"{\"");					// Object-Start		
	_written += sprintf(buf+_written,this->pname);	// Name
	_written += sprintf(buf+_written,"\":[\"");		// Elemente-Start
	_written += sprintf(buf+_written,this->_ref[this->_sel]);	
	while(this->_ref[_elem]!=NULL){
		if(this->_sel == _elem) { 					// Überspringe das Element da es das _sel-Element ist	
			_elem++;
			continue;
		}
		_written += sprintf(buf+_written,"\",\"");	// Trennzeichen setzen
		_written += sprintf(buf+_written,this->_ref[_elem]);
		_elem++;	
		continue;
	}	
	_written += sprintf(buf+_written,"\"]}");		// Elemente-Ende
	return _written;
};

//--------------------------------------------------------------------------------------------------
// Diese Funktion druckt nur das _sel-Element aus und den Objektnamen aus
//--------------------------------------------------------------------------------------------------
int box_sprintf_sel(struct prop_box * this, char * buf)	{
	int _written = 0;
	DEBUG_BOX_CALL;
	_written += sprintf(buf,"\"");					// Object-Start		
	_written += sprintf(buf+_written,this->pname);	// Name
	_written += sprintf(buf+_written,"\":\"");		// Elemente-Start
	_written += sprintf(buf+_written,this->_ref[this->_sel]);	
	_written += sprintf(buf+_written,"\"");		// Elemente-Ende
	return _written;
};
//--------------------------------------------------------------------------------------------------
// liefert den aktuellen Schlüssel
//--------------------------------------------------------------------------------------------------
unsigned int box_get_id(struct prop_box * this)	{
 	return this->_id;
};
//--------------------------------------------------------------------------------------------------
// liefert den internen Selektor (_sel) 
//--------------------------------------------------------------------------------------------------
unsigned char box_get_sel(struct prop_box * this)	{
 	return this->_sel;
};
//--------------------------------------------------------------------------------------------------
// liefert das Element an der Position
//--------------------------------------------------------------------------------------------------
char * box_get_elem(struct prop_box * this, unsigned char pos)	{
	if(pos > PROP_BOX_ELEMENTS-1)
		return NULL;		// kein Element an der Position!
	else
		return this->_ref[pos];	
};
