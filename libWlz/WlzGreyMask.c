#pragma ident "MRC HGU $Id$"
/***********************************************************************
* Project:      Woolz
* Title:        WlzGreyMask.c
* Date:         March 1999
* Author:       Richard Baldock
* Copyright:	1999 Medical Research Council, UK.
*		All rights reserved.
* Address:	MRC Human Genetics Unit,
*		Western General Hospital,
*		Edinburgh, EH4 2XU, UK.
* Purpose:      Functions to set the value within the domain of a
*		Woolz object.
* $Revision$
* Maintenance:	Log changes below, with most recent at top of list.
************************************************************************/
#include <stdlib.h>
#include <Wlz.h>

static WlzObject *WlzGreyMask3d(WlzObject	*obj,
				WlzObject	*mask,
				WlzPixelV	maskVal,
				WlzErrorNum	*dstErr);


/************************************************************************
*   Function   : WlzGreyMask						*
*   Date       : Sat Oct 18 18:54:53 1997				*
*************************************************************************
*   Synopsis   :Set the value maskVal within the domain given by the	*
*		mask object. The mask object can be a 2D, 3D, polygon	*
*		or boundary object. A 3D mask with a 2D object is an	*
*		error. A 2D mask with a 3D object will be applied to	*
*		each plane in turn.					*
*   Returns    :WlzObject*: new object with the same domain as obj but	*
*			values in the intersection with mask set to	*
*			maskVal. NULL on error.				*
*   Parameters :obj:	object to which mask is applied			*
*		mask:	mask domain					*
*		maskVal:	mask value - any grey type		*
*		dstErr:	pointer for error return			*
*   Global refs:None.							*
************************************************************************/

WlzObject *WlzGreyMask(
  WlzObject	*obj,
  WlzObject	*mask,
  WlzPixelV	maskVal,
  WlzErrorNum	*dstErr)
{
  WlzObject	*rtnObj=NULL;
  WlzObject	*tmpMask, *obj1;
  WlzValues	values;
  WlzPixelV	tmpMaskval;
  WlzIntervalWSpace	iwsp;
  WlzGreyWSpace		gwsp;
  WlzGreyP		gptr;
  int			i;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  /* check obj */
  if( obj == NULL ){
    errNum = WLZ_ERR_OBJECT_NULL;
  }
  else {
    switch( obj->type ){
    case WLZ_2D_DOMAINOBJ:
      if( obj->values.core == NULL ){
	errNum = WLZ_ERR_VALUES_NULL;
      }
      break;

    case WLZ_3D_DOMAINOBJ:
      return WlzGreyMask3d(obj, mask, maskVal, dstErr);

    case WLZ_TRANS_OBJ:
      if( values.obj = WlzGreyMask(obj->values.obj, mask, maskVal, &errNum) ){
	return WlzMakeMain(WLZ_TRANS_OBJ, obj->domain, values,
			   NULL, NULL, dstErr);
      }
      break;

    case WLZ_EMPTY_OBJ:
      return WlzMakeEmpty(dstErr);

    default:
      errNum = WLZ_ERR_OBJECT_TYPE;
      break;
    }
  }

  /* check the mask */
  if( errNum == WLZ_ERR_NONE ){
    if( mask == NULL ){
      errNum = WLZ_ERR_OBJECT_NULL;
    }
    else {
      values.core = NULL;
      switch( mask->type ){
      case WLZ_2D_DOMAINOBJ:
	tmpMask = WlzMakeMain(WLZ_2D_DOMAINOBJ, mask->domain, values,
			      NULL, NULL, &errNum);
	break;

      case WLZ_TRANS_OBJ:
	tmpMask = WlzMakeMain(WLZ_2D_DOMAINOBJ, mask->values.obj->domain,
			      values, NULL, NULL, &errNum);
	break;

      case WLZ_EMPTY_OBJ:
	return WlzMakeMain(WLZ_2D_DOMAINOBJ, obj->domain, obj->values,
			   NULL, NULL, dstErr);

      case WLZ_2D_POLYGON:
	tmpMask = WlzPolyToObj(mask->domain.poly, WLZ_SIMPLE_FILL, &errNum);
	break;

      case WLZ_BOUNDLIST:
	tmpMask = WlzBoundToObj(mask->domain.b, WLZ_SIMPLE_FILL, &errNum);
	break;

      default:
	errNum = WLZ_ERR_OBJECT_TYPE;
	break;
      }
      if( errNum == WLZ_ERR_NONE ){
	tmpMask = WlzAssignObject(tmpMask, NULL);
      }
    }
  }

  /* copy input obj and setvalues in the intersection */
  if( errNum == WLZ_ERR_NONE ){
    if( rtnObj = WlzNewGrey(obj, &errNum) ){
      if( obj1 = WlzIntersect2(obj, tmpMask, &errNum) ){
	obj1->values = WlzAssignValues(rtnObj->values, NULL);
	errNum = WlzInitGreyScan(obj1, &iwsp, &gwsp);
	WlzValueConvertPixel(&tmpMaskval, maskVal, gwsp.pixeltype);
	while( (errNum = WlzNextGreyInterval(&iwsp)) == WLZ_ERR_NONE ){
	  gptr = gwsp.u_grintptr;
	  switch( gwsp.pixeltype ){
	  case WLZ_GREY_INT:
	    for(i=0; i<iwsp.colrmn; i++, gptr.inp++){
	      *gptr.inp = tmpMaskval.v.inv;
	    }
	    break;
	  case WLZ_GREY_SHORT:
	    for(i=0; i<iwsp.colrmn; i++, gptr.shp++){
	      *gptr.shp = tmpMaskval.v.shv;
	    }
	    break;
	  case WLZ_GREY_UBYTE:
	    for(i=0; i<iwsp.colrmn; i++, gptr.ubp++){
	      *gptr.ubp = tmpMaskval.v.ubv;
	    }
	    break;
	  case WLZ_GREY_FLOAT:
	    for(i=0; i<iwsp.colrmn; i++, gptr.flp++){
	      *gptr.flp = tmpMaskval.v.flv;
	    }
	    break;
	  case WLZ_GREY_DOUBLE:
	    for(i=0; i<iwsp.colrmn; i++, gptr.dbp++){
	      *gptr.dbp = tmpMaskval.v.dbv;
	    }
	    break;
	  }
	}
	if( errNum == WLZ_ERR_EOO ){
	  errNum = WLZ_ERR_NONE;
	}
	WlzFreeObj(obj1);
      }
      else {
	WlzFreeObj(rtnObj);
	rtnObj = NULL;
      }
    }
    WlzFreeObj(tmpMask);
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return rtnObj;
}

static WlzObject *WlzGreyMask3d(
  WlzObject	*obj,
  WlzObject	*mask,
  WlzPixelV	maskVal,
  WlzErrorNum	*dstErr)
{
  WlzObject	*rtnObj=NULL;
  WlzObject	*tmpMask, *obj1, *obj2;
  WlzDomain	*domains;
  WlzValues	values, *valuess, *newvaluess;
  WlzPlaneDomain	*pdom;
  int		p;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  /* check the object - it is non-NULL and 3D but the
     domain needs checking */
  if( obj->domain.p == NULL ){
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else {
    switch( obj->domain.p->type ){
    case WLZ_2D_DOMAINOBJ:
      /* check there is a valuetable */
      if( obj->values.core == NULL ){
	errNum = WLZ_ERR_VALUES_NULL;
      }
      break;

    case WLZ_EMPTY_DOMAIN:
      return WlzMakeEmpty(dstErr);

    default:
      errNum = WLZ_ERR_DOMAIN_TYPE;
      break;
    }
  }

  /* check the mask */
  if( errNum == WLZ_ERR_NONE ){
    if( mask == NULL ){
      errNum = WLZ_ERR_OBJECT_NULL;
    }
    else {
      /* set some local variables */
      pdom = obj->domain.p;
      domains = obj->domain.p->domains;
      valuess = obj->values.vox->values;

      /* create a new voxel object, initially no values */
      values.vox = WlzMakeVoxelValueTb(obj->values.vox->type,
				       obj->values.vox->plane1,
				       obj->values.vox->lastpl,
				       obj->values.vox->bckgrnd,
				       obj, NULL);
      rtnObj = WlzMakeMain(WLZ_3D_DOMAINOBJ, obj->domain, values,
			   NULL, NULL, NULL);
      newvaluess = rtnObj->values.vox->values;

      /* switch on mask type - a 2D mask is applied to each plane */
      switch( mask->type ){
      case WLZ_2D_DOMAINOBJ:
      case WLZ_2D_POLYGON:
      case WLZ_BOUNDLIST:
      case WLZ_EMPTY_OBJ:
	for(p=pdom->plane1; p <= pdom->lastpl; p++, domains++, valuess++){
	  if( (*domains).core ){
	    obj1 = WlzAssignObject(
	      WlzMakeMain(WLZ_2D_DOMAINOBJ, *domains, *valuess,
			  NULL, NULL, NULL), NULL);
	    if( obj2 = WlzGreyMask(obj1, mask, maskVal, &errNum) ){
	      newvaluess[p - pdom->plane1] =
		WlzAssignValues(obj->values, NULL);
	      WlzFreeObj(obj2);
	    }
	    WlzFreeObj(obj1);
	  }
	}
	break;

      case WLZ_3D_DOMAINOBJ:
	/* switch on plane domain type - polygon and boundlist
	   are legal but need transforming first. */
	switch( obj->domain.p->type ){
	  WlzDomain	*maskDoms;

	case WLZ_PLANEDOMAIN_DOMAIN:
	  maskDoms = mask->domain.p->domains;
	  for(p=pdom->plane1; p <= pdom->lastpl; p++, domains++, valuess++){
	    if(((*domains).core)){
	      if((p >= mask->domain.p->plane1) &&
		 (p <= mask->domain.p->lastpl) &&
		 (maskDoms[p - mask->domain.p->plane1].core) ){
		values.core = NULL;
		tmpMask = WlzMakeMain(WLZ_2D_DOMAINOBJ,
				      maskDoms[p - mask->domain.p->plane1],
				      values, NULL, NULL, NULL);
	      }
	      else {
		tmpMask = WlzMakeEmpty(NULL);
	      }

	      obj1 = WlzAssignObject(
		WlzMakeMain(WLZ_2D_DOMAINOBJ, *domains, *valuess,
			    NULL, NULL, NULL), NULL);
	      if( obj2 = WlzGreyMask(obj1, tmpMask, maskVal, &errNum) ){
		newvaluess[p - pdom->plane1] =
		  WlzAssignValues(obj2->values, NULL);
		WlzFreeObj(obj2);
	      }
	      WlzFreeObj(obj1);
	      WlzFreeObj(tmpMask);
	    }
	  }
	  break;

	case WLZ_PLANEDOMAIN_POLYGON:
	  maskDoms = mask->domain.p->domains;
	  for(p=pdom->plane1; p <= pdom->lastpl; p++, domains++, valuess++){
	    if(((*domains).core)){
	      if((p >= mask->domain.p->plane1) &&
		 (p <= mask->domain.p->lastpl) &&
		 (maskDoms[p - mask->domain.p->plane1].core) ){
		values.core = NULL;
		tmpMask = WlzMakeMain(WLZ_2D_POLYGON,
				      maskDoms[p - mask->domain.p->plane1],
				      values, NULL, NULL, NULL);
	      }
	      else {
		tmpMask = WlzMakeEmpty(NULL);
	      }

	      obj1 = WlzAssignObject(
		WlzMakeMain(WLZ_2D_DOMAINOBJ, *domains, *valuess,
			    NULL, NULL, NULL), NULL);
	      if( obj2 = WlzGreyMask(obj1, tmpMask, maskVal, &errNum) ){
		newvaluess[p - pdom->plane1] =
		  WlzAssignValues(obj2->values, NULL);
		WlzFreeObj(obj2);
	      }
	      WlzFreeObj(obj1);
	      WlzFreeObj(tmpMask);
	    }
	  }
	  break;

	case WLZ_PLANEDOMAIN_BOUNDLIST:
	  maskDoms = mask->domain.p->domains;
	  for(p=pdom->plane1; p <= pdom->lastpl; p++, domains++, valuess++){
	    if(((*domains).core)){
	      if((p >= mask->domain.p->plane1) &&
		 (p <= mask->domain.p->lastpl) &&
		 (maskDoms[p - mask->domain.p->plane1].core) ){
		values.core = NULL;
		tmpMask = WlzMakeMain(WLZ_BOUNDLIST,
				      maskDoms[p - mask->domain.p->plane1],
				      values, NULL, NULL, NULL);
	      }
	      else {
		tmpMask = WlzMakeEmpty(NULL);
	      }

	      obj1 = WlzAssignObject(
		WlzMakeMain(WLZ_2D_DOMAINOBJ, *domains, *valuess,
			    NULL, NULL, NULL), NULL);
	      if( obj2 = WlzGreyMask(obj1, tmpMask, maskVal, &errNum) ){
		newvaluess[p - pdom->plane1] =
		  WlzAssignValues(obj2->values, NULL);
		WlzFreeObj(obj2);
	      }
	      WlzFreeObj(obj1);
	      WlzFreeObj(tmpMask);
	    }
	  }
	  break;

	default:
	  errNum = WLZ_ERR_DOMAIN_TYPE;
	  WlzFreeObj(rtnObj);
	  rtnObj = NULL;
	  break;
	}
	break;

      default:
	errNum = WLZ_ERR_OBJECT_NULL;
	WlzFreeObj(rtnObj);
	rtnObj = NULL;
	break;
      }
    }
  }


  if( dstErr ){
    *dstErr = errNum;
  }
  return rtnObj;
}

