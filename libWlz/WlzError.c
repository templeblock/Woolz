#if defined(__GNUC__)
#ident "MRC HGU $Id$"
#else
#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma ident "MRC HGU $Id$"
#else
static char _WlzError_c[] = "MRC HGU $Id$";
#endif
#endif
/*!
* \file         libWlz/WlzError.c
* \author       Bill Hill
* \date         November 1999
* \version      $Id$
* \par
* Address:
*               MRC Human Genetics Unit,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \par
* Copyright (C) 2005 Medical research Council, UK.
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be
* useful but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
* PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the Free
* Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA  02110-1301, USA.
* \brief	Error related functions for the Woolz library.
* \ingroup	WlzError
* \todo         -
* \bug          None known.
*/

#include <stdlib.h>
#include <float.h>
#include <Wlz.h>

/*!
* \return	Woolz error code.
* \ingroup	WlzError
* \brief	Converts an Alg error code to a Woolz error code.
* \param	algErr			Given Alg error code.
*/
WlzErrorNum	WlzErrorFromAlg(AlgError algErr)
{
  WlzErrorNum	wlzErr = WLZ_ERR_ALG;

  switch(algErr)
  {
    case ALG_ERR_NONE:
      wlzErr = WLZ_ERR_NONE;
      break;
    case ALG_ERR_FUNC:
      wlzErr = WLZ_ERR_PARAM_DATA;
      break;
    case ALG_ERR_MALLOC:
      wlzErr = WLZ_ERR_MEM_ALLOC;
      break;
    case ALG_ERR_SINGULAR:
      wlzErr = WLZ_ERR_ALG_SINGULAR;
      break;
    case ALG_ERR_HOMOGENEOUS:
      wlzErr = WLZ_ERR_ALG_HOMOGENEOUS;
      break;
    case ALG_ERR_CONVERGENCE:
      wlzErr = WLZ_ERR_ALG_CONVERGENCE;
      break;
    case ALG_ERR_DIVZERO:
      wlzErr = WLZ_ERR_ALG;
      break;
  }
  return(wlzErr);
}