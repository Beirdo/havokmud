#ifndef _config_h
#define _config_h

#define _GNU_SOURCE

#ifdef __CYGWIN__
#define __SAVE_ANSI__ __STRICT_ANSI__
#undef __STRICT_ANSI__
#endif

#else

#error you missed a spot, config.h included twice

#endif

