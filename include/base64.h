#ifndef __base64_h
#define __base64_h
/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

//#include <stdio.h>
//#include <stdlib.h>
//#include <stdint.h>

//#include "stdafx.h"
#include <iostream>

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#define ARRAY_SIZE(a)       (sizeof(a) / sizeof((a)[0]))
//#define CRIT(a)             fprintf(stderr, "\nCRITICAL file:%s line:%u msg:%s\n", __FILE__, __LINE__,a);exit(EXIT_FAILURE)

//#define DEBUG(args...)    fprintf(stderr,"debug: " args) /* diagnostic message that is destined to the user */
//#define DEBUG(args...)

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MODULE-WIDE VARIABLES ---------------------------------------- */

static char code_62 = '+';    /* RFC 1421 standard character for code 62 */
static char code_63 = '/';    /* RFC 1421 standard character for code 63 */
static char code_pad = '=';    /* RFC 1421 padding character if padding */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DECLARATION ---------------------------------------- */

/**
@brief Convert a code in the range 0-63 to an ASCII character
*/
//char code_to_char(unsigned char x);

/**
@brief Convert an ASCII character to a code in the range 0-63
*/
//unsigned char char_to_code(char x);

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DEFINITION ----------------------------------------- */

char code_to_char(unsigned char x);

unsigned char char_to_code(char x);
/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

int bin_to_b64_nopad(const unsigned char * in, int size, char * out, int max_len);

int b64_to_bin_nopad(const char * in, int size, unsigned char * out, int max_len);

int bin_to_b64(const unsigned char * in, int size, char * out, int max_len);

int b64_to_bin(const char * in, int size, unsigned char * out, int max_len);

unsigned char b4_to_ascii(const unsigned char in_char);

/* --- EOF ------------------------------------------------------------------ */
#endif