#pragma ident "MRC HGU $Id$"
/*!
* \file         WlzBndGreyInvert.c
* \author       Bill Hill
* \date         August 2003
* \version      $Id$
* \note
*               Copyright
*               2003 Medical Research Council, UK.
*               All rights reserved.
*               All rights reserved.
* \par Address:
*               MRC Human Genetics Unit,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \brief
* \ingroup
* \todo         -
* \bug          None known.
*/
#include <WlzBnd.h>

/*!
* \return	Woolz error code.
* \ingroup	WlzBnd
* \brief	A binding for the WlzGreyRange() and WlzGreyInvertMinMax()
* 		which inverts an objects grey value while keeping the same
*		grey range as in the Woolz binary.
* \param	obj			Given object.
*/
WlzErrorNum	WlzBndGreyInvert(WlzObject *obj)
{
  WlzPixelV	min,
  		max;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((errNum = WlzGreyRange(obj, &min, &max)) == WLZ_ERR_NONE)
  {
    errNum = WlzGreyInvertMinMax(obj, min, max);
  }
  return(errNum);
}
