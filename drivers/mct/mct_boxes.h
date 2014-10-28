#ifndef MCT_BOXES_H_
	#define MCT_BOXES_H_

#include "mct_types.h"		// declare bool-Type

#define PROP_BOX_ELEMENTS	15

struct	prop_box{
	// globals
	char *			pname;
	// locals
	unsigned int	_id;							// Schlüsselnummer 
	unsigned int 	_idx;							// Schlüsselmultiplikator z.B TYPE_IDX, ATTR_IDX
	unsigned char	_sel;
	char *			_ref[PROP_BOX_ELEMENTS];		// Referenz-Elemente-Queue der zulässigen Elemente	
	// functions
	bool			(* setup) 	(struct prop_box*, unsigned int idx, char * const []);
	bool 			(* select) 	(struct prop_box*, char * pattern);
	int 			(* sprintf) 	(struct prop_box*, char * buf);
	int 			(* sprintf_sel) (struct prop_box*, char * buf);
	unsigned int	(* get_id) 	(struct prop_box*); 					// Liefert den aktuellen Schlüssel (_id )
	char *			(* get_elem)(struct prop_box*, unsigned char pos); 	// Liefert Element als String, der Position pos
};


extern bool 			box_setup(struct prop_box * this, unsigned int idx, char * const new_table[]);
extern bool 			box_select(struct prop_box * this, char * pattern);
extern int 				box_sprintf(struct prop_box * this, char * buf);
extern int 				box_sprintf_sel(struct prop_box * this, char * buf);

extern unsigned int 	box_get_id(struct prop_box * this);
extern unsigned char 	box_get_sel(struct prop_box * this);
extern char * 			box_get_elem(struct prop_box * this, unsigned char pos);
#endif
