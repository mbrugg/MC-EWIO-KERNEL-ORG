/*********************************************************************************

 Copyright 2006-2008 MakingThings

 Licensed under the Apache License, 
 Version 2.0 (the "License"); you may not use this file except in compliance 
 with the License. You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0 
 
 Unless required by applicable law or agreed to in writing, software distributed
 under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied. See the License for
 the specific language governing permissions and limitations under the License.
*********************************************************************************/
#ifndef MCT_JSON_H
#define MCT_JSON_H

#include "mct_types.h"	// bool-Type

#define JSON_MAX_DEPTH		32
#define JSON_LIMIT_SIZE		2048	// max. Laenge einer JSON-Struktur

// state object for decoding
typedef enum {
  JSON_DECODE_OBJECT_START,
  JSON_DECODE_IN_OBJECT,
  JSON_DECODE_IN_ARRAY
} JsonDecode_Step;

typedef struct {
  JsonDecode_Step steps[JSON_MAX_DEPTH]; /**< An array to keep track of each step of the decoder. */
  int depth;                             /**< The current depth of the decoder (how many elements have been opened). */
  bool gotcomma;                         /**< Used internally by the decoder. */
  void* context;                         /**< A pointer to the user context. */
  char* p;                               /**< A pointer to the data. */
  int len;                               /**< The current length. */
} JsonDecode_State;

// Decode
void JsonDecode_Init(JsonDecode_State* state, void* context);
void JsonDecode_SetIntCallback(bool(*int_callback)(void *ctx, int val));
void JsonDecode_SetFloatCallback(bool(*float_callback)(void *ctx, float val));
void JsonDecode_SetBoolCallback(bool(*bool_callback)(void *ctx, bool val));
void JsonDecode_SetStringCallback(bool(*string_callback)(void *ctx, char *string, int len));
void JsonDecode_SetStartObjCallback(bool(*start_obj_callback)(void *ctx));
void JsonDecode_SetObjKeyCallback(bool(*obj_key_callback)(void *ctx, char *key, int len));
void JsonDecode_SetEndObjCallback(bool(*end_obj_callback)(void *ctx));
void JsonDecode_SetStartArrayCallback(bool(*start_array_callback)(void *ctx));
void JsonDecode_SetEndArrayCallback(bool(*end_array_callback)(void *ctx));
bool JsonDecode(JsonDecode_State* state, char* text, int len);

//*********************************************************************************************
// JsonExpand
//*********************************************************************************************
#define MAX_STACK_SIZE			JSON_MAX_DEPTH		// max. subkeys 
#define MAX_OBJECTS_IN_ARRAY	JSON_MAX_DEPTH		// max. elements in arrays
#define MAX_STRING_SIZE			PROPERTY_SIZE + 1
#define MAX_KEY_SIZE			PROPERTY_SIZE + 1

// supported node types 
typedef enum	{	
		nType_unused	= 0,	// standard none 
		nType_string,
		nType_array,
} nType;

// supported value types [string,object,int,bool] 
typedef enum	{	
		vType_str	= 0,		// standard string
		vType_obj,
		vType_int,
		vType_bool,
		vType_float,
} vType;

//supported value states
typedef enum	{	
		vState_unknown	= 0,	// Inhalt des Wertes ist bedeutungslos ... z.B. wegen Fehler, oder nicht gefunden
		vState_found	= 1,	// Ein Wert wurde über vollständigen Schlüsselvergleich mit dem Stack gefunden 
		vState_direct	= 1,	// Ein Wert wurde gefunden, wobei nicht unbedingt ein vollständiger Schlüssel-
								// vergleich ausgeführt wurde!!! z.B. Wenn ein Feldelement ein direkter int-Wert 
								// ist!
		vState_pick,			// Den nächsten Wert müssen wir einsammeln, weil der vollständige Schlüsselvergleich
} vState;						// erfolgt ist

// supported working-modes inside arrays
typedef enum	{	
		arr_mode_idle	= 0,	// standard
		arr_mode_start,			// array-started set on "["
		arr_mode_obj,			// array special: working inside objects 
		arr_mode_next,			// looking for next element inside array
		arr_mode_stop,			// array-started set on "]" same as idle!
} e_arr_mode;


typedef enum	{
	err_none,					// O.K.
								// Ab hier: STACK-REWIND
	err_stack_level = 1,		// Vorgänger Element auf dem Stack hat mindestens schon die Tiefe (depth)
								// des aktuellen Elements, das abgelegt werden sollte!
								// Wir brechen erst mal ab, da (STACK_REWIND) nicht implementiert ist!
	err_array_overflow,			// Der aktuelle Index im Array ist größer, als der von uns gesuchte Index
								// Wir brechen erst mal ab, da (STACK_REWIND) nicht implementiert ist!
								// Ab hier: FINAL_BREAKS
	err_stack_overflow,			// STACK_SIZE
	err_stack_bottom,			// < 0
	err_brack_push,
	err_brack_pop,
	err_obj_depth,				// Zu viele offene Objecte 
} e_err;

typedef struct error {				
	e_err			number;		// see above - error types  
	char		*	p;			// see JsonDecode_State < A pointer to the data. 
	int				len;		// see JsonDecode_State < The current length. 
} o_err;

// values [int, string, float, bool, object]
typedef union {
	struct	{
		vType			type;
		int				state;
	} v;
	struct 	{
		vType			type;
		int				state;
		unsigned char	value[MAX_STRING_SIZE];	//string
	} vS;
	struct	{
		vType			type;
		int				state;
		int				value;					//int
	}  vI;
	struct {
		vType			type;
		int				state;
		bool			value;					//boolean
	} vB;		
	struct 	{
		vType			type;
		int				state;
		float			value;					//float
	} vF;
	struct	{
		vType			type;
		int				state;
		unsigned char	value[MAX_STRING_SIZE];	// Object = {...}
	} vO;	
} u_val; 

// nodes [array,key]
typedef union {
	struct  {
		nType			type;
		int				mapp;
	} n;
	struct	{
		nType			type;
		int				mapp;
		unsigned char	node[MAX_KEY_SIZE];
	} nK;
	struct	{
		nType			type;
		int				mapp;
		int				node;		// Zielindex innerhalb des Arrays
									// array-spezifische Elemente ...
		int				_arrIdx;	// aktueller Array-Index
		vType			_arrType;	// aktueller Array-Type, kann im Arry durchaus wechseln [Int,String,Bool,Float,Object]
		int				_arrMode;	// verwaltet den internen Array-Modus
		int				_arrBracks; // verwaltet den Status von Objekten in einem Array über
	} nA;							// (die open/close Klammern zum ermitteln der Endekennung - balanced/unbalanced)	
} u_node;

// context 		 
typedef struct {
	JsonDecode_State *	state;
	u_val				val;					// value (union)
	u_node			*   _pstack;				// stackpointer
	u_node				_stack[MAX_STACK_SIZE];	// stack
	int					_stacki;				// stackindex
	o_err				err;					// error-object
} elivator;

void Context_Init(void* ctx, JsonDecode_State* state);
//void Context_Builder( void* ctx);
void Context_Build( void* ctx, u_node * template, int count);

#endif /* JSON_H */
