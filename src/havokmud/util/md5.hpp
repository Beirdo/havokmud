/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2013 Gavin Hurlbut
 *
 *  havokmud is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * @file
 * @brief MD5/OPIE Interface
 */

#ifndef __havokmud_util_md5__
#define __havokmud_util_md5__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * From md5c.c
 */
void *opiemd5init(void);
void opiemd5update(void *ctx, unsigned char *input, unsigned int inputLen);
void opiemd5final(unsigned char *digest, void *ctx);

/*
 * From btoe.c
 */
char *opiebtoe (char *engout, char *c);
int opieetob ( char *out, char *e );


#ifdef __cplusplus
}
#endif

#endif  // __havokmud_util_md5__
