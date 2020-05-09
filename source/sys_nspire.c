/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// sys_null.h -- null system driver to aid porting efforts

#ifndef WIN32FORNSPIRE
#include <os.h>
//#include <nspireio2.h>
#include <keys.h>
#endif

#include "quakedef.h"
#include "errno.h"

void Sys_NSpireInput( void );

qboolean			isDedicated = false;

void *p_nspire_stack_redirect = NULL;
void *p_nspire_membase = NULL;
/*
===============================================================================

FILE IO

===============================================================================
*/

#define MAX_HANDLES             10
FILE    *sys_handles[MAX_HANDLES];

int             findhandle (void)
{
	int             i;
	
	for (i=1 ; i<MAX_HANDLES ; i++)
		if (!sys_handles[i])
			return i;
	Sys_Error ("out of handles");
	return -1;
}

/*
================
filelength
================
*/
int filelength (FILE *f)
{
	int             pos;
	int             end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

int Sys_FileOpenRead (char *path, int *hndl)
{
	FILE    *f;
	int             i;
	
	i = findhandle ();

	f = fopen(path, "rb");
	if (!f)
	{
		*hndl = -1;
		return -1;
	}
	sys_handles[i] = f;
	*hndl = i;
	
	return filelength(f);
}

int Sys_FileOpenWrite (char *path)
{
	FILE    *f;
	int             i;
	
	i = findhandle ();

	f = fopen(path, "wb");
	if (!f)
		Sys_Error ("Error opening %s: %s", path,strerror(errno));
	sys_handles[i] = f;
	
	return i;
}

void Sys_FileClose (int handle)
{
	fclose (sys_handles[handle]);
	sys_handles[handle] = NULL;
#if FORNSPIRE
	/*bkpt();*/
#endif
}

void Sys_FileSeek (int handle, int position)
{
	fseek (sys_handles[handle], position, SEEK_SET);
}

int Sys_FileRead (int handle, void *dest, int count)
{
	return fread (dest, 1, count, sys_handles[handle]);
}

int Sys_FileWrite (int handle, void *data, int count)
{
	return fwrite (data, 1, count, sys_handles[handle]);
}

int     Sys_FileTime (char *path)
{
	FILE    *f;
	
	f = fopen(path, "rb");
	if (f)
	{
		fclose(f);
		return 1;
	}
	
	return -1;
}

void Sys_mkdir (char *path)
{
}


/*
===============================================================================

SYSTEM IO

===============================================================================
*/

int b_valid_timer = 0;
unsigned int ui32_nspire_cx_timer_load_value;
unsigned long long ui64_nspire_cx_timer_start_value;
unsigned long long ui64_nspire_cx_timer_current_value;
unsigned int ui32_last_nspire_cx_timer_value;

#define NSPIRE_CX_TIMER_LOAD_VALUE (*(volatile unsigned int * )0x900D0000)
#define NSPIRE_CX_TIMER_VALUE (*(volatile unsigned int * )0x900D0004)

void Sys_Init( void )
{
#if FORNSPIRE
	if( has_colors )
	{
		ui32_nspire_cx_timer_load_value = NSPIRE_CX_TIMER_LOAD_VALUE;
		b_valid_timer = 1;

		NSPIRE_CX_TIMER_LOAD_VALUE = 0xffffffff;
		ui32_last_nspire_cx_timer_value = NSPIRE_CX_TIMER_VALUE;
		ui64_nspire_cx_timer_start_value = ui64_nspire_cx_timer_current_value = ui32_last_nspire_cx_timer_value;
	}
#endif
}


void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
}


void Sys_Error (char *error, ...)
{
#ifndef WIN32FORNSPIRE
//	nio_console csl;
#endif
	va_list         argptr;
	char rgc_errorstr[ 256 ];

	va_start (argptr,error);
	vsprintf (rgc_errorstr, error,argptr);
	va_end (argptr);

#ifndef WIN32FORNSPIRE
//	nio_InitConsole(&csl, 53, 29, 0, 0, 0, 15);
//	nio_DrawConsole(&csl);
//	nio_printf(&csl, "Sys Error:\n");
//	nio_printf(&csl, rgc_errorstr);
//	nio_printf(&csl, "\n");
//	wait_key_pressed();
//	nio_CleanUp(&csl);
#else
	printf("Sys Error:\n");
	printf( rgc_errorstr );
	printf("\n");
#endif

	exit (1);
}

void Sys_Printf (char *fmt, ...)
{
/*	va_list         argptr;
	
	va_start (argptr,fmt);
	vprintf (fmt,argptr);
	va_end (argptr); */
}

void Sys_Quit (void)
{
	int i;
#if FORNSPIRE
	if( has_colors && b_valid_timer )
	{
		NSPIRE_CX_TIMER_LOAD_VALUE = ui32_nspire_cx_timer_load_value;
		b_valid_timer = 0;
	}
#endif

	Host_Shutdown();

	for( i = 0; i < MAX_HANDLES; i++ )
	{
		if( sys_handles[ i ] )
		{
			fclose( sys_handles[ i ] );
			sys_handles[ i ] = NULL;
		}
	}
	if( p_nspire_membase )
	{
		free( p_nspire_membase );
	}
	if( p_nspire_stack_redirect )
	{
		free( p_nspire_stack_redirect );
	}
	exit (0);
}

double Sys_FloatTime (void)
{
	static double t;

#if FORNSPIRE
	if( has_colors && b_valid_timer )
	{
		unsigned int ui_timer_value, ui_delta;
		ui_timer_value = NSPIRE_CX_TIMER_VALUE;
		ui_delta = ui32_last_nspire_cx_timer_value - ui_timer_value;
		/*printf( "timer: %u %u %u\n", ui_delta, ui_timer_value, ui32_last_nspire_cx_timer_value );*/
		ui32_last_nspire_cx_timer_value = ui_timer_value;
		ui64_nspire_cx_timer_current_value += ui_delta;
		t += ( ( float )ui_delta ) / 2048.0;
		/*printf( "timer2: %f\n", ( float )t );*/
	}
	else
	{
		t += 0.05;
	}
#else
	t += 0.05;
#endif
	
	return t;
}

char *Sys_ConsoleInput (void)
{
	return NULL;
}

void Sys_Sleep (void)
{
}

void Sys_SendKeyEvents (void)
{
#ifdef FORNSPIRE
	Sys_NSpireInput();
#endif
}

void Sys_HighFPPrecision (void)
{
}

void Sys_LowFPPrecision (void)
{
}


#if 0
#define KEY_NSPIRE_RET          KEY_(0x10, 0x001)
#define KEY_NSPIRE_ENTER        KEY_(0x10, 0x002)
#define KEY_NSPIRE_SPACE        KEYTPAD_(0x10, 0x004, 0x10, 0x10)
#define KEY_NSPIRE_NEGATIVE     KEY_(0x10, 0x008)
#define KEY_NSPIRE_Z            KEYTPAD_(0x10, 0x010, 0x10, 0x20)
#define KEY_NSPIRE_PERIOD       KEYTPAD_(0x10, 0x20, 0x1A, 0x010)
#define KEY_NSPIRE_Y            KEY_(0x10, 0x040)
#define KEY_NSPIRE_0            KEY_(0x10, 0x080)
#define KEY_NSPIRE_X            KEYTPAD_(0x10, 0x100, 0x12, 0x001)
#define KEY_NSPIRE_THETA        KEYTPAD_(0x10, 0x400, _KEY_DUMMY_ROW, _KEY_DUMMY_COL)
#define KEY_NSPIRE_COMMA        KEYTPAD_(0x12, 0x001, 0x1E, 0x400)
#define KEY_NSPIRE_PLUS         KEYTPAD_(0x12, 0x002, 0x1C, 0x004)
#define KEY_NSPIRE_W            KEYTPAD_(0x12, 0x004, 0x12, 0x002)
#define KEY_NSPIRE_3            KEY_(0x12, 0x008)
#define KEY_NSPIRE_V            KEYTPAD_(0x12, 0x010, 0x12, 0x004)
#define KEY_NSPIRE_2            KEYTPAD_(0x12, 0x020, 0x1C, 0x010)
#define KEY_NSPIRE_U            KEYTPAD_(0x12, 0x040, 0x12, 0x010)
#define KEY_NSPIRE_1            KEY_(0x12, 0x080)
#define KEY_NSPIRE_T            KEYTPAD_(0x12, 0x100, 0x12, 0x020)
#define KEY_NSPIRE_eEXP         KEYTPAD_(0x12, 0x200, 0x16, 0x200)
#define KEY_NSPIRE_PI           KEYTPAD_(0x12, 0x400, 0x12, 0x100)
#define KEY_NSPIRE_QUES         KEYTPAD_(0x14, 0x001, _KEY_DUMMY_ROW, _KEY_DUMMY_COL)
#define KEY_NSPIRE_QUESEXCL     KEYTPAD_(_KEY_DUMMY_ROW, _KEY_DUMMY_COL, 0x10, 0x100)
#define KEY_NSPIRE_MINUS        KEYTPAD_(0x14, 0x002, 0x1A, 0x004)
#define KEY_NSPIRE_S            KEYTPAD_(0x14, 0x004, 0x12, 0x040)
#define KEY_NSPIRE_6            KEY_(0x14, 0x008)
#define KEY_NSPIRE_R            KEYTPAD_(0x14, 0x010, 0x14, 0x001)
#define KEY_NSPIRE_5            KEYTPAD_(0x14, 0x020, 0x1A, 0x040)
#define KEY_NSPIRE_Q            KEYTPAD_(0x14, 0x040, 0x14, 0x002)
#define KEY_NSPIRE_4            KEY_(0x14, 0x080)
#define KEY_NSPIRE_P            KEYTPAD_(0x14, 0x100, 0x14, 0x004)
#define KEY_NSPIRE_TENX         KEYTPAD_(0x14, 0x200, 0x12, 0x400)
#define KEY_NSPIRE_EE           KEYTPAD_(0x14, 0x400, 0x14, 0x100)
#define KEY_NSPIRE_COLON        KEYTPAD_(0x16, 0x001, _KEY_DUMMY_ROW, _KEY_DUMMY_COL)
#define KEY_NSPIRE_MULTIPLY     KEYTPAD_(0x16, 0x002, 0x18, 0x100)
#define KEY_NSPIRE_O            KEYTPAD_(0x16, 0x004, 0x14, 0x010)
#define KEY_NSPIRE_9            KEY_(0x16, 0x008)
#define KEY_NSPIRE_N            KEYTPAD_(0x16, 0x010, 0x14, 0x020)
#define KEY_NSPIRE_8            KEYTPAD_(0x16, 0x020, 0x1C, 0x040)
#define KEY_NSPIRE_M            KEYTPAD_(0x16, 0x040, 0x14, 0x040)
#define KEY_NSPIRE_7            KEY_(0x16, 0x080)
#define KEY_NSPIRE_L            KEYTPAD_(0x16, 0x100, 0x16, 0x001)
#define KEY_NSPIRE_SQU          KEYTPAD_(0x16, 0x200, 0x14, 0x200)
#define KEY_NSPIRE_II           KEYTPAD_(0x16, 0x400, _KEY_DUMMY_ROW, _KEY_DUMMY_COL)
#define KEY_NSPIRE_QUOTE        KEYTPAD_(0x18, 0x001, _KEY_DUMMY_ROW, _KEY_DUMMY_COL)
#define KEY_NSPIRE_DIVIDE       KEYTPAD_(0x18, 0x002, 0x16, 0x100)
#define KEY_NSPIRE_K            KEYTPAD_(0x18, 0x004, 0x16, 0x002)
#define KEY_NSPIRE_TAN          KEY_(0x18, 0x008)
#define KEY_NSPIRE_J            KEYTPAD_(0x18, 0x010, 0x16, 0x004)
#define KEY_NSPIRE_COS          KEYTPAD_(0x18, 0x020, _KEY_DUMMY_ROW, _KEY_DUMMY_COL)
#define KEY_NSPIRE_I            KEYTPAD_(0x18, 0x040, 0x16, 0x010)
#define KEY_NSPIRE_SIN          KEYTPAD_(0x18, 0x080, _KEY_DUMMY_ROW, _KEY_DUMMY_COL)
#define KEY_NSPIRE_H            KEYTPAD_(0x18, 0x100, 0x16, 0x020)
#define KEY_NSPIRE_EXP          KEYTPAD_(0x18, 0x200, 0x18, 0x200)
#define KEY_NSPIRE_GTHAN        KEYTPAD_(0x18, 0x400, _KEY_DUMMY_ROW, _KEY_DUMMY_COL)
#define KEY_NSPIRE_APOSTROPHE   KEY_(0x1A, 0x001)
#define KEY_NSPIRE_CAT          KEYTPAD_(0x1A, 0x002, 0x1A, 0x080)
#define KEY_NSPIRE_FRAC         KEYTPAD_(_KEY_DUMMY_ROW, _KEY_DUMMY_COL, 0x1A, 0x100)
#define KEY_NSPIRE_G            KEYTPAD_(0x1A, 0x004, 0x16, 0x040)
#define KEY_NSPIRE_RP           KEYTPAD_(0x1A, 0x008, 0x1A, 0x008)
#define KEY_NSPIRE_F            KEYTPAD_(0x1A, 0x010, 0x18, 0x001)
#define KEY_NSPIRE_LP           KEYTPAD_(0x1A, 0x020, 0x1A, 0x020)
#define KEY_NSPIRE_E            KEYTPAD_(0x1A, 0x040, 0x18, 0x002)
#define KEY_NSPIRE_VAR          KEYTPAD_(0x1A, 0x080, 0x1A, 0x002)
#define KEY_NSPIRE_D            KEYTPAD_(0x1A, 0x100, 0x18, 0x004)
#define KEY_NSPIRE_DEL          KEYTPAD_(0x1E, 0x100, 0x1A, 0x200)
#define KEY_NSPIRE_LTHAN        KEYTPAD_(0x1A, 0x400, _KEY_DUMMY_ROW, _KEY_DUMMY_COL)
#define KEY_NSPIRE_FLAG         KEY_(0x1C, 0x001)
#define KEY_NSPIRE_CLICK        KEYTPAD_ARROW_(0x1C, 0x002, TPAD_ARROW_CLICK)
#define KEY_NSPIRE_C            KEYTPAD_(0x1C, 0x004, 0x18, 0x010)
#define KEY_NSPIRE_HOME         KEYTPAD_(0x1C, 0x008, _KEY_DUMMY_ROW, _KEY_DUMMY_COL)
#define KEY_NSPIRE_B            KEYTPAD_(0x1C, 0x010, 0x18, 0x020)
#define KEY_NSPIRE_MENU         KEY_(0x1C, 0x020)
#define KEY_NSPIRE_A            KEYTPAD_(0x1C, 0x040, 0x18, 0x040)
#define KEY_NSPIRE_ESC          KEY_(0x1C, 0x080)
#define KEY_NSPIRE_BAR          KEY_(0x1C, 0x100)
#define KEY_NSPIRE_TAB          KEY_(0x1C, 0x200)
#define KEY_NSPIRE_EQU          KEYTPAD_(0x1E, 0x400, 0x18, 0x080)
#define KEY_NSPIRE_UP           KEYTPAD_ARROW_(0x1E, 0x001, TPAD_ARROW_UP)
#define KEY_NSPIRE_UPRIGHT      KEYTPAD_ARROW_(0x1E, 0x002, TPAD_ARROW_UPRIGHT)
#define KEY_NSPIRE_RIGHT        KEYTPAD_ARROW_(0x1E, 0x004, TPAD_ARROW_RIGHT)
#define KEY_NSPIRE_RIGHTDOWN    KEYTPAD_ARROW_(0x1E, 0x008, TPAD_ARROW_RIGHTDOWN)
#define KEY_NSPIRE_DOWN         KEYTPAD_ARROW_(0x1E, 0x010, TPAD_ARROW_DOWN)
#define KEY_NSPIRE_DOWNLEFT     KEYTPAD_ARROW_(0x1E, 0x020, TPAD_ARROW_DOWNLEFT)
#define KEY_NSPIRE_LEFT         KEYTPAD_ARROW_(0x1E, 0x040, TPAD_ARROW_LEFT)
#define KEY_NSPIRE_LEFTUP       KEYTPAD_ARROW_(0x1E, 0x080, TPAD_ARROW_LEFTUP)
#define KEY_NSPIRE_SHIFT        KEYTPAD_(0x1A, 0x200, 0x1E, 0x100)
// KEY_NSPIRE_CAPS is deprecated
#define KEY_NSPIRE_CAPS KEY_NSPIRE_SHIFT
#define KEY_NSPIRE_CTRL         KEY_(0x1E, 0x200)
#define KEY_NSPIRE_DOC          KEYTPAD_(_KEY_DUMMY_ROW, _KEY_DUMMY_COL, 0x1C, 0x008)
#define KEY_NSPIRE_TRIG         KEYTPAD_(_KEY_DUMMY_ROW, _KEY_DUMMY_COL, 0x12, 0x200)
#define KEY_NSPIRE_SCRATCHPAD   KEYTPAD_(_KEY_DUMMY_ROW, _KEY_DUMMY_COL, 0x1A, 0x400)
#endif

#if FORNSPIRE
// For this to compile on newer SDKs, make sure the key definitions in <keys.h> are #defines, not variables (even consts)
t_key rgi_nspire_key_map[ 128 ] = {
    KEY_NSPIRE_ENTER,
	KEY_NSPIRE_ESC,
	KEY_NSPIRE_SPACE,
	KEY_NSPIRE_CTRL,
	KEY_NSPIRE_TAB,
	KEY_NSPIRE_SHIFT,
	KEY_NSPIRE_DEL,
	KEY_NSPIRE_QUES,
	KEY_NSPIRE_QUESEXCL,
	KEY_NSPIRE_MINUS,
	KEY_NSPIRE_PLUS,
	KEY_NSPIRE_A,
	KEY_NSPIRE_B,
	KEY_NSPIRE_C,
	KEY_NSPIRE_D,
	KEY_NSPIRE_E,
	KEY_NSPIRE_F,
	KEY_NSPIRE_G,
	KEY_NSPIRE_H,
	KEY_NSPIRE_I,
	KEY_NSPIRE_J,
	KEY_NSPIRE_K,
	KEY_NSPIRE_L,
	KEY_NSPIRE_M,
	KEY_NSPIRE_N,
	KEY_NSPIRE_O,
	KEY_NSPIRE_P,
	KEY_NSPIRE_Q,
	KEY_NSPIRE_R,
	KEY_NSPIRE_S,
	KEY_NSPIRE_T,
	KEY_NSPIRE_U,
	KEY_NSPIRE_V,
	KEY_NSPIRE_W,
	KEY_NSPIRE_X,
	KEY_NSPIRE_Y,
	KEY_NSPIRE_Z,
	KEY_NSPIRE_1,
	KEY_NSPIRE_2,
	KEY_NSPIRE_3,
	KEY_NSPIRE_4,
	KEY_NSPIRE_5,
	KEY_NSPIRE_6,
	KEY_NSPIRE_7,
	KEY_NSPIRE_8,
	KEY_NSPIRE_9,
	KEY_NSPIRE_0,
};

int rgi_nspire_key_map_to_quake[ 128 ] = {
	K_ENTER,
	K_ESCAPE,
	K_SPACE,
	K_CTRL,
	K_TAB,
	K_SHIFT,
	K_BACKSPACE,
	'`',
	'`',
	'-',
	'+',
	'a',
	'b',
	'c',
	'd',
	'e',
	'f',
	'g',
	'h',
	'i',
	'j',
	'k',
	'l',
	'm',
	'n',
	'o',
	'p',
	'q',
	'r',
	's',
	't',
	'u',
	'v',
	'w',
	'x',
	'y',
	'z',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'0',
	-1
};

unsigned char rgi_nspire_key_state[ 128 ] = { 0, };
unsigned char rgi_nspire_arrow_key_state[ 4 ] = { 0, };
#endif


#define isKeyPressed_optimized(key) ( \
	(key).tpad_arrow != TPAD_ARROW_NONE && is_touchpad_optimized ? touchpad_arrow_pressed((key).tpad_arrow) \
	                                    : !is_classic_optimized ^ ((is_touchpad_optimized ? !(rgui16_key_map_optimized[(key).tpad_row >> 1] & (key).tpad_col) \
	                                                   : !(rgui16_key_map_optimized[(key).row >> 1] & (key).col) ) ) )

void Sys_NSpireInput( void )
{
	int i_idx;
#if FORNSPIRE
	int is_touchpad_optimized = is_touchpad;
	int is_classic_optimized = is_classic;
	unsigned short rgui16_key_map_optimized[ 16 ];
	int up_down_optimized, up_right_down_optimized, right_down_optimized, right_down_down_optimized;
	int down_down_optimized, down_left_down_optimized, left_down_optimized, left_up_down_optimized;

	for( i_idx = 0x10; i_idx < 0x20; i_idx+=2 )
	{
        rgui16_key_map_optimized[ i_idx >> 1 ] = (*(volatile short*)(0x900E0000 + i_idx)); // this address was KEY_MAP in the early days
	}

	for( i_idx = 0; i_idx < 128; i_idx++ )
	{
		if( rgi_nspire_key_map_to_quake[ i_idx ] < 0 )
		{
			break;
		}
		if( isKeyPressed_optimized( rgi_nspire_key_map[ i_idx ] ) )
		{
			if( rgi_nspire_key_state[ i_idx ] == 0 )
			{
				Key_Event( rgi_nspire_key_map_to_quake[ i_idx ], true );
				rgi_nspire_key_state[ i_idx ] = 1;
			}
		}
		else
		{
			if( rgi_nspire_key_state[ i_idx ] == 1 )
			{
				Key_Event( rgi_nspire_key_map_to_quake[ i_idx ], false );
				rgi_nspire_key_state[ i_idx ] = 0;
			}
		}
	}

	up_down_optimized = isKeyPressed_optimized( KEY_NSPIRE_UP );
	up_right_down_optimized = isKeyPressed_optimized( KEY_NSPIRE_UPRIGHT );
	right_down_optimized = isKeyPressed_optimized( KEY_NSPIRE_RIGHT );
	right_down_down_optimized = isKeyPressed_optimized( KEY_NSPIRE_RIGHTDOWN );
	down_down_optimized = isKeyPressed_optimized( KEY_NSPIRE_DOWN );
	down_left_down_optimized = isKeyPressed_optimized( KEY_NSPIRE_DOWNLEFT );
	left_down_optimized = isKeyPressed_optimized( KEY_NSPIRE_LEFT );
	left_up_down_optimized = isKeyPressed_optimized( KEY_NSPIRE_LEFTUP );

	if( left_down_optimized || down_left_down_optimized || left_up_down_optimized )
	{
		/*printf("LEFT DOWN\n");*/
		if( rgi_nspire_arrow_key_state[ 0 ] == 0 )
		{
			Key_Event( K_LEFTARROW, true );
			rgi_nspire_arrow_key_state[ 0 ] = 1;
		}
	}
	else
	{
		if( rgi_nspire_arrow_key_state[ 0 ] == 1 )
		{
			Key_Event( K_LEFTARROW, false );
			rgi_nspire_arrow_key_state[ 0 ] = 0;
		}
	}
	if( up_down_optimized || up_right_down_optimized || left_up_down_optimized )
	{
		/*printf("UP DOWN\n");*/
		if( rgi_nspire_arrow_key_state[ 1 ] == 0 )
		{
			Key_Event( K_UPARROW, true );
			rgi_nspire_arrow_key_state[ 1 ] = 1;
		}
	}
	else
	{
		if( rgi_nspire_arrow_key_state[ 1 ] == 1 )
		{
			Key_Event( K_UPARROW, false );
			rgi_nspire_arrow_key_state[ 1 ] = 0;
		}
	}
	if( right_down_optimized || up_right_down_optimized || right_down_down_optimized )
	{
		/*printf("RIGHT DOWN\n");*/
		if( rgi_nspire_arrow_key_state[ 2 ] == 0 )
		{
			Key_Event( K_RIGHTARROW, true );
			rgi_nspire_arrow_key_state[ 2 ] = 1;
		}
	}
	else
	{
		if( rgi_nspire_arrow_key_state[ 2 ] == 1 )
		{
			Key_Event( K_RIGHTARROW, false );
			rgi_nspire_arrow_key_state[ 2 ] = 0;
		}
	}
	if( down_down_optimized || right_down_down_optimized || down_left_down_optimized )
	{
		/*printf("DOWN DOWN\n");*/
		if( rgi_nspire_arrow_key_state[ 3 ] == 0 )
		{
			Key_Event( K_DOWNARROW, true );
			rgi_nspire_arrow_key_state[ 3 ] = 1;
		}
	}
	else
	{
		if( rgi_nspire_arrow_key_state[ 3 ] == 1 )
		{
			Key_Event( K_DOWNARROW, false );
			rgi_nspire_arrow_key_state[ 3 ] = 0;
		}
	}
#endif
}

//=============================================================================

void main_s( int argc, char **argv )
{
	char *pc_basedir_term;
	static quakeparms_t    parms;
	unsigned int i = 0;
	double f64_start, f64_end, f64_delta;

	char rgc_test[ 128 ];

	/*printf("main_s\n" );*/

	Sys_Init();

#ifndef WIN32
	if (!has_colors)
		return Sys_Error("needs colors...");
	lcd_incolor();
#endif

	/*printf("main_s alloc\n" );*/

	parms.memsize = 10*1024*1024;
	parms.membase = p_nspire_membase = malloc (parms.memsize);
	parms.basedir = ".";

	if( !p_nspire_membase )
	{
		Sys_Error( "could not alloc %.2f kb of memory\n", ( ( double )parms.memsize ) / 1024.0 );
	}

	pc_basedir_term = argv[ 0 ] + Q_strlen( argv[ 0 ] );
	while( pc_basedir_term >= argv[ 0 ] )
	{
		if( *pc_basedir_term == '/' || *pc_basedir_term == '\\' )
		{
			*pc_basedir_term = 0;
			break;
		}
		pc_basedir_term--;
	}
	parms.basedir = argv[ 0 ];

	for( i = 0; i < parms.memsize; i++ )
	{
		( ( unsigned char * )parms.membase )[ i ] = 0;
	}

	/*printf("membase: %p\n", parms.membase );*/

	/*printf("argc: %d\n", argc );*/

	COM_InitArgv (argc, argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

	/*printf ("Host_Init\n");*/
	/*printf ("Chained..\n");*/
#ifndef WIN32
	/*bkpt();*/
#endif
	Host_Init (&parms);
	f64_start = f64_end = Sys_FloatTime();
	while (1)
	{
		f64_delta = f64_end - f64_start;
		/*Sys_NSpireInput();*/
		Host_Frame ( f64_delta );
		f64_start = f64_end;
		f64_end = Sys_FloatTime();
	}
}

#ifdef FORNSPIRE
extern void nspire_stack_redirect( void (*f_func)(void), void *new_stack );
extern int nspire_stack_align( int argc, char *argv[] );
extern void *p_nspire_stack_redirect;
#endif

int main (int argc, char **argv)
{

#ifdef FORNSPIRE
	p_nspire_stack_redirect = malloc( 0x60000 );
	nspire_stack_align( argc, argv );
	Sys_Quit();
#else
	p_nspire_stack_redirect = malloc( 0x60000 );
	main_s( argc, argv );
	free( p_nspire_stack_redirect );
#endif

	return 0;
}


