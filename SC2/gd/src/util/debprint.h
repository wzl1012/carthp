#ifndef   _DEBPRINT_H_
#define   _DEBPRINT_H_
#include "gd32f10x.h"
#include <stdio.h>
#define  DEBUG  0
void void_printf(void*f,...);
void __no_operation(void);
#if DEBUG
#define  debug_printf   printf
#else
#define  debug_printf   void_printf 
#endif 
#define  info_printf   printf
#endif

