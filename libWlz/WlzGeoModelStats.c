#if defined(__GNUC__)
#ident "University of Edinburgh $Id$"
#else
static char _WlzGeoModelStats_c[] = "University of Edinburgh $Id$";
#endif
/*!
* \file         libWlz/WlzGeoModelStats.c
* \author       Bill Hill
* \date         December 2003
* \version      $Id$
* \par
* Address:
*               MRC Human Genetics Unit,
*               MRC Institute of Genetics and Molecular Medicine,
*               University of Edinburgh,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \par
* Copyright (C), [2012],
* The University Court of the University of Edinburgh,
* Old College, Edinburgh, UK.
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
* \brief	Functions to compute statistics about geometric models.
* \ingroup	WlzGMModel
*/

#include <float.h>
#include <Wlz.h>

/*!
* \return	Number of simplices in the model.
* \ingroup	WlzGeoModel
* \brief	Calculates statistics of the simplices in the given
*		geometric model. This can be useful for checking the
*		quality of a given model.
* \param	model			Given model.
* \param	dstMin			Destination pointer for minimum
*					simplex length or area, may be NULL.
* \param	dstMax			Destination pointer for maximum
*					simplex length or area, may be NULL.
* \param	dstSum			Destination pointer for sum of
*					simplex length or area, may be NULL.
* \param	dstSumSq		Destination pointer for sum of
*					squares of simplex length or area,
*					may be NULL.
* \param	dstMean			Destination pointer for mean
*					simplex length or area, may be NULL.
* \param	dstStdDev		Standard deviation of simplex length
*					or area, may be NULL.
* \param	dstErr			Destination pointer for error
*					number, may be NULL if not required.
*/
int		WlzGMModelSpxStats(WlzGMModel *model,
				   double *dstMin, double *dstMax,
				   double *dstSum, double *dstSumSq,
				   double *dstMean, double *dstStdDev,
				   WlzErrorNum *dstErr)
{
  int		cnt = 0;
  double	val,
  		minVal = DBL_MAX,
		maxVal = 0.0,
		sumVal = 0.0,
		sumSqVal = 0.0;
  WlzVertex	p0,
  		p1,
		p2;
  AlcVector	*vec;
  WlzGMVertex	*vtx;
  WlzGMEdge	*edge;
  WlzGMEdgeT	*edgeT;
  WlzGMFace	*face;
  WlzGMLoopT	*loopT;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    switch(model->type)
    {
      case WLZ_GMMOD_2I:
      case WLZ_GMMOD_2D:
      case WLZ_GMMOD_2N:
        vec = model->res.edge.vec;
	while(cnt < model->res.edge.numIdx)
	{
	  edge = (WlzGMEdge *)AlcVectorItemGet(vec, cnt);
	  if(edge && (edge->idx >= 0))
	  {
	    edgeT = edge->edgeT;
	    vtx = edgeT->vertexT->diskT->vertex;
	    (void )WlzGMVertexGetG2D(vtx, &(p0.d2));
	    vtx = edgeT->opp->vertexT->diskT->vertex;
	    (void )WlzGMVertexGetG2D(vtx, &(p1.d2));
	    WLZ_VTX_2_SUB(p2.d2, p0.d2, p1.d2);
	    val = WLZ_VTX_2_SQRLEN(p2.d2);
	    val = (val > 0.0)? sqrt(val): 0.0;
	    sumVal += val;
	    sumSqVal += val * val;
	    if(minVal > val)
	    {
	      minVal = val;
	    }
	    if(maxVal < val)
	    {
	      maxVal = val;
	    }
	  }
	  ++cnt;
	}
	break;
      case WLZ_GMMOD_3I:
      case WLZ_GMMOD_3D:
      case WLZ_GMMOD_3N:
        vec = model->res.face.vec;
	while(cnt < model->res.face.numIdx)
	{
	  face = (WlzGMFace *)AlcVectorItemGet(vec, cnt);
	  if(face && (face->idx >= 0))
	  {
	    loopT = face->loopT;
	    vtx = loopT->edgeT->vertexT->diskT->vertex;
	    (void )WlzGMVertexGetG3D(vtx, &(p0.d3));
	    vtx = loopT->edgeT->next->vertexT->diskT->vertex;
	    (void )WlzGMVertexGetG3D(vtx, &(p1.d3));
	    vtx = loopT->edgeT->prev->vertexT->diskT->vertex;
	    (void )WlzGMVertexGetG3D(vtx, &(p2.d3));
	    val = WlzGeomTriangleArea2Sq3(p0.d3, p1.d3, p2.d3);
	    val = (val > 0.0)? sqrt(val) * 0.5: 0.0;
	    sumVal += val;
	    sumSqVal += val * val;
	    if(minVal > val)
	    {
	      minVal = val;
	    }
	    if(maxVal < val)
	    {
	      maxVal = val;
	    }
	  }
	  ++cnt;
	}
	break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    if(dstMin)
    {
      *dstMin = minVal;
    }
    if(dstMax)
    {
      *dstMax = maxVal;
    }
    if(dstSum)
    {
      *dstSum = sumVal;
    }
    if(dstSumSq)
    {
      *dstSumSq = sumSqVal;
    }
    if(dstMean)
    {
      *dstMean = (cnt > 0)? sumVal / cnt: 0.0;
    }
    if(dstStdDev)
    {
      *dstStdDev = (cnt > 1)?
      		   sqrt((sumSqVal - (sumVal * sumVal / cnt)) / (cnt - 1)):
		   0.0;
    }
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(cnt);
}
