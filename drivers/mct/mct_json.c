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
#include <linux/module.h>	// function: strtol, strncmp
#include <linux/ctype.h>	// function: isdigit
#include "mct_json.h"

/****************************************************************************
 JsonDecode
****************************************************************************/
typedef enum {
    token_true,
    token_false,
    token_colon,
    token_comma,     
    token_eof,
    token_error,
    token_left_brace,     
    token_left_bracket,
    token_null,         
    token_right_brace,     
    token_right_bracket,
    token_number, 
    token_maybe_negative,
    token_string,
    token_unknown
} JsonDecode_Token;

struct {
  bool (*null_callback)(void*);
  bool (*bool_callback)(void*, bool);
  bool (*int_callback)(void*, int);
  bool (*string_callback)(void*, char*, int);
  bool (*start_obj_callback)(void*);
  bool (*end_obj_callback)(void*); 
  bool (*obj_key_callback)(void*, char*, int);
  bool (*start_array_callback)(void*);
  bool (*end_array_callback)(void*);
} JsonDecode_Callbacks;

static JsonDecode_Token JsonDecode_GetToken(char* text, int len);
void JsonDecode_SetIntCallback(bool(*int_callback)(void *ctx, int val)) {
  JsonDecode_Callbacks.int_callback = int_callback;
}
void JsonDecode_SetBoolCallback(bool(*bool_callback)(void *ctx, bool val))	{
  JsonDecode_Callbacks.bool_callback = bool_callback;
}
void JsonDecode_SetStringCallback(bool(*string_callback)(void *ctx, char *string, int len)) {
  JsonDecode_Callbacks.string_callback = string_callback;
}
void JsonDecode_SetStartObjCallback(bool(*start_obj_callback)(void *ctx))	{
  JsonDecode_Callbacks.start_obj_callback = start_obj_callback;
}
void JsonDecode_SetObjKeyCallback(bool(*obj_key_callback)(void *ctx, char *key, int len))	{
  JsonDecode_Callbacks.obj_key_callback = obj_key_callback;
}
void JsonDecode_SetEndObjCallback(bool(*end_obj_callback)(void *ctx))	{
  JsonDecode_Callbacks.end_obj_callback = end_obj_callback;
}
void JsonDecode_SetStartArrayCallback(bool(*start_array_callback)(void *ctx))	{
  JsonDecode_Callbacks.start_array_callback = start_array_callback;
}
void JsonDecode_SetEndArrayCallback(bool(*end_array_callback)(void *ctx))	{
  JsonDecode_Callbacks.end_array_callback = end_array_callback;
}

void JsonDecode_Init(JsonDecode_State* state, void* context)	{
  state->depth = 0;
  state->gotcomma = false;
  state->context = context;
  state->p = 0;
  state->len = 0;
}

bool JsonDecode(JsonDecode_State* state, char* text, int len)	{
  JsonDecode_Token token;
  bool keepgoing = true;
  bool gotdecimal = false;
  bool objkey = false;
  int size;
  // if these haven't been initialized, do it
  if(!state->p) 
    state->p = text;
  if(!state->len)
    state->len = len;
  while(state->len)	{
    while((*state->p == ' ') || 
		  (*state->p == '\n') || 
		  (*state->p == '\t')) //HACK launch white space, newline or tab
      state->p++;
    token = JsonDecode_GetToken( state->p, state->len);
    switch(token)
    {
      case token_true:
        if(JsonDecode_Callbacks.bool_callback)	{
          if(!JsonDecode_Callbacks.bool_callback(state->context, true))
            return false;
        }
        state->p += 4;
        state->len -= 4;
        break;
      case token_false:
        if(JsonDecode_Callbacks.bool_callback)	{
          if(!JsonDecode_Callbacks.bool_callback(state->context, false))
            return false;
        }
        state->p += 5;
        state->len -= 5;
        break;
      case token_null:
        if(JsonDecode_Callbacks.null_callback)	{
          if(!JsonDecode_Callbacks.null_callback(state->context))
            return false;
        }
        state->p += 4;
        state->len -= 4;
        break;
      case token_comma:
        state->gotcomma = true;
        // intentional fall-through
      case token_colon: // just get the next one      
        state->p++;
        state->len--;
        break;
      case token_left_bracket:
        state->steps[++state->depth] = JSON_DECODE_OBJECT_START;
        if(JsonDecode_Callbacks.start_obj_callback)	{
          if(!JsonDecode_Callbacks.start_obj_callback(state->context))
            return false;
        }
        state->p++;
        state->len--;
        break;
      case token_right_bracket:
        state->depth--;
        if(JsonDecode_Callbacks.end_obj_callback)	{
          if(!JsonDecode_Callbacks.end_obj_callback(state->context))
            return false;
        }
        state->p++;
        state->len--;
        break;
      case token_left_brace:
        state->steps[++state->depth] = JSON_DECODE_IN_ARRAY;
        if(JsonDecode_Callbacks.start_array_callback)	{
          if(!JsonDecode_Callbacks.start_array_callback(state->context))
            return false;
        }
        state->p++;
        state->len--;
        break;
      case token_right_brace:
        state->depth--;
        if(JsonDecode_Callbacks.end_array_callback)	{
          if(!JsonDecode_Callbacks.end_array_callback(state->context))
            return false;
        }
        state->p++;
        state->len--;
        break;
      case token_number:
      {
        const char* p = state->p;
        keepgoing = true;
        gotdecimal = false;
        do	{
          if(*p == '.')	{
            if(gotdecimal) // we only expect to get one decimal place in a number
              return false;
            gotdecimal = true;
            p++;
          }
          else if(!isdigit(*p))
            keepgoing = false;
          else
            p++;
        } while(keepgoing);
        size = p - state->p;
        if(JsonDecode_Callbacks.int_callback)	{
 //HACK	    if(!JsonDecode_Callbacks.int_callback(state->context, atoi(state->p)))
 		   	if(!JsonDecode_Callbacks.int_callback(state->context, simple_strtol(state->p, NULL, 10)))
	          return false;
        }
        state->p += size;
        state->len -= size;
        break;
      }
      case token_string:
      {
        char* p = ++state->p; // move past the opening "
        state->len--; 
        keepgoing = true;
		while(keepgoing)	{
          if(*p == '\\') // we got an escape - skip the escape and the next character
            p += 2;
          else if(*p == '"') // we got the end of a string
            keepgoing = false;
          else
            p++; // keep looking for the end of the string
        }
        size = p - state->p;
		*p = 0; // replace the trailing " with a null to make a string
        // figure out if this is a key or a normal string
        objkey = false;
		if(state->steps[state->depth] == JSON_DECODE_OBJECT_START)	{
          state->steps[state->depth] = JSON_DECODE_IN_OBJECT;
          objkey = true;
        }
        if(state->gotcomma && state->steps[state->depth] == JSON_DECODE_IN_OBJECT)	{
          state->gotcomma = false;
          objkey = true;
        }
        if(objkey) {	// last one was a comma - next string has to be a key
          if(JsonDecode_Callbacks.obj_key_callback)	{
            if(!JsonDecode_Callbacks.obj_key_callback(state->context, state->p, size))
              return false;
          }
        }
        else 	{	// just a normal string
          if(JsonDecode_Callbacks.string_callback)	{
            if(!JsonDecode_Callbacks.string_callback(state->context, state->p, size))
              return false;
          }
        }
        state->p += (size+1); // account for the trailing "
        state->len -= (size+1);
        break;
      }
      case token_eof: // we don't expect to get this, since our len should run out before we get eof
        return false;
      default:
        return false;
    }
  }
  return true;
}

// static
JsonDecode_Token JsonDecode_GetToken(char* text, int len)	{
  char c = text[0];
  switch(c)	{
    case ':':
      return token_colon;
    case ',':
      return token_comma;
    case '{':
      return token_left_bracket;
    case '}':
      return token_right_bracket;
    case '[':
      return token_left_brace;
    case ']':
      return token_right_brace;
    case '"':
      return token_string;
    case '0': case '1': case '2': case '3': case '4': 
    case '5': case '6': case '7': case '8': case '9':
      return token_number;
    case '-':
      return token_maybe_negative;
    case 't':	{
      if(len < 4) // not enough space;
        return token_unknown;
      if(!strncmp(text, "true", 4))
        return token_true;
      else 
        return token_unknown;
    }
    case 'f':	{
      if(len < 5) // not enough space;
        return token_unknown;
      if(!strncmp(text, "false", 5))
        return token_false;
      else
        return token_unknown;
    }
    case 'n':	{
      if(len < 4) // not enough space;
        return token_unknown;
      if(!strncmp(text, "null", 4))
        return token_null;
      else
        return token_unknown;
    }
    case '\0':
      return token_eof;
    default:
      return token_unknown;
  }
}

/****************************************************************************
 JsonContext

- Der Schalter CONTEXT_DEBUG bestimmt ob bei DEBUG auch alle json Aktivitäten
  angezeigt werden sollen. 
- Die Ausgaben sind dabei sehr umfangreich und nur notwendig, um die 
 Funktionsweise des Parsers zu testen.

- trace_ctx_call()	= trace all function-calls
- trace_ctx_track()	= tracking and printing the JSON structure
- trace_ctx_array()	= tracking and printing array elements	 
- trace_ctx_error()	= printing errors  

****************************************************************************/
#define BUILD_CONTEXT(name)	elivator * name = (elivator*) ctx


#undef	CONTEXT_DEBUG
//#define	CONTEXT_DEBUG

#ifdef CONTEXT_DEBUG
	#define trace_ctx_call() 					printk("%s\n",  __FUNCTION__)
	#define trace_ctx_track(format, arg...)		printk(format,  ## arg)
	#define trace_ctx_array(format, arg...)		printk("array-element: " format,  ## arg)
	#define trace_ctx_compare(format, arg...)	printk("compare: " format,  ## arg)
	#define trace_ctx_error(format, arg...) 	printk("%s: error: " format, __FUNCTION__, ## arg)
#else
	#define trace_ctx_call() 
	#define trace_ctx_track(format, arg...)
	#define trace_ctx_array(format, arg...)
	#define trace_ctx_compare(format, arg...)
	#define trace_ctx_error(format, arg...)	
#endif

//*************************
// _STACK functions
//*************************
void _stack_reset( elivator * elv) {
	trace_ctx_call();
	elv->_pstack = &elv->_stack[0];
	elv->_stacki = 0;
	elv->_stack[elv->_stacki].n.mapp = 0;
	memset(&elv->err,0,sizeof(o_err));		// clean error object
	memset(&elv->val,0,sizeof(u_val));		// clean value unit	
}
// false on Stackoverflow ++ Action: FINAL_BREAK
bool _stack_push( elivator * elv)	{
	trace_ctx_call();
	if(elv->_stacki < MAX_STACK_SIZE)	{
		elv->_pstack++;	
		elv->_stacki++;
		elv->_stack[elv->_stacki].n.mapp = 0;
	}
	else	{
		elv->err.number = err_stack_overflow;
		elv->err.len = elv->state->len;
		elv->err.p = elv->state->p;
		trace_ctx_error("%s code: %d\n","stack_overflow++",elv->err.number);
		return false;
	}
	return true;
}
// false on stackground -- Action: FINAL_BREAK (err_stack_bottom)
// false on stacklevel  -- Action: FINAL_BREAK (err_stack_level)
bool _stack_pop( elivator * elv)	{
	trace_ctx_call();
	if(elv->_stacki > 0)	{
		elv->_pstack--;	
		elv->_stacki--;
		elv->_stack[elv->_stacki].n.mapp = 0;
		if(elv->_stack[elv->_stacki-1].n.mapp >= elv->state->depth) {
			elv->err.number = err_stack_level;
			elv->err.len = elv->state->len;
			elv->err.p = elv->state->p;
			trace_ctx_error("%s code: %d\n","stack_level++",elv->err.number);
			return false;
		}
		elv->_stack[elv->_stacki].n.mapp = elv->state->depth;	// copy actual depth
	}
	else	{
		elv->err.number = err_stack_bottom;
		elv->err.len = elv->state->len;
		elv->err.p = elv->state->p;
		trace_ctx_error("%s code: %d\n","stack_bottom--",elv->err.number);
		return false;
	}
	return true;
}
// Placeholder for future
// true		= succsess
// false	= stack corruption!
// Wenn wir einen Fehler erhalten, dann ist unser Array-Index auf dem Stack
// bereits kleiner als der aktuell erkannte Array-Index. Wir müssen den Stack-
// pointer um 2 Positionen erniedrigen (SPn). 
// Beispielstack für:   oDevice.oDriver[2].oValue.Value	
//			oDevice	<--- SPn
//			oDriver
//	SPe --->2
//			oValue
//			Value
// Wir haben somit die Möglichkeit weitere Arraystrukturen zu analysieren! 
int _stack_rewind( elivator * elv)	{	
	trace_ctx_call();
	trace_ctx_error("%s\n","Not implemented yet!");
	if(elv->_pstack->n.type == nType_array)	{
	}
	return false;
}

//*************************
// __LOW_LEVEL functions
//*************************
// Used ever on actual _stackposion! Please adjust _stackposition first!  
#define _STACK_ARR_SET_MODE(ctx,mode)	(ctx)->_stack[(ctx)->_stacki].nA._arrMode	= mode;
#define _STACK_ARR_GET_MODE(ctx)		(ctx)->_stack[(ctx)->_stacki].nA._arrMode
#define _STACK_ARR_SET_TYPE(ctx,type)	(ctx)->_stack[(ctx)->_stacki].nA._arrType	= type;	
#define _STACK_ARR_GET_TYPE(ctx)		(ctx)->_stack[(ctx)->_stacki].nA._arrType	
// return:	false = too much open brackets  Action: FINAL-BREAK
//			true = succsess
bool  __brack_push(elivator * elv) {
	trace_ctx_call();
	if(elv->_stack[elv->_stacki].nA._arrBracks >= MAX_OBJECTS_IN_ARRAY ) {
		elv->err.number = err_brack_push;		
		elv->err.len = elv->state->len;
		elv->err.p = elv->state->p;
		trace_ctx_error("%s code: %d\n","To much open brackets!",elv->err.number);	
		return false;
	}
	elv->_stack[elv->_stacki].nA._arrBracks++;
	return true;
}
// return: -1 while unbalanced brackets				Action: FINAL-BREAK
//			0 = {} balanced brackets - end of object
//			1...x  rest of open brackets
int __brack_pop(elivator * elv) {
	trace_ctx_call();
	if(elv->_stack[elv->_stacki].nA._arrBracks <= 0) {
		elv->err.number = err_brack_pop;
		elv->err.len = elv->state->len;
		elv->err.p = elv->state->p;
		trace_ctx_error("%s code: %d\n","Unbalanced count of brackets!",elv->err.number);	
		return -1;
	}
	elv->_stack[elv->_stacki].nA._arrBracks--;
//	printk("_brack_pop: %d\n",elv->_stack[elv->_stacki].nA._arrBracks);
	return elv->_stack[elv->_stacki].nA._arrBracks;
}
// reset array-index
void __elem_reset(elivator * elv)	{
	trace_ctx_call();
	elv->_stack[elv->_stacki].nA._arrIdx = 0;
}
// true = array-index smaller or equal
// false = array-index overflow (ACTION: STACK_REWIND)
int __elem_push (elivator * elv) {
	trace_ctx_call();
	elv->_stack[elv->_stacki].nA._arrIdx++;
	if(elv->_stack[elv->_stacki].nA._arrIdx <= elv->_stack[elv->_stacki].nA.node)	{
		trace_ctx_array(" %d\n", elv->_stack[elv->_stacki].nA._arrIdx);
		return true;
	}
	elv->err.number = err_array_overflow;
	elv->err.len = elv->state->len;
	elv->err.p = elv->state->p;
	trace_ctx_error("%s code: %d\n","To much array elements!", elv->err.number);
	return false;
}
// true = element gefunden 
// false = aktuelles element passt nicht
int __elem_found (elivator * elv) {
	trace_ctx_call();
	if(elv->_stack[elv->_stacki].nA._arrIdx == elv->_stack[elv->_stacki].nA.node)	{
		trace_ctx_array("%d found!\n", elv->_stack[elv->_stacki].nA._arrIdx);
//		trace_ctx_compare("%d array position found!\n", elv->_stack[elv->_stacki].nA._arrIdx);
		return true;
	} 
	return false;
}

//*******************
// CALL-BACK-Functions
//*******************
bool obj_start(void *ctx)	{ 
	BUILD_CONTEXT(elv);
	trace_ctx_call();
	trace_ctx_track("%s\n","{");
//	printk("*** waterlevel: %d\n", elv->state->depth);
	switch(_STACK_ARR_GET_MODE(elv))	{
		case arr_mode_start:						// 1. element inside array
			_STACK_ARR_SET_TYPE(elv, vType_obj);	// element type is an object inside array
			_STACK_ARR_SET_MODE(elv,arr_mode_obj);	// we are walking
			__elem_reset(elv);						// reset elementcounter
			if(__elem_found(elv) == true)	{
				return(_stack_push(elv));			// true / false on  		
				break;
			}
			return(__brack_push(elv));
			break;
		case arr_mode_next:							// X. element inside array
			_STACK_ARR_SET_TYPE(elv, vType_obj);	// element type is an object inside array
			_STACK_ARR_SET_MODE(elv,arr_mode_obj);	// we are walking
			if(__elem_push(elv) == false)	{
				// ToDo: Handle Array-Index-Overflow 
				//		(STACK_REWIND)
				return _stack_rewind(elv);
				break;
			}
			if(__elem_found(elv) == true)	{
				return(_stack_push(elv));			// true / false on  		
				break;
			}
			return(__brack_push(elv));
			break;
		case arr_mode_obj:
			return(__brack_push(elv));				// object_mode we have to use additional __brack stack
			break;
		default:
			break;
	}
	return true;
}

bool obj_stop(void *ctx)	{ 
	BUILD_CONTEXT(elv);
	trace_ctx_call();
	trace_ctx_track("%s\n","}");
	if((_STACK_ARR_GET_MODE(elv) ==  arr_mode_obj) ||
		(_STACK_ARR_GET_MODE(elv) ==  arr_mode_stop)) {	// Hack: 08.06.2009 or array inside object 
		switch(__brack_pop(elv)) {		
			case 0 :									// object complete closed - balanced brackets!
				_STACK_ARR_SET_MODE(elv,arr_mode_next);	// next element ... what ever
				break;
			case -1:									// object unbalanced brackets - 	
				return false;							
				break;
			default:
				break;
		}
	}
	return true;
}

bool arr_start(void *ctx)	{ 
	BUILD_CONTEXT(elv);
	trace_ctx_call();
	trace_ctx_track("%s\n","[");
	_STACK_ARR_SET_MODE(elv,arr_mode_start);	// go into array
	return true;
}

bool arr_stop(void *ctx)	{
	BUILD_CONTEXT(elv);
	trace_ctx_call();
	trace_ctx_track("%s\n","]");
	_STACK_ARR_SET_MODE(elv,arr_mode_stop);
	return true;
}

bool key_pass(void *ctx, char *string, int len) {
	BUILD_CONTEXT(elv);
	trace_ctx_call();
	trace_ctx_track("Key: %s\n",string);
	// key compare
	if(!(memcmp(string, elv->_pstack->nK.node, sizeof(string)))){	
//		trace_ctx_compare("%s found!\n",string);
		if(_stack_push(elv) == true)	{
			if(elv->_pstack->n.type == nType_unused)	{	// Ende?
				_stack_pop(elv);
				elv->val.v.state = vState_pick;				// Wert einsammeln, vollstädiger Stackvergleich
				trace_ctx_compare(" Pick the value!\n");	// ist erfolgt!
			}
		}
		// Auswertung!	
		else {
			_stack_reset(elv);
		}
	}	
	return true;
}

bool val_bool(void *ctx,bool b_val) {
	BUILD_CONTEXT(elv);	
	trace_ctx_call();
	trace_ctx_track("bool: %d\n",b_val);
	if(elv->val.v.state == vState_pick)	{
		trace_ctx_compare("found: %d\n",b_val);
		elv->val.v.type = vType_bool;				// set value type_bool	
		elv->val.v.state = vState_found;			// say: value found
		elv->val.vB.value = b_val;					// copy value
		return false;								// VALUE 	
	}
	// ToDo: Here for array-elements as boolean support !
	return true;
}

bool val_int(void *ctx, int i_val) {
	BUILD_CONTEXT(elv);
	trace_ctx_call();
	trace_ctx_track("int: %i\n",i_val);
	if(elv->val.v.state == vState_pick)	{
		trace_ctx_compare("found: %i\n",i_val);
		elv->val.v.type = vType_int;				// set value type integer	
		elv->val.v.state = vState_found;			// say: value found
		elv->val.vI.value = i_val;					// copy value
		return false;								// VALUE 	
	}
	switch(_STACK_ARR_GET_MODE(elv))	{			// *** inside [array]
		case arr_mode_start:
			_STACK_ARR_SET_TYPE(elv, vType_int);	// 1. element type is an integer inside array
			_STACK_ARR_SET_MODE(elv,arr_mode_next);	// we are walking
			__elem_reset(elv);						// elements reset 
			if(	__elem_found(elv) == true)	{		// element index catched 
				elv->val.v.type = vType_int;		// set value type integer	
				elv->val.v.state = vState_direct;	// say: value direct found
				elv->val.vI.value = i_val;			// copy value
				return false;						// VALUE
				break;
			}
			break;
		case arr_mode_next:
			_STACK_ARR_SET_TYPE(elv, vType_int);	// X. element type is an integer inside array
			_STACK_ARR_SET_MODE(elv,arr_mode_next);	// we are walking
			if(__elem_push(elv) == false)	{
				// ToDo: Handle Array-Index-Overflow 
				//		 (STACK_REWIND)
				return _stack_rewind(elv);
				break;
			}
			if(	__elem_found(elv) == true)	{		// element index catched 
				elv->val.v.type = vType_int;		// set value type integer	
				elv->val.v.state = vState_direct;	// say: value direct found
				elv->val.vI.value = i_val;			// copy value
				return false;						// VALUE
				break;
			}
			break;
		default:
			break;
	}
	return true;
}

bool val_string(void *ctx, char * s_val, int len) {
	BUILD_CONTEXT(elv);
	trace_ctx_call();
	trace_ctx_track("string: %s\n",s_val);
	if(elv->val.v.state == vState_pick)	{
		trace_ctx_compare("found: %s\n",s_val);
		elv->val.v.type = vType_str;		// set value type string
		elv->val.v.state = vState_found;	// say: value found
		strncpy(elv->val.vS.value,s_val, MAX_STRING_SIZE);
		elv->val.vS.value[MAX_STRING_SIZE-1] = 0;	// terminate string
		return false;						
	}
	// ToDo: Here for array-elements as string support !
	return true;
}

//*******************
// CONTEXT-Functions
//*******************
void Context_Init(  void* ctx, JsonDecode_State* state) {
	BUILD_CONTEXT(elv);
	trace_ctx_call();

	// JsonCallbacks	
	JsonDecode_SetStartObjCallback(obj_start);		// {
	JsonDecode_SetEndObjCallback(obj_stop);			// }
	JsonDecode_SetStartArrayCallback(arr_start);	// [
	JsonDecode_SetEndArrayCallback(arr_stop);		// ]
	JsonDecode_SetObjKeyCallback(key_pass);			// KEY
	JsonDecode_SetBoolCallback(val_bool);			// bool
	JsonDecode_SetIntCallback(val_int);				// int
//	JsonDecode_SetFloatCallback(val_float);			// float
	JsonDecode_SetStringCallback(val_string);		// string
	memset(elv,0,sizeof(elivator));					// clean ctx
	elv->state	= state;							// crossover ctx <-> state
	memset(&elv->val, 0, sizeof(u_val));	
	memset(&elv->_stack, 0, sizeof(u_node[MAX_STACK_SIZE]));
}

void Context_Build( void* ctx, u_node * template, int count)	{
	BUILD_CONTEXT(elv);
	trace_ctx_call(); 	
	memcpy(&elv->_stack, template, count);
   _stack_reset(elv);		// Init Stackpointer
}
