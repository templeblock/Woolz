#pragma ident "MRC HGU $Id$"
/*!
* \file		WlzGeoModel.c
* \author	Bill Hill
* \date		February 2000
* \version	$Id$
* \note
*               Copyright
*               2001 Medical Research Council, UK.
*               All rights reserved.
* \par Address:
*               MRC Human Genetics Unit,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \brief        Basic operators for manipulating Woolz geometric models.
*		These can be either planar graphs or 3D surfaces, with
*		the surfaces being either manifold or non-manifold.
* \ingroup      WlzGeoModel
* \todo         - The element deletion functions are only partly written
*		  and only partly work for 2D models.
* \bug          None known.
*/
#include <Wlz.h>
#include <limits.h>

static WlzGMShell 	*WlzGMLoopTFindShell(
			  WlzGMLoopT *gLT);
static WlzErrorNum 	WlzGMShellMergeG(
			  WlzGMShell *shell0,
			  WlzGMShell *shell1);
static WlzErrorNum 	WlzGMShellTestOutVTK(
			  WlzGMShell *shell,
			  int *vIdxTb,
			  char *lFlg,
			  FILE *fP);
static WlzErrorNum 	WlzGMLoopTTestOutputVTK(
			  WlzGMLoopT *fLT,
			  int *vIdxTb,
			  FILE *fP);
static WlzErrorNum 	WlzGMShellTestOutPS(
			  WlzGMShell *shell,
			  char *lFlg,
			  char *eFlg,
			  FILE *fP,
			  WlzDVertex2 offset,
			  WlzDVertex2 scale);
static WlzErrorNum	WlzGMLoopTTestOutPS(
			  WlzGMLoopT *loopT,
			  char *eFlg,
			  FILE *fP,
			  WlzDVertex2 offset,
			  WlzDVertex2 scale);
static WlzErrorNum	WlzGMEdgeTTestOutPS(
			  WlzGMEdgeT *edgeT,
			  FILE *fP,
			  WlzDVertex2 offset,
			  WlzDVertex2 scale);
static void		WlzGMTestOutLinePS(
			  FILE *fP,
			  WlzDVertex2 pos0,
			  WlzDVertex2 pos1,
			  WlzDVertex2 offset,
			  WlzDVertex2 scale);
static unsigned int 	WlzGMHashPos2D(
			  WlzDVertex2 pos);
static unsigned int 	WlzGMHashPos3D(
			  WlzDVertex3 pos);
static void		WlzGMModelMatchEdgeTG2D(
			  WlzGMModel *model,
			  WlzGMEdgeT **matchET,
			  WlzDVertex2 *pos);
static WlzErrorNum      WlzGMModelConstructNewS3D(
                          WlzGMModel *model,
                          WlzDVertex3 *pos);
static WlzErrorNum      WlzGMModelExtend1V0E1S3D(
                          WlzGMModel *model,
                          WlzGMVertex *eV,
                          WlzDVertex3 pos0,
                          WlzDVertex3 pos1);
static WlzErrorNum      WlzGMModelExtend2V1E1S3D(
                          WlzGMModel *model,
                          WlzGMEdge *eE,
                          WlzDVertex3 pos2);
static WlzErrorNum      WlzGMModelExtend2V0E1S3D(
                          WlzGMModel *model,
                          WlzGMVertex *eV0,
                          WlzGMVertex *eV1,
                          WlzDVertex3 pos2);
static WlzErrorNum      WlzGMModelJoin2V0E0S3D(
                          WlzGMModel *model,
                          WlzGMVertex *eV0,
                          WlzGMVertex *eV1,
                          WlzDVertex3 pos2);
static WlzErrorNum      WlzGMModelJoin3V0E3S3D(
                          WlzGMModel *model,
                          WlzGMVertex **eV);
static WlzErrorNum      WlzGMModelJoin3V0E2S3D(
                          WlzGMModel *model,
                          WlzGMVertex **eV);
static WlzErrorNum      WlzGMModelExtend3V0E1S3D(
                          WlzGMModel *model,
                          WlzGMVertex **eV);
static WlzErrorNum      WlzGMModelExtend3V1E1S3D(
                          WlzGMModel *model,
                          WlzGMEdge *eE,
                          WlzGMVertex *eV);
static WlzErrorNum      WlzGMModelJoin3V1E2S3D(
                          WlzGMModel *model,
                          WlzGMEdge *eE,
                          WlzGMVertex *eV);
static WlzErrorNum      WlzGMModelExtend3V2E1S3D(
                          WlzGMModel *model,
                          WlzGMEdge *eE0,
                          WlzGMEdge *eE1);
static WlzErrorNum      WlzGMModelConstructNewS2D(
                          WlzGMModel *model,
                          WlzDVertex2 *pos);
static WlzErrorNum      WlzGMModelExtendL2D(
                          WlzGMModel *model,
                          WlzGMEdgeT *eET,
                          WlzDVertex2 nPos);
static WlzErrorNum      WlzGMModelConstructSplitL2D(
                          WlzGMModel *model,
                          WlzGMEdgeT *eET0,
                          WlzGMEdgeT *eET1);
static WlzErrorNum      WlzGMModelJoinL2D(
                          WlzGMModel *model,
                          WlzGMEdgeT *eET0,
                          WlzGMEdgeT *eET1);
static void             WlzGMLoopTSetT(
                          WlzGMLoopT *eLT);
static void             WlzGMModelAddVertex(
                          WlzGMModel *model,
                          WlzGMVertex *nV);
static void		WlzGMModelRemVertex(
			  WlzGMModel *model,
			  WlzGMVertex *dV);
static void		WlzGMElmMarkFree(
			  int *idxP);
static void		WlzGMModelDeleteET(
			  WlzGMModel *model,
			  WlzGMEdgeT *dET);
static void 		WlzGMModelDeleteLT(
			  WlzGMModel *model,
			  WlzGMLoopT *dLT);

/* Resource callback function list manipulation. */

/*!
* \return				Woolz error code.
* \brief	Add a resource allocation callback to the models resources.
* \param	model			The model.
* \param	fn			The callback function.
* \param	data			The callback data to be passed on
*					to the callback function.
*/
WlzErrorNum	WlzGMModelAddResCb(WlzGMModel *model, WlzGMCbFn fn,
				   void *data)
{
  WlzGMCbEntry	*newCbE = NULL;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model && fn)
  {
    if((newCbE = (WlzGMCbEntry *)AlcMalloc(sizeof(WlzGMCbEntry))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
    else
    {
      newCbE->fn = fn;
      newCbE->data = data;
      newCbE->next = model->res.callbacks;
      model->res.callbacks = newCbE;
    }
  }
  return(errNum);
}

/*!
* \return	<void>
* \brief	Removes a resource allocation callback from the models
*		resources. Both the callback function and callback data
*		must have the same values as when the callback was added
*		since they are used to find the corresponding callback
*		entry.
* \param	model			The model.
* \param	fn			The callback function.
* \param	data			The callback data.
*/
void		WlzGMModelRemResCb(WlzGMModel *model, WlzGMCbFn fn,
				   void *data)
{
  WlzGMCbEntry	*cBE0,
  		*cBE1;

  if(model && fn && ((cBE1 = model->res.callbacks) != NULL))
  {
    cBE0 = NULL;
    while((fn != cBE1->fn) && (data != cBE1->data) && cBE1->next)
    {
      cBE0 = cBE1;
      cBE1 = cBE1->next;
    }
    if((fn == cBE1->fn) && (data == cBE1->data))
    {
      if(cBE0)
      {
        cBE0->next = cBE1->next;
      }
      else
      {
        model->res.callbacks = NULL;
      }
      AlcFree(cBE1);
    }
  }
}

/*!
* \return	<void>
* \brief	Calls all resource allocation callbacks passing on the
*		given model, element and reason along with the data from
*		the callback entry.
* \param	model			The model.
* \param	elm			The callback function.
* \param	reason			The callback data.
*/
static void	WlzGMModelCallResCb(WlzGMModel *model, WlzGMElemP elm,
				    WlzGMCbReason reason)
{
  WlzGMCbEntry	*cBE;

  if(model && elm.core && ((cBE = model->res.callbacks) != NULL))
  {
    do
    {
      (*(cBE->fn))(model, elm, reason, cBE->data);
    } while((cBE = cBE->next) != NULL);
  }
}

/* Creation  of geometric modeling elements */

/*!
* \return				New empty model.
* \ingroup      WlzGeoModel
* \brief	Creates an empty non-manifold geometry model.
* \param	modType			Type of model to create.
* \param	blkSz			Resource block size, used for
*					allocating storage for model
*					elements. A default size is
*					used if <= 0.
* \param	vHTSz			Vertex matching hash table size,
*                                       A default size is used if <= 0.
* \param	dstErr			Destination error pointer, may
*                                       be null.
*/
WlzGMModel	*WlzGMModelNew(WlzGMModelType modType,
			       int blkSz, int vHTSz, WlzErrorNum *dstErr)
{
  WlzGMModel	*model = NULL;
  WlzErrorNum	errNum = WLZ_ERR_NONE;
  unsigned int	vertexGSz,
		shellGSz;
  const unsigned int defBlkSz = 1024;
  const unsigned int defVHTSz = 1024 * 1024;

  if(blkSz <= 0)
  {
    blkSz = defBlkSz;
  }
  if(vHTSz <= 0)
  {
    vHTSz = defVHTSz;
  }
  switch(modType)
  {
    case WLZ_GMMOD_2I:
      vertexGSz = sizeof(WlzGMVertexG2I);
      shellGSz = sizeof(WlzGMShellG2I);
      break;
    case WLZ_GMMOD_2D:
      vertexGSz = sizeof(WlzGMVertexG2D);
      shellGSz = sizeof(WlzGMShellG2D);
      break;
    case WLZ_GMMOD_3I:
      vertexGSz = sizeof(WlzGMVertexG3I);
      shellGSz = sizeof(WlzGMShellG3I);
      break;
    case WLZ_GMMOD_3D:
      vertexGSz = sizeof(WlzGMVertexG3D);
      shellGSz = sizeof(WlzGMShellG3D);
      break;
    default:
      errNum = WLZ_ERR_DOMAIN_TYPE;
      break;
  }
  /* Create the new model. All elements of the model including it's
   * resources are set to zero. */
  if((errNum == WLZ_ERR_NONE) &&
     ((model = AlcCalloc(1, sizeof(WlzGMModel))) == NULL))
  {
    errNum = WLZ_ERR_MEM_ALLOC;
  }
  /* Create initial model resources */
  if(errNum == WLZ_ERR_NONE)
  {
    model->type = modType;
    if(((model->res.vertex.vec = AlcVectorNew(1, sizeof(WlzGMVertex),
    					      blkSz, NULL)) == NULL) ||
       ((model->res.vertexT.vec = AlcVectorNew(1, sizeof(WlzGMVertexT),
    					       blkSz, NULL)) == NULL) ||
       ((model->res.vertexG.vec = AlcVectorNew(1, vertexGSz,
    					       blkSz, NULL)) == NULL) ||
       ((model->res.diskT.vec = AlcVectorNew(1, sizeof(WlzGMDiskT),
    					     blkSz, NULL)) == NULL) ||
       ((model->res.edge.vec = AlcVectorNew(1, sizeof(WlzGMEdge),
    					    blkSz, NULL)) == NULL) ||
       ((model->res.edgeT.vec = AlcVectorNew(1, sizeof(WlzGMEdgeT),
    					     blkSz, NULL)) == NULL) ||
       ((model->res.face.vec = AlcVectorNew(1, sizeof(WlzGMFace),
    					    blkSz, NULL)) == NULL) ||
       ((model->res.loopT.vec = AlcVectorNew(1, sizeof(WlzGMLoopT),
    					     blkSz, NULL)) == NULL) ||
       ((model->res.shell.vec = AlcVectorNew(1, sizeof(WlzGMShell),
    					     blkSz, NULL)) == NULL) ||
       ((model->res.shellG.vec = AlcVectorNew(1, shellGSz,
    					      blkSz, NULL)) == NULL))
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  /* Create vertex hash table. */
  if(errNum == WLZ_ERR_NONE)
  {
    if((model->vertexHT = (WlzGMVertex **)
    			  AlcCalloc(vHTSz, sizeof(WlzGMVertex *))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
    else
    {
      model->vertexHTSz = vHTSz;
    }
  }
  /* Clear up on error */
  if(errNum != WLZ_ERR_NONE)
  {
    (void )WlzGMModelFree(model);
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(model);
}

/*!
* \return				New empty shell.
* \ingroup      WlzGeoModel
* \brief	Creates an empty shell with a shell geometry element
*               for the model.
* \param	model			The geometric model.
* \param	dstErr			Destination error pointer, may
*                                       be null.
*/
WlzGMShell	*WlzGMModelNewS(WlzGMModel *model, WlzErrorNum *dstErr)
{
  WlzGMResource	*resS,
  		*resSG;
  WlzGMShell	*shell = NULL;
  WlzGMElemP	elm;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    /* Get a new shell and shell geometry element from the shell */
    resS = &(model->res.shell);
    resSG = &(model->res.shellG);
    if(((shell = (WlzGMShell *)
     	        (AlcVectorExtendAndGet(resS->vec, resS->numIdx))) == NULL) ||
       ((shell->geo.core = (WlzGMCore *)
    			   (AlcVectorExtendAndGet(resSG->vec,
					 	  resSG->numIdx))) == NULL))
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    ++(resS->numElm);
    ++(resSG->numElm);
    shell->type = WLZ_GMELM_SHELL;
    shell->idx = (resS->numIdx)++;
    shell->geo.core->type = WlzGMModelGetSGeomType(model);
    shell->geo.core->idx = (resSG->numIdx)++;
    elm.shell = shell;
    WlzGMModelCallResCb(model, elm, WLZ_GMCB_NEW);
  }
  /* Clear up on error */
  if(errNum != WLZ_ERR_NONE)
  {
    (void )WlzGMModelFreeS(model, shell);
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(shell);
}

/*!
* \return				New face.
* \ingroup      WlzGeoModel
* \brief	Creates an empty face.
* \param	model			Parent model.
* \param	dstErr			Destination error pointer, may
*                                       be null.
*/
WlzGMFace	*WlzGMModelNewF(WlzGMModel *model, WlzErrorNum *dstErr)
{
  WlzGMResource	*resF;
  WlzGMFace	*face = NULL;
  WlzGMElemP	elm;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    /* Get a new face element from the model */
    resF = &(model->res.face);
    if((face = (WlzGMFace *)
     	       (AlcVectorExtendAndGet(resF->vec, resF->numIdx))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    ++(resF->numElm);
    face->type = WLZ_GMELM_FACE;
    face->idx = (resF->numIdx)++;
    elm.face = face;
    WlzGMModelCallResCb(model, elm, WLZ_GMCB_NEW);
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(face);
}

/*!
* \return				New loop topology element.
* \ingroup      WlzGeoModel
* \brief	Creates a loop topology element.
* \param	model			Parent model.
* \param	dstErr			Destination error pointer, may
*					be null.
*/
WlzGMLoopT	*WlzGMModelNewLT(WlzGMModel *model, WlzErrorNum *dstErr)
{
  WlzGMResource	*resLT;
  WlzGMLoopT	*loopT = NULL;
  WlzGMElemP	elm;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    /* Get a new loop topology element from the model */
    resLT = &(model->res.loopT);
    if((loopT = (WlzGMLoopT *)
     	        (AlcVectorExtendAndGet(resLT->vec, resLT->numIdx))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    ++(resLT->numElm);
    loopT->type = WLZ_GMELM_LOOP_T;
    loopT->idx = (resLT->numIdx)++;
    elm.loopT = loopT;
    WlzGMModelCallResCb(model, elm, WLZ_GMCB_NEW);
  }
  /* Clear up on error */
  if(errNum != WLZ_ERR_NONE)
  {
    (void )WlzGMModelFreeLT(model, loopT);
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(loopT);
}

/*!
* \return				New disk topology element.
* \ingroup      WlzGeoModel
* \brief	Creates a new disk topology element.
* \param	model			Model with resources.
* \param	dstErr			Destination error pointer, may
*                                       be null.
*/
WlzGMDiskT     *WlzGMModelNewDT(WlzGMModel *model, WlzErrorNum *dstErr)
{
  WlzGMResource	*resDT;
  WlzGMDiskT	*diskT = NULL;
  WlzGMElemP	elm;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    /* Get a new disk topology element from the model */
    resDT = &(model->res.diskT);
    if((diskT = (WlzGMDiskT *)
     	        (AlcVectorExtendAndGet(resDT->vec, resDT->numIdx))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    ++(resDT->numElm);
    diskT->type = WLZ_GMELM_DISK_T;
    diskT->idx = (resDT->numIdx)++;
    elm.diskT = diskT;
    WlzGMModelCallResCb(model, elm, WLZ_GMCB_NEW);
  }
  /* Clear up on error */
  if(errNum != WLZ_ERR_NONE)
  {
    (void )WlzGMModelFreeDT(model, diskT);
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(diskT);
}

/*!
* \return				New edge.
* \ingroup      WlzGeoModel
* \brief	Creates a new edge and an edge geometry element.
*               The edge geometry element only has it's index set
*               to a meaningful value.
* \param	model			Model with resources.
* \param	dstErr			Destination error pointer, may
*                                       be null.
*/
WlzGMEdge      *WlzGMModelNewE(WlzGMModel *model, WlzErrorNum *dstErr)
{
  WlzGMResource	*resE;
  WlzGMEdge	*edge = NULL;
  WlzGMElemP	elm;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    /* Get a new edge and edge geometry element from the model */
    resE = &(model->res.edge);
    if((edge = (WlzGMEdge *)
     	       (AlcVectorExtendAndGet(resE->vec, resE->numIdx))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    ++(resE->numElm);
    edge->type = WLZ_GMELM_EDGE;
    edge->idx = (model->res.edge.numIdx)++;
    elm.edge = edge;
    WlzGMModelCallResCb(model, elm, WLZ_GMCB_NEW);
  }
  /* Clear up on error */
  if(errNum != WLZ_ERR_NONE)
  {
    (void )WlzGMModelFreeE(model, edge);
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(edge);
}

/*!
* \return				New edge topology element.
* \ingroup      WlzGeoModel
* \brief	Creates a new edge topology element.
* \param	model			Model with resources.
* \param	dstErr			Destination error pointer, may
*                                       be null.
*/
WlzGMEdgeT     *WlzGMModelNewET(WlzGMModel *model, WlzErrorNum *dstErr)
{
  WlzGMResource	*resET;
  WlzGMEdgeT	*edgeT = NULL;
  WlzGMElemP	elm;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    /* Get a new edge topology element from the model */
    resET = &(model->res.edgeT);
    if((edgeT = (WlzGMEdgeT *)
     	        (AlcVectorExtendAndGet(resET->vec, resET->numIdx))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    ++(resET->numElm);
    edgeT->type = WLZ_GMELM_EDGE_T;
    edgeT->idx = (resET->numIdx)++;
    elm.edgeT = edgeT;
    WlzGMModelCallResCb(model, elm, WLZ_GMCB_NEW);
  }
  /* Clear up on error */
  if(errNum != WLZ_ERR_NONE)
  {
    (void )WlzGMModelFreeET(model, edgeT);
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(edgeT);
}

/*!
* \return				New vertex.
* \ingroup      WlzGeoModel
* \brief	Creates a new vertex and a vertex geometry element.
*               The vertex geometry element only has it's index set
*               to a meaningful value.
* \param	model			Model with resources.
* \param	dstErr			Destination error pointer, may
*                                       be null.
*/
WlzGMVertex      *WlzGMModelNewV(WlzGMModel *model, WlzErrorNum *dstErr)
{
  WlzGMResource	*resV,
  		*resVG;
  WlzGMVertex	*vertex = NULL;
  WlzGMElemP	elm;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    /* Get a new vertex and vertex geometry element from the model */
    resV = &(model->res.vertex);
    resVG = &(model->res.vertexG);
    if(((vertex = (WlzGMVertex *)
     	        (AlcVectorExtendAndGet(resV->vec, resV->numIdx))) == NULL) ||
       ((vertex->geo.core = (WlzGMCore *)
    			    (AlcVectorExtendAndGet(resVG->vec,
					 	   resVG->numIdx))) == NULL))
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    ++(resV->numElm);
    ++(resVG->numElm);
    vertex->type = WLZ_GMELM_VERTEX;
    vertex->idx = (resV->numIdx)++;
    vertex->geo.core->type = WlzGMModelGetVGeomType(model);
    vertex->geo.core->idx = (resVG->numIdx)++;
    elm.vertex = vertex;
    WlzGMModelCallResCb(model, elm, WLZ_GMCB_NEW);
  }
  /* Clear up on error */
  if(errNum != WLZ_ERR_NONE)
  {
    (void )WlzGMModelFreeV(model, vertex);
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(vertex);
}

/*!
* \return				New vertex topology element.
* \ingroup      WlzGeoModel
* \brief	Creates a new vertex topology element.
* \param	model			Model with resources.
* \param	dstErr			Destination error pointer, may
*                                       be null.
*/
WlzGMVertexT      *WlzGMModelNewVT(WlzGMModel *model, WlzErrorNum *dstErr)
{
  WlzGMResource	*resVT;
  WlzGMVertexT	*vertexT = NULL;
  WlzGMElemP	elm;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    /* Get a new vertex topology element from the model */
    resVT = &(model->res.vertexT);
    if((vertexT = (WlzGMVertexT *)
     	          (AlcVectorExtendAndGet(resVT->vec, resVT->numIdx))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    ++(resVT->numElm);
    vertexT->type = WLZ_GMELM_VERTEX_T;
    vertexT->idx = (resVT->numIdx)++;
    elm.vertexT = vertexT;
    WlzGMModelCallResCb(model, elm, WLZ_GMCB_NEW);
  }
  /* Clear up on error */
  if(errNum != WLZ_ERR_NONE)
  {
    (void )WlzGMModelFreeVT(model, vertexT);
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(vertexT);
}

/*!
* \return				New copy of the given model.
* \ingroup      WlzGeoModel
* \brief	Copies the given model. All unused elements are squeezed
*		out.
* \param	gM			Given model.
* \param	dstErr			Destination error pointer, may
*					be null.
*/
WlzGMModel 	*WlzGMModelCopy(WlzGMModel *gM, WlzErrorNum *dstErr)
{
  int		gIdx,
  		nIdx,
  		nBkSz,
		nHTSz;
  WlzGMElemP	gElmP,
  		nElmP;
  WlzGMResIdxTb *resIdxTb = NULL;
  const int	minBkSz = 1024,
  		minHTSz = 1024;
  WlzGMModel 	*nM = NULL;
  WlzErrorNum   errNum = WLZ_ERR_NONE;

  if(gM == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    /* Make a new model. */
    if((nBkSz = gM->res.vertex.numElm / 16) < minBkSz)
    {
      nBkSz = minBkSz;
    }
    if((nHTSz = gM->res.vertex.numElm / 8) < minHTSz)
    {
      nHTSz = minHTSz;
    }
    nM = WlzGMModelNew(gM->type, nBkSz, nHTSz, &errNum);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    /* Compute element index tables to be used in setting up the elements
     * of the new model while sqeezing out the unused elements. */
    resIdxTb = WlzGMModelResIdx(gM,
				WLZ_GMELMFLG_VERTEX |
				WLZ_GMELMFLG_VERTEX_G |
				WLZ_GMELMFLG_VERTEX_T |
				WLZ_GMELMFLG_DISK_T |
				WLZ_GMELMFLG_EDGE |
				WLZ_GMELMFLG_EDGE_T |
				WLZ_GMELMFLG_FACE |
				WLZ_GMELMFLG_LOOP_T |
				WLZ_GMELMFLG_SHELL |
				WLZ_GMELMFLG_SHELL_G,
    				&errNum);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    /* Copy verticies. */
    gIdx = 0;
    while((errNum == WLZ_ERR_NONE) && (gIdx < gM->res.vertex.numIdx))
    {
      gElmP.core = (WlzGMCore *)AlcVectorItemGet(gM->res.vertex.vec, gIdx);
      if((gElmP.core != NULL) && (gElmP.core->idx >= 0))
      {
	nIdx = *(resIdxTb->vertex.idxLut + gIdx);
        if((nElmP.core = (WlzGMCore *)AlcVectorExtendAndGet(nM->res.vertex.vec,
							 nIdx)) == NULL)
	{
	  errNum = WLZ_ERR_MEM_ALLOC;
	}
	else
	{
	  nElmP.core->type = WLZ_GMELM_VERTEX;
	  nElmP.core->idx = nIdx;
	  nElmP.vertex->diskT = (WlzGMDiskT *)
	  			AlcVectorExtendAndGet(nM->res.diskT.vec,
						 *(resIdxTb->diskT.idxLut + 
						   gElmP.vertex->diskT->idx));
	  nElmP.vertex->geo.core = (WlzGMCore *)
	  			   AlcVectorExtendAndGet(nM->res.vertexG.vec,
						 *(resIdxTb->vertexG.idxLut + 
						 gElmP.vertex->geo.core->idx));
	  if(gElmP.vertex->next)
	  {
	    nElmP.vertex->next = (WlzGMVertex *)
				 AlcVectorExtendAndGet(nM->res.vertex.vec,
						  *(resIdxTb->vertex.idxLut +
						    gElmP.vertex->next->idx));
	  }
	  else
	  {
	    nElmP.vertex->next = NULL;
	  }
	  if((nElmP.vertex->diskT == NULL) ||
	     (nElmP.vertex->geo.core == NULL))
	  {
	    errNum = WLZ_ERR_MEM_ALLOC;
	  }
	}
      }
      ++gIdx;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    /* Copy vertex geometries */
    gIdx = 0;
    while((errNum == WLZ_ERR_NONE) && (gIdx < gM->res.vertexG.numIdx))
    {
      gElmP.core = (WlzGMCore *)AlcVectorItemGet(gM->res.vertexG.vec, gIdx);
      if((gElmP.core != NULL) && (gElmP.core->idx >= 0))
      {
	nIdx = *(resIdxTb->vertexG.idxLut + gIdx);
	if((nElmP.core = (WlzGMCore *)AlcVectorExtendAndGet(nM->res.vertexG.vec,
						       nIdx)) == NULL)
	{
	  errNum = WLZ_ERR_MEM_ALLOC;
	}
	else
	{
	  nElmP.core->type = gElmP.core->type;
	  nElmP.core->idx = nIdx;
	  switch(nElmP.core->type)
	  {
	    case WLZ_GMELM_VERTEX_G2I:
	      nElmP.vertexG2I->vtx = gElmP.vertexG2I->vtx;
	      break;
	    case WLZ_GMELM_VERTEX_G2D:
	      nElmP.vertexG2D->vtx = gElmP.vertexG2D->vtx;
	      break;
	    case WLZ_GMELM_VERTEX_G3I:
	      nElmP.vertexG3I->vtx = gElmP.vertexG3I->vtx;
	      break;
	    case WLZ_GMELM_VERTEX_G3D:
	      nElmP.vertexG3D->vtx = gElmP.vertexG3D->vtx;
	      break;
	  }
        }
      }
      ++gIdx;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    /* Copy vertexT's */
    gIdx = 0;
    while((errNum == WLZ_ERR_NONE) && (gIdx < gM->res.vertexT.numIdx))
    {
      gElmP.core = (WlzGMCore *)AlcVectorItemGet(gM->res.vertexT.vec, gIdx);
      if((gElmP.core != NULL) && (gElmP.core->idx >= 0))
      {
	nIdx = *(resIdxTb->vertexT.idxLut + gIdx);
	if((nElmP.core = (WlzGMCore *)AlcVectorExtendAndGet(nM->res.vertexT.vec,
						       nIdx)) == NULL)
	{
	  errNum = WLZ_ERR_MEM_ALLOC;
	}
	else
	{
	  nElmP.core->type = gElmP.core->type;
	  nElmP.core->idx = nIdx;
	  nElmP.vertexT->next = (WlzGMVertexT *)
	  			AlcVectorExtendAndGet(nM->res.vertexT.vec,
						 *(resIdxTb->vertexT.idxLut +
						   gElmP.vertexT->next->idx));
	  nElmP.vertexT->prev = (WlzGMVertexT *)
	  			AlcVectorExtendAndGet(nM->res.vertexT.vec,
						 *(resIdxTb->vertexT.idxLut +
						   gElmP.vertexT->prev->idx));
	  nElmP.vertexT->diskT = (WlzGMDiskT *)
	  			 AlcVectorExtendAndGet(nM->res.diskT.vec,
						 *(resIdxTb->diskT.idxLut +
						   gElmP.vertexT->diskT->idx));
	  nElmP.vertexT->parent = (WlzGMEdgeT *)
	  			  AlcVectorExtendAndGet(nM->res.edgeT.vec,
						 *(resIdxTb->edgeT.idxLut +
						  gElmP.vertexT->parent->idx));
	  if((nElmP.vertexT->next == NULL) ||
	     (nElmP.vertexT->prev == NULL) ||
	     (nElmP.vertexT->diskT == NULL) ||
	     (nElmP.vertexT->parent == NULL))
	  {
	    errNum = WLZ_ERR_MEM_ALLOC;
	  }
        }
      }
      ++gIdx;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    /* Copy diskT's */
    gIdx = 0;
    while((errNum == WLZ_ERR_NONE) && (gIdx < gM->res.diskT.numIdx))
    {
      gElmP.core = (WlzGMCore *)AlcVectorItemGet(gM->res.diskT.vec, gIdx);
      if((gElmP.core != NULL) && (gElmP.core->idx >= 0))
      {
	nIdx = *(resIdxTb->diskT.idxLut + gIdx);
	if((nElmP.core = (WlzGMCore *)AlcVectorExtendAndGet(nM->res.diskT.vec,
						       nIdx)) == NULL)
	{
	  errNum = WLZ_ERR_MEM_ALLOC;
	}
	else
	{
	  nElmP.core->type = gElmP.core->type;
	  nElmP.core->idx = nIdx;
	  nElmP.diskT->next = (WlzGMDiskT *)
	  		      AlcVectorExtendAndGet(nM->res.diskT.vec,
					       *(resIdxTb->diskT.idxLut +
						 gElmP.diskT->next->idx));
	  nElmP.diskT->prev = (WlzGMDiskT *)
			      AlcVectorExtendAndGet(nM->res.diskT.vec,
					       *(resIdxTb->diskT.idxLut +
					         gElmP.diskT->prev->idx));
	  nElmP.diskT->vertex = (WlzGMVertex *)
	  			 AlcVectorExtendAndGet(nM->res.vertex.vec,
						 *(resIdxTb->vertex.idxLut +
						   gElmP.diskT->vertex->idx));
	  nElmP.diskT->vertexT = (WlzGMVertexT *)
	  			  AlcVectorExtendAndGet(nM->res.vertexT.vec,
						 *(resIdxTb->vertexT.idxLut +
						  gElmP.diskT->vertexT->idx));
	  if((nElmP.diskT->next == NULL) ||
	     (nElmP.diskT->prev == NULL) ||
	     (nElmP.diskT->vertex == NULL) ||
	     (nElmP.diskT->vertexT == NULL))
	  {
	    errNum = WLZ_ERR_MEM_ALLOC;
	  }
        }
      }
      ++gIdx;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    /* Copy edge's */
    gIdx = 0;
    while((errNum == WLZ_ERR_NONE) && (gIdx < gM->res.edge.numIdx))
    {
      gElmP.core = (WlzGMCore *)AlcVectorItemGet(gM->res.edge.vec, gIdx);
      if((gElmP.core != NULL) && (gElmP.core->idx >= 0))
      {
	nIdx = *(resIdxTb->edge.idxLut + gIdx);
	if((nElmP.core = (WlzGMCore *)AlcVectorExtendAndGet(nM->res.edge.vec,
						       nIdx)) == NULL)
	{
	  errNum = WLZ_ERR_MEM_ALLOC;
	}
	else
	{
	  nElmP.core->type = gElmP.core->type;
	  nElmP.core->idx = nIdx;
	  nElmP.edge->edgeT = (WlzGMEdgeT *)
	  		      AlcVectorExtendAndGet(nM->res.edgeT.vec,
					       *(resIdxTb->edgeT.idxLut +
						 gElmP.edge->edgeT->idx));
	  if(nElmP.edge->edgeT == NULL)
	  {
	    errNum = WLZ_ERR_MEM_ALLOC;
	  }
        }
      }
      ++gIdx;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    /* Copy edgeT's */
    gIdx = 0;
    while((errNum == WLZ_ERR_NONE) && (gIdx < gM->res.edgeT.numIdx))
    {
      gElmP.core = (WlzGMCore *)AlcVectorItemGet(gM->res.edgeT.vec, gIdx);
      if((gElmP.core != NULL) && (gElmP.core->idx >= 0))
      {
	nIdx = *(resIdxTb->edgeT.idxLut + gIdx);
	if((nElmP.core = (WlzGMCore *)AlcVectorExtendAndGet(nM->res.edgeT.vec,
						       nIdx)) == NULL)
	{
	  errNum = WLZ_ERR_MEM_ALLOC;
	}
	else
	{
	  nElmP.core->type = gElmP.core->type;
	  nElmP.core->idx = nIdx;
	  nElmP.edgeT->next = (WlzGMEdgeT *)
	  		       AlcVectorExtendAndGet(nM->res.edgeT.vec,
					       *(resIdxTb->edgeT.idxLut +
						 gElmP.edgeT->next->idx));
	  nElmP.edgeT->prev = (WlzGMEdgeT *)
	  		       AlcVectorExtendAndGet(nM->res.edgeT.vec,
					       *(resIdxTb->edgeT.idxLut +
						 gElmP.edgeT->prev->idx));
	  nElmP.edgeT->opp = (WlzGMEdgeT *)
	  		     AlcVectorExtendAndGet(nM->res.edgeT.vec,
					      *(resIdxTb->edgeT.idxLut +
					        gElmP.edgeT->opp->idx));
	  nElmP.edgeT->rad = (WlzGMEdgeT *)
	  		       AlcVectorExtendAndGet(nM->res.edgeT.vec,
					       *(resIdxTb->edgeT.idxLut +
						 gElmP.edgeT->rad->idx));
	  nElmP.edgeT->edge = (WlzGMEdge *)
	  		      AlcVectorExtendAndGet(nM->res.edge.vec,
					       *(resIdxTb->edge.idxLut +
						 gElmP.edgeT->edge->idx));
	  nElmP.edgeT->vertexT = (WlzGMVertexT *)
	  		         AlcVectorExtendAndGet(nM->res.vertexT.vec,
					          *(resIdxTb->vertexT.idxLut +
						   gElmP.edgeT->vertexT->idx));
	  nElmP.edgeT->parent = (WlzGMLoopT *)
	  		        AlcVectorExtendAndGet(nM->res.loopT.vec,
					       *(resIdxTb->loopT.idxLut +
						 gElmP.edgeT->parent->idx));

	  if((nElmP.edgeT->next == NULL) || 
	     (nElmP.edgeT->prev == NULL) || 
	     (nElmP.edgeT->opp == NULL) || 
	     (nElmP.edgeT->rad == NULL) || 
	     (nElmP.edgeT->edge == NULL) || 
	     (nElmP.edgeT->vertexT == NULL) || 
	     (nElmP.edgeT->parent == NULL))
	  {
	    errNum = WLZ_ERR_MEM_ALLOC;
	  }
        }
      }
      ++gIdx;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    /* Copy faces */
    gIdx = 0;
    while((errNum == WLZ_ERR_NONE) && (gIdx < gM->res.face.numIdx))
    {
      gElmP.core = (WlzGMCore *)AlcVectorItemGet(gM->res.face.vec, gIdx);
      if((gElmP.core != NULL) && (gElmP.core->idx >= 0))
      {
	nIdx = *(resIdxTb->face.idxLut + gIdx);
	if((nElmP.core = (WlzGMCore *)AlcVectorExtendAndGet(nM->res.face.vec,
						       nIdx)) == NULL)
	{
	  errNum = WLZ_ERR_MEM_ALLOC;
	}
	else
	{
	  nElmP.core->type = gElmP.core->type;
	  nElmP.core->idx = nIdx;
	  nElmP.face->loopT = (WlzGMLoopT *)
	  		      AlcVectorExtendAndGet(nM->res.loopT.vec,
					       *(resIdxTb->loopT.idxLut +
						 gElmP.face->loopT->idx));
	  if(nElmP.face->loopT == NULL)
	  {
	    errNum = WLZ_ERR_MEM_ALLOC;
	  }
        }
      }
      ++gIdx;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    /* Copy loopT's */
    gIdx = 0;
    while((errNum == WLZ_ERR_NONE) && (gIdx < gM->res.loopT.numIdx))
    {
      gElmP.core = (WlzGMCore *)AlcVectorItemGet(gM->res.loopT.vec, gIdx);
      if((gElmP.core != NULL) && (gElmP.core->idx >= 0))
      {
	nIdx = *(resIdxTb->loopT.idxLut + gIdx);
	if((nElmP.core = (WlzGMCore *)AlcVectorExtendAndGet(nM->res.loopT.vec,
						       nIdx)) == NULL)
	{
	  errNum = WLZ_ERR_MEM_ALLOC;
	}
	else
	{
	  nElmP.core->type = gElmP.core->type;
	  nElmP.core->idx = nIdx;
	  nElmP.loopT->next = (WlzGMLoopT *)
	  		      AlcVectorExtendAndGet(nM->res.loopT.vec,
					       *(resIdxTb->loopT.idxLut +
						 gElmP.loopT->next->idx));
	  nElmP.loopT->prev = (WlzGMLoopT *)
	  		      AlcVectorExtendAndGet(nM->res.loopT.vec,
					       *(resIdxTb->loopT.idxLut +
						 gElmP.loopT->prev->idx));
	  nElmP.loopT->opp = (WlzGMLoopT *)
	  		      AlcVectorExtendAndGet(nM->res.loopT.vec,
					       *(resIdxTb->loopT.idxLut +
						 gElmP.loopT->opp->idx));
	  nElmP.loopT->face = (WlzGMFace *)
	  		      AlcVectorExtendAndGet(nM->res.face.vec,
					       *(resIdxTb->face.idxLut +
						 gElmP.loopT->face->idx));
	  nElmP.loopT->edgeT = (WlzGMEdgeT *)
	  		      AlcVectorExtendAndGet(nM->res.edgeT.vec,
					       *(resIdxTb->edgeT.idxLut +
						 gElmP.loopT->edgeT->idx));
	  nElmP.loopT->parent = (WlzGMShell *)
				AlcVectorExtendAndGet(nM->res.shell.vec,
						 *(resIdxTb->shell.idxLut +
						   gElmP.loopT->parent->idx));
	  if((nElmP.loopT->next == NULL) ||
	     (nElmP.loopT->prev == NULL) ||
	     (nElmP.loopT->opp == NULL) ||
	     (nElmP.loopT->face == NULL) ||
	     (nElmP.loopT->edgeT == NULL) ||
	     (nElmP.loopT->parent == NULL))
	  {
	    errNum = WLZ_ERR_MEM_ALLOC;
	  }
        }
      }
      ++gIdx;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    /* Copy shell's */
    gIdx = 0;
    while((errNum == WLZ_ERR_NONE) && (gIdx < gM->res.shell.numIdx))
    {
      gElmP.core = (WlzGMCore *)AlcVectorItemGet(gM->res.shell.vec, gIdx);
      if((gElmP.core != NULL) && (gElmP.core->idx >= 0))
      {
	nIdx = *(resIdxTb->shell.idxLut + gIdx);
	if((nElmP.core = (WlzGMCore *)AlcVectorExtendAndGet(nM->res.shell.vec,
						       nIdx)) == NULL)
	{
	  errNum = WLZ_ERR_MEM_ALLOC;
	}
	else
	{
	  nElmP.core->type = gElmP.core->type;
	  nElmP.core->idx = nIdx;
	  nElmP.shell->next = (WlzGMShell *)
	  		      AlcVectorExtendAndGet(nM->res.shell.vec,
					       *(resIdxTb->shell.idxLut +
						 gElmP.shell->next->idx));
	  nElmP.shell->prev = (WlzGMShell *)
	  		      AlcVectorExtendAndGet(nM->res.shell.vec,
					       *(resIdxTb->shell.idxLut +
						 gElmP.shell->prev->idx));
	  nElmP.shell->geo.core = (WlzGMCore *)
	  		      AlcVectorExtendAndGet(nM->res.shellG.vec,
					       *(resIdxTb->shellG.idxLut +
						 gElmP.shell->geo.core->idx));
	  nElmP.shell->child = (WlzGMLoopT *)
	  		       AlcVectorExtendAndGet(nM->res.loopT.vec,
					       *(resIdxTb->loopT.idxLut +
						 gElmP.shell->child->idx));
	  nElmP.shell->parent = nM;
	  if((nElmP.shell->next == NULL) ||
	     (nElmP.shell->prev == NULL) ||
	     (nElmP.shell->geo.core == NULL) ||
	     (nElmP.shell->child == NULL))
	  {
	    errNum = WLZ_ERR_MEM_ALLOC;
	  }
        }
      }
      ++gIdx;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    /* Copy shellG's */
    gIdx = 0;
    while((errNum == WLZ_ERR_NONE) && (gIdx < gM->res.shellG.numIdx))
    {
      gElmP.core = (WlzGMCore *)AlcVectorItemGet(gM->res.shellG.vec, gIdx);
      if((gElmP.core != NULL) && (gElmP.core->idx >= 0))
      {
	nIdx = *(resIdxTb->shellG.idxLut + gIdx);
	if((nElmP.core = (WlzGMCore *)AlcVectorExtendAndGet(nM->res.shellG.vec,
						       nIdx)) == NULL)
	{
	  errNum = WLZ_ERR_MEM_ALLOC;
	}
	else
	{
	  nElmP.core->type = gElmP.core->type;
	  nElmP.core->idx = nIdx;
	  switch(nElmP.core->type)
	  {
	    case WLZ_GMELM_SHELL_G2I:
	      nElmP.shellG2I->bBox = gElmP.shellG2I->bBox;
	      break;
	    case WLZ_GMELM_SHELL_G2D:
	      nElmP.shellG2D->bBox = gElmP.shellG2D->bBox;
	      break;
	    case WLZ_GMELM_SHELL_G3I:
	      nElmP.shellG3I->bBox = gElmP.shellG3I->bBox;
	      break;
	    case WLZ_GMELM_SHELL_G3D:
	      nElmP.shellG3D->bBox = gElmP.shellG3D->bBox;
	      break;
	  }
        }
      }
      ++gIdx;
    }
  }
  if(resIdxTb)
  {
    WlzGMModelResIdxFree(resIdxTb);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    nM->child = (WlzGMShell *)AlcVectorItemGet(nM->res.shell.vec,
					       *(resIdxTb->shell.idxLut +
						 gM->child->idx));
    nM->res.vertex.numElm = nM->res.vertex.numIdx = gM->res.vertex.numElm;
    nM->res.vertexG.numElm = nM->res.vertexG.numIdx = gM->res.vertexG.numElm;
    nM->res.vertexT.numElm = nM->res.vertexT.numIdx = gM->res.vertexT.numElm;
    nM->res.diskT.numElm = nM->res.diskT.numIdx = gM->res.diskT.numElm;
    nM->res.edge.numElm = nM->res.edge.numIdx = gM->res.edge.numElm;
    nM->res.edgeT.numElm = nM->res.edgeT.numIdx = gM->res.edgeT.numElm;
    nM->res.face.numElm = nM->res.face.numIdx = gM->res.face.numElm;
    nM->res.loopT.numElm = nM->res.loopT.numIdx = gM->res.loopT.numElm;
    nM->res.shell.numElm = nM->res.shell.numIdx = gM->res.shell.numElm;
    nM->res.shellG.numElm = nM->res.shellG.numIdx = gM->res.shellG.numElm;
    errNum = WlzGMModelRehashVHT(nM, 0);
  }
  if(errNum != WLZ_ERR_NONE)
  {
    (void )WlzGMModelFree(nM);
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(nM);
}

/* Freeing  of geometric modeling elements */

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Free's a geometric model and it's elements.
* \param	model			The shell to free.
*/
WlzErrorNum	WlzGMModelFree(WlzGMModel *model)
{
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    errNum = WlzGMModelTypeValid(model->type);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    (void )AlcVectorFree(model->res.vertex.vec);
    (void )AlcVectorFree(model->res.vertexT.vec);
    (void )AlcVectorFree(model->res.vertexG.vec);
    (void )AlcVectorFree(model->res.diskT.vec);
    (void )AlcVectorFree(model->res.edge.vec);
    (void )AlcVectorFree(model->res.edgeT.vec);
    (void )AlcVectorFree(model->res.face.vec);
    (void )AlcVectorFree(model->res.loopT.vec);
    (void )AlcVectorFree(model->res.shell.vec);
    (void )AlcVectorFree(model->res.shellG.vec);
    if(model->vertexHT)
    {
      AlcFree(model->vertexHT);
    }
    AlcFree((void *)model);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Marks an element as free by setting it's index to
*               a -ve value, the actual choice of value aids debugging.
* \param	idxP			Pointer to elements index.
*/
static void	WlzGMElmMarkFree(int *idxP)
{
  if(*idxP >= 0)
  {
    *idxP = -(*idxP + 1);
  }
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Marks a shell and it's geometry as invalid and suitable
*               for reclaiming.
* \param	model			Model with resources.
* \param	shell			Shell to free.
*/
WlzErrorNum	WlzGMModelFreeS(WlzGMModel *model, WlzGMShell *shell)
{
  WlzGMElemP	elm;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((model == NULL) || (shell == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    errNum = WlzGMModelTypeValid(model->type);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    elm.shell = shell;
    WlzGMModelCallResCb(model, elm, WLZ_GMCB_FREE);
    /* Can't really free the shell so just mark it and it's geometry element
     * as invalid by making the indicies < 0. */
    WlzGMElmMarkFree(&(shell->idx));
    if(shell->geo.core != NULL)
    {
      WlzGMElmMarkFree(&(shell->geo.core->idx));
      --(model->res.shellG.numElm);
    }
    --(model->res.shell.numElm);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Marks a face as invalid and suitable for reclaiming.
* \param	model			Model with resources.
* \param	face			Face to free.
*/
WlzErrorNum	WlzGMModelFreeF(WlzGMModel *model, WlzGMFace *face)
{
  WlzGMElemP	elm;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((model == NULL) || (face == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    errNum = WlzGMModelTypeValid(model->type);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    elm.face = face;
    WlzGMModelCallResCb(model, elm, WLZ_GMCB_FREE);
    /* Can't really free the face so just mark it as invalid by making
     * the indicies < 0. */
    WlzGMElmMarkFree(&(face->idx));
    --(model->res.face.numElm);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Marks a loop topology as invalid and suitable
*               for reclaiming.
* \param	model			Model with resources.
* \param	loopT			LoopT to free.
*/
WlzErrorNum	WlzGMModelFreeLT(WlzGMModel *model, WlzGMLoopT *loopT)
{
  WlzGMElemP	elm;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((model == NULL) || (loopT == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    errNum = WlzGMModelTypeValid(model->type);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    elm.loopT = loopT;
    WlzGMModelCallResCb(model, elm, WLZ_GMCB_FREE);
    /* Can't really free the loopT so just mark it as invalid by making
     * the index < 0. */
    WlzGMElmMarkFree(&(loopT->idx));
    --(model->res.loopT.numElm);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Marks an disk topology as invalid and suitable
*               for reclaiming.
* \param	model			Model with resources.
* \param	diskT			DiskT to free.
*/
WlzErrorNum	WlzGMModelFreeDT(WlzGMModel *model, WlzGMDiskT *diskT)
{
  WlzGMElemP	elm;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((model == NULL) || (diskT == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    errNum = WlzGMModelTypeValid(model->type);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    elm.diskT = diskT;
    WlzGMModelCallResCb(model, elm, WLZ_GMCB_FREE);
    /* Can't really free the diskT so just mark it as invalid by making
     * the index < 0. */
    WlzGMElmMarkFree(&(diskT->idx));
    --(model->res.diskT.numElm);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Marks an edge as invalid and suitable for reclaiming.
* \param	model			Model with resources.
* \param	edge			Edge to free.
*/
WlzErrorNum	WlzGMModelFreeE(WlzGMModel *model, WlzGMEdge *edge)
{
  WlzGMElemP	elm;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((model == NULL) || (edge == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    errNum = WlzGMModelTypeValid(model->type);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    elm.edge = edge;
    WlzGMModelCallResCb(model, elm, WLZ_GMCB_FREE);
    /* Can't really free the edge so just mark it as invalid by making
     * the indicies < 0. */
    WlzGMElmMarkFree(&(edge->idx));
    --(model->res.edge.numElm);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Marks an edge topology as invalid and suitable for
*		reclaiming.
* \param	model			Model with resources.
* \param	edgeT			EdgeT to free.
*/
WlzErrorNum	WlzGMModelFreeET(WlzGMModel *model, WlzGMEdgeT *edgeT)
{
  WlzGMElemP	elm;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((model == NULL) || (edgeT == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    errNum = WlzGMModelTypeValid(model->type);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    elm.edgeT = edgeT;
    WlzGMModelCallResCb(model, elm, WLZ_GMCB_FREE);
    /* Can't really free the edgeT so just mark it as invalid by making
     * the index < 0. */
    WlzGMElmMarkFree(&(edgeT->idx));
    --(model->res.edgeT.numElm);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Marks a vertex and it's geometry as invalid and suitable
*               for reclaiming.
* \param	model			Model with resources.
* \param	vertex			Vertex to free.
*/
WlzErrorNum	WlzGMModelFreeV(WlzGMModel *model, WlzGMVertex *vertex)
{
  WlzGMElemP	elm;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((model == NULL) || (vertex == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    errNum = WlzGMModelTypeValid(model->type);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    elm.vertex = vertex;
    WlzGMModelCallResCb(model, elm, WLZ_GMCB_FREE);
    /* Can't really free the vertex so just mark it and it's geometry element
     * as invalid by making the indicies < 0. */
    WlzGMElmMarkFree(&(vertex->idx));
    if(vertex->geo.core != NULL)
    {
      WlzGMElmMarkFree(&(vertex->geo.core->idx));
      --(model->res.vertexG.numElm);
    }
    --(model->res.vertex.numElm);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Marks a vertex topology as invalid and suitable for
*		reclaiming.
* \param	model			Model with resources.
* \param	vertexT			VertexT to free.
*/
WlzErrorNum	WlzGMModelFreeVT(WlzGMModel *model, WlzGMVertexT *vertexT)
{
  WlzGMElemP	elm;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((model == NULL) || (vertexT == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    errNum = WlzGMModelTypeValid(model->type);
  }
  if(errNum == WLZ_ERR_NONE)
  {
    elm.vertexT = vertexT;
    WlzGMModelCallResCb(model, elm, WLZ_GMCB_FREE);
    /* Can't really free the vertexT so just mark it as invalid by making
     * the index < 0. */
    WlzGMElmMarkFree(&(vertexT->idx));
    --(model->res.vertexT.numElm);
  }
  return(errNum);
}

/* Deletion of geometric modeling elements along with children */

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Deletes a shell by unlinking it and then freeing it and
*               all it's children.
* \param	model			Model with resources.
* \param	dS			Shell to delete.
*/
void		WlzGMModelDeleteS(WlzGMModel *model, WlzGMShell *dS)
{
  WlzGMVertexT	*tVT;
  WlzGMEdgeT	*fET,
  		*nET,
		*tET;
  WlzGMLoopT	*fLT,
  		*nLT,
  		*tLT;

  if(model && dS && (dS->idx >= 0))
  {
    /* Unlink the shell. */
    WlzGMShellUnlink(dS);
    /* For each loopT. */
    fLT = tLT = dS->child;
    do
    {
      /* For each edgeT. */
      fET = tET = tLT->edgeT;
      do
      {
	tVT = tET->vertexT;
	if(tVT->diskT->vertex->idx >= 0)
	{
	  (void )WlzGMModelFreeV(model, tVT->diskT->vertex);
	}
	if(tVT->diskT->idx >= 0)
	{
	  (void )WlzGMModelFreeDT(model, tVT->diskT);
	}
	if(tVT->idx >= 0)
	{
	  (void )WlzGMModelFreeVT(model, tVT);
	}
	if(tET->rad->idx >= 0)
	{
	  (void )WlzGMModelFreeET(model, tET->rad);
	}
	if(tET->edge->idx >= 0)
	{
	  (void )WlzGMModelFreeE(model, tET->edge);
        }
	nET = tET->next;
	if(tET->idx >= 0)
	{
	  (void )WlzGMModelFreeET(model, tET);
	}
	tET = nET;
      } while(tET != fET);
      /* Free the loopT and face. */
      nLT = tLT->next;
      if(tLT->face->idx >= 0)
      {
        (void )WlzGMModelFreeF(model, tLT->face);
      }
      (void )WlzGMModelFreeLT(model, tLT);
      tLT = nLT;
    } while(tLT != fLT);
    (void )WlzGMModelFreeS(model, dS);
  }
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Deletes a vertex along with all the elements which depend
*		on it. All elements which depend on the vertex are unlinked
*		and then freed. If the vertex's parents were to depend
*		soley on the vertex then they would be free'd too. However
*		the Woolz geometric models can not hold an isolated
*		vertex.
*		The basic algorithm used is: Build a collection of edges
*		which use the vertex and then delete all edges in the
*		collection. If there are no edges (in the collection) then
*		just unlink and free the vertex.
*		The geometry of the existing and possible new shells may
*		not be correct after deleting a vertex using this function
*		and this should be taken care of by the calling function.
*		TODO This only works for 2D models I need to extend it to
*		3D models.
* \param	model			Model with resources.
* \param	dV			The vertex to delete.
*/
WlzErrorNum	WlzGMModelDeleteV(WlzGMModel *model, WlzGMVertex *dV)
{
  int		eCnt,
  		eMax;
  WlzGMVertexT	*tVT0,
  		*tVT1;
  WlzGMDiskT	*tDT0,
  		*tDT1;
  WlzGMEdge	**edgeCol = NULL;
  const int	eStp = 16;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model && dV && (dV->idx >= 0))
  {
    /* Allocate initial storage for the edge collection. With luck eStp
     * will have been chosen to be some small value that's > than the the
     * number of edges incident with most verticies and the edge collection
     * won't need reallocating. */
    eCnt = 0;
    eMax = eStp;
    if((edgeCol = (WlzGMEdge **)AlcMalloc(sizeof(WlzGMEdge *) * eMax)) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
    else
    {
      /* Build a collection of all the edges that will be destroyed by deleting
       * the vertex. */
      tDT1 = tDT0 = dV->diskT;
      do
      {
	tDT1 = tDT1->next;
	tVT1 = tVT0 = tDT1->vertexT;
	do
	{
	  tVT1 = tVT1->next;
	  if(eCnt >= eMax)
	  {
	    eMax += eStp;
	    if((edgeCol = (WlzGMEdge **)AlcRealloc(edgeCol,
					  sizeof(WlzGMEdge *) * eMax)) == NULL)
	    {
	      errNum = WLZ_ERR_MEM_ALLOC;
	    }
	  }
	  if(errNum == WLZ_ERR_NONE)
	  {
	    *(edgeCol + eCnt++) = tVT1->parent->edge;
	  }
	} while((errNum == WLZ_ERR_NONE) && (tVT1 != tVT0));
      } while((errNum == WLZ_ERR_NONE) && (tDT1 != tDT0));
    }
    if(errNum == WLZ_ERR_NONE)
    {
      if(eCnt == 0)
      {
	/* Unlink the lone vertex (removing it from the hash table)
	 * and then free it. Currently this isn't needed because lone
	 * verticies aren't allowed in the models. */
      }
      else
      {
	/* Delete all the edges in the collection. */
	while(eCnt > 0)
	{
	  errNum = WlzGMModelDeleteE(model, *(edgeCol + --eCnt));
	}
      }
    }
    if(errNum == WLZ_ERR_NONE)
    {
      /* TODO Update the shell's geometry. */
    }
    if(edgeCol)
    {
      AlcFree(edgeCol);
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Deletes an edge along with all the elements which depend
*		on it. All elements which depend on the edge are unlinked
*		and then freed. If the edge's parents depend solely on the
*		edge then they free'd too.
*		Because many of the operations on Woolz geometric model
*		are only implemented for models containing simplicies this
*		function should be used with care for 3D models.
*		The basic algorithm used is: Check to see if the edge is the
*		only edge in a loop lopology element, if so then delete the
*		loop topology element, otherwise delete all elements which
*		depend on the edge and the edge itself.
* \param	model			Model with resources.
* \param	dE			The edge to delete.
*/
WlzErrorNum	WlzGMModelDeleteE(WlzGMModel *model, WlzGMEdge *dE)
{
  int		termEdgFlg;
  WlzGMEdgeT	*tET0,
  		*tET1,
		*tET2,
		*tET3,
		*tET4;
  WlzGMShell	*eS,
  		*nS = NULL;
  WlzGMFace	*nF = NULL;
  WlzGMLoopT	*nLT = NULL;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model && dE && (dE->idx >= 0))
  {
    /* If the edge is either the only edge in it's parent loopT or deleting the
     * edge would leave the loopT consisting of a single shared edge, then
     * delete the loopT. */
    tET0 = dE->edgeT;
    if((tET0 == tET0->rad) && (tET0->prev == tET0->next))
    {
      if(tET0->parent->face)
      {
        WlzGMModelDeleteF(model, tET0->parent->face);
      }
      else
      {
        WlzGMModelDeleteLT(model, tET0->parent);
      }
    }
    else
    {
      /* Delete all edge topology elements that lie along the edge. */
      tET1 = tET0;
      do
      {
        tET1 = tET1->rad;
	/* By deleting an edge we can either split and existing loopT or
	 * joint 2 existing loopT's. */
	tET2 = tET1->opp;
	tET3 = tET1->next;
	tET4 = tET2->next;
	termEdgFlg = (tET2 == tET1->next) || (tET1 == tET2->next);
	WlzGMModelDeleteET(model, tET1);
	WlzGMModelDeleteET(model, tET2);
	if(tET3->parent == tET4->parent) /* A single loopT has been split */
	{
	  /* Update the loop topology elements. */
	  WlzGMLoopTSetT(tET3->parent); /* TODO: Not needed? */
	  /* Either one loopT has been split into two, or a terminal
	   * edge has been removed. */
	  /* If a terminal edge has been removed then there is no change
	   * to the loopT/shell. But if the loopT has been split
	   * into two, creating a new loopT. Possibley
	   * also create a new shell too. */
	  if(!termEdgFlg)
	  {
	    nLT = WlzGMModelNewLT(model, &errNum);
	    if(errNum == WLZ_ERR_NONE)
	    {
	      if(tET3->parent->face)
	      {
	        nF = WlzGMModelNewF(model, &errNum);
	      }
	    }
	    if(errNum == WLZ_ERR_NONE)
	    {
	      /* Set the parent field in all the edgeTs to be the new loopT. */
	      nLT->next = nLT->prev = nLT->opp = nLT;
	      nLT->face = nF;
	      nLT->edgeT = tET4;
	      nLT->parent = tET3->parent->parent;
	      WlzGMLoopTSetT(nLT);
	      /* Look to see if the loopT has edgeT elements which have
	       * opposite elements in a different loopT. */
              if((eS = WlzGMLoopTFindShell(nLT)) != NULL)
	      {
	        nLT->parent = eS;
	      }
	      else
	      {
	        /* Need to create a new shell. */
		if((nS = WlzGMModelNewS(model, &errNum)) != NULL)
		{
		  WlzGMShellAppend(tET3->parent->parent, nS);
		  nS->child = nLT;
		  nS->parent = model;
		  nLT->parent = nS;
		  errNum = WlzGMShellComputeGBB(nS);
		}
	      }
	      if(errNum == WLZ_ERR_NONE)
	      {
	        errNum = WlzGMShellComputeGBB(tET3->parent->parent);
	      }
	    }
	  }
	}
	else				/* Two loopTs have been joined. */
	{
	  /* Unlink and free the unused face and loopT. */
	  WlzGMLoopTUnlink(tET4->parent);
	  if(tET4->parent->face && (tET4->parent == tET4->parent->opp))
	  {
	    WlzGMModelFreeF(model, tET4->parent->face);
	  }
	  WlzGMModelFreeLT(model, tET4->parent);
	  /* Update the loop topology elements. */
	  WlzGMLoopTSetT(tET3->parent);
	}
      } while(tET1 != tET0);
      (void )WlzGMModelFreeE(model, dE);
    }
  }
  return(errNum);
}


/*!
* \return				<void>
* \ingroup	WlzGeoModel
* \brief	Deletes a face along with all the elements which depend
*		on it. All elements which depend on the face are unlinked
*		and then freed. If the parent of the face depends solely
*		on this face then it is free'd too.
* \param	model			Model with resources.
* \param	dF			The face to delete.
*/
void		WlzGMModelDeleteF(WlzGMModel *model, WlzGMFace *dF)
{
  WlzGMEdgeT	*tET0,
  		*tET1;
  WlzGMLoopT	*tLT0,
  		*tLT1;

  if(model && dF && (dF->idx >- 0))
  {
    tLT0 = dF->loopT;
    /* If the face is the only face in it's parent shell then delete the
     * shell. */
    if(tLT0->next == tLT0->opp)
    {
      WlzGMModelDeleteS(model, tLT0->parent);
    }
    else
    {
      /* Delete all loop topology elements of the face. */
      WlzGMModelDeleteLT(model, tLT0->opp);
      WlzGMModelDeleteLT(model, tLT0);
      WlzGMModelFreeF(model, dF);
    }
  }
}

/*!
* \return
* \brief	Deletes a loop topology element along with all edge
*		topology elements which depend on it.
*		If the loop topology element's shell depends solely
*		on this loop topology element then the shell is deleted.
* \param	model
* \param	dLT
*/
static void 	WlzGMModelDeleteLT(WlzGMModel *model, WlzGMLoopT *dLT)
{
  WlzGMEdgeT	*tET0,
  		*tET1;

  if(model && dLT && (dLT->idx >= 0))
  {
    if(dLT->next == dLT)
    {
      WlzGMModelDeleteS(model, dLT->parent);
    }
    else
    {
      tET0 = tET1 = dLT->edgeT;
      do
      {
	tET1 = tET1->next;
	WlzGMModelDeleteET(model, tET1);
      } while(tET1 != tET0);
      WlzGMLoopTUnlink(dLT);
      WlzGMModelFreeLT(model, dLT);
    }
  }
}

/*!
* \return
* \brief	Deletes an edge topology element and all the elements which
* 		depend on it. All elements which depend on the edge topology
* 		element are unlinked and then freed. No parent of the edge
*		topology element is ever freed by this function.
* \param	model			The model.
* \param	dET			The edge topology element to be
* 					deleted.
*/
static void	WlzGMModelDeleteET(WlzGMModel *model, WlzGMEdgeT *dET)
{
  WlzGMVertexT	*tVT0;
  WlzGMDiskT	*tDT0;

  if(model && dET && (dET->idx >= 0))
  {
    tVT0 = dET->vertexT;
    /* If the edge topology element's vertex topology element is the only
     * member of it's disk topology element then delete the disk topology
     * element. */
    if(tVT0 == tVT0->next)
    {
      tDT0 = tVT0->diskT;
      /* If the vertex topology is the only on in the disk topology element
       * then delete the vertex too. */
      if(tDT0 == tDT0->next)
      {
	WlzGMModelRemVertex(model, tDT0->vertex);
	WlzGMModelFreeV(model, tDT0->vertex);
      }
      WlzGMDiskTUnlink(tDT0);
      WlzGMModelFreeDT(model, tDT0);
    }
    WlzGMVertexTUnlink(tVT0);
    WlzGMModelFreeVT(model, tVT0);
    WlzGMEdgeTUnlink(dET);
    WlzGMModelFreeET(model, dET);
  }
}

/* Model access and testing. */

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Checks the model type is valid.
* \param	type			Model type to check.
*/
WlzErrorNum 	WlzGMModelTypeValid(WlzGMModelType type)
{
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  switch(type)
  {
    case WLZ_GMMOD_2I: /* FALLTHROUGH */
    case WLZ_GMMOD_2D: /* FALLTHROUGH */
    case WLZ_GMMOD_3I: /* FALLTHROUGH */
    case WLZ_GMMOD_3D:
      break;
    default:
      errNum = WLZ_ERR_DOMAIN_TYPE;
      break;
  }
  return(errNum);
}

/* Geometry access, update and testing */

/*!
* \return				Shell's geometry type.
* \ingroup      WlzGeoModel
* \brief	Gets the shell's geometry type from the model's type.
* \param	model			The model.
*/
WlzGMElemType 	WlzGMModelGetSGeomType(WlzGMModel *model)
{
  WlzGMElemType	sGType;

  switch(model->type)
  {
    case WLZ_GMMOD_2I:
      sGType = WLZ_GMELM_SHELL_G2I;
      break;
    case WLZ_GMMOD_2D:
      sGType = WLZ_GMELM_SHELL_G2D;
      break;
    case WLZ_GMMOD_3I:
      sGType = WLZ_GMELM_SHELL_G3I;
      break;
    case WLZ_GMMOD_3D:
      sGType = WLZ_GMELM_SHELL_G3D;
      break;
  }
  return(sGType);
}

/*!
* \return				Shell's geometry type.
* \ingroup      WlzGeoModel
* \brief	Gets the verticies geometry type from the model's type.
* \param	model			The model.
*/
WlzGMElemType 	WlzGMModelGetVGeomType(WlzGMModel *model)
{
  WlzGMElemType	vGType;

  switch(model->type)
  {
    case WLZ_GMMOD_2I:
      vGType = WLZ_GMELM_VERTEX_G2I;
      break;
    case WLZ_GMMOD_2D:
      vGType = WLZ_GMELM_VERTEX_G2D;
      break;
    case WLZ_GMMOD_3I:
      vGType = WLZ_GMELM_VERTEX_G3I;
      break;
    case WLZ_GMMOD_3D:
      vGType = WLZ_GMELM_VERTEX_G3D;
      break;
  }
  return(vGType);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Gets a shell's geometry using the given destination
*               pointer to a double precision bounding box.
* \param	shell			Given shell with geometry to
*                                       be set.
* \param	bBox			Given destination pointer to a
*                                       double precision bounding box.
*/
WlzErrorNum	WlzGMShellGetGBB3D(WlzGMShell *shell, WlzDBox3 *bBox)
{
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((shell == NULL) || (shell->geo.core == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else if(bBox == NULL)
  {
    errNum = WLZ_ERR_PARAM_NULL;
  }
  else
  {
    switch(shell->geo.core->type)
    {
      case WLZ_GMELM_SHELL_G2I:
        bBox->xMin = shell->geo.sg2I->bBox.xMin;
        bBox->yMin = shell->geo.sg2I->bBox.yMin;
	bBox->zMin = 0.0;
        bBox->xMax = shell->geo.sg2I->bBox.xMax;
        bBox->yMax = shell->geo.sg2I->bBox.yMax;
	bBox->zMax = 0.0;
	break;
      case WLZ_GMELM_SHELL_G2D:
        bBox->xMin = shell->geo.sg2D->bBox.xMin;
        bBox->yMin = shell->geo.sg2D->bBox.yMin;
	bBox->zMin = 0.0;
        bBox->xMax = shell->geo.sg2D->bBox.xMax;
        bBox->yMax = shell->geo.sg2D->bBox.yMax;
	bBox->zMax = 0.0;
	break;
      case WLZ_GMELM_SHELL_G3I:
        bBox->xMin = shell->geo.sg3I->bBox.xMin;
        bBox->yMin = shell->geo.sg3I->bBox.yMin;
	bBox->zMin = shell->geo.sg3I->bBox.zMin;
        bBox->xMax = shell->geo.sg3I->bBox.xMax;
        bBox->yMax = shell->geo.sg3I->bBox.yMax;
	bBox->zMax = shell->geo.sg3I->bBox.zMax;
	break;
      case WLZ_GMELM_SHELL_G3D:
        *bBox = shell->geo.sg3D->bBox;
	break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Gets the volume of the shell's geometry's bounding box.
* \param	shell			Given shell with geometry to
*                                       be set.
* \param	vol			Given destination pointer for
*                                       the volume.
*/
WlzErrorNum	WlzGMShellGetGBBV3D(WlzGMShell *shell, double *vol)
{
  WlzDBox3	bBox;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(vol == NULL)
  {
    errNum = WLZ_ERR_PARAM_NULL;
  }
  else if((errNum = WlzGMShellGetGBB3D(shell, &bBox)) == WLZ_ERR_NONE)
  {
    *vol = (bBox.xMax - bBox.xMin) * (bBox.yMax - bBox.yMin) *
    	   (bBox.zMax - bBox.zMin);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Gets a shell's geometry using the given destination
*               pointer to a double precision bounding box.
* \param	shell			Given shell with geometry to
*                                       be set.
* \param	bBox			Given destination pointer to a
*                                       double precision bounding box.
*/
WlzErrorNum	WlzGMShellGetGBB2D(WlzGMShell *shell, WlzDBox2 *bBox)
{
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((shell == NULL) || (shell->geo.core == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else if(bBox == NULL)
  {
    errNum = WLZ_ERR_PARAM_NULL;
  }
  else
  {
    switch(shell->geo.core->type)
    {
      case WLZ_GMELM_SHELL_G2I:
        bBox->xMin = shell->geo.sg2I->bBox.xMin;
        bBox->yMin = shell->geo.sg2I->bBox.yMin;
        bBox->xMax = shell->geo.sg2I->bBox.xMax;
        bBox->yMax = shell->geo.sg2I->bBox.yMax;
	break;
      case WLZ_GMELM_SHELL_G2D:
        *bBox = shell->geo.sg2D->bBox;
	break;
      case WLZ_GMELM_SHELL_G3I:
        bBox->xMin = shell->geo.sg3I->bBox.xMin;
        bBox->yMin = shell->geo.sg3I->bBox.yMin;
        bBox->xMax = shell->geo.sg3I->bBox.xMax;
        bBox->yMax = shell->geo.sg3I->bBox.yMax;
	break;
      case WLZ_GMELM_SHELL_G3D:
	bBox->xMin = shell->geo.sg3D->bBox.xMin;
	bBox->yMin = shell->geo.sg3D->bBox.yMin;
	bBox->xMax = shell->geo.sg3D->bBox.xMax;
	bBox->yMax = shell->geo.sg3D->bBox.yMax;
	break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Sets a shell's geometry using the given double
*               precision bounding box.
* \param	shell			Given shell with geometry to
*                                       be set.
* \param	bBox			Given double precision bounding
*                                       box.
*/
WlzErrorNum	WlzGMShellSetGBB3D(WlzGMShell *shell, WlzDBox3 bBox)
{
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((shell == NULL) || (shell->geo.core == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    switch(shell->geo.core->type)
    {
      case WLZ_GMELM_SHELL_G2I:
        shell->geo.sg2I->bBox.xMin = WLZ_NINT(bBox.xMin);
        shell->geo.sg2I->bBox.yMin = WLZ_NINT(bBox.yMin);
        shell->geo.sg2I->bBox.xMax = WLZ_NINT(bBox.xMax);
        shell->geo.sg2I->bBox.yMax = WLZ_NINT(bBox.yMax);
	break;
      case WLZ_GMELM_SHELL_G2D:
        shell->geo.sg2D->bBox.xMin = bBox.xMin;
	shell->geo.sg2D->bBox.yMin = bBox.yMin;
	shell->geo.sg2D->bBox.xMax = bBox.xMax;
	shell->geo.sg2D->bBox.yMax = bBox.yMax;
	break;
      case WLZ_GMELM_SHELL_G3I:
        shell->geo.sg3I->bBox.xMin = WLZ_NINT(bBox.xMin);
        shell->geo.sg3I->bBox.yMin = WLZ_NINT(bBox.yMin);
        shell->geo.sg3I->bBox.zMin = WLZ_NINT(bBox.zMin);
        shell->geo.sg3I->bBox.xMax = WLZ_NINT(bBox.xMax);
        shell->geo.sg3I->bBox.yMax = WLZ_NINT(bBox.yMax);
        shell->geo.sg3I->bBox.zMax = WLZ_NINT(bBox.zMax);
	break;
      case WLZ_GMELM_SHELL_G3D:
        shell->geo.sg3D->bBox = bBox;
	break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Sets a shell's geometry using the given double
*               precision bounding box.
* \param	shell			Given shell with geometry to
*                                       be set.
* \param	bBox			Given double precision bounding
*                                       box.
*/
WlzErrorNum	WlzGMShellSetGBB2D(WlzGMShell *shell, WlzDBox2 bBox)
{
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((shell == NULL) || (shell->geo.core == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    switch(shell->geo.core->type)
    {
      case WLZ_GMELM_SHELL_G2I:
        shell->geo.sg2I->bBox.xMin = WLZ_NINT(bBox.xMin);
        shell->geo.sg2I->bBox.yMin = WLZ_NINT(bBox.yMin);
        shell->geo.sg2I->bBox.xMax = WLZ_NINT(bBox.xMax);
        shell->geo.sg2I->bBox.yMax = WLZ_NINT(bBox.yMax);
	break;
      case WLZ_GMELM_SHELL_G2D:
        shell->geo.sg2D->bBox = bBox;
	break;
      case WLZ_GMELM_SHELL_G3I:
        shell->geo.sg3I->bBox.xMin = WLZ_NINT(bBox.xMin);
        shell->geo.sg3I->bBox.yMin = WLZ_NINT(bBox.yMin);
        shell->geo.sg3I->bBox.zMin = 0;
        shell->geo.sg3I->bBox.xMax = WLZ_NINT(bBox.xMax);
        shell->geo.sg3I->bBox.yMax = WLZ_NINT(bBox.yMax);
        shell->geo.sg3I->bBox.zMax = 0;
	break;
      case WLZ_GMELM_SHELL_G3D:
        shell->geo.sg3D->bBox.xMin = bBox.xMin;
        shell->geo.sg3D->bBox.yMin = bBox.yMin;
        shell->geo.sg3D->bBox.zMin = 0.0;
        shell->geo.sg3D->bBox.xMax = bBox.xMax;
        shell->geo.sg3D->bBox.yMax = bBox.yMax;
        shell->geo.sg3D->bBox.zMax = 0.0;
	break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Sets a shell's geometry using the pair of double
*               precision points.
* \param	shell			Given shell with geometry to
*                                       be set.
* \param	nPnt			Number of points.
* \param	pos			Array of points.
*/
WlzErrorNum	WlzGMShellSetG3D(WlzGMShell *shell,
				 int nPnt, WlzDVertex3 *pos)
{
  WlzDBox3	bBox;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((nPnt > 0) && pos)
  {
    bBox.xMin = bBox.xMax = pos->vtX;
    bBox.yMin = bBox.yMax = pos->vtY;
    bBox.zMin = bBox.zMax = pos->vtZ;
    while(--nPnt > 0)
    {
      ++pos;
      if(pos->vtX > bBox.xMax)
      {
	bBox.xMax = pos->vtX;
      }
      else if(pos->vtX < bBox.xMin)
      {
	bBox.xMin = pos->vtX;
      }
      if(pos->vtY > bBox.yMax)
      {
	bBox.yMax = pos->vtY;
      }
      else if(pos->vtY < bBox.xMin)
      {
	bBox.yMin = pos->vtY;
      }
      if(pos->vtZ > bBox.zMax)
      {
	bBox.zMax = pos->vtZ;
      }
      else if(pos->vtZ < bBox.zMin)
      {
	bBox.zMin = pos->vtZ;
      }
    }
    errNum = WlzGMShellSetGBB3D(shell, bBox);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Sets a shell's geometry using the pair of double
*               precision points.
* \param	shell			Given shell with geometry to
*                                       be set.
* \param	nPnt			Number of points.
* \param	pos			Array of points.
*/
WlzErrorNum	WlzGMShellSetG2D(WlzGMShell *shell,
				 int nPnt, WlzDVertex2 *pos)
{
  WlzDBox2	bBox;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((nPnt > 0) && pos)
  {
    bBox.xMin = bBox.xMax = pos->vtX;
    bBox.yMin = bBox.yMax = pos->vtY;
    while(--nPnt > 0)
    {
      ++pos;
      if(pos->vtX > bBox.xMax)
      {
	bBox.xMax = pos->vtX;
      }
      else if(pos->vtX < bBox.xMin)
      {
	bBox.xMin = pos->vtX;
      }
      if(pos->vtY > bBox.yMax)
      {
	bBox.yMax = pos->vtY;
      }
      else if(pos->vtY < bBox.xMin)
      {
	bBox.yMin = pos->vtY;
      }
    }
    errNum = WlzGMShellSetGBB2D(shell, bBox);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Updates a shell's geometry using the given double
*               precision position.
* \param	shell			Given shell with geometry to
*                                       be set.
* \param	pos			Given position.
*/
WlzErrorNum	WlzGMShellUpdateG3D(WlzGMShell *shell, WlzDVertex3 pos)
{
  int		tI0;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((shell == NULL) || (shell->geo.core == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    switch(shell->geo.core->type)
    {
      case WLZ_GMELM_SHELL_G2I:
	if((tI0 = pos.vtX) < shell->geo.sg2I->bBox.xMin)
	{
	  shell->geo.sg2I->bBox.xMin = tI0;
	}
	else if(tI0 > shell->geo.sg2I->bBox.xMax)
	{
	  shell->geo.sg2I->bBox.xMax = tI0;
	}
	if((tI0 = pos.vtY) < shell->geo.sg2I->bBox.yMin)
	{
	  shell->geo.sg2I->bBox.yMin = tI0;
	}
	else if(tI0 > shell->geo.sg2I->bBox.yMax)
	{
	  shell->geo.sg2I->bBox.yMax = tI0;
	}
	break;
      case WLZ_GMELM_SHELL_G2D:
	if(pos.vtX < shell->geo.sg2D->bBox.xMin)
	{
	  shell->geo.sg2D->bBox.xMin = pos.vtX;
	}
	else if(pos.vtX > shell->geo.sg2D->bBox.xMax)
	{
	  shell->geo.sg2D->bBox.xMax = pos.vtX;
	}
	if(pos.vtY < shell->geo.sg2D->bBox.yMin)
	{
	  shell->geo.sg2D->bBox.yMin = pos.vtY;
	}
	else if(pos.vtY > shell->geo.sg2D->bBox.yMax)
	{
	  shell->geo.sg2D->bBox.yMax = pos.vtY;
	}
	break;
      case WLZ_GMELM_SHELL_G3I:
	if((tI0 = pos.vtX) < shell->geo.sg3I->bBox.xMin)
	{
	  shell->geo.sg3I->bBox.xMin = tI0;
	}
	else if(tI0 > shell->geo.sg3I->bBox.xMax)
	{
	  shell->geo.sg3I->bBox.xMax = tI0;
	}
	if((tI0 = pos.vtY) < shell->geo.sg3I->bBox.yMin)
	{
	  shell->geo.sg3I->bBox.yMin = tI0;
	}
	else if(tI0 > shell->geo.sg3I->bBox.yMax)
	{
	  shell->geo.sg3I->bBox.yMax = tI0;
	}
	if((tI0 = pos.vtZ) < shell->geo.sg3I->bBox.zMin)
	{
	  shell->geo.sg3I->bBox.zMin = tI0;
	}
	else if(tI0 > shell->geo.sg3I->bBox.zMax)
	{
	  shell->geo.sg3I->bBox.zMax = tI0;
	}
	break;
      case WLZ_GMELM_SHELL_G3D:
	if(pos.vtX < shell->geo.sg3D->bBox.xMin)
	{
	  shell->geo.sg3D->bBox.xMin = pos.vtX;
	}
	else if(pos.vtX > shell->geo.sg3D->bBox.xMax)
	{
	  shell->geo.sg3D->bBox.xMax = pos.vtX;
	}
	if(pos.vtY < shell->geo.sg3D->bBox.yMin)
	{
	  shell->geo.sg3D->bBox.yMin = pos.vtY;
	}
	else if(pos.vtY > shell->geo.sg3D->bBox.yMax)
	{
	  shell->geo.sg3D->bBox.yMax = pos.vtY;
	}
	if(pos.vtZ < shell->geo.sg3D->bBox.zMin)
	{
	  shell->geo.sg3D->bBox.zMin = pos.vtZ;
	}
	else if(pos.vtZ > shell->geo.sg3D->bBox.zMax)
	{
	  shell->geo.sg3D->bBox.zMax = pos.vtZ;
	}
	break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Updates a shell's geometry using the given double
*               precision position.
* \param	shell			Given shell with geometry to
*                                       be set.
* \param	pos			Given position.
*/
WlzErrorNum	WlzGMShellUpdateG2D(WlzGMShell *shell, WlzDVertex2 pos)
{
  int		tI0;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((shell == NULL) || (shell->geo.core == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    switch(shell->geo.core->type)
    {
      case WLZ_GMELM_SHELL_G2I:
	if((tI0 = pos.vtX) < shell->geo.sg2I->bBox.xMin)
	{
	  shell->geo.sg2I->bBox.xMin = tI0;
	}
	else if(tI0 > shell->geo.sg2I->bBox.xMax)
	{
	  shell->geo.sg2I->bBox.xMax = tI0;
	}
	if((tI0 = pos.vtY) < shell->geo.sg2I->bBox.yMin)
	{
	  shell->geo.sg2I->bBox.yMin = tI0;
	}
	else if(tI0 > shell->geo.sg2I->bBox.yMax)
	{
	  shell->geo.sg2I->bBox.yMax = tI0;
	}
	break;
      case WLZ_GMELM_SHELL_G2D:
	if(pos.vtX < shell->geo.sg2D->bBox.xMin)
	{
	  shell->geo.sg2D->bBox.xMin = pos.vtX;
	}
	else if(pos.vtX > shell->geo.sg2D->bBox.xMax)
	{
	  shell->geo.sg2D->bBox.xMax = pos.vtX;
	}
	if(pos.vtY < shell->geo.sg2D->bBox.yMin)
	{
	  shell->geo.sg2D->bBox.yMin = pos.vtY;
	}
	else if(pos.vtY > shell->geo.sg2D->bBox.yMax)
	{
	  shell->geo.sg2D->bBox.yMax = pos.vtY;
	}
	break;
      case WLZ_GMELM_SHELL_G3I:
	if((tI0 = pos.vtX) < shell->geo.sg3I->bBox.xMin)
	{
	  shell->geo.sg3I->bBox.xMin = tI0;
	}
	else if(tI0 > shell->geo.sg3I->bBox.xMax)
	{
	  shell->geo.sg3I->bBox.xMax = tI0;
	}
	if((tI0 = pos.vtY) < shell->geo.sg3I->bBox.yMin)
	{
	  shell->geo.sg3I->bBox.yMin = tI0;
	}
	else if(tI0 > shell->geo.sg3I->bBox.yMax)
	{
	  shell->geo.sg3I->bBox.yMax = tI0;
	}
	break;
      case WLZ_GMELM_SHELL_G3D:
	if(pos.vtX < shell->geo.sg3D->bBox.xMin)
	{
	  shell->geo.sg3D->bBox.xMin = pos.vtX;
	}
	else if(pos.vtX > shell->geo.sg3D->bBox.xMax)
	{
	  shell->geo.sg3D->bBox.xMax = pos.vtX;
	}
	if(pos.vtY < shell->geo.sg3D->bBox.yMin)
	{
	  shell->geo.sg3D->bBox.yMin = pos.vtY;
	}
	else if(pos.vtY > shell->geo.sg3D->bBox.yMax)
	{
	  shell->geo.sg3D->bBox.yMax = pos.vtY;
	}
	break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Updates a shell's geometry using the given double
*               precision position bounding box. The updated geometry
*               is the union of the existing geometry and the given
*               bounding box.
* \param	shell			Given shell with geometry to
*                                       be set.
* \param	bBox			Given bounding box.
*/
WlzErrorNum	WlzGMShellUpdateGBB3D(WlzGMShell *shell, WlzDBox3 bBox)
{
  int		tI0;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((shell == NULL) || (shell->geo.core == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    switch(shell->geo.core->type)
    {
      case WLZ_GMELM_SHELL_G2I:
	if((tI0 = WLZ_NINT(bBox.xMin)) < shell->geo.sg2I->bBox.xMin)
	{
	  shell->geo.sg2I->bBox.xMin = tI0;
	}
	if((tI0 = WLZ_NINT(bBox.yMin)) < shell->geo.sg2I->bBox.yMin)
	{
	  shell->geo.sg2I->bBox.yMin = tI0;
	}
	if((tI0 = WLZ_NINT(bBox.xMax)) > shell->geo.sg2I->bBox.xMax)
	{
	  shell->geo.sg2I->bBox.xMax = tI0;
	}
	if((tI0 = WLZ_NINT(bBox.yMax)) > shell->geo.sg2I->bBox.yMax)
	{
	  shell->geo.sg2I->bBox.yMax = tI0;
	}
	break;
      case WLZ_GMELM_SHELL_G2D:
	if(bBox.xMin < shell->geo.sg2D->bBox.xMin)
	{
	  shell->geo.sg2D->bBox.xMin = bBox.xMin;
	}
	if(bBox.yMin < shell->geo.sg2D->bBox.yMin)
	{
	  shell->geo.sg2D->bBox.yMin = bBox.yMin;
	}
	if(bBox.xMax > shell->geo.sg2D->bBox.xMax)
	{
	  shell->geo.sg2D->bBox.xMax = bBox.xMax;
	}
	if(bBox.yMax > shell->geo.sg2D->bBox.yMax)
	{
	  shell->geo.sg2I->bBox.yMax = bBox.yMax;
	}
	break;
      case WLZ_GMELM_SHELL_G3I:
	if((tI0 = WLZ_NINT(bBox.xMin)) < shell->geo.sg3I->bBox.xMin)
	{
	  shell->geo.sg3I->bBox.xMin = tI0;
	}
	if((tI0 = WLZ_NINT(bBox.yMin)) < shell->geo.sg3I->bBox.yMin)
	{
	  shell->geo.sg3I->bBox.yMin = tI0;
	}
	if((tI0 = WLZ_NINT(bBox.zMin)) < shell->geo.sg3I->bBox.zMin)
	{
	  shell->geo.sg3I->bBox.zMin = tI0;
	}
	if((tI0 = WLZ_NINT(bBox.xMax)) > shell->geo.sg3I->bBox.xMax)
	{
	  shell->geo.sg3I->bBox.xMax = tI0;
	}
	if((tI0 = WLZ_NINT(bBox.yMax)) > shell->geo.sg3I->bBox.yMax)
	{
	  shell->geo.sg3I->bBox.yMax = tI0;
	}
	if((tI0 = WLZ_NINT(bBox.zMax)) > shell->geo.sg3I->bBox.zMax)
	{
	  shell->geo.sg3I->bBox.zMax = tI0;
	}
	break;
      case WLZ_GMELM_SHELL_G3D:
	if(bBox.xMin < shell->geo.sg3D->bBox.xMin)
	{
	  shell->geo.sg3D->bBox.xMin = bBox.xMin;
	}
	if(bBox.yMin < shell->geo.sg3D->bBox.yMin)
	{
	  shell->geo.sg3D->bBox.yMin = bBox.yMin;
	}
	if(bBox.zMin < shell->geo.sg3D->bBox.zMin)
	{
	  shell->geo.sg3D->bBox.zMin = bBox.zMin;
	}
	if(bBox.xMax > shell->geo.sg3D->bBox.xMax)
	{
	  shell->geo.sg3D->bBox.xMax = bBox.xMax;
	}
	if(bBox.yMax > shell->geo.sg3D->bBox.yMax)
	{
	  shell->geo.sg3D->bBox.yMax = bBox.yMax;
	}
	if(bBox.zMax > shell->geo.sg3D->bBox.zMax)
	{
	  shell->geo.sg3D->bBox.zMax = bBox.zMax;
	}
	break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Updates a shell's geometry using the given double
*               precision position bounding box. The updated geometry
*               is the union of the existing geometry and the given
*               bounding box.
* \param	shell			Given shell with geometry to
*                                       be set.
* \param	bBox			Given bounding box.
*/
WlzErrorNum	WlzGMShellUpdateGBB2D(WlzGMShell *shell, WlzDBox2 bBox)
{
  int		tI0;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((shell == NULL) || (shell->geo.core == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    switch(shell->geo.core->type)
    {
      case WLZ_GMELM_SHELL_G2I:
	if((tI0 = WLZ_NINT(bBox.xMin)) < shell->geo.sg2I->bBox.xMin)
	{
	  shell->geo.sg2I->bBox.xMin = tI0;
	}
	if((tI0 = WLZ_NINT(bBox.yMin)) < shell->geo.sg2I->bBox.yMin)
	{
	  shell->geo.sg2I->bBox.yMin = tI0;
	}
	if((tI0 = WLZ_NINT(bBox.xMax)) > shell->geo.sg2I->bBox.xMax)
	{
	  shell->geo.sg2I->bBox.xMax = tI0;
	}
	if((tI0 = WLZ_NINT(bBox.yMax)) > shell->geo.sg2I->bBox.yMax)
	{
	  shell->geo.sg2I->bBox.yMax = tI0;
	}
	break;
      case WLZ_GMELM_SHELL_G2D:
	if(bBox.xMin < shell->geo.sg2D->bBox.xMin)
	{
	  shell->geo.sg2D->bBox.xMin = bBox.xMin;
	}
	if(bBox.yMin < shell->geo.sg2D->bBox.yMin)
	{
	  shell->geo.sg2D->bBox.yMin = bBox.yMin;
	}
	if(bBox.xMax > shell->geo.sg2D->bBox.xMax)
	{
	  shell->geo.sg2D->bBox.xMax = bBox.xMax;
	}
	if(bBox.yMax > shell->geo.sg2D->bBox.yMax)
	{
	  shell->geo.sg2I->bBox.yMax = bBox.yMax;
	}
	break;
      case WLZ_GMELM_SHELL_G3I:
	if((tI0 = WLZ_NINT(bBox.xMin)) < shell->geo.sg3I->bBox.xMin)
	{
	  shell->geo.sg3I->bBox.xMin = tI0;
	}
	if((tI0 = WLZ_NINT(bBox.yMin)) < shell->geo.sg3I->bBox.yMin)
	{
	  shell->geo.sg3I->bBox.yMin = tI0;
	}
	if((tI0 = WLZ_NINT(bBox.xMax)) > shell->geo.sg3I->bBox.xMax)
	{
	  shell->geo.sg3I->bBox.xMax = tI0;
	}
	if((tI0 = WLZ_NINT(bBox.yMax)) > shell->geo.sg3I->bBox.yMax)
	{
	  shell->geo.sg3I->bBox.yMax = tI0;
	}
	break;
      case WLZ_GMELM_SHELL_G3D:
	if(bBox.xMin < shell->geo.sg3D->bBox.xMin)
	{
	  shell->geo.sg3D->bBox.xMin = bBox.xMin;
	}
	if(bBox.yMin < shell->geo.sg3D->bBox.yMin)
	{
	  shell->geo.sg3D->bBox.yMin = bBox.yMin;
	}
	if(bBox.xMax > shell->geo.sg3D->bBox.xMax)
	{
	  shell->geo.sg3D->bBox.xMax = bBox.xMax;
	}
	if(bBox.yMax > shell->geo.sg3D->bBox.yMax)
	{
	  shell->geo.sg3D->bBox.yMax = bBox.yMax;
	}
	break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Recomputes the shell's geometry by walking through
*               it's children.
* \param	shell			Given shell with geometry to
*                                       be set.
*/
WlzErrorNum	WlzGMShellComputeGBB(WlzGMShell *shell)
{
  int		tI0,
  		fETIdx,
  		fLTIdx;
  double	tD0;
  WlzGMEdgeT	*tET;
  WlzGMLoopT	*tLT;
  WlzGMVertexGU	vGU;
  WlzGMShellGU	sGU;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((shell == NULL) || ((sGU = shell->geo).core == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else if(shell->child)
  {
    switch(sGU.core->type)
    {
      case WLZ_GMELM_SHELL_G2I:
	vGU = shell->child->edgeT->vertexT->diskT->vertex->geo;
	sGU.sg2I->bBox.xMin = sGU.sg2I->bBox.xMax = vGU.vg2I->vtx.vtX;
	sGU.sg2I->bBox.yMin = sGU.sg2I->bBox.yMax = vGU.vg2I->vtx.vtY;
	/* For each loopT */
	tLT = shell->child;
	fLTIdx = tLT->idx;
	do
	{
	  /* For each edgeT */
	  tET = tLT->edgeT;
	  fETIdx = tET->idx;
	  do
	  {
	    /* Update the new bounding box. */
	    vGU = tET->vertexT->diskT->vertex->geo;
	    if((tI0 = vGU.vg2I->vtx.vtX) < sGU.sg2I->bBox.xMin)
	    {
	      sGU.sg2I->bBox.xMin = tI0;
	    }
	    else if(tI0 > sGU.sg2I->bBox.xMax)
	    {
	      sGU.sg2I->bBox.xMax = tI0;
	    }
	    if((tI0 = vGU.vg2I->vtx.vtY) < sGU.sg2I->bBox.yMin)
	    {
	      sGU.sg2I->bBox.yMin = tI0;
	    }
	    else if(tI0 > sGU.sg2I->bBox.yMax)
	    {
	      sGU.sg2I->bBox.yMax = tI0;
	    }
	    /* Next edge topology element. */
	    tET = tET->next;
	  } while(tET->idx != fETIdx);
	  tLT = tLT->next;
	} while(tLT->idx != fLTIdx);
	break;
      case WLZ_GMELM_SHELL_G2D:
	vGU = shell->child->edgeT->vertexT->diskT->vertex->geo;
	sGU.sg2D->bBox.xMin = sGU.sg2D->bBox.xMax = vGU.vg2D->vtx.vtX;
	sGU.sg2D->bBox.yMin = sGU.sg2D->bBox.yMax = vGU.vg2D->vtx.vtY;
	/* For each loopT */
	tLT = shell->child;
	fLTIdx = tLT->idx;
	do
	{
	  /* For each edgeT */
	  tET = tLT->edgeT;
	  fETIdx = tET->idx;
	  do
	  {
	    /* Update the new bounding box. */
	    vGU = tET->vertexT->diskT->vertex->geo;
	    if((tD0 = vGU.vg2D->vtx.vtX) < sGU.sg2D->bBox.xMin)
	    {
	      sGU.sg2D->bBox.xMin = tD0;
	    }
	    else if(tD0 > sGU.sg2D->bBox.xMax)
	    {
	      sGU.sg2D->bBox.xMax = tD0;
	    }
	    if((tD0 = vGU.vg2D->vtx.vtY) < sGU.sg2D->bBox.yMin)
	    {
	      sGU.sg2D->bBox.yMin = tD0;
	    }
	    else if(tD0 > sGU.sg2D->bBox.yMax)
	    {
	      sGU.sg2D->bBox.yMax = tD0;
	    }
	    /* Next edge topology element. */
	    tET = tET->next;
	  } while(tET->idx != fETIdx);
	  tLT = tLT->next;
	} while(tLT->idx != fLTIdx);
	break;
      case WLZ_GMELM_SHELL_G3I:
	vGU = shell->child->edgeT->vertexT->diskT->vertex->geo;
	sGU.sg3I->bBox.xMin = sGU.sg3I->bBox.xMax = vGU.vg3I->vtx.vtX;
	sGU.sg3I->bBox.yMin = sGU.sg3I->bBox.yMax = vGU.vg3I->vtx.vtY;
	sGU.sg3I->bBox.zMin = sGU.sg3I->bBox.zMax = vGU.vg3I->vtx.vtZ;
	/* For each loopT */
	tLT = shell->child;
	fLTIdx = tLT->idx;
	do
	{
	  /* For each edgeT */
	  tET = tLT->edgeT;
	  fETIdx = tET->idx;
	  do
	  {
	    /* Update the new bounding box. */
	    vGU = tET->vertexT->diskT->vertex->geo;
	    if((tI0 = vGU.vg3I->vtx.vtX) < sGU.sg3I->bBox.xMin)
	    {
	      sGU.sg3I->bBox.xMin = tI0;
	    }
	    else if(tI0 > sGU.sg3I->bBox.xMax)
	    {
	      sGU.sg3I->bBox.xMax = tI0;
	    }
	    if((tI0 = vGU.vg3I->vtx.vtY) < sGU.sg3I->bBox.yMin)
	    {
	      sGU.sg3I->bBox.yMin = tI0;
	    }
	    else if(tI0 > sGU.sg3I->bBox.yMax)
	    {
	      sGU.sg3I->bBox.yMax = tI0;
	    }
	    if((tI0 = vGU.vg3I->vtx.vtZ) < sGU.sg3I->bBox.zMin)
	    {
	      sGU.sg3I->bBox.zMin = tI0;
	    }
	    else if(tI0 > sGU.sg3I->bBox.zMax)
	    {
	      sGU.sg3I->bBox.zMax = tI0;
	    }
	    /* Next edge topology element. */
	    tET = tET->next;
	  } while(tET->idx != fETIdx);
	  tLT = tLT->next;
	} while(tLT->idx != fLTIdx);
	break;
      case WLZ_GMELM_SHELL_G3D:
	vGU = shell->child->edgeT->vertexT->diskT->vertex->geo;
	sGU.sg3D->bBox.xMin = sGU.sg3D->bBox.xMax = vGU.vg3D->vtx.vtX;
	sGU.sg3D->bBox.yMin = sGU.sg3D->bBox.yMax = vGU.vg3D->vtx.vtY;
	sGU.sg3D->bBox.zMin = sGU.sg3D->bBox.zMax = vGU.vg3D->vtx.vtZ;
	/* For each loopT */
	tLT = shell->child;
	fLTIdx = tLT->idx;
	do
	{
	  /* For each edgeT */
	  tET = tLT->edgeT;
	  fETIdx = tET->idx;
	  do
	  {
	    /* Update the new bounding box. */
	    vGU = tET->vertexT->diskT->vertex->geo;
	    if((tD0 = vGU.vg3D->vtx.vtX) < sGU.sg3D->bBox.xMin)
	    {
	      sGU.sg3D->bBox.xMin = tD0;
	    }
	    else if(tD0 > sGU.sg3D->bBox.xMax)
	    {
	      sGU.sg3D->bBox.xMax = tD0;
	    }
	    if((tD0 = vGU.vg3D->vtx.vtY) < sGU.sg3D->bBox.yMin)
	    {
	      sGU.sg3D->bBox.yMin = tD0;
	    }
	    else if(tD0 > sGU.sg3D->bBox.yMax)
	    {
	      sGU.sg3D->bBox.yMax = tD0;
	    }
	    if((tD0 = vGU.vg3D->vtx.vtZ) < sGU.sg3D->bBox.zMin)
	    {
	      sGU.sg3D->bBox.zMin = tD0;
	    }
	    else if(tD0 > sGU.sg3D->bBox.zMax)
	    {
	      sGU.sg3D->bBox.zMax = tD0;
	    }
	    /* Next edge topology element. */
	    tET = tET->next;
	  } while(tET->idx != fETIdx);
	  tLT = tLT->next;
	} while(tLT->idx != fLTIdx);
	break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
        break;
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Merges the geometries of the two shells so that the
*               first shell's geometry is set to the union of the
*               two shell's bounding boxes.
* \param	shell0			First shell with geometry to
*                                       be both used and set.
* \param	shell1			Second shell with geometry to
*                                       be used.
*/
static WlzErrorNum WlzGMShellMergeG(WlzGMShell *shell0, WlzGMShell *shell1)
{
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((shell0 == NULL) || (shell0->geo.core == NULL) ||
     (shell1 == NULL) || (shell1->geo.core == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else if(shell0->geo.core->type != shell1->geo.core->type)
  {
    errNum = WLZ_ERR_DOMAIN_DATA;
  }
  {
    switch(shell0->geo.core->type)
    {
      case WLZ_GMELM_SHELL_G2I:
	if(shell1->geo.sg2I->bBox.xMin < shell0->geo.sg2I->bBox.xMin)
	{
	  shell0->geo.sg2I->bBox.xMin = shell1->geo.sg2I->bBox.xMin;
	}
	if(shell1->geo.sg2I->bBox.yMin < shell0->geo.sg2I->bBox.yMin)
	{
	  shell0->geo.sg2I->bBox.yMin = shell1->geo.sg2I->bBox.yMin;
	}
	if(shell1->geo.sg2I->bBox.xMax > shell0->geo.sg2I->bBox.xMax)
	{
	  shell0->geo.sg2I->bBox.xMax = shell1->geo.sg2I->bBox.xMax;
	}
	if(shell1->geo.sg2I->bBox.yMax > shell0->geo.sg2I->bBox.yMax)
	{
	  shell0->geo.sg2I->bBox.yMax = shell1->geo.sg2I->bBox.yMax;
	}
	break;
      case WLZ_GMELM_SHELL_G2D:
	if(shell1->geo.sg2D->bBox.xMin < shell0->geo.sg2D->bBox.xMin)
	{
	  shell0->geo.sg2D->bBox.xMin = shell1->geo.sg2D->bBox.xMin;
	}
	if(shell1->geo.sg2D->bBox.yMin < shell0->geo.sg2D->bBox.yMin)
	{
	  shell0->geo.sg2D->bBox.yMin = shell1->geo.sg2D->bBox.yMin;
	}
	if(shell1->geo.sg2D->bBox.xMax > shell0->geo.sg2D->bBox.xMax)
	{
	  shell0->geo.sg2D->bBox.xMax = shell1->geo.sg2D->bBox.xMax;
	}
	if(shell1->geo.sg2D->bBox.yMax > shell0->geo.sg2D->bBox.yMax)
	{
	  shell0->geo.sg2D->bBox.yMax = shell1->geo.sg2D->bBox.yMax;
	}
	break;
      case WLZ_GMELM_SHELL_G3I:
	if(shell1->geo.sg3I->bBox.xMin < shell0->geo.sg3I->bBox.xMin)
	{
	  shell0->geo.sg3I->bBox.xMin = shell1->geo.sg3I->bBox.xMin;
	}
	if(shell1->geo.sg3I->bBox.yMin < shell0->geo.sg3I->bBox.yMin)
	{
	  shell0->geo.sg3I->bBox.yMin = shell1->geo.sg3I->bBox.yMin;
	}
	if(shell1->geo.sg3I->bBox.zMin < shell0->geo.sg3I->bBox.zMin)
	{
	  shell0->geo.sg3I->bBox.zMin = shell1->geo.sg3I->bBox.zMin;
	}
	if(shell1->geo.sg3I->bBox.xMax > shell0->geo.sg3I->bBox.xMax)
	{
	  shell0->geo.sg3I->bBox.xMax = shell1->geo.sg3I->bBox.xMax;
	}
	if(shell1->geo.sg3I->bBox.yMax > shell0->geo.sg3I->bBox.yMax)
	{
	  shell0->geo.sg3I->bBox.yMax = shell1->geo.sg3I->bBox.yMax;
	}
	if(shell1->geo.sg3I->bBox.zMax > shell0->geo.sg3I->bBox.zMax)
	{
	  shell0->geo.sg3I->bBox.zMax = shell1->geo.sg3I->bBox.zMax;
	}
	break;
      case WLZ_GMELM_SHELL_G3D:
	if(shell1->geo.sg3D->bBox.xMin < shell0->geo.sg3D->bBox.xMin)
	{
	  shell0->geo.sg3D->bBox.xMin = shell1->geo.sg3D->bBox.xMin;
	}
	if(shell1->geo.sg3D->bBox.yMin < shell0->geo.sg3D->bBox.yMin)
	{
	  shell0->geo.sg3D->bBox.yMin = shell1->geo.sg3D->bBox.yMin;
	}
	if(shell1->geo.sg3D->bBox.zMin < shell0->geo.sg3D->bBox.zMin)
	{
	  shell0->geo.sg3D->bBox.zMin = shell1->geo.sg3D->bBox.zMin;
	}
	if(shell1->geo.sg3D->bBox.xMax > shell0->geo.sg3D->bBox.xMax)
	{
	  shell0->geo.sg3D->bBox.xMax = shell1->geo.sg3D->bBox.xMax;
	}
	if(shell1->geo.sg3D->bBox.yMax > shell0->geo.sg3D->bBox.yMax)
	{
	  shell0->geo.sg3D->bBox.yMax = shell1->geo.sg3D->bBox.yMax;
	}
	if(shell1->geo.sg3D->bBox.zMax > shell0->geo.sg3D->bBox.zMax)
	{
	  shell0->geo.sg3D->bBox.zMax = shell1->geo.sg3D->bBox.zMax;
	}
	break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
  }
  return(errNum);
}

/*!
* \return				Non zero if point inside shell's
* \ingroup      WlzGeoModel
*                                       bounding box.
* \brief	Checks to see if the given double precision position
*               is within the shell's bounding box.
* \param	shell			Given shell with geometry to
*                                       be set.
* \param	pos			Given double precision position.
*/
int		WlzGMShellGInBB3D(WlzGMShell *shell, WlzDVertex3 pos)
{
  int		inside = 0;

  if((shell != NULL) && (shell->geo.core != NULL))
  {
    switch(shell->geo.core->type)
    {
      case WLZ_GMELM_SHELL_G2I:
	inside = (pos.vtX >= shell->geo.sg2I->bBox.xMin) &&
		 (pos.vtX <= shell->geo.sg2I->bBox.xMax) &&
	         (pos.vtY >= shell->geo.sg2I->bBox.yMin) &&
		 (pos.vtY <= shell->geo.sg2I->bBox.yMax);
	break;
      case WLZ_GMELM_SHELL_G2D:
	inside = (pos.vtX >= shell->geo.sg2D->bBox.xMin) &&
		 (pos.vtX <= shell->geo.sg2D->bBox.xMax) &&
	         (pos.vtY >= shell->geo.sg2D->bBox.yMin) &&
		 (pos.vtY <= shell->geo.sg2D->bBox.yMax);
	break;
      case WLZ_GMELM_SHELL_G3I:
	inside = (pos.vtX >= shell->geo.sg3I->bBox.xMin) &&
		 (pos.vtX <= shell->geo.sg3I->bBox.xMax) &&
	         (pos.vtY >= shell->geo.sg3I->bBox.yMin) &&
		 (pos.vtY <= shell->geo.sg3I->bBox.yMax) &&
	         (pos.vtZ >= shell->geo.sg3I->bBox.zMin) &&
		 (pos.vtZ <= shell->geo.sg3I->bBox.zMax);
	break;
      case WLZ_GMELM_SHELL_G3D:
	inside = (pos.vtX >= shell->geo.sg3D->bBox.xMin) &&
		 (pos.vtX <= shell->geo.sg3D->bBox.xMax) &&
	         (pos.vtY >= shell->geo.sg3D->bBox.yMin) &&
		 (pos.vtY <= shell->geo.sg3D->bBox.yMax) &&
	         (pos.vtZ >= shell->geo.sg3D->bBox.zMin) &&
		 (pos.vtZ <= shell->geo.sg3D->bBox.zMax);
	break;
      default:
	break;
    }
  }
  return(inside);
}

/*!
* \return				Non zero if point inside shell's
* \ingroup      WlzGeoModel
*                                       bounding box.
* \brief	Checks to see if the given double precision position
*               is within the shell's bounding box.
* \param	shell			Given shell with geometry to
*                                       be set.
* \param	pos			Given double precision position.
*/
int		WlzGMShellGInBB2D(WlzGMShell *shell, WlzDVertex2 pos)
{
  int		inside = 0;

  if((shell != NULL) && (shell->geo.core != NULL))
  {
    switch(shell->geo.core->type)
    {
      case WLZ_GMELM_SHELL_G2I:
	inside = (pos.vtX >= shell->geo.sg2I->bBox.xMin) &&
		 (pos.vtX <= shell->geo.sg2I->bBox.xMax) &&
	         (pos.vtY >= shell->geo.sg2I->bBox.yMin) &&
		 (pos.vtY <= shell->geo.sg2I->bBox.yMax);
	break;
      case WLZ_GMELM_SHELL_G2D:
	inside = (pos.vtX >= shell->geo.sg2D->bBox.xMin) &&
		 (pos.vtX <= shell->geo.sg2D->bBox.xMax) &&
	         (pos.vtY >= shell->geo.sg2D->bBox.yMin) &&
		 (pos.vtY <= shell->geo.sg2D->bBox.yMax);
	break;
      case WLZ_GMELM_SHELL_G3I:
	inside = (pos.vtX >= shell->geo.sg3I->bBox.xMin) &&
		 (pos.vtX <= shell->geo.sg3I->bBox.xMax) &&
	         (pos.vtY >= shell->geo.sg3I->bBox.yMin) &&
		 (pos.vtY <= shell->geo.sg3I->bBox.yMax);
	break;
      case WLZ_GMELM_SHELL_G3D:
	inside = (pos.vtX >= shell->geo.sg3D->bBox.xMin) &&
		 (pos.vtX <= shell->geo.sg3D->bBox.xMax) &&
	         (pos.vtY >= shell->geo.sg3D->bBox.yMin) &&
		 (pos.vtY <= shell->geo.sg3D->bBox.yMax);
	break;
      default:
	break;
    }
  }
  return(inside);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Sets a veticies geometry using the given double
*               precision position.
* \param	vertex			Given vertex with geometry to
*                                       be set.
* \param	pos			Given position.
*/
WlzErrorNum	WlzGMVertexSetG3D(WlzGMVertex *vertex, WlzDVertex3 pos)
{
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((vertex == NULL) || (vertex->geo.core == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    switch(vertex->geo.core->type)
    {
      case WLZ_GMELM_VERTEX_G2I:
        vertex->geo.vg2I->vtx.vtX = WLZ_NINT(pos.vtX);
        vertex->geo.vg2I->vtx.vtY = WLZ_NINT(pos.vtY);
	break;
      case WLZ_GMELM_VERTEX_G2D:
        vertex->geo.vg2D->vtx.vtX = pos.vtX;
	vertex->geo.vg2D->vtx.vtY = pos.vtY;
	break;
      case WLZ_GMELM_VERTEX_G3I:
        vertex->geo.vg3I->vtx.vtX = WLZ_NINT(pos.vtX);
	vertex->geo.vg3I->vtx.vtY = WLZ_NINT(pos.vtY);
	vertex->geo.vg3I->vtx.vtZ = WLZ_NINT(pos.vtZ);
	break;
      case WLZ_GMELM_VERTEX_G3D:
        vertex->geo.vg3D->vtx = pos;
	break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Sets a veticies geometry using the given double
*               precision position.
* \param	vertex			Given vertex with geometry to
*                                       be set.
* \param	pos			Given position.
*/
WlzErrorNum	WlzGMVertexSetG2D(WlzGMVertex *vertex, WlzDVertex2 pos)
{
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((vertex == NULL) || (vertex->geo.core == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    switch(vertex->geo.core->type)
    {
      case WLZ_GMELM_VERTEX_G2I:
        vertex->geo.vg2I->vtx.vtX = WLZ_NINT(pos.vtX);
        vertex->geo.vg2I->vtx.vtY = WLZ_NINT(pos.vtY);
	break;
      case WLZ_GMELM_VERTEX_G2D:
        vertex->geo.vg2D->vtx = pos;
	break;
      case WLZ_GMELM_VERTEX_G3I:
        vertex->geo.vg3I->vtx.vtX = WLZ_NINT(pos.vtX);
	vertex->geo.vg3I->vtx.vtY = WLZ_NINT(pos.vtY);
	vertex->geo.vg3I->vtx.vtZ = 0;
	break;
      case WLZ_GMELM_VERTEX_G3D:
        vertex->geo.vg3D->vtx.vtX = pos.vtX;
	vertex->geo.vg3D->vtx.vtY = pos.vtY;
	vertex->geo.vg3D->vtx.vtZ = 0.0;
	break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Gets a veticies geometry into the given double
*               precision position destination pointer.
* \param	vertex			Given vertex with geometry to
*                                       be set.
* \param	dstPos			Given position destination
*                                       pointer, may NOT be NULL.
*/
WlzErrorNum	WlzGMVertexGetG3D(WlzGMVertex *vertex, WlzDVertex3 *dstPos)
{
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((vertex == NULL) || (vertex->geo.core == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    switch(vertex->geo.core->type)
    {
      case WLZ_GMELM_VERTEX_G2I:
	dstPos->vtX = vertex->geo.vg2I->vtx.vtX;
	dstPos->vtY = vertex->geo.vg2I->vtx.vtY;
	dstPos->vtZ = 0;
	break;
      case WLZ_GMELM_VERTEX_G2D:
	dstPos->vtX = vertex->geo.vg2D->vtx.vtX;
	dstPos->vtY = vertex->geo.vg2D->vtx.vtY;
	dstPos->vtZ = 0.0;
	break;
      case WLZ_GMELM_VERTEX_G3I:
	dstPos->vtX = vertex->geo.vg3I->vtx.vtX;
	dstPos->vtY = vertex->geo.vg3I->vtx.vtY;
	dstPos->vtZ = vertex->geo.vg3I->vtx.vtZ;
	break;
      case WLZ_GMELM_VERTEX_G3D:
	*dstPos = vertex->geo.vg3D->vtx;
	break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Gets a veticies geometry into the given double
*               precision position destination pointer.
* \param	vertex			Given vertex with geometry to
*                                       be set.
* \param	dstPos			Given position destination
*                                       pointer, may NOT be NULL.
*/
WlzErrorNum	WlzGMVertexGetG2D(WlzGMVertex *vertex, WlzDVertex2 *dstPos)
{
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((vertex == NULL) || (vertex->geo.core == NULL))
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    switch(vertex->geo.core->type)
    {
      case WLZ_GMELM_VERTEX_G2I:
	dstPos->vtX = vertex->geo.vg2I->vtx.vtX;
	dstPos->vtY = vertex->geo.vg2I->vtx.vtY;
	break;
      case WLZ_GMELM_VERTEX_G2D:
	*dstPos = vertex->geo.vg2D->vtx;
	break;
      case WLZ_GMELM_VERTEX_G3I:
	dstPos->vtX = vertex->geo.vg3I->vtx.vtX;
	dstPos->vtY = vertex->geo.vg3I->vtx.vtY;
	break;
      case WLZ_GMELM_VERTEX_G3D:
	dstPos->vtX = vertex->geo.vg3D->vtx.vtX;
	dstPos->vtY = vertex->geo.vg3D->vtx.vtY;
	break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
  }
  return(errNum);
}

/*!
* \return				Position of vertex - given
* \ingroup      WlzGeoModel
*					position.
* \brief	Compares the position of the given vertex with the
*		given 3D double precision position.
* \param	vertex			Given vertex.
* \param	pos			Given position.
*/
WlzDVertex3	WlzGMVertexCmp3D(WlzGMVertex *vertex, WlzDVertex3 pos)
{
  WlzDVertex3	cmp;


  cmp.vtZ = cmp.vtY = cmp.vtX = 0.0;
  if(vertex && vertex->geo.core)
  {
    switch(vertex->geo.core->type)
    {
      case WLZ_GMELM_VERTEX_G2I:
        cmp.vtX = vertex->geo.vg2I->vtx.vtX - pos.vtX;
        cmp.vtY = vertex->geo.vg2I->vtx.vtY - pos.vtY;
        break;
      case WLZ_GMELM_VERTEX_G2D:
        cmp.vtX = vertex->geo.vg2D->vtx.vtX - pos.vtX;
        cmp.vtY = vertex->geo.vg2D->vtx.vtY - pos.vtY;
        break;
      case WLZ_GMELM_VERTEX_G3I:
        cmp.vtX = vertex->geo.vg3I->vtx.vtX - pos.vtX;
        cmp.vtY = vertex->geo.vg3I->vtx.vtY - pos.vtY;
        cmp.vtZ = vertex->geo.vg3I->vtx.vtZ - pos.vtZ;
        break;
      case WLZ_GMELM_VERTEX_G3D:
        cmp.vtX = vertex->geo.vg3D->vtx.vtX - pos.vtX;
        cmp.vtY = vertex->geo.vg3D->vtx.vtY - pos.vtY;
        cmp.vtZ = vertex->geo.vg3D->vtx.vtZ - pos.vtZ;
        break;
      default:
        break;
    }
  }
  return(cmp);
}

/*!
* \return                               Position of vertex - given
* \ingroup      WlzGeoModel
*                                       position.
* \brief        Compares the position of the given vertex with the
*               given 2D double precision position.
* \param        vertex                  Given vertex.
* \param        pos                     Given position.
*/
WlzDVertex2	WlzGMVertexCmp2D(WlzGMVertex *vertex, WlzDVertex2 pos)
{
  WlzDVertex2	cmp;


  cmp.vtY = cmp.vtX = 0.0;
  if(vertex && vertex->geo.core)
  {
    switch(vertex->geo.core->type)
    {
      case WLZ_GMELM_VERTEX_G2I:
        cmp.vtX = vertex->geo.vg2I->vtx.vtX - pos.vtX;
        cmp.vtY = vertex->geo.vg2I->vtx.vtY - pos.vtY;
        break;
      case WLZ_GMELM_VERTEX_G2D:
        cmp.vtX = vertex->geo.vg2D->vtx.vtX - pos.vtX;
        cmp.vtY = vertex->geo.vg2D->vtx.vtY - pos.vtY;
        break;
      case WLZ_GMELM_VERTEX_G3I:
        cmp.vtX = vertex->geo.vg3I->vtx.vtX - pos.vtX;
        cmp.vtY = vertex->geo.vg3I->vtx.vtY - pos.vtY;
        break;
      case WLZ_GMELM_VERTEX_G3D:
        cmp.vtX = vertex->geo.vg3D->vtx.vtX - pos.vtX;
        cmp.vtY = vertex->geo.vg3D->vtx.vtY - pos.vtY;
        break;
      default:
        break;
    }
  }
  return(cmp);
}

/*!
* \return				The sign of the vertex position
* \ingroup      WlzGeoModel
*					- the given position: -1, 0 or +1.
* \brief	Compares the coordinates of the given vertex and
*		3D double precision position to find a signed value
*               for sorting.
* \param	vertex			Given vertex.
* \param	pos			Given position.
*/
int		WlzGMVertexCmpSign3D(WlzGMVertex *vertex, WlzDVertex3 pos)
{
  int		cmp;
  WlzDVertex3	cmp3D;

  cmp3D = WlzGMVertexCmp3D(vertex, pos);
  if(cmp3D.vtZ < -WLZ_GM_TOLERANCE)
  {
    cmp = -1;
  }
  else if(cmp3D.vtZ > WLZ_GM_TOLERANCE)
  {
    cmp = 1;
  }
  else
  {
    if(cmp3D.vtY < -WLZ_GM_TOLERANCE)
    {
      cmp = -1;
    }
    else if(cmp3D.vtY > WLZ_GM_TOLERANCE)
    {
      cmp = 1;
    }
    else
    {
      if(cmp3D.vtX < -WLZ_GM_TOLERANCE)
      {
	cmp = -1;
      }
      else if(cmp3D.vtX > WLZ_GM_TOLERANCE)
      {
	cmp = 1;
      }
      else
      {
	cmp = 0;
      }
    }
  }
  return(cmp);
}

/*!
* \return				The sign of the vertex position
* \ingroup      WlzGeoModel
*					- the given position: -1, 0 or +1.
* \brief	Compares the coordinates of the given vertex and
*		2D double precision position to find a signed value
*               for sorting.
* \param	vertex			Given vertex.
* \param	pos			Given position.
*/
int		WlzGMVertexCmpSign2D(WlzGMVertex *vertex, WlzDVertex2 pos)
{
  int		cmp;
  WlzDVertex2	cmp2D;

  cmp2D = WlzGMVertexCmp2D(vertex, pos);
  if(cmp2D.vtY < -WLZ_GM_TOLERANCE)
  {
    cmp = -1;
  }
  else if(cmp2D.vtY > WLZ_GM_TOLERANCE)
  {
    cmp = 1;
  }
  else
  {
    if(cmp2D.vtX < -WLZ_GM_TOLERANCE)
    {
      cmp = -1;
    }
    else if(cmp2D.vtX > WLZ_GM_TOLERANCE)
    {
      cmp = 1;
    }
    else
    {
      cmp = 0;
    }
  }
  return(cmp);
}

/*!
* \return				Square of distance, -1.0 on
* \ingroup      WlzGeoModel
*                                       error.
* \brief	Calculates the square of the Euclidean distance
*               between the given vertex and the given 3D double
*               precision position.
* \param	vertex			Given vertex.
* \param	pos			Given position.
*/
double		WlzGMVertexDistSq3D(WlzGMVertex *vertex, WlzDVertex3 pos)
{
  double	tD0,
		tD1,
		tD2,
		dstSq = -1.0;

  if(vertex && vertex->geo.core)
  {
    switch(vertex->geo.core->type)
    {
      case WLZ_GMELM_VERTEX_G2I:
        tD0 = pos.vtX - vertex->geo.vg2I->vtx.vtX;
	tD1 = pos.vtY - vertex->geo.vg2I->vtx.vtY;
	dstSq = (tD0 * tD0) + (tD1 * tD1);
        break;
      case WLZ_GMELM_VERTEX_G2D:
        tD0 = pos.vtX - vertex->geo.vg2D->vtx.vtX;
	tD1 = pos.vtY - vertex->geo.vg2D->vtx.vtY;
	dstSq = (tD0 * tD0) + (tD1 * tD1);
        break;
      case WLZ_GMELM_VERTEX_G3I:
        tD0 = pos.vtX - vertex->geo.vg3I->vtx.vtX;
	tD1 = pos.vtY - vertex->geo.vg3I->vtx.vtY;
	tD2 = pos.vtZ - vertex->geo.vg3I->vtx.vtZ;
	dstSq = (tD0 * tD0) + (tD1 * tD1) + (tD2 * tD2);
        break;
      case WLZ_GMELM_VERTEX_G3D:
        tD0 = pos.vtX - vertex->geo.vg3D->vtx.vtX;
	tD1 = pos.vtY - vertex->geo.vg3D->vtx.vtY;
	tD2 = pos.vtZ - vertex->geo.vg3D->vtx.vtZ;
	dstSq = (tD0 * tD0) + (tD1 * tD1) + (tD2 * tD2);
        break;
      default:
        break;
    }
  }
  return(dstSq);
}

/*!
* \return				Value of normal.
* \ingroup      WlzGeoModel
* \brief	Computes the value of the normal at the given vertex
*               which lies within the given model.
*               This function requires a buffer in which to store the
*               verticies found on the loops surrounding the given
*               vertex. For efficiency this can/should be reused
*               between calls of this function.
* \param	model			The given model.
* \param	gV			Given vertex in the model.
* \param	sVBufSz			Ptr to the number WlzGMVertex's
*                                       that can be held in *sVBuf.
* \param	sVBuf			Ptr to an allocated buffer for
*                                       verticies, may NOT be NULL
*                                       although the buffer it points
*                                       to may be. The buffer should
*                                       be free'd using AlcFree when
*                                       it is no longer needed.
* \param	dstErr			Destination error pointer, may
*                                       be null.
*/
WlzDVertex3	WlzGMVertexNormal3D(WlzGMModel *model, WlzGMVertex *gV,
				    int *sVBufSz, WlzGMVertex ***sVBuf,
				    WlzErrorNum *dstErr)
{
  int		sIdx,
  		sCnt,
  		manifold = 1;
  double	tD0;
  WlzGMVertexT	*vT0,
  		*vT1;
  WlzGMEdgeT	*eT1;
  WlzDVertex3	nrm,
  		cNrm,
  		sNrm;
  WlzDVertex3	sVG[3];
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  nrm.vtX = 0.0;
  nrm.vtY = 0.0;
  nrm.vtZ = 0.0;
  if(gV->diskT->idx != gV->diskT->next->idx)
  {
    /* The surface in the imediate neighbourhood of the vertex is
     * not manifold.*/
    manifold = 0;
  }
  else
  {
    /* Find the ordered ring of verticies which surround the current
     * vertex, checking that each of the edges is part of a manifold
     * surface. */

    sCnt = 0;
    vT1 = vT0 = gV->diskT->vertexT;
    eT1 = vT1->parent;
    do
    {
      if((eT1->idx == eT1->rad->idx) ||
	  (eT1->idx != eT1->rad->rad->idx))
      {
	/* More than two loops are joined by the edge, so the surface
	 * in the imediate neighbourhood of the vertex is not manifold.
	 */
	manifold = 0;
      }
      else
      {
	if((*sVBufSz <= sCnt) || (*sVBuf == NULL))
	{
	  *sVBufSz = (*sVBufSz <= 0)? 64: *sVBufSz * 2;
	  if((*sVBuf = (WlzGMVertex **)
		      AlcRealloc(*sVBuf,
		      *sVBufSz * sizeof(WlzGMVertex *))) == NULL)
	  {
	    errNum = WLZ_ERR_MEM_ALLOC;
	  }
	}
	if(errNum == WLZ_ERR_NONE)
	{
	  *(*sVBuf + sCnt++) = eT1->opp->vertexT->diskT->vertex;
	}
	/* Find the next edgeT directed from gV. */
	eT1 = eT1->prev->opp->rad;
	vT1 = eT1->vertexT;
      }
    }
    while((errNum == WLZ_ERR_NONE) && manifold &&
	(vT1->idx != vT0->idx));
  }
  if(errNum == WLZ_ERR_NONE)
  {
    if(manifold)
    {
      /* Compute the mean of the normals of the loops around the
       * current vertex. */
      sNrm.vtX = 0.0;
      sNrm.vtY = 0.0;
      sNrm.vtZ = 0.0;
      (void )WlzGMVertexGetG3D(gV, sVG + 0);
      (void )WlzGMVertexGetG3D(*(*sVBuf + 0), sVG + 2);
      for(sIdx = 0; sIdx < sCnt; ++sIdx)
      {
	*(sVG + 1) = *(sVG + 2);
	(void )WlzGMVertexGetG3D(*(*sVBuf + ((sIdx + 1) % sCnt)), sVG + 2);
	cNrm = WlzGeomTriangleNormal(*(sVG + 0), *(sVG + 1), *(sVG + 2));
	WLZ_VTX_3_ADD(sNrm, sNrm, cNrm);
      }
      tD0 = 1.0 / (double)sCnt;
      WLZ_VTX_3_SCALE(nrm, sNrm, tD0);
    }
    /* else normal undefined. */
  }
  if(dstErr)
  {
    *dstErr == errNum;
  }
  return(nrm);
}

/*!
* \return				Square of distance, -1.0 on
* \ingroup      WlzGeoModel
*                                       error.
* \brief	Calculates the square of the Euclidean distance
*               between the given vertex and the given 2D double
*               precision position.
* \param	vertex			Given vertex.
* \param	pos			Given position.
*/
double		WlzGMVertexDistSq2D(WlzGMVertex *vertex, WlzDVertex2 pos)
{
  double	tD0,
		tD1,
		dstSq = -1.0;

  if(vertex && vertex->geo.core)
  {
    switch(vertex->geo.core->type)
    {
      case WLZ_GMELM_VERTEX_G2I:
        tD0 = pos.vtX - vertex->geo.vg2I->vtx.vtX;
	tD1 = pos.vtY - vertex->geo.vg2I->vtx.vtY;
	dstSq = (tD0 * tD0) + (tD1 * tD1);
        break;
      case WLZ_GMELM_VERTEX_G2D:
        tD0 = pos.vtX - vertex->geo.vg2D->vtx.vtX;
	tD1 = pos.vtY - vertex->geo.vg2D->vtx.vtY;
	dstSq = (tD0 * tD0) + (tD1 * tD1);
        break;
      case WLZ_GMELM_VERTEX_G3I:
        tD0 = pos.vtX - vertex->geo.vg3I->vtx.vtX;
	tD1 = pos.vtY - vertex->geo.vg3I->vtx.vtY;
	dstSq = (tD0 * tD0) + (tD1 * tD1);
        break;
      case WLZ_GMELM_VERTEX_G3D:
        tD0 = pos.vtX - vertex->geo.vg3D->vtx.vtX;
	tD1 = pos.vtY - vertex->geo.vg3D->vtx.vtY;
	dstSq = (tD0 * tD0) + (tD1 * tD1);
        break;
      default:
        break;
    }
  }
  return(dstSq);
}

/*!
* \return				Hash value.
* \ingroup      WlzGeoModel
* \brief	Computes a hash value from a given 3D double precision
*               position.
* \param	pos			Given position.
*/
static unsigned int WlzGMHashPos3D(WlzDVertex3 pos)
{
  unsigned int	hashVal;
  double	fF,
  		fI;
  const unsigned int pX = 399989, /* These are just different 6 digit primes */
  		pY = 599999,
		pZ = 999983;
  
  fF = modf(pos.vtX, &fI);
  fF = floor(fF / WLZ_GM_TOLERANCE) * WLZ_GM_TOLERANCE;
  fI *= pX;
  fF *= pY * pZ;
  hashVal = ((long long )fI + (long long )fF) & UINT_MAX;
  fF = modf(pos.vtY, &fI);
  fF = floor(fF / WLZ_GM_TOLERANCE) * WLZ_GM_TOLERANCE;
  fI *= pY;
  fF *= pZ * pX;
  hashVal ^= ((long long )fI + (long long )fF) & UINT_MAX;
  fF = modf(pos.vtZ, &fI);
  fF = floor(fF / WLZ_GM_TOLERANCE) * WLZ_GM_TOLERANCE;
  fI *= pZ;
  fF *= pX * pY;
  hashVal ^= ((long long )fI + (long long )fF) & UINT_MAX;
  return(hashVal);
}

/*!
* \return				Hash value.
* \ingroup      WlzGeoModel
* \brief	Computes a hash value from a given 2D double precision
*               position.
* \param	pos			Given position.
*/
static unsigned int WlzGMHashPos2D(WlzDVertex2 pos)
{
  unsigned int	hashVal;
  double	fF,
  		fI;
  const unsigned int pX = 399989, /* These are just different 6 digit primes */
  		pY = 599999,
		pZ = 999983;
  
  fF = modf(pos.vtX, &fI);
  fF = floor(fF / WLZ_GM_TOLERANCE) * WLZ_GM_TOLERANCE;
  fI *= pX;
  fF *= pY * pZ;
  hashVal = ((long long )fI + (long long )fF) & UINT_MAX;
  fF = modf(pos.vtY, &fI);
  fF = floor(fF / WLZ_GM_TOLERANCE) * WLZ_GM_TOLERANCE;
  fI *= pY;
  fF *= pZ * pX;
  hashVal ^= ((long long )fI + (long long )fF) & UINT_MAX;
  return(hashVal);
}

/*!
* \return				Matched vertex, or NULL if no
* \ingroup      WlzGeoModel
*                                       vertex with the given geometry.
* \brief	Attempts to find a vertex which matches the given
*               double precision 3D position.
* \param	model			Model with resources.
* \param	gPos			Position to match.
*/
WlzGMVertex	*WlzGMModelMatchVertexG3D(WlzGMModel *model, WlzDVertex3 gPos)
{
  int		cmp;
  unsigned int	hVal;
  WlzGMVertex	*tV,
  		*mV = NULL;

  hVal = WlzGMHashPos3D(gPos);
  if((tV = *(model->vertexHT + (hVal % model->vertexHTSz))) != NULL)
  {
    /* Have found the hash table head, a linked list of verticies with this
     * hash value. Now need to search for a matching vertex in the linked 
     * list. */
    do
    {
      if((cmp = WlzGMVertexCmpSign3D(tV, gPos)) == 0)
      {
        mV = tV;
      }
      else
      {
        tV = tV->next;
      }
    }
    while((cmp < 0) && tV);
  }
  return(mV);
}

/*!
* \return				Matched vertex, or NULL if no
* \ingroup      WlzGeoModel
*                                       vertex with the given geometry.
* \brief	Attempts to find a vertex which matches the given
*               double precision 2D position.
* \param	model			Model with resources.
* \param	gPos			Position to match.
*/
WlzGMVertex	*WlzGMModelMatchVertexG2D(WlzGMModel *model, WlzDVertex2 gPos)
{
  int		cmp;
  unsigned int	hVal;
  WlzGMVertex	*tV,
  		*mV = NULL;

  hVal = WlzGMHashPos2D(gPos);
  if((tV = *(model->vertexHT + (hVal % model->vertexHTSz))) != NULL)
  {
    /* Have found the hash table head, a linked list of verticies with this
     * hash value. Now need to search for a matching vertex in the linked 
     * list. */
    do
    {
      if((cmp = WlzGMVertexCmpSign2D(tV, gPos)) == 0)
      {
        mV = tV;
      }
      else
      {
        tV = tV->next;
      }
    }
    while((cmp < 0) && tV);
  }
  return(mV);
}

/* Topology query */

/*!
* \return				Ptr to an array of non-manifold
* \ingroup      WlzGeoModel
*                                       edge ptrs, NULL if none exist or
*                                       on error.
* \brief	Finds a loop topology element in common for the two
*               edge topology elements.
* \param	model			The given model.
* \param	dstNMCnt		Destination pointer for number
*                                       of non-manifold edges found.
* \param	dstErr			Destination error pointer, may
*                                       be null.
*/
WlzGMEdge	**WlzGMModelFindNMEdges(WlzGMModel *model, int *dstNMCnt,
				        WlzErrorNum *dstErr)
{
  int		tEI,
  		nmEI;
  int		*nmEIP;
  WlzGMEdge	*tE;
  WlzGMEdge	**nmE = NULL;
  WlzGMEdgeT	*tET;
  AlcVector	*nmEIVec = NULL;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if((nmEIVec = AlcVectorNew(1, sizeof(int), 1024, NULL)) == NULL)
  {
    errNum = WLZ_ERR_MEM_ALLOC;
  }
  if(errNum == WLZ_ERR_NONE)
  {
    switch(model->type)
    {
      case WLZ_GMMOD_2I:
      case WLZ_GMMOD_2D:
        break;
      case WLZ_GMMOD_3I:
      case WLZ_GMMOD_3D:
	tEI = 0;
	nmEI = 0;
	while((errNum == WLZ_ERR_NONE) && (tEI < model->res.edge.numIdx))
	{
	  tE = (WlzGMEdge *)AlcVectorItemGet(model->res.edge.vec, tEI);
	  if(tE->idx >= 0)
	  {
	    tET = tE->edgeT;
	    if((tET->rad->idx == tET->idx) ||
	       (tET->rad->rad->idx != tET->idx))
	    {
	      if((nmEIP = (int *)
			  (AlcVectorExtendAndGet(nmEIVec, nmEI))) == NULL)
	      {
		errNum = WLZ_ERR_MEM_ALLOC;
	      }
	      else
	      {
		*nmEIP = tE->idx;
	      }
	      ++nmEI;
	    }
	  }
	  ++tEI;
	}
	break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
  }
  if((errNum == WLZ_ERR_NONE) && (nmEI > 0))
  {
    if((nmE = (WlzGMEdge **)AlcMalloc(sizeof(WlzGMEdge *) * nmEI)) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
    else
    {
      for(tEI = 0; tEI < nmEI; ++tEI)
      {
        *(nmE + tEI) = (WlzGMEdge *)AlcVectorItemGet(model->res.edge.vec,
						     tEI);
      }
    }
  }
  if(dstNMCnt)
  {
    *dstNMCnt = (errNum == WLZ_ERR_NONE)? nmEI: 0;
  }
  if(nmEIVec)
  {
    (void )AlcVectorFree(nmEIVec);
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(nmE);
}

/*!
* \return				Common loop topology element,
* \ingroup      WlzGeoModel
*                                       NULL if it doesn't exist.
* \brief	Finds a loop topology element in common for the two
*               edge topology elements.
* \param	eT0			First edge topology element.
* \param	eT1			Second edge topology element.
*/
WlzGMLoopT	*WlzGMEdgeTCommonLoopT(WlzGMEdgeT *eT0, WlzGMEdgeT *eT1)
{
  WlzGMLoopT	*loopT = NULL;

  if(eT0->parent->idx == eT1->parent->idx)
  {
    loopT = eT0->parent;
  }
  return(loopT);
}

/*!
* \return				Common edge, NULL if it doesn't
* \ingroup      WlzGeoModel
                                        exist.
* \brief	Finds the edge common to the two given verticies.
* \param	eV0			First vertex element.
* \param	eV1			Second vertex element.
*/
WlzGMEdge	*WlzGMVertexCommonEdge(WlzGMVertex *eV0, WlzGMVertex *eV1)
{
  WlzGMVertexT	*vT0,
		*vT1,
  		*eVT0,
  		*eVT1;
  WlzGMEdge	*e0,
		*edge = NULL;

  vT0 = eVT0 = eV0->diskT->vertexT;
  vT1 = eVT1 = eV1->diskT->vertexT;
  do
  {
    e0 = vT0->parent->edge;
    do
    {
      if(e0->idx == vT1->parent->edge->idx)
      {
	edge = e0;
      }
    } while((edge == NULL) && ((vT1 = vT1->next)->idx != eVT1->idx));
  } while((edge == NULL) && ((vT0 = vT0->next)->idx != eVT0->idx));
  return(edge);
}

/*!
* \return				Common shell, NULL if it doesn't
* \ingroup      WlzGeoModel
                                        exist.
* \brief	Finds the shell common to the two given verticies.
* \param	eV0			First vertex element.
* \param	eV1			Second vertex element.
*/
WlzGMShell	*WlzGMVertexCommonShell(WlzGMVertex *eV0, WlzGMVertex *eV1)
{
  WlzGMShell	*s0,
  		*s1,
		*shell = NULL;

  if(((s0 = WlzGMVertexGetShell(eV0)) != NULL) &&
     ((s1 = WlzGMVertexGetShell(eV1)) != NULL) &&
     (s0->idx == s1->idx))
  {
    shell = s0;
  }
  return(shell);
}

/*!
* \return				The parent shell.
* \ingroup      WlzGeoModel
* \brief	Finds the parent shell of the given vertex.
* \param	eV			Given vertex element.
*/
WlzGMShell	*WlzGMVertexGetShell(WlzGMVertex *eV)
{
  WlzGMShell	*pS = NULL;

  if(eV && (eV->type == WLZ_GMELM_VERTEX))
  {
    pS = eV->diskT->vertexT->parent->parent->parent;
  }
  return(pS);
}

/*!
* \return				The parent shell.
* \ingroup      WlzGeoModel
* \brief	Finds the parent shell of the given edge.
* \param	eE			Given edge element.
*/
WlzGMShell	*WlzGMEdgeGetShell(WlzGMEdge *eE)
{
  WlzGMShell	*pS = NULL;

  if(eE && (eE->type == WLZ_GMELM_EDGE))
  {
    pS = eE->edgeT->parent->parent;
  }
  return(pS);
}

/*!
* \return				The common vertex, NULL if it
* \ingroup      WlzGeoModel
*                                       doesn't exist.
* \brief	Finds the common vertex of the two given edges.
* \param	eE0			First edge element.
* \param	eE1			Second edge element.
*/
WlzGMVertex	*WlzGMEdgeCommonVertex(WlzGMEdge *eE0, WlzGMEdge *eE1)
{
  WlzGMVertex	*tV,
  		*cV = NULL;

  if(((tV = eE0->edgeT->vertexT->diskT->vertex)->idx ==
      eE1->edgeT->vertexT->diskT->vertex->idx) ||
     (tV->idx == eE1->edgeT->opp->vertexT->diskT->vertex->idx))
  {
    cV = tV;
  }
  else if(((tV = eE0->edgeT->opp->vertexT->diskT->vertex)->idx ==
           eE1->edgeT->vertexT->diskT->vertex->idx) ||
          (tV->idx == eE1->edgeT->opp->vertexT->diskT->vertex->idx))
  {
    cV = tV;
  }
  return(cV);
}

/* Model list management */

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Append new vertex topology element onto a doubly
*               linked list of vertex topology element's, knowing that
*               neither is NULL.
*               Vertex topology elements are maintained in an unordered
*               doubly linked list.
* \param	eVT			Existing vertexT in list of
*                                       vertexT's.
* \param	nVT			New vertex topology element to
*                                       append to list after existing
*                                       element.
*/
void	   	WlzGMVertexTAppend(WlzGMVertexT *eVT, WlzGMVertexT *nVT)
{
  nVT->next = eVT->next;
  nVT->prev = eVT;
  nVT->next->prev = nVT;
  nVT->prev->next = nVT;
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Append new edge topology onto a doubly linked list
*               of disk topology element's, knowing that neither is NULL.
*               Disk topology elements are maintained in an unordered
*               doubly linked list.
* \param	eDT			Existing disk topology element in
*                                       list.
* \param	nDT			New disk topology element to
*                                       append to list after existing
*                                       element.
*/
void	   	WlzGMDiskTAppend(WlzGMDiskT *eDT, WlzGMDiskT *nDT)
{
  nDT->next = eDT->next;
  nDT->prev = eDT;
  nDT->next->prev = nDT;
  nDT->prev->next = nDT;
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Append new edge topology onto a doubly linked list
*               of edge topology element's, knowing that neither is NULL.
*               Edge topology elements are maintained in an ordered
*               doubly linked list, CCW around the inside of loops and
*               CW around the outside of 2D loops.
* \param	eET			Existing edge topology element in
*                                       list.
* \param	nET			New edge topology element to
*                                       append to list after existing
*                                       element.
*/
void	   	WlzGMEdgeTAppend(WlzGMEdgeT *eET, WlzGMEdgeT *nET)
{
  nET->next = eET->next;
  nET->prev = eET;
  nET->next->prev = nET;
  nET->prev->next = nET;
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Insert new edge topology into a doubly linked list
*               of edge topology element's, knowing that neither is NULL.
*               Edge topology elements are maintained in an ordered
*               doubly linked list, CCW around the inside of loops and
*               CW around the outside of 2D loops.
* \param	eET			Existing edge topology element in
*                                       list.
* \param	nET			New edge topology element to
*                                       insert in list before existing
*                                       element.
*/
void	   	WlzGMEdgeTInsert(WlzGMEdgeT *eET, WlzGMEdgeT *nET)
{
  nET->next = eET;
  nET->prev = eET->prev;
  nET->next->prev = nET;
  nET->prev->next = nET;
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Inserts the given new edge topology element into a
*		radially sorted cyclic list of edge topology element's.
*
*       	Inserts the given new edge topology element into a
*		radially sorted cyclic list of edge topology element's.
*		In 2D the radial edgeT is always the edgeT itself and
*		this function should not be called.
*		In 3D the radial edge is a similarly directed edge
*		topology element to the given edge topology element
*		who's off edge vertex is the next CCW off edge vertex
*		when viewed along the edge.
* \verbatim
*                                      O <-                       
*                                     /    \                      
*                                    /      \ Next radial edge    
*                                   /        |                    
*                                  /         |                    
*                                 X----------O                    
*                           Given edge into    Off edge vertex
*                           screen/page        of new simplex
* \endverbatim
*                                                                 
*		Given a directed edge which forms part of a loop
*		topology element in a new triangle simplex. Define
*		points around the the simplex:
* \verbatim
*                                O p1                                     
*                              ^ |\                                      
*                              | | \                                     
*                              | |  \                                    
*                              | |   \                                   
*                              | |    \                                  
*                          nET | |     O p2                               
*                              | |    /                                  
*                              | |   /                                   
*                              | |  /                                    
*                              | | /                                     
*                              | |/                                      
*                                O p0                                    
* \endverbatim
* 		To find the next CCW simplex.
*		Find the normal vector:
*		  \f[ n_0 = \frac{(p_0 - p_1)}{|p_0 - p_1|} \f]
*		Find the normal vector perpendicular to the new
*		(triangular) simplex:
*		  \f[ n_1 = \frac{n_0(p_2 - p_0)}{|n_0(p_2 - p_0)}\f]
*		Find the normalized vector perpendicular to \f$n_0\f$
*		and \f$n_1\f$:
*		  \f[n_2 = n_0 n_1 \f]
*		Because \f$n_1\f$ and \f$n_2\f$ are perpendicular axes
*		in the plane perpendicular to \f$p_1 - p_0\f$ we can
*		compute the angle of any vector projected onto the
*		plane as:
* \f[ \theta = \tan^{-1}{\frac{v_i n_2}{v_i n_1}}, 0 \leq \theta \leq 2\pi \f]
*		where
*		  \f[v_i = p_{2i} - p_0\f]
*		and \f$p_{2i}\f$ are the \f$p_2\f$ verticies on the
*		candidate face.
*		Use the projections to find the previous radial edge,
*		then insert the edge in the cyclic radial list.
* \param	nET 			New edge topology element to
*					insert in list. The new edge 
*					topology element MUST have all
*					it's fields set EXCEPT for the
*					radial edge link.
*/
void	   	WlzGMEdgeTInsertRadial(WlzGMEdgeT *nET)
{
  double	tD0;
  WlzDVertex2	nGrd,
  		pGrd,
		tGrd;
  WlzDVertex3	p0,
  		p1,
		p2,
		n0,
		n1,
		n2,
		v2;
  WlzGMEdgeT	*pET,
		*fET,
		*tET;

  /* Get an edgeT of this edge with same direction as the given edgeT. */
  fET = nET->edge->edgeT;
  if(fET->vertexT->diskT->vertex->idx != nET->vertexT->diskT->vertex->idx)
  {
    /* fET is antiparrallel to the new edgeT so use it's opposite. */
    fET = fET->opp;
  }
  if(fET->rad->idx == fET->idx)
  {
    /* Edge not shared. */
    pET = fET;
  }
  else
  {
    /* Get the geometry of the new edgeT's simplex. */
    (void )WlzGMVertexGetG3D(nET->vertexT->diskT->vertex, &p0);
    (void )WlzGMVertexGetG3D(nET->next->vertexT->diskT->vertex, &p1);
    (void )WlzGMVertexGetG3D(nET->prev->vertexT->diskT->vertex, &p2);
    /* Compute the normal vectors: n0, n1 and n2. */
    WLZ_VTX_3_SUB(n0, p0, p1);
    tD0 = 1.0 / WLZ_VTX_3_LENGTH(n0);
    WLZ_VTX_3_SCALE(n0, n0, tD0);
    WLZ_VTX_3_SUB(v2, p2, p0);
    WLZ_VTX_3_CROSS(n1, n0, v2);
    tD0 = 1.0 / WLZ_VTX_3_LENGTH(n1);
    WLZ_VTX_3_SCALE(n1, n1, tD0);
    WLZ_VTX_3_CROSS(n2, n0, n1);
    /* Compute the angle from the origin to the point projected onto the
     * plane (defined by n1 and n2) by p2. */
    nGrd.vtX = WLZ_VTX_3_DOT(v2, n1);
    nGrd.vtY = WLZ_VTX_3_DOT(v2, n2);
    /* Search for the edgeT that should be the previous radial edge to the
     * new edgeT by searching for the edgeT which forms part of a simplex
     * with the smallest CW angle when viewed along the directed edgeT. */
    pET = tET = fET;
    (void )WlzGMVertexGetG3D(pET->prev->vertexT->diskT->vertex, &p2);
    WLZ_VTX_3_SUB(v2, p2, p0);
    pGrd.vtX = WLZ_VTX_3_DOT(v2, n1);
    pGrd.vtY = WLZ_VTX_3_DOT(v2, n2);
    while((tET = tET->rad)->idx != fET->idx)
    {
      (void )WlzGMVertexGetG3D(tET->prev->vertexT->diskT->vertex, &p2);
      WLZ_VTX_3_SUB(v2, p2, p0);
      tGrd.vtX = WLZ_VTX_3_DOT(v2, n1);
      tGrd.vtY = WLZ_VTX_3_DOT(v2, n2);
      /* Check to see if the test angle (gradient tGrd), is CCW wrt
       * the previous angle (gradient pGrd) and CW wrt the new angle
       * (gradient nGrd). If so the test angle and edgeT become the new
       * previous values. Do this by testing the orientation of the
       * triangle. */
      if(WlzGeomTriangleSnArea2(nGrd, pGrd, tGrd) > 0)
      {
	pET = tET;
        pGrd = tGrd;
      }
    }
  }
  /* Insert nET after pET and before pET->rad. */
  nET->rad = pET->rad;
  pET->rad = nET;
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Append new loop topology onto a doubly linked list
*               of loop topology element's, knowing that neither is NULL.
*               Loop topology elements are maintained in an unordered
*               doubly linked list.
* \param	eLT			Existing loop topology element in
*                                       list.
* \param	nLT			New loop topology element to
*                                       append to list after existing
*                                       element.
*/
void	   	WlzGMLoopTAppend(WlzGMLoopT *eLT, WlzGMLoopT *nLT)
{
  nLT->next = eLT->next;
  nLT->prev = eLT;
  nLT->next->prev = nLT;
  nLT->prev->next = nLT;
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Unlinks the given loop topology element from a doubly
*               linked list of loop topology element's, knowing that
*               it is not NULL. If this is the only loop topology
*		element in parent the parent's list of loop topology
*		elements is set to NULL. Other elements which point
*		to this loop topology element are not modified.
* \param	dLT			Loop topology element to be
*                                       unlinked.
*/
void		WlzGMLoopTUnlink(WlzGMLoopT *dLT)
{
  dLT->prev->next = dLT->next;
  dLT->next->prev = dLT->prev;
  if(dLT == dLT->next)
  {
    dLT->next = dLT->prev = NULL;
  }
  if(dLT == dLT->parent->child)
  {
    dLT->parent->child = dLT->next;
  }
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Unlinks the given edge topology element from it's loop
*		topology element and it's opposite edge topology element.
*		The opposite edge topology element MUST be unlinked before
*		the model is valid again.
* \param	dET			Edge topology element to be
*                                       unlinked.
*/
void		WlzGMEdgeTUnlink(WlzGMEdgeT *dET)
{
    WlzGMEdgeT	*cET,
    		*pET;

  if(dET != dET->opp)
  {
    dET->prev->next = dET->opp->next;  
    dET->next->prev = dET->opp->prev;
    dET->opp->opp = dET->opp;
  }
  else
  {
    /* No longer have an opposite edgeT as it's already been unlinked. */
    pET = dET->prev;
    while((cET = pET->opp->prev) != dET->prev)
    {
      pET = cET;
    }
    dET->prev->next = pET->opp;
    pET = dET->next;
    while((cET = pET->opp->next) != dET->next)
    {
      pET = cET;
    }
    dET->next->prev = pET->opp;
  }
  if(dET == dET->next)
  {
    dET->next = NULL;
  }
  if(dET == dET->parent->edgeT)
  {
    dET->parent->edgeT = (dET->next->idx >= 0)? dET->next: dET->prev;
  }
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Unlinks the given vertex topology element from a doubly
*               linked list of vertex topology element's, knowing that
*               it is not NULL. If this is the only vertex topology
*		element in parent the parent's list of vertex topology
*		elements is set to NULL. Other elements which point
*		to this vertex topology element are not modified.
* \param	dVT			Vertex topology element to be
*                                       unlinked.
*/
void		WlzGMVertexTUnlink(WlzGMVertexT *dVT)
{
  dVT->prev->next = dVT->next;  
  dVT->next->prev = dVT->prev;
  if(dVT == dVT->next)
  {
    dVT->next = dVT->prev = NULL;
  }
  if(dVT == dVT->parent->vertexT)
  {
    dVT->parent->vertexT = dVT->next;
  }
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Unlinks the given disk topology element from a doubly
*               linked list of disk topology element's, knowing that
*               it is not NULL. If this is the only disk topology
*		element of the vertex the vertex's list of disk topology
*		elements is set to NULL. Other elements which point
*		to this disk topology element are not modified.
* \param	dDT			Disk topology element to be
*                                       unlinked.
*/
void		WlzGMDiskTUnlink(WlzGMDiskT *dDT)
{
  dDT->prev->next = dDT->next;
  dDT->next->prev = dDT->prev;
  if(dDT == dDT->next)
  {
    dDT->next = dDT->prev = NULL;
  }
  if(dDT == dDT->vertex->diskT)
  {
    dDT->vertex->diskT = dDT->next;
  }
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Append new shell onto a doubly linked list of
*               shells, knowing that neither is NULL.
*               A model's shells are maintained in an unordered
*               doubly linked list.
* \param	eS			Existing shell in list of shells.
* \param	nS			New shell to append to list
*                                       after existing shell.
*/
void	   	WlzGMShellAppend(WlzGMShell *eS, WlzGMShell *nS)
{
  nS->next = eS->next;
  nS->prev = eS;
  nS->next->prev = nS;
  nS->prev->next = nS;
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Unlinks the given shell from a doubly linked list of
*               shells kept by the model, knowing that the given shell
*               ptr is not NULL.
* \param	dS			Shell to be unlinked.
*/
void		WlzGMShellUnlink(WlzGMShell *dS)
{
  if(dS->idx == dS->parent->child->idx)
  {
    /* Change child held by parent model */
    if(dS->idx == dS->next->idx)
    {
      /* The model only has one shell and this is it. */
      dS->parent = NULL;
    }
    else
    {
      dS->parent->child = dS->next;
    }
  }
  if(dS->next != NULL)
  {
    /* Unlink the shell from the doubly linked list of shells. */
    dS->prev->next = dS->next;
    dS->next->prev = dS->prev;
  }
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Joins the shell to be unlinked onto the shell that's to
*               be extended and then unlinks (deletes) it.
* \param	eShell			Shell to be extended.
* \param	dShell			Shell to be joined and then
*                                       unlinked.
*/
void		WlzGMShellJoinAndUnlink(WlzGMShell *eShell, WlzGMShell *dShell)
{
  WlzGMLoopT	*eLT,
  		*dLT,
		*tLT;
  WlzDBox3	bBox;

  if(eShell && dShell &&
     ((eLT = eShell->child) != NULL) && ((dLT = dShell->child) != NULL))
  {
    /* Update extended shells geometry. */
    (void )WlzGMShellGetGBB3D(dShell, &bBox);
    (void )WlzGMShellUpdateGBB3D(eShell, bBox);
    /* Update extended shells topology. */
    tLT = dLT;
    do
    {
      tLT->parent = eShell;
      tLT = tLT->next;
    } while(tLT->idx != dLT->idx);
    tLT = eLT->prev;
    tLT->next = dLT;
    eLT->prev = dLT->prev;
    tLT->next->prev = tLT;
    eLT->prev->next = eLT;
    /* Unlink the now childless shell. */
    WlzGMShellUnlink(dShell);
  }
}

/*!
* \return				New resource index table, NULL
* \ingroup      WlzGeoModel
*                                       on error.
* \brief	Makes an index look up table data structure for the
*               given model.
* \param	model			The model with the vertex
*                                       resources.
* \param	eMsk			Element mask with bits set for
*                                       elemet resources to index.
* \param	dstErr			Destination error pointer, may
*                                       be null.
*/
WlzGMResIdxTb	*WlzGMModelResIdx(WlzGMModel *model, unsigned int eMsk,
				  WlzErrorNum *dstErr)
{
  int		dim,
  		iCnt,
		vCnt;
  unsigned int 	eMskTst;
  int		*iLut;
  AlcVector	*vec;
  WlzGMElemP	eP;
  WlzGMResIdxTb	*resIdxTb = NULL;
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
        dim = 2;
	break;
      case WLZ_GMMOD_3I:
      case WLZ_GMMOD_3D:
	dim = 3;
        break;
      default:
        errNum = WLZ_ERR_DOMAIN_TYPE;
	break;
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    eMskTst = eMsk;
    if((resIdxTb = (WlzGMResIdxTb *)
    		   AlcCalloc(1, sizeof(WlzGMResIdxTb))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  /* Set initial element counts and allocate the index luts. */
  if((errNum == WLZ_ERR_NONE) && (eMsk & WLZ_GMELMFLG_VERTEX))
  {
    eMskTst &= ~(WLZ_GMELMFLG_VERTEX);
    vec = model->res.vertex.vec;
    resIdxTb->vertex.idxCnt = model->res.vertex.numIdx;
    if((resIdxTb->vertex.idxLut = (int *)
	    	AlcCalloc(resIdxTb->vertex.idxCnt, sizeof(int))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if((errNum == WLZ_ERR_NONE) && (eMsk & WLZ_GMELMFLG_VERTEX_T))
  {
    eMskTst &= ~(WLZ_GMELMFLG_VERTEX_T);
    vec = model->res.vertexT.vec;
    resIdxTb->vertexT.idxCnt = model->res.vertexT.numIdx;
    if((resIdxTb->vertexT.idxLut = (int *)
    		AlcCalloc(resIdxTb->vertexT.idxCnt, sizeof(int))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if((errNum == WLZ_ERR_NONE) && (eMsk & WLZ_GMELMFLG_VERTEX_G))
  {
    eMskTst &= ~(WLZ_GMELMFLG_VERTEX_G);
    vec = model->res.vertexG.vec;
    resIdxTb->vertexG.idxCnt = model->res.vertexG.numIdx;
    if((resIdxTb->vertexG.idxLut = (int *)
    		AlcCalloc(resIdxTb->vertexG.idxCnt, sizeof(int))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if((errNum == WLZ_ERR_NONE) && (eMsk & WLZ_GMELMFLG_DISK_T))
  {
    eMskTst &= ~(WLZ_GMELMFLG_DISK_T);
    vec = model->res.diskT.vec;
    resIdxTb->diskT.idxCnt = model->res.diskT.numIdx;
    if((resIdxTb->diskT.idxLut = (int *)
	    	AlcCalloc(resIdxTb->diskT.idxCnt, sizeof(int))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if((errNum == WLZ_ERR_NONE) && (eMsk & WLZ_GMELMFLG_EDGE))
  {
    eMskTst &= ~(WLZ_GMELMFLG_EDGE);
    vec = model->res.edge.vec;
    resIdxTb->edge.idxCnt = model->res.edge.numIdx;
    if((resIdxTb->edge.idxLut = (int *)
	    	AlcCalloc(resIdxTb->edge.idxCnt, sizeof(int))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if((errNum == WLZ_ERR_NONE) && (eMsk & WLZ_GMELMFLG_EDGE_T))
  {
    eMskTst &= ~(WLZ_GMELMFLG_EDGE_T);
    vec = model->res.edgeT.vec;
    resIdxTb->edgeT.idxCnt = model->res.edgeT.numIdx;
    if((resIdxTb->edgeT.idxLut = (int *)
	    	AlcCalloc(resIdxTb->edgeT.idxCnt, sizeof(int))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if((errNum == WLZ_ERR_NONE) && (eMsk & WLZ_GMELMFLG_FACE) && (dim == 3))
  {
    eMskTst &= ~(WLZ_GMELMFLG_FACE);
    vec = model->res.face.vec;
    resIdxTb->face.idxCnt = model->res.face.numIdx;
    if((resIdxTb->face.idxLut = (int *)
	    	AlcCalloc(resIdxTb->face.idxCnt, sizeof(int))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if((errNum == WLZ_ERR_NONE) && (eMsk & WLZ_GMELMFLG_LOOP_T))
  {
    eMskTst &= ~(WLZ_GMELMFLG_LOOP_T);
    vec = model->res.loopT.vec;
    resIdxTb->loopT.idxCnt = model->res.loopT.numIdx;
    if((resIdxTb->loopT.idxLut = (int *)
	    	AlcCalloc(resIdxTb->loopT.idxCnt, sizeof(int))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if((errNum == WLZ_ERR_NONE) && (eMsk & WLZ_GMELMFLG_SHELL))
  {
    eMskTst &= ~(WLZ_GMELMFLG_SHELL);
    vec = model->res.shell.vec;
    resIdxTb->shell.idxCnt = model->res.shell.numIdx;
    if((resIdxTb->shell.idxLut = (int *)
	    	AlcCalloc(resIdxTb->shell.idxCnt, sizeof(int))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if((errNum == WLZ_ERR_NONE) && (eMsk & WLZ_GMELMFLG_SHELL_G))
  {
    eMskTst &= ~(WLZ_GMELMFLG_SHELL_G);
    vec = model->res.shellG.vec;
    resIdxTb->shellG.idxCnt = model->res.shellG.numIdx;
    if((resIdxTb->shellG.idxLut = (int *)
	    	AlcCalloc(resIdxTb->shellG.idxCnt, sizeof(int))) == NULL)
    {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }
  if((errNum == WLZ_ERR_NONE) && (eMskTst != 0))
  {
    errNum = WLZ_ERR_PARAM_DATA;
  }
  if(errNum == WLZ_ERR_NONE)
  {
    if(eMsk & WLZ_GMELMFLG_VERTEX)
    {
      /* Compute vertex index lut. */
      vCnt = iCnt = 0;
      vec = model->res.vertex.vec;
      iLut = resIdxTb->vertex.idxLut;
      while(vCnt < resIdxTb->vertex.idxCnt)
      {
	eP.vertex = (WlzGMVertex *)AlcVectorItemGet(vec, vCnt++);
	if(eP.vertex->idx >= 0)
	{
	  *(iLut + eP.vertex->idx) = iCnt++;
	}
      }
      resIdxTb->vertex.idxCnt = iCnt;
    }
    if(eMsk & WLZ_GMELMFLG_VERTEX_T)
    {
      /* Compute vertexT index lut. */
      vCnt = iCnt = 0;
      vec = model->res.vertexT.vec;
      iLut = resIdxTb->vertexT.idxLut;
      while(vCnt < resIdxTb->vertexT.idxCnt)
      {
	eP.vertexT = (WlzGMVertexT *)AlcVectorItemGet(vec, vCnt++);
	if(eP.vertexT->idx >= 0)
	{
	  *(iLut + eP.vertexT->idx) = iCnt++;
	}
      }
      resIdxTb->vertexT.idxCnt = iCnt;
    }
    if(eMsk & WLZ_GMELMFLG_VERTEX_G)
    {
      /* Compute vertexG index lut. */
      vCnt = iCnt = 0;
      vec = model->res.vertexG.vec;
      iLut = resIdxTb->vertexG.idxLut;
      switch(model->type)
      {
	case WLZ_GMMOD_2I:
	  while(vCnt < resIdxTb->vertexG.idxCnt)
	  {
	    eP.vertexG2I = (WlzGMVertexG2I *)AlcVectorItemGet(vec, vCnt++);
	    if(eP.vertexG2I->idx >= 0)
	    {
	      *(iLut + eP.vertexG2I->idx) = iCnt++;
	    }
	  }
	  break;
	case WLZ_GMMOD_2D:
	  while(vCnt < resIdxTb->vertexG.idxCnt)
	  {
	    eP.vertexG2D = (WlzGMVertexG2D *)AlcVectorItemGet(vec, vCnt++);
	    if(eP.vertexG2D->idx >= 0)
	    {
	      *(iLut + eP.vertexG2D->idx) = iCnt++;
	    }
	  }
	  break;
	case WLZ_GMMOD_3I:
	  while(vCnt < resIdxTb->vertexG.idxCnt)
	  {
	    eP.vertexG3I = (WlzGMVertexG3I *)AlcVectorItemGet(vec, vCnt++);
	    if(eP.vertexG3I->idx >= 0)
	    {
	      *(iLut + eP.vertexG3I->idx) = iCnt++;
	    }
	  }
	  break;
	case WLZ_GMMOD_3D:
	  while(vCnt < resIdxTb->vertexG.idxCnt)
	  {
	    eP.vertexG3D = (WlzGMVertexG3D *)AlcVectorItemGet(vec, vCnt++);
	    if(eP.vertexG3D->idx >= 0)
	    {
	      *(iLut + eP.vertexG3D->idx) = iCnt++;
	    }
	  }
	  break;
      }
      resIdxTb->vertexG.idxCnt = iCnt;
    }
    if(eMsk & WLZ_GMELMFLG_DISK_T)
    {
      /* Compute diskT index lut. */
      vCnt = iCnt = 0;
      vec = model->res.diskT.vec;
      iLut = resIdxTb->diskT.idxLut;
      while(vCnt < resIdxTb->diskT.idxCnt)
      {
	eP.diskT = (WlzGMDiskT *)AlcVectorItemGet(vec, vCnt++);
	if(eP.diskT->idx >= 0)
	{
	  *(iLut + eP.diskT->idx) = iCnt++;
	}
      }
      resIdxTb->diskT.idxCnt = iCnt;
    }
    if(eMsk & WLZ_GMELMFLG_EDGE)
    {
      /* Compute edge index lut. */
      vCnt = iCnt = 0;
      vec = model->res.edge.vec;
      iLut = resIdxTb->edge.idxLut;
      while(vCnt < resIdxTb->edge.idxCnt)
      {
	eP.edge = (WlzGMEdge *)AlcVectorItemGet(vec, vCnt++);
	if(eP.edge->idx >= 0)
	{
	  *(iLut + eP.edge->idx) = iCnt++;
	}
      }
      resIdxTb->edge.idxCnt = iCnt;
    }
    if(eMsk & WLZ_GMELMFLG_EDGE_T)
    {
      /* Compute edgeT index lut. */
      iCnt = vCnt = 0;
      vec = model->res.edgeT.vec;
      iLut = resIdxTb->edgeT.idxLut;
      while(vCnt < resIdxTb->edgeT.idxCnt)
      {
	eP.edgeT = (WlzGMEdgeT *)AlcVectorItemGet(vec, vCnt++);
	if(eP.edgeT->idx >= 0)
	{
	  *(iLut + eP.edgeT->idx) = iCnt++;
	}
      }
      resIdxTb->edgeT.idxCnt = iCnt;
    }
    if((eMsk & WLZ_GMELMFLG_FACE) && (dim == 3))
    {
      /* Compute face index lut. */
      iCnt = vCnt = 0;
      vec = model->res.face.vec;
      iLut = resIdxTb->face.idxLut;
      while(vCnt < resIdxTb->face.idxCnt)
      {
	eP.face = (WlzGMFace *)AlcVectorItemGet(vec, vCnt++);
	if(eP.face->idx >= 0)
	{
	  *(iLut + eP.face->idx) = iCnt++;
	}
      }
      resIdxTb->face.idxCnt = iCnt;
    }
    if(eMsk & WLZ_GMELMFLG_LOOP_T)
    {
      /* Compute loopT index lut. */
      iCnt = vCnt = 0;
      vec = model->res.loopT.vec;
      iLut = resIdxTb->loopT.idxLut;
      while(vCnt < resIdxTb->loopT.idxCnt)
      {
	eP.loopT = (WlzGMLoopT *)AlcVectorItemGet(vec, vCnt++);
	if(eP.loopT->idx >= 0)
	{
	  *(iLut + eP.loopT->idx) = iCnt++;
	}
      }
      resIdxTb->loopT.idxCnt = iCnt;
    }
    if(eMsk & WLZ_GMELMFLG_SHELL)
    {
      /* Compute shell index lut. */
      iCnt = vCnt = 0;
      vec = model->res.shell.vec;
      iLut = resIdxTb->shell.idxLut;
      while(vCnt < resIdxTb->shell.idxCnt)
      {
	eP.shell = (WlzGMShell *)AlcVectorItemGet(vec, vCnt++);
	if(eP.shell->idx >= 0)
	{
	  *(iLut + eP.shell->idx) = iCnt++;
	}
      }
      resIdxTb->shell.idxCnt = iCnt;
    }
    if(eMsk & WLZ_GMELMFLG_SHELL_G)
    {
      /* Compute shellG index lut. */
      iCnt = vCnt = 0;
      vec = model->res.shellG.vec;
      iLut = resIdxTb->shellG.idxLut;
      switch(model->type)
      {
	case WLZ_GMMOD_2I:
	  while(vCnt < resIdxTb->shellG.idxCnt)
	  {
	    eP.shellG2I = (WlzGMShellG2I *)AlcVectorItemGet(vec, vCnt++);
	    if(eP.shellG2I->idx >= 0)
	    {
	      *(iLut + eP.shellG2I->idx) = iCnt++;
	    }
	  }
	  break;
	case WLZ_GMMOD_2D:
	  while(vCnt < resIdxTb->shellG.idxCnt)
	  {
	    eP.shellG2D = (WlzGMShellG2D *)AlcVectorItemGet(vec, vCnt++);
	    if(eP.shellG2D->idx >= 0)
	    {
	      *(iLut + eP.shellG2D->idx) = iCnt++;
	    }
	  }
	  break;
	case WLZ_GMMOD_3I:
	  while(vCnt < resIdxTb->shellG.idxCnt)
	  {
	    eP.shellG3I = (WlzGMShellG3I *)AlcVectorItemGet(vec, vCnt++);
	    if(eP.shellG3I->idx >= 0)
	    {
	      *(iLut + eP.shellG3I->idx) = iCnt++;
	    }
	  }
	  break;
	case WLZ_GMMOD_3D:
	  while(vCnt < resIdxTb->shellG.idxCnt)
	  {
	    eP.shellG3D = (WlzGMShellG3D *)AlcVectorItemGet(vec, vCnt++);
	    if(eP.shellG3D->idx >= 0)
	    {
	      *(iLut + eP.shellG3D->idx) = iCnt++;
	    }
	  }
	  break;
      }
      resIdxTb->shellG.idxCnt = iCnt;
    }
  }
  if((errNum != WLZ_ERR_NONE) && resIdxTb)
  {
    WlzGMModelResIdxFree(resIdxTb);
    resIdxTb = NULL;
  }
  if(dstErr)
  {
    *dstErr = errNum;
  }
  return(resIdxTb);
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Frees a GM index look up table data structure.
* \param	resIdxTb		Given index lut data structure.
*/
void		WlzGMModelResIdxFree(WlzGMResIdxTb *resIdxTb)
{
  if(resIdxTb)
  {
    if(resIdxTb->vertex.idxLut)
    {
      AlcFree(resIdxTb->vertex.idxLut);
    }
    if(resIdxTb->vertexT.idxLut)
    {
      AlcFree(resIdxTb->vertexT.idxLut);
    }
    if(resIdxTb->vertexG.idxLut)
    {
      AlcFree(resIdxTb->vertexG.idxLut);
    }
    if(resIdxTb->diskT.idxLut)
    {
      AlcFree(resIdxTb->diskT.idxLut);
    }
    if(resIdxTb->edge.idxLut)
    {
      AlcFree(resIdxTb->edge.idxLut);
    }
    if(resIdxTb->edgeT.idxLut)
    {
      AlcFree(resIdxTb->edgeT.idxLut);
    }
    if(resIdxTb->face.idxLut)
    {
      AlcFree(resIdxTb->face.idxLut);
    }
    if(resIdxTb->shell.idxLut)
    {
      AlcFree(resIdxTb->shell.idxLut);
    }
    if(resIdxTb->shellG.idxLut)
    {
      AlcFree(resIdxTb->shellG.idxLut);
    }
  }
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Rehash the vertex matching hash table.
* \param	model			The model.
* \param	vHTSz			New vertex matching hash table
*                                       size, no change if <= 0.
*/
WlzErrorNum 	WlzGMModelRehashVHT(WlzGMModel *model, int vHTSz)
{
  int		idV,
  		vCnt;
  WlzGMVertex	*vertex;
  WlzGMVertex	**newVHT,
  		**oldVHT;
  AlcVector	*vec;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else
  {
    if(vHTSz > 0)
    {
      if((newVHT = AlcCalloc(vHTSz, sizeof(WlzGMVertex *))) == NULL)
      {
	errNum = WLZ_ERR_MEM_ALLOC;
      }
      else
      {
	model->vertexHTSz = vHTSz;
	oldVHT = model->vertexHT;
	model->vertexHT = newVHT;
	AlcFree(oldVHT);
      }
    }
    else
    {
      (void )memset(model->vertexHT, 0,
      		    model->vertexHTSz * sizeof(WlzGMVertex *));
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    idV = 0;
    vec = model->res.vertex.vec;
    vCnt = model->res.vertex.numIdx;
    while(idV < vCnt)
    {
      vertex = (WlzGMVertex *)AlcVectorItemGet(vec, idV);
      if(vertex->idx >= 0)
      {
	vertex->next = NULL;
	WlzGMModelAddVertex(model, vertex);
      }
      ++idV;
    }
  }
  return(errNum);
}

/* Model construction */

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Constructs a new shell, face and 3 edges (with
*		topological elements) and adds them to the model. Given
*		3 3D double precision points which are known not to
*		be in the model.
* 
*   	        Constructs a new shell, face and 3 edges (with
*		topological elements) and adds them to the model. Given
*		3 3D double precision points which are known not to
*		be in the model.
*		None of the verticies already exists within the shell.
*		Need to create a new shell (nShell) with: 1 face (nF),
*		2 loop topology element (nLT0, nLT1), 3 edges (*nE),
*		6 edge topology elements (*nET0, *nET1), 3 disk
*		topology elements *(nDT), 3 verticies (*nV) with
*		geometry (*pos) and 6 vertex topology elements
*		(*nVT0, *nVT1).
* \verbatim
*                           nVT00        nET00
*                         O ------------------------------->
*                   nV0 @- - - - - - - - - nE0 - - - - - - - -@ nV1
*                        \   <--------------------------- O  /
*                       ^   O nVT10      nET11     nVT11  ^   O nVT01
*                        \ \ \                           / / /
*                         \   \                         /   /
*                    nE2 ----\ \                       / / /
*                           \   \ nET10               /   /
*                            \ \ \                   / /---- nE1
*                             \   \                 /   /
*                        nET02 \ \ \         nET12 / / / nET01
*                               \   \             /   /
*                                \ \ \           / / /
*                                 \   \ nVT12   /   /
*                                  \ \ \    |  / / /
*                                   \   \   | /   /
*                                    \ \ \  |/ / /
*                               nVT02 O   v O   v
*                                        \   /
*                                          @
*                                            nV2
*
*                   DT0 = {nVT00, nVT10},
*                   DT1 = {nVT01, nVT11},
*                   DT2 = {nVT02, nVT12},
*                   LT0 = {nET00, nET01, nET02},
*                   LT1 = {nET10, nET11, nET12}
* \endverbatim
*                                                                      
* \param	model			The model to add the segment to.
* \param	pos			Positions of the 3 points.
*/
static WlzErrorNum WlzGMModelConstructNewS3D(WlzGMModel *model,
					     WlzDVertex3 *pos)
{
  int		idx,
  		nIdx,
		pIdx;
  WlzGMShell	*eShell,
  		*nShell;
  WlzGMFace	*nF;
  WlzGMLoopT	*nLT[2];
  WlzGMEdge	*nE[3];
  WlzGMEdgeT	*nET0[3],
  		*nET1[3];
  WlzGMDiskT	*nDT[3];
  WlzGMVertex	*nV[3];
  WlzGMVertexT	*nVT0[3],
  		*nVT1[3];
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model &&
     ((nShell = WlzGMModelNewS(model, &errNum)) != NULL) &&
     ((nF = WlzGMModelNewF(model, &errNum)) != NULL) &&
     ((nLT[0] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nLT[1] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nE[0] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[1] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[2] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nET0[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nDT[0] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nDT[1] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nDT[2] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nV[0] = WlzGMModelNewV(model, &errNum)) != NULL) &&
     ((nV[1] = WlzGMModelNewV(model, &errNum)) != NULL) &&
     ((nV[2] = WlzGMModelNewV(model, &errNum)) != NULL) &&
     ((nVT0[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[2] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[2] = WlzGMModelNewVT(model, &errNum)) != NULL))
  {
    /* New verticies. */
    for(idx = 0; idx < 3; ++idx)
    {
      (void )WlzGMVertexSetG3D(nV[idx], *(pos + idx));
      nV[idx]->diskT = nDT[idx];
      WlzGMModelAddVertex(model, nV[idx]);
    }
    /* New vertex topology elements */
    for(idx = 0; idx < 3; ++idx)
    {
      nVT0[idx]->next = nVT0[idx]->prev = nVT1[idx];
      nVT1[idx]->next = nVT1[idx]->prev = nVT0[idx];
      nVT0[idx]->diskT = nVT1[idx]->diskT = nDT[idx];
      nVT0[idx]->parent = nET0[idx];
      nVT1[idx]->parent = nET1[idx];
    }
    /* New disk topology elements */
    for(idx = 0; idx < 3; ++idx)
    {
      nDT[idx]->next = nDT[idx]->prev = nDT[idx];
      nDT[idx]->vertex = nV[idx];
      nDT[idx]->vertexT = nVT0[idx];
    }
    /* New edges. */
    for(idx = 0; idx < 3; ++idx)
    {
      nE[idx]->edgeT = nET0[idx];
    }
    /* New edge topology elements */
    for(idx = 0; idx < 3; ++idx)
    {
      nIdx = (idx + 1) % 3;
      pIdx = (idx + 3 - 1) % 3;
      nET0[idx]->next = nET0[nIdx];
      nET0[idx]->prev = nET0[pIdx];
      nET0[idx]->opp = nET1[nIdx];
      nET0[idx]->rad = nET0[idx];
      nET0[idx]->edge = nE[idx];
      nET0[idx]->vertexT = nVT0[idx];
      nET0[idx]->parent = nLT[0];
      nET1[idx]->next = nET1[pIdx];   /* Previous because reverse direction. */
      nET1[idx]->prev = nET1[nIdx];       /* Next because reverse direction. */
      nET1[idx]->opp = nET0[pIdx];
      nET1[idx]->rad = nET1[idx];
      nET1[idx]->edge = nE[pIdx];
      nET1[idx]->vertexT = nVT1[idx];
      nET1[idx]->parent = nLT[1];
    }
    /* New face and loop topology elements. */
    nF->loopT = nLT[0];
    nLT[0]->next = nLT[0]->prev = nLT[1];
    nLT[1]->next = nLT[1]->prev = nLT[0];
    nLT[0]->opp = nLT[1];
    nLT[1]->opp = nLT[0];
    nLT[0]->face = nF;
    nLT[1]->face = nF;
    nLT[0]->edgeT = nET0[0];
    nLT[1]->edgeT = nET1[0];
    nLT[0]->parent = nShell;
    nLT[1]->parent = nShell;
    /* New shell in model */
    (void )WlzGMShellSetG3D(nShell, 3, pos);
    nShell->child = nLT[0];
    nShell->parent = model;
    nShell->next = nShell->prev = nShell;
    /* Is this the first shell of the model? */
    if((eShell = model->child) == NULL)
    {
      /* First shell in model */
      model->child = nShell;
    }
    else
    {
      /* Append new shell onto end of model's list of shells. */
      WlzGMShellAppend(eShell, nShell);
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Extends an existing shell within the model by adding a:
*		face, 3 edges and 2 verticies (with topological elements).
*
*       	Extends an existing shell within the model by adding a:
*		loop, 3 edges and 2 verticies (with topological elements).
*		The 2 given 3D double precision points which are known
*		not to be in the model.
*		Need to create: 1 face (nF), 2 loop topology elements
*		(nLT[0,1]), 3 edges (nE[0,1,2]), 6 edge topology elements
*		(nET0[0,1,2], nET1[0,1,2]), 3 disk topology elements
*		(nDT[0,1,2]), 2 verticies (nV[0,1]) with geometry
*		(pos0, pos1) and 6 vertex topology elements (nVT[0,1,2]
*		and nVT1[0,1,2]).
* \verbatim
*                           nVT00        nET00
*                         O ------------------------------->
*                   nV0 @- - - - - - - - - nE0 - - - - - - - -@ nV1
*                        \   <--------------------------- O  /
*                       ^   O nVT10      nET11     nVT11  ^   O nVT01
*                        \ \ \                           / / /
*                         \   \                         /   /
*                    nE2 ----\ \                       / / /
*                           \   \ nET10               /   /
*                            \ \ \                   / /---- nE1
*                             \   \                 /   /
*                        nET02 \ \ \         nET12 / / / nET01
*                               \   \             /   /
*                                \ \ \           / / /
*                                 \   \ nVT12   /   /
*                                  \ \ \    |  / / /
*                                   \   \   | /   /
*                                    \ \ \  |/ / /
*                               nVT02 O   v O   v
*                                        \   /
*                                          @
*                                            eV
*
*                   DT0 = {nVT00, nVT10},
*                   DT1 = {nVT01, nVT11},
*                   DT2 = {nVT02, nVT12},
*                   LT0 = {nET00, nET01, nET02},
*                   LT1 = {nET10, nET11, nET12}
*
* \endverbatim
* \param	model			The model to add the segment to.
* \param	eV			Shared vertex.
* \param	pos0			Position of the first point.
* \param	pos1			Position of the second point.
*/
static WlzErrorNum WlzGMModelExtend1V0E1S3D(WlzGMModel *model, WlzGMVertex *eV,
					    WlzDVertex3 pos0, WlzDVertex3 pos1)
{
  int		idx,
  		nIdx,
		pIdx;
  WlzGMShell	*eShell;
  WlzGMFace	*nF;
  WlzGMLoopT	*nLT[2];
  WlzGMEdge	*nE[3];
  WlzGMEdgeT	*nET0[3],
  		*nET1[3];
  WlzGMDiskT	*nDT[3];
  WlzGMVertex	*nV[2];
  WlzGMVertexT	*nVT0[3],
  		*nVT1[3];
  WlzDVertex3	pos[3];
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model &&
     ((nF = WlzGMModelNewF(model, &errNum)) != NULL) &&
     ((nLT[0] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nLT[1] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nE[0] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[1] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[2] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nET0[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nDT[0] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nDT[1] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nDT[2] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nV[0] = WlzGMModelNewV(model, &errNum)) != NULL) &&
     ((nV[1] = WlzGMModelNewV(model, &errNum)) != NULL) &&
     ((nVT0[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[2] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[2] = WlzGMModelNewVT(model, &errNum)) != NULL))
  {
    pos[0] = pos0;
    pos[1] = pos1;
    (void )WlzGMVertexGetG3D(eV, pos + 2);
    /* Get shell that's to be extended. */
    eShell = WlzGMVertexGetShell(eV);
    /* New verticies. */
    for(idx = 0; idx < 2; ++idx)
    {
      (void )WlzGMVertexSetG3D(nV[idx], pos[idx]);
      nV[idx]->diskT = nDT[idx];
      WlzGMModelAddVertex(model, nV[idx]);
    }
    /* New vertex topology elements */
    for(idx = 0; idx < 3; ++idx)
    {
      nVT0[idx]->next = nVT0[idx]->prev = nVT1[idx];
      nVT1[idx]->next = nVT1[idx]->prev = nVT0[idx];
      nVT0[idx]->diskT = nVT1[idx]->diskT = nDT[idx];
      nVT0[idx]->parent = nET0[idx];
      nVT1[idx]->parent = nET1[idx];
    }
    /* New disk topology elements */
    for(idx = 0; idx < 2; ++idx)
    {
      nDT[idx]->next = nDT[idx]->prev = nDT[idx];
      nDT[idx]->vertex = nV[idx];
      nDT[idx]->vertexT = nVT0[idx];
    }
    WlzGMDiskTAppend(eV->diskT, nDT[2]);
    nDT[2]->vertex = eV;
    nDT[2]->vertexT = nVT0[2];
    /* New edges. */
    for(idx = 0; idx < 3; ++idx)
    {
      nE[idx]->edgeT = nET0[idx];
    }
    /* New edge topology elements */
    for(idx = 0; idx < 3; ++idx)
    {
      nIdx = (idx + 1) % 3;
      pIdx = (idx + 3 - 1) % 3;
      nET0[idx]->next = nET0[nIdx];
      nET0[idx]->prev = nET0[pIdx];
      nET0[idx]->opp = nET1[nIdx];
      nET0[idx]->rad = nET0[idx];
      nET0[idx]->edge = nE[idx];
      nET0[idx]->vertexT = nVT0[idx];
      nET0[idx]->parent = nLT[0];
      nET1[idx]->next = nET1[pIdx];   /* Previous because reverse direction. */
      nET1[idx]->prev = nET1[nIdx];       /* Next because reverse direction. */
      nET1[idx]->opp = nET0[pIdx];
      nET1[idx]->rad = nET1[idx];
      nET1[idx]->edge = nE[pIdx];
      nET1[idx]->vertexT = nVT1[idx];
      nET1[idx]->parent = nLT[1];
    }
    /* New face and loop topology elements. */
    nF->loopT = nLT[0];
    WlzGMLoopTAppend(eShell->child, nLT[0]);
    WlzGMLoopTAppend(eShell->child, nLT[1]);
    nLT[0]->opp = nLT[1];
    nLT[1]->opp = nLT[0];
    nLT[0]->face = nF;
    nLT[1]->face = nF;
    nLT[0]->edgeT = nET0[0];
    nLT[1]->edgeT = nET1[0];
    nLT[0]->parent = eShell;
    nLT[1]->parent = eShell;
    /* Update shell */
    (void )WlzGMShellUpdateG3D(eShell, pos0);
    (void )WlzGMShellUpdateG3D(eShell, pos1);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Extends an existing shell within the model by adding a:
*		face, 2 edges and 1 vertex (with topological elements).
*
*       	Extends an existing shell within the model by adding a:
*		face, 2 edges and 1 vertex (with topological elements).
*		The given 3D double precision point is known not to be
*		in the model.
*		Need to create: 1 face (nF), 2 loop topology elements
*		(nLT[0,1]), 2 edges (nE[0,1]), 6 edge topology elements
*		(nET0[0,1,2], nET1[0,1,2]), 1 disk topology element
*		(nDT), 1 vertex (nV) with geometry (pos0) and
*		6 vertex topology elements (nVT[0,1,2] and nVT1[0,1,2]).
*
*                           nVT00        nET00
*                         O ------------------------------->
*                   eV0 @- - - - - - - - - eE- - - - - - - - -@ eV1
*                        \   <--------------------------- O  /
*                       ^   O nVT10      nET11     nVT11  ^   O nVT01
*                        \ \ \                           / / /
*                         \   \                         /   /
*                  nE1 ------\ \                       / / /
*                           \   \ nET10               /   /
*                            \ \ \                   / /---- nE0
*                             \   \                 /   /
*                        nET02 \ \ \         nET12 / / / nET01
*                               \   \             /   /
*                                \ \ \           / / /
*                                 \   \ nVT12   /   /
*                                  \ \ \    |  / / /
*                                   \   \   | /   /
*                                    \ \ \  |/ / /
*                               nVT02 O   v O   v
*                                        \   /
*                                          @
*                                            nV
*
*                   eDT0 += {nVT00, nVT10}
*                   eDT1 += {nVT01, nVT11}
*                   nDT = {nVT02, nVT12}
*                   nLT0 = {nET00, nET01, nET02}
*                   nLT1 = {nET10, nET11, nET12}
* \endverbatim
* \param	model			The model to add the segment to.
* \param	eE			Shared edge.
* \param	nPos			Position of new vertex.
*/
static WlzErrorNum WlzGMModelExtend2V1E1S3D(WlzGMModel *model, WlzGMEdge *eE,
					    WlzDVertex3 nPos)
{
  int		idx,
  		nIdx,
		pIdx;
  WlzGMShell	*eShell;
  WlzGMFace	*nF;
  WlzGMLoopT	*nLT[2];
  WlzGMEdge	*nE[2];
  WlzGMEdgeT	*nET0[3],
  		*nET1[3];
  WlzGMDiskT	*nDT;
  WlzGMVertex	*nV;
  WlzGMVertexT	*nVT0[3],
  		*nVT1[3];
  WlzDVertex3	pos[3];
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model &&
     ((nF = WlzGMModelNewF(model, &errNum)) != NULL) &&
     ((nLT[0] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nLT[1] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nE[0] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[1] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nET0[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nDT = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nV = WlzGMModelNewV(model, &errNum)) != NULL) &&
     ((nVT0[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[2] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[2] = WlzGMModelNewVT(model, &errNum)) != NULL))
  {
    /* Get shell that's to be extended. */
    eShell = WlzGMEdgeGetShell(eE);
    /* Get geometry of all 3 verticies. */
    (void )WlzGMVertexGetG3D(eE->edgeT->vertexT->diskT->vertex, pos + 0);
    (void )WlzGMVertexGetG3D(eE->edgeT->opp->vertexT->diskT->vertex, pos + 1);
    pos[2] = nPos;
    /* New vertex. */
    (void )WlzGMVertexSetG3D(nV, nPos);
    nV->diskT = nDT;
    WlzGMModelAddVertex(model, nV);
    /* New vertex topology elements. */
    WlzGMVertexTAppend(eE->edgeT->vertexT, nVT0[0]);
    WlzGMVertexTAppend(eE->edgeT->vertexT, nVT1[0]);
    nVT0[0]->diskT = nVT1[0]->diskT = eE->edgeT->vertexT->diskT;
    WlzGMVertexTAppend(eE->edgeT->opp->vertexT, nVT0[1]);
    WlzGMVertexTAppend(eE->edgeT->opp->vertexT, nVT1[1]);
    nVT0[1]->diskT = nVT1[1]->diskT = eE->edgeT->opp->vertexT->diskT;
    nVT0[2]->next = nVT0[2]->prev = nVT1[2];
    nVT1[2]->next = nVT1[2]->prev = nVT0[2];
    nVT0[2]->diskT = nVT1[2]->diskT = nDT;
    for(idx = 0; idx < 3; ++idx)
    {
      nVT0[idx]->parent = nET0[idx];
      nVT1[idx]->parent = nET1[idx];
    }
    /* New disk topology element. */
    nDT->next = nDT->prev = nDT;
    nDT->vertex = nV;
    nDT->vertexT = nVT0[2];
    /* New edges. */
    for(idx = 0; idx < 2; ++idx)
    {
      nE[idx]->edgeT = nET0[idx + 1];
    }
    /* New edge topology elements */
    for(idx = 0; idx < 3; ++idx)
    {
      nIdx = (idx + 1) % 3;
      pIdx = (idx + 3 - 1) % 3;
      nET0[idx]->next = nET0[nIdx];
      nET0[idx]->prev = nET0[pIdx];
      nET0[idx]->opp = nET1[nIdx];
      nET0[idx]->rad = nET0[idx];
      nET0[idx]->vertexT = nVT0[idx];
      nET0[idx]->parent = nLT[0];
      nET1[idx]->next = nET1[pIdx];   /* Previous because reverse direction. */
      nET1[idx]->prev = nET1[nIdx];       /* Next because reverse direction. */
      nET1[idx]->opp = nET0[pIdx];
      nET1[idx]->rad = nET1[idx];
      nET1[idx]->vertexT = nVT1[idx];
      nET1[idx]->parent = nLT[1];
    }
    nET1[1]->edge = nET0[0]->edge = eE;
    nET1[2]->edge = nET0[1]->edge = nE[0];
    nET1[0]->edge = nET0[2]->edge = nE[1];
    /* Need to set radial edges for nET0[0] and nET1[1]. */
    WlzGMEdgeTInsertRadial(nET0[0]);
    WlzGMEdgeTInsertRadial(nET1[1]);
    /* New face and loop topology elements. */
    nF->loopT = nLT[0];
    WlzGMLoopTAppend(eShell->child, nLT[0]);
    WlzGMLoopTAppend(eShell->child, nLT[1]);
    nLT[0]->opp = nLT[1];
    nLT[1]->opp = nLT[0];
    nLT[0]->face = nF;
    nLT[1]->face = nF;
    nLT[0]->edgeT = nET0[0];
    nLT[1]->edgeT = nET1[0];
    nLT[0]->parent = eShell;
    nLT[1]->parent = eShell;
    /* Update shell */
    (void )WlzGMShellUpdateG3D(eShell, pos[2]);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Extends an existing shell within the model by adding a:
*		face, 3 edges and 1 vertex (with topological elements).
* 
*        	Extends an existing shell within the model by adding a:
*		face, 3 edges and 1 vertex (with topological elements).
*		Both the given 3D double precision points are known not
*		to be in the model.
*		Need to create: 1 face (nF), 2 loop topology elements
*		(nLT[0,1]), 3 edges (nE[0,1,2]), 6 edge topology elements
*		(nET0[0,1,2], nET1[0,1,2]), 3 disk topology element
*		(nDT[0,1,2]), 1 vertex (nV) with geometry (pos0) and
*		6 vertex topology elements (nVT[0,1,2] and nVT1[0,1,2]).
* \verbatim
*                           nVT00        nET00
*                         O ------------------------------->
*                   eV0 @- - - - - - - - nE0 - - - - - - - - -@ eV1
*                        \   <--------------------------- O  /
*                       ^   O nVT10      nET11     nVT11  ^   O nVT01
*                        \ \ \                           / / /
*                         \   \                         /   /
*                  nE2 ------\ \                       / / /
*                           \   \ nET10               /   /
*                            \ \ \                   / /---- nE1
*                             \   \                 /   /
*                        nET02 \ \ \         nET12 / / / nET01
*                               \   \             /   /
*                                \ \ \           / / /
*                                 \   \ nVT12   /   /
*                                  \ \ \    |  / / /
*                                   \   \   | /   /
*                                    \ \ \  |/ / /
*                               nVT02 O   v O   v
*                                        \   /
*                                          @
*                                            nV
*
*                   nDT0 = {nVT00, nVT10}
*                   nDT1 = {nVT01, nVT11}
*		    nDT2 = {nVT02, nVT12}
*                   nLT0 = {nET00, nET01, nET02}
*                   nLT1 = {nET10, nET11, nET12}
* \endverbatim
* \param	model			The model to add the segment to.
* \param	eV0			First shared vertex.
* \param	eV1			Second shared vertex.
* \param	nPos			Position of new vertex.
*/
static WlzErrorNum WlzGMModelExtend2V0E1S3D(WlzGMModel *model,
					    WlzGMVertex *eV0, WlzGMVertex *eV1,
					    WlzDVertex3 nPos)
{
  int		idx,
  		nIdx,
		pIdx;
  WlzGMShell	*eShell;
  WlzGMFace	*nF;
  WlzGMLoopT	*nLT[2];
  WlzGMEdge	*nE[3];
  WlzGMEdgeT	*nET0[3],
  		*nET1[3];
  WlzGMDiskT	*nDT[3];
  WlzGMVertex	*nV;
  WlzGMVertexT	*nVT0[3],
  		*nVT1[3];
  WlzDVertex3	pos[3];
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model &&
     ((nF = WlzGMModelNewF(model, &errNum)) != NULL) &&
     ((nLT[0] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nLT[1] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nE[0] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[1] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[2] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nET0[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nDT[0] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nDT[1] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nDT[2] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nV = WlzGMModelNewV(model, &errNum)) != NULL) &&
     ((nVT0[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[2] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[2] = WlzGMModelNewVT(model, &errNum)) != NULL))
  {
    /* Get shell that's to be extended. */
    eShell = WlzGMVertexGetShell(eV0);
    /* Get geometry of the 2 existing verticies and the new vertex. */
    (void )WlzGMVertexGetG3D(eV0, pos + 0);
    (void )WlzGMVertexGetG3D(eV1, pos + 1);
    pos[2] = nPos;
    /* New vertex. */
    (void )WlzGMVertexSetG3D(nV, nPos);
    nV->diskT = nDT[2];
    WlzGMModelAddVertex(model, nV);
    /* New vertex topology elements. */
    for(idx = 0; idx < 3; ++idx)
    {
      nVT0[idx]->next = nVT0[idx]->prev = nVT1[idx];
      nVT1[idx]->next = nVT1[idx]->prev = nVT0[idx];
      nVT0[idx]->diskT = nVT1[idx]->diskT = nDT[idx];
      nVT0[idx]->parent = nET0[idx];
      nVT1[idx]->parent = nET1[idx];
    }
    /* New disk topology elements. */
    WlzGMDiskTAppend(eV0->diskT, nDT[0]);
    nDT[0]->vertex = eV0;
    nDT[0]->vertexT = nVT0[0];
    WlzGMDiskTAppend(eV1->diskT, nDT[1]);
    nDT[1]->vertex = eV1;
    nDT[1]->vertexT = nVT0[1];
    nDT[2]->next = nDT[2]->prev = nDT[2];
    nDT[2]->vertex = nV;
    nDT[2]->vertexT = nVT0[2];
    /* New edges. */
    for(idx = 0; idx < 3; ++idx)
    {
      nE[idx]->edgeT = nET0[idx];
    }
    /* New edge topology elements */
    for(idx = 0; idx < 3; ++idx)
    {
      nIdx = (idx + 1) % 3;
      pIdx = (idx + 3 - 1) % 3;
      nET0[idx]->next = nET0[nIdx];
      nET0[idx]->prev = nET0[pIdx];
      nET0[idx]->opp = nET1[nIdx];
      nET0[idx]->rad = nET0[idx];
      nET0[idx]->edge = nE[idx];
      nET0[idx]->vertexT = nVT0[idx];
      nET0[idx]->parent = nLT[0];
      nET1[idx]->next = nET1[pIdx];   /* Previous because reverse direction. */
      nET1[idx]->prev = nET1[nIdx];       /* Next because reverse direction. */
      nET1[idx]->opp = nET0[pIdx];
      nET1[idx]->rad = nET1[idx];
      nET1[idx]->edge = nE[pIdx];
      nET1[idx]->vertexT = nVT1[idx];
      nET1[idx]->parent = nLT[1];
    }
    /* New face and loop topology elements. */
    nF->loopT = nLT[0];
    WlzGMLoopTAppend(eShell->child, nLT[0]);
    WlzGMLoopTAppend(eShell->child, nLT[1]);
    nLT[0]->opp = nLT[1];
    nLT[1]->opp = nLT[0];
    nLT[0]->face = nF;
    nLT[1]->face = nF;
    nLT[0]->edgeT = nET0[0];
    nLT[1]->edgeT = nET1[0];
    nLT[0]->parent = eShell;
    nLT[1]->parent = eShell;
    /* Update shell */
    (void )WlzGMShellUpdateG3D(eShell, nPos);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Joins two existing shells within the model by adding:
*		1 face, 3 edges and 1 vertex (with topological elements).
*
*		Joins two existing shells within the model by adding:
*		1 face, 3 edges and 1 vertex (with topological elements).
*		Both the given 3D double precision points are known not
*		to be in the model.
*		Need to create: 1 face (nF), 2 loop topology elements
*		(nLT[0,1]), 3 edges (nE[0,1,2]), 6 edge topology elements
*		(nET0[0,1,2], nET1[0,1,2]), 3 disk topology element
*		(nDT[0,1,2]), 1 vertex (nV) with geometry (pos0) and
*		6 vertex topology elements (nVT[0,1,2] and nVT1[0,1,2]).
*		Need to delete 1 shell (dShell).
* \verbatim
*                           nVT00        nET00
*                         O ------------------------------->
*                   eV0 @- - - - - - - - nE0 - - - - - - - - -@ eV1
*                        \   <--------------------------- O  /
*                       ^   O nVT10      nET11     nVT11  ^   O nVT01
*                        \ \ \                           / / /
*                         \   \                         /   /
*                  nE2 ------\ \                       / / /
*                           \   \ nET10               /   /
*                            \ \ \                   / /---- nE1
*                             \   \                 /   /
*                        nET02 \ \ \         nET12 / / / nET01
*                               \   \             /   /
*                                \ \ \           / / /
*                                 \   \ nVT12   /   /
*                                  \ \ \    |  / / /
*                                   \   \   | /   /
*                                    \ \ \  |/ / /
*                               nVT02 O   v O   v
*                                        \   /
*                                          @
*                                            nV
*
*                   nDT0 = {nVT00, nVT10}
*                   nDT1 = {nVT01, nVT11}
*		    nDT2 = {nVT02, nVT12}
*                   nLT0 = {nET00, nET01, nET02}
*                   nLT1 = {nET10, nET11, nET12}
* \endverbatim
* \param	model			The model to add the segment to.
* \param	eV0			First shared vertex.
* \param	eV1			Second shared vertex.
* \param	nPos			Position of new vertex.
*/
static WlzErrorNum WlzGMModelJoin2V0E0S3D(WlzGMModel *model,
					  WlzGMVertex *eV0, WlzGMVertex *eV1,
					  WlzDVertex3 nPos)
{
  int		idx,
  		nIdx,
		pIdx;
  double	eVol,
  		dVol;
  WlzGMShell	*eShell,
  		*dShell,
		*tShell;
  WlzGMFace	*nF;
  WlzGMLoopT	*nLT[2];
  WlzGMEdge	*nE[3];
  WlzGMEdgeT	*nET0[3],
  		*nET1[3];
  WlzGMDiskT	*nDT[3];
  WlzGMVertex	*nV,
  		*tV;
  WlzGMVertexT	*nVT0[3],
  		*nVT1[3];
  WlzDVertex3	pos[3];
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model &&
     ((nF = WlzGMModelNewF(model, &errNum)) != NULL) &&
     ((nLT[0] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nLT[1] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nE[0] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[1] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[2] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nET0[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nDT[0] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nDT[1] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nDT[2] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nV = WlzGMModelNewV(model, &errNum)) != NULL) &&
     ((nVT0[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[2] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[2] = WlzGMModelNewVT(model, &errNum)) != NULL))
  {
    /* Get shell that's to be extended and the shell thats to be joined
     * with it (and then deleated). */
    eShell = WlzGMVertexGetShell(eV0);
    dShell = WlzGMVertexGetShell(eV1);
    /* Optimization: Get the shell geometries and compare their volume.
     * Then make sure that the shell with the smaller volume (and hopefully
     * the smaller number of loops) is the one that's to be deleted. */
    (void )WlzGMShellGetGBBV3D(eShell, &eVol);
    (void )WlzGMShellGetGBBV3D(dShell, &dVol);
    if(eVol < dVol)
    {
      tShell = eShell; eShell = dShell; dShell = tShell;
      tV = eV0; eV0 = eV1; eV1 = tV;
    }
    /* Get geometry of the 2 existing verticies and the new vertex. */
    (void )WlzGMVertexGetG3D(eV0, pos + 0);
    (void )WlzGMVertexGetG3D(eV1, pos + 1);
    pos[2] = nPos;
    /* New vertex. */
    (void )WlzGMVertexSetG3D(nV, nPos);
    nV->diskT = nDT[2];
    WlzGMModelAddVertex(model, nV);
    /* New vertex topology elements. */
    for(idx = 0; idx < 3; ++idx)
    {
      nVT0[idx]->next = nVT0[idx]->prev = nVT1[idx];
      nVT1[idx]->next = nVT1[idx]->prev = nVT0[idx];
      nVT0[idx]->diskT = nVT1[idx]->diskT = nDT[idx];
      nVT0[idx]->parent = nET0[idx];
      nVT1[idx]->parent = nET1[idx];
    }
    /* New disk topology elements. */
    WlzGMDiskTAppend(eV0->diskT, nDT[0]);
    nDT[0]->vertex = eV0;
    nDT[0]->vertexT = nVT0[0];
    WlzGMDiskTAppend(eV1->diskT, nDT[1]);
    nDT[1]->vertex = eV1;
    nDT[1]->vertexT = nVT0[1];
    nDT[2]->next = nDT[2]->prev = nDT[2];
    nDT[2]->vertex = nV;
    nDT[2]->vertexT = nVT0[2];
    /* New edges. */
    for(idx = 0; idx < 3; ++idx)
    {
      nE[idx]->edgeT = nET0[idx];
    }
    /* New edge topology elements */
    for(idx = 0; idx < 3; ++idx)
    {
      nIdx = (idx + 1) % 3;
      pIdx = (idx + 3 - 1) % 3;
      nET0[idx]->next = nET0[nIdx];
      nET0[idx]->prev = nET0[pIdx];
      nET0[idx]->opp = nET1[nIdx];
      nET0[idx]->rad = nET0[idx];
      nET0[idx]->edge = nE[idx];
      nET0[idx]->vertexT = nVT0[idx];
      nET0[idx]->parent = nLT[0];
      nET1[idx]->next = nET1[pIdx];   /* Previous because reverse direction. */
      nET1[idx]->prev = nET1[nIdx];       /* Next because reverse direction. */
      nET1[idx]->opp = nET0[pIdx];
      nET1[idx]->rad = nET1[idx];
      nET1[idx]->edge = nE[pIdx];
      nET1[idx]->vertexT = nVT1[idx];
      nET1[idx]->parent = nLT[1];
    }
    /* New face and loop topology elements. */
    nF->loopT = nLT[0];
    WlzGMLoopTAppend(eShell->child, nLT[0]);
    WlzGMLoopTAppend(eShell->child, nLT[1]);
    nLT[0]->opp = nLT[1];
    nLT[1]->opp = nLT[0];
    nLT[0]->face = nF;
    nLT[1]->face = nF;
    nLT[0]->edgeT = nET0[0];
    nLT[1]->edgeT = nET1[0];
    nLT[0]->parent = eShell;
    nLT[1]->parent = eShell;
    /* Update the extended shell and delete the joined shell. */
    (void )WlzGMShellUpdateG3D(eShell, nPos);
    if(model->child->idx == dShell->idx)
    {
      model->child = eShell;
    }
    WlzGMShellJoinAndUnlink(eShell, dShell);
    WlzGMModelFreeS(model, dShell);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Joins three existing shells within the model by adding:
*		1 face and 3 edges (with topological elements).
*
*		Joins three existing shells within the model by adding:
*		1 face and 3 edges (with topological elements).
*		Need to create: 1 face (nF), 2 loop topology elements
*		(nLT[0,1]), 3 edges (nE[0,1,2]), 6 edge topology elements
*		(nET0[0,1,2], nET1[0,1,2]), 3 disk topology element
*		(nDT[0,1,2]) and 6 vertex topology elements (nVT[0,1,2]
*		and nVT1[0,1,2]).
*		Need to delete 2 shells (dShell[0,1]).
* \verbatim
*                           nVT00        nET00
*                         O ------------------------------->
*                   eV0 @- - - - - - - - nE2 - - - - - - - - -@ eV1
*                        \   <--------------------------- O  /
*                       ^   O nVT10      nET11     nVT11  ^   O nVT01
*                        \ \ \                           / / /
*                         \   \                         /   /
*                  nE2 ------\ \                       / / /
*                           \   \ nET10               /   /
*                            \ \ \                   / /---- nE1
*                             \   \                 /   /
*                        nET02 \ \ \         nET12 / / / nET01
*                               \   \             /   /
*                                \ \ \           / / /
*                                 \   \ nVT12   /   /
*                                  \ \ \    |  / / /
*                                   \   \   | /   /
*                                    \ \ \  |/ / /
*                               nVT02 O   v O   v
*                                        \   /
*                                          @
*                                            eV2
*
*                   nDT0 = {nVT00, nVT10}
*                   nDT1 = {nVT01, nVT11}
*		    nDT2 = {nVT02, nVT12}
*                   nLT0 = {nET00, nET01, nET02}
*                   nLT1 = {nET10, nET11, nET12}
* \endverbatim
* \param	model			The model to add the segment to.
* \param	gEV			The three given shared verticies.
*/
static WlzErrorNum WlzGMModelJoin3V0E3S3D(WlzGMModel *model, WlzGMVertex **gEV)
{
  int		idx,
  		nIdx,
		pIdx;
  double	eVol;
  double	dVol[2];
  WlzGMShell	*eShell,
  		*tShell;
  WlzGMShell	*dShell[2];
  WlzGMFace	*nF;
  WlzGMLoopT	*nLT[2];
  WlzGMEdge	*nE[3];
  WlzGMEdgeT	*nET0[3],
  		*nET1[3];
  WlzGMDiskT	*nDT[3];
  WlzGMVertex	*tV;
  WlzGMVertex	*eV[3];
  WlzGMVertexT	*nVT0[3],
  		*nVT1[3];
  WlzDVertex3	pos[3];
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  eV[0] = gEV[0]; eV[1] = gEV[1]; eV[2] = gEV[2];
  if(model &&
     ((nF = WlzGMModelNewF(model, &errNum)) != NULL) &&
     ((nLT[0] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nLT[1] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nE[0] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[1] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[2] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nET0[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nDT[0] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nDT[1] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nDT[2] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nVT0[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[2] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[2] = WlzGMModelNewVT(model, &errNum)) != NULL))
  {
    /* Get shell that's to be extended and the shells that are to be joined
     * with it (and then deleated). */
    eShell = WlzGMVertexGetShell(eV[0]);
    dShell[0] = WlzGMVertexGetShell(eV[1]);
    dShell[1] = WlzGMVertexGetShell(eV[2]);
    /* Optimization: Get the shell geometries and compare their volume.
     * Then make sure that the shell with the greatest volume (and hopefully
     * the greatest number of loops) is the one that's to be kept. */
    (void )WlzGMShellGetGBBV3D(eShell, &eVol);
    (void )WlzGMShellGetGBBV3D(dShell[0], &dVol[0]);
    (void )WlzGMShellGetGBBV3D(dShell[1], &dVol[1]);
    if((eVol < dVol[0]) || (eVol < dVol[1]))
    {
      if(dVol[0] > dVol[1])
      {
        tShell = eShell; eShell = dShell[0]; dShell[0] = tShell;
	tV = eV[0]; eV[0] = eV[1]; eV[1] = tV;
      }
      else
      {
        tShell = eShell; eShell = dShell[1]; dShell[1] = tShell;
	tV = eV[0]; eV[0] = eV[2]; eV[2] = tV;
      }
    }
    /* Get geometry of the 3 existing verticies. */
    for(idx = 0; idx < 3; ++idx)
    {
      (void )WlzGMVertexGetG3D(*(eV + idx), pos + idx);
    }
    /* New vertex topology elements. */
    for(idx = 0; idx < 3; ++idx)
    {
      nVT0[idx]->next = nVT0[idx]->prev = nVT1[idx];
      nVT1[idx]->next = nVT1[idx]->prev = nVT0[idx];
      nVT0[idx]->diskT = nVT1[idx]->diskT = nDT[idx];
      nVT0[idx]->parent = nET0[idx];
      nVT1[idx]->parent = nET1[idx];
    }
    /* New disk topology elements. */
    for(idx = 0; idx < 3; ++idx)
    {
      WlzGMDiskTAppend(eV[idx]->diskT, nDT[idx]);
      nDT[idx]->vertex = eV[idx];
      nDT[idx]->vertexT = nVT0[idx];
    }
    /* New edges. */
    for(idx = 0; idx < 3; ++idx)
    {
      nE[idx]->edgeT = nET0[idx];
    }
    /* New edge topology elements */
    for(idx = 0; idx < 3; ++idx)
    {
      nIdx = (idx + 1) % 3;
      pIdx = (idx + 3 - 1) % 3;
      nET0[idx]->next = nET0[nIdx];
      nET0[idx]->prev = nET0[pIdx];
      nET0[idx]->opp = nET1[nIdx];
      nET0[idx]->rad = nET0[idx];
      nET0[idx]->edge = nE[idx];
      nET0[idx]->vertexT = nVT0[idx];
      nET0[idx]->parent = nLT[0];
      nET1[idx]->next = nET1[pIdx];   /* Previous because reverse direction. */
      nET1[idx]->prev = nET1[nIdx];       /* Next because reverse direction. */
      nET1[idx]->opp = nET0[pIdx];
      nET1[idx]->rad = nET1[idx];
      nET1[idx]->edge = nE[pIdx];
      nET1[idx]->vertexT = nVT1[idx];
      nET1[idx]->parent = nLT[1];
    }
    /* New face and loop topology elements. */
    nF->loopT = nLT[0];
    WlzGMLoopTAppend(eShell->child, nLT[0]);
    WlzGMLoopTAppend(eShell->child, nLT[1]);
    nLT[0]->opp = nLT[1];
    nLT[1]->opp = nLT[0];
    nLT[0]->face = nF;
    nLT[1]->face = nF;
    nLT[0]->edgeT = nET0[0];
    nLT[1]->edgeT = nET1[0];
    nLT[0]->parent = eShell;
    nLT[1]->parent = eShell;
    /* Update the extended shell and delete the joined shells. */
    for(idx = 0; idx < 2; ++idx)
    {
      if(model->child->idx == dShell[idx]->idx)
      {
	model->child = eShell;
      }
      WlzGMShellJoinAndUnlink(eShell, dShell[idx]);
      WlzGMModelFreeS(model, dShell[idx]);
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Joins two existing shells within the model by adding:
*		1 face and 3 edges (with topological elements).
*
*		Joins two existing shells within the model by adding:
*		1 face and 3 edges (with topological elements).
*		Need to create: 1 face (nF), 2 loop topology elements
*		(nLT[0,1]), 3 edges (nE[0,1,2]), 6 edge topology elements
*		(nET0[0,1,2], nET1[0,1,2]), 3 disk topology element
*		(nDT[0,1,2]) and 6 vertex topology elements (nVT[0,1,2]
*		and nVT1[0,1,2]).
*		Need to delete 1 shell (dShell).
* \verbatim
*                           nVT00        nET00
*                         O ------------------------------->
*                   eV0 @- - - - - - - - nE0 - - - - - - - - -@ eV1
*                        \   <--------------------------- O  /
*                       ^   O nVT10      nET11     nVT11  ^   O nVT01
*                        \ \ \                           / / /
*                         \   \                         /   /
*                  nE2 ------\ \                       / / /
*                           \   \ nET10               /   /
*                            \ \ \                   / /---- nE1
*                             \   \                 /   /
*                        nET02 \ \ \         nET12 / / / nET01
*                               \   \             /   /
*                                \ \ \           / / /
*                                 \   \ nVT12   /   /
*                                  \ \ \    |  / / /
*                                   \   \   | /   /
*                                    \ \ \  |/ / /
*                               nVT02 O   v O   v
*                                        \   /
*                                          @
*                                            eV2
*
*                   nDT0 = {nVT00, nVT10}
*                   nDT1 = {nVT01, nVT11}
*		    nDT2 = {nVT02, nVT12}
*                   nLT0 = {nET00, nET01, nET02}
*                   nLT1 = {nET10, nET11, nET12}
* \endverbatim
* \param	model			The model to add the segment to.
* \param	eV			The three given shared verticies.
*/
static WlzErrorNum WlzGMModelJoin3V0E2S3D(WlzGMModel *model, WlzGMVertex **eV)
{
  int		idx,
  		nIdx,
		pIdx;
  double	sVol[3];
  WlzGMShell	*dShell,
  		*eShell;
  WlzGMShell	*tShell[3];
  WlzGMFace	*nF;
  WlzGMLoopT	*nLT[2];
  WlzGMEdge	*nE[3];
  WlzGMEdgeT	*nET0[3],
  		*nET1[3];
  WlzGMDiskT	*nDT[3];
  WlzGMVertex   *ev[3];
  WlzGMVertexT	*nVT0[3],
  		*nVT1[3];
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model &&
     ((nF = WlzGMModelNewF(model, &errNum)) != NULL) &&
     ((nLT[0] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nLT[1] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nE[0] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[1] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[2] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nET0[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nDT[0] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nDT[1] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nDT[2] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nVT0[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[2] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[2] = WlzGMModelNewVT(model, &errNum)) != NULL))
  {
    /* Get shells and find which is to be extended and which is to be joined
     * with it (and then deleated). Know that 2 of the shells are the same.
     * Compute volumes of the shells bounding boxes, compare the volumes,
     * delete the shell with the smallest volume and keep the shell with
     * the largest volume. This is an optimazation, hopefuly the shell with
     * greatest volume will have the greatest number of loops. */
    for(idx = 0; idx < 3; ++idx)
    {
      tShell[idx] = WlzGMVertexGetShell(*(eV + idx));
      (void )WlzGMShellGetGBBV3D(tShell[idx],sVol + idx);
    }
    if(sVol[0] > sVol[1])
    {
      if(sVol[0] > sVol[2])
      {
        idx = 0;
      }
      else
      {
        idx = 2;
      }
    }
    else
    {
      if(sVol[1] > sVol[2])
      {
        idx = 1;
      }
      else
      {
        idx = 2;
      }
    }
    eShell = tShell[idx];
    dShell = (tShell[idx]->idx == tShell[(idx + 1) % 3]->idx)?
             tShell[(idx + 2) % 3]: tShell[(idx + 1) % 3];
    if(tShell[0]->idx == tShell[1]->idx)
    {
      eShell = tShell[0];
      dShell = tShell[2];
    }
    else if(tShell[1]->idx == tShell[2]->idx)
    {
      eShell = tShell[1];
      dShell = tShell[0];
    }
    else /* tShell[2]->idx == tShell[0]->idx */
    {
      eShell = tShell[2];
      dShell = tShell[1];
    }
    /* New vertex topology elements. */
    for(idx = 0; idx < 3; ++idx)
    {
      nVT0[idx]->next = nVT0[idx]->prev = nVT1[idx];
      nVT1[idx]->next = nVT1[idx]->prev = nVT0[idx];
      nVT0[idx]->diskT = nVT1[idx]->diskT = nDT[idx];
      nVT0[idx]->parent = nET0[idx];
      nVT1[idx]->parent = nET1[idx];
    }
    /* New disk topology elements. */
    for(idx = 0; idx < 3; ++idx)
    {
      WlzGMDiskTAppend(eV[idx]->diskT, nDT[idx]);
      nDT[idx]->vertex = eV[idx];
      nDT[idx]->vertexT = nVT0[idx];
    }
    /* New edges. */
    for(idx = 0; idx < 3; ++idx)
    {
      nE[idx]->edgeT = nET0[idx];
    }
    /* New edge topology elements */
    for(idx = 0; idx < 3; ++idx)
    {
      nIdx = (idx + 1) % 3;
      pIdx = (idx + 3 - 1) % 3;
      nET0[idx]->next = nET0[nIdx];
      nET0[idx]->prev = nET0[pIdx];
      nET0[idx]->opp = nET1[nIdx];
      nET0[idx]->rad = nET0[idx];
      nET0[idx]->edge = nE[idx];
      nET0[idx]->vertexT = nVT0[idx];
      nET0[idx]->parent = nLT[0];
      nET1[idx]->next = nET1[pIdx];   /* Previous because reverse direction. */
      nET1[idx]->prev = nET1[nIdx];       /* Next because reverse direction. */
      nET1[idx]->opp = nET0[pIdx];
      nET1[idx]->rad = nET1[idx];
      nET1[idx]->edge = nE[pIdx];
      nET1[idx]->vertexT = nVT1[idx];
      nET1[idx]->parent = nLT[1];
    }
    /* New face and loop topology elements. */
    nF->loopT = nLT[0];
    WlzGMLoopTAppend(eShell->child, nLT[0]);
    WlzGMLoopTAppend(eShell->child, nLT[1]);
    nLT[0]->opp = nLT[1];
    nLT[1]->opp = nLT[0];
    nLT[0]->face = nF;
    nLT[1]->face = nF;
    nLT[0]->edgeT = nET0[0];
    nLT[1]->edgeT = nET1[0];
    nLT[0]->parent = eShell;
    nLT[1]->parent = eShell;
    /* Update the extended shell and delete the joined shell. */
    if(model->child->idx == dShell->idx)
    {
      model->child = eShell;
    }
    WlzGMShellJoinAndUnlink(eShell, dShell);
    WlzGMModelFreeS(model, dShell);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Extends an existing shell within the model by adding:
*		1 face and 3 edges (with topological elements).
*
*		Extends an existing shell within the model by adding:
*		1 face and 3 edges (with topological elements).
*		Need to create: 1 face (nF), 2 loop topology elements
*		(nLT[0,1]), 3 edges (nE[0,1,2]), 6 edge topology elements
*		(nET0[0,1,2], nET1[0,1,2]), 3 disk topology element
*		(nDT[0,1,2]) and 6 vertex topology elements (nVT[0,1,2]
*		and nVT1[0,1,2]).
* \verbatim
*                           nVT00        nET00
*                         O ------------------------------->
*                   eV0 @- - - - - - - - nE0 - - - - - - - - -@ eV1
*                        \   <--------------------------- O  /
*                       ^   O nVT10      nET11     nVT11  ^   O nVT01
*                        \ \ \                           / / /
*                         \   \                         /   /
*                  nE2 ------\ \                       / / /
*                           \   \ nET10               /   /
*                            \ \ \                   / /---- nE1
*                             \   \                 /   /
*                        nET02 \ \ \         nET12 / / / nET01
*                               \   \             /   /
*                                \ \ \           / / /
*                                 \   \ nVT12   /   /
*                                  \ \ \    |  / / /
*                                   \   \   | /   /
*                                    \ \ \  |/ / /
*                               nVT02 O   v O   v
*                                        \   /
*                                          @
*                                            eV2
*
*                   nDT0 = {nVT00, nVT10}
*                   nDT1 = {nVT01, nVT11}
*		    nDT2 = {nVT02, nVT12}
*                   nLT0 = {nET00, nET01, nET02}
*                   nLT1 = {nET10, nET11, nET12}
* \endverbatim
* \param	model			The model to add the segment to.
* \param	eV			The three shared vertex.
*/
static WlzErrorNum WlzGMModelExtend3V0E1S3D(WlzGMModel *model,
					    WlzGMVertex **eV)
{
  int		idx,
  		nIdx,
		pIdx;
  WlzGMShell	*eShell;
  WlzGMFace	*nF;
  WlzGMLoopT	*nLT[2];
  WlzGMEdge	*nE[3];
  WlzGMEdgeT	*nET0[3],
  		*nET1[3];
  WlzGMDiskT	*nDT[3];
  WlzGMVertexT	*nVT0[3],
  		*nVT1[3];
  WlzDVertex3	pos[3];
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model &&
     ((nF = WlzGMModelNewF(model, &errNum)) != NULL) &&
     ((nLT[0] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nLT[1] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nE[0] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[1] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[2] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nET0[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nDT[0] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nDT[1] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nDT[2] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nVT0[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[2] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[2] = WlzGMModelNewVT(model, &errNum)) != NULL))
  {
    /* Get shell. */
    eShell = WlzGMVertexGetShell(*eV);
    /* Get geometry of the 3 existing verticies. */
    for(idx = 0; idx < 3; ++idx)
    {
      (void )WlzGMVertexGetG3D(*(eV + idx), pos + idx);
    }
    /* New vertex topology elements. */
    for(idx = 0; idx < 3; ++idx)
    {
      nVT0[idx]->next = nVT0[idx]->prev = nVT1[idx];
      nVT1[idx]->next = nVT1[idx]->prev = nVT0[idx];
      nVT0[idx]->diskT = nVT1[idx]->diskT = nDT[idx];
      nVT0[idx]->parent = nET0[idx];
      nVT1[idx]->parent = nET1[idx];
    }
    /* New disk topology elements. */
    for(idx = 0; idx < 3; ++idx)
    {
      WlzGMDiskTAppend(eV[idx]->diskT, nDT[idx]);
      nDT[idx]->vertex = eV[idx];
      nDT[idx]->vertexT = nVT0[idx];
    }
    /* New edges. */
    for(idx = 0; idx < 3; ++idx)
    {
      nE[idx]->edgeT = nET0[idx];
    }
    /* New edge topology elements */
    for(idx = 0; idx < 3; ++idx)
    {
      nIdx = (idx + 1) % 3;
      pIdx = (idx + 3 - 1) % 3;
      nET0[idx]->next = nET0[nIdx];
      nET0[idx]->prev = nET0[pIdx];
      nET0[idx]->opp = nET1[nIdx];
      nET0[idx]->rad = nET0[idx];
      nET0[idx]->edge = nE[idx];
      nET0[idx]->vertexT = nVT0[idx];
      nET0[idx]->parent = nLT[0];
      nET1[idx]->next = nET1[pIdx];   /* Previous because reverse direction. */
      nET1[idx]->prev = nET1[nIdx];       /* Next because reverse direction. */
      nET1[idx]->opp = nET0[pIdx];
      nET1[idx]->rad = nET1[idx];
      nET1[idx]->edge = nE[pIdx];
      nET1[idx]->vertexT = nVT1[idx];
      nET1[idx]->parent = nLT[1];
    }
    /* New face and loop topology elements. */
    nF->loopT = nLT[0];
    WlzGMLoopTAppend(eShell->child, nLT[0]);
    WlzGMLoopTAppend(eShell->child, nLT[1]);
    nLT[0]->opp = nLT[1];
    nLT[1]->opp = nLT[0];
    nLT[0]->face = nF;
    nLT[1]->face = nF;
    nLT[0]->edgeT = nET0[0];
    nLT[1]->edgeT = nET1[0];
    nLT[0]->parent = eShell;
    nLT[1]->parent = eShell;
    /* No change to shell. */
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Extends an existing shell within the model by adding:
*		1 face and 2 edges (with topological elements).
*
*		Extends an existing shell within the model by adding:
*		1 face and 2 edges (with topological elements).
*		Need to create: 1 face (nF), 2 loop topology elements
*		(nLT[0,1]), 2 edges (nE[0,1]), 6 edge topology elements
*		(nET0[0,1,2], nET1[0,1,2]), 3 disk topology element
*		(nDT[0,1,2]) and 6 vertex topology elements (nVT[0,1,2]
*		and nVT1[0,1,2]).
* \verbatim
*                           nVT00        nET00
*                         O ------------------------------->
*                   eV0 @- - - - - - - - eE  - - - - - - - - -@ eV1
*                        \   <--------------------------- O  /
*                       ^   O nVT10      nET11     nVT11  ^   O nVT01
*                        \ \ \                           / / /
*                         \   \                         /   /
*                  nE1 ------\ \                       / / /
*                           \   \ nET10               /   /
*                            \ \ \                   / /---- nE0
*                             \   \                 /   /
*                        nET02 \ \ \         nET12 / / / nET01
*                               \   \             /   /
*                                \ \ \           / / /
*                                 \   \ nVT12   /   /
*                                  \ \ \    |  / / /
*                                   \   \   | /   /
*                                    \ \ \  |/ / /
*                               nVT02 O   v O   v
*                                        \   /
*                                          @
*                                            eV2 = sV
*
*		    nDT = {nVT02, nVT12}
*                   nLT0 = {nET00, nET01, nET02}
*                   nLT1 = {nET10, nET11, nET12}
* \endverbatim
* \param	model			The model to add the segment to.
* \param	eE			The shared edge.
* \param	sV			The shared vertex (not on the
*                                       shared edge).
*/
static WlzErrorNum WlzGMModelExtend3V1E1S3D(WlzGMModel *model,
					    WlzGMEdge *eE, WlzGMVertex *sV)
{
  int		idx,
  		nIdx,
		pIdx;
  WlzGMShell	*eShell;
  WlzGMFace	*nF;
  WlzGMLoopT	*nLT[2];
  WlzGMEdge	*nE[2];
  WlzGMEdgeT	*nET0[3],
  		*nET1[3];
  WlzGMDiskT	*nDT;
  WlzGMVertex	*eV[3];
  WlzGMVertexT	*nVT0[3],
  		*nVT1[3];
  WlzDVertex3	pos[3];
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model &&
     ((nF = WlzGMModelNewF(model, &errNum)) != NULL) &&
     ((nLT[0] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nLT[1] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nE[0] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[1] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nET0[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nDT = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nVT0[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[2] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[2] = WlzGMModelNewVT(model, &errNum)) != NULL))
  {
    /* Get shell that's to be extended. */
    eShell = WlzGMEdgeGetShell(eE);
    /* Get verticies. */
    eV[0] = eE->edgeT->vertexT->diskT->vertex;
    eV[1] = eE->edgeT->opp->vertexT->diskT->vertex;
    eV[2] = sV;
    /* Get geometry of all 3 verticies. */
    for(idx = 0; idx < 3; ++idx)
    {
      (void )WlzGMVertexGetG3D(*(eV + idx), pos + idx);
    }
    /* New vertex topology elements. */
    WlzGMVertexTAppend(eE->edgeT->vertexT, nVT0[0]);
    WlzGMVertexTAppend(eE->edgeT->vertexT, nVT1[0]);
    nVT0[0]->diskT = nVT1[0]->diskT = eE->edgeT->vertexT->diskT;
    WlzGMVertexTAppend(eE->edgeT->opp->vertexT, nVT0[1]);
    WlzGMVertexTAppend(eE->edgeT->opp->vertexT, nVT1[1]);
    nVT0[1]->diskT = nVT1[1]->diskT = eE->edgeT->opp->vertexT->diskT;
    nVT0[2]->next = nVT0[2]->prev = nVT1[2];
    nVT1[2]->next = nVT1[2]->prev = nVT0[2];
    nVT0[2]->diskT = nVT1[2]->diskT = nDT;
    for(idx = 0; idx < 3; ++idx)
    {
      nVT0[idx]->parent = nET0[idx];
      nVT1[idx]->parent = nET1[idx];
    }
    /* New disk topology element. */
    nDT->next = nDT->prev = nDT;
    nDT->vertex = eV[2];
    nDT->vertexT = nVT0[2];
    /* New edges. */
    nE[0]->edgeT = nET0[1];
    nE[1]->edgeT = nET0[2];
    /* New edge topology elements */
    for(idx = 0; idx < 3; ++idx)
    {
      nIdx = (idx + 1) % 3;
      pIdx = (idx + 3 - 1) % 3;
      nET0[idx]->next = nET0[nIdx];
      nET0[idx]->prev = nET0[pIdx];
      nET0[idx]->opp = nET1[nIdx];
      nET0[idx]->rad = nET0[idx];
      nET0[idx]->vertexT = nVT0[idx];
      nET0[idx]->parent = nLT[0];
      nET1[idx]->next = nET1[pIdx];   /* Previous because reverse direction. */
      nET1[idx]->prev = nET1[nIdx];       /* Next because reverse direction. */
      nET1[idx]->opp = nET0[pIdx];
      nET1[idx]->rad = nET1[idx];
      nET1[idx]->vertexT = nVT1[idx];
      nET1[idx]->parent = nLT[1];
    }
    nET1[1]->edge = nET0[0]->edge = eE;
    nET1[2]->edge = nET0[1]->edge = nE[0];
    nET1[0]->edge = nET0[2]->edge = nE[1];
    /* Need to set radial edges for nET0[0] and nET1[1]. */
    WlzGMEdgeTInsertRadial(nET0[0]);
    WlzGMEdgeTInsertRadial(nET1[1]);
    /* New face and loop topology elements. */
    nF->loopT = nLT[0];
    WlzGMLoopTAppend(eShell->child, nLT[0]);
    WlzGMLoopTAppend(eShell->child, nLT[1]);
    nLT[0]->opp = nLT[1];
    nLT[1]->opp = nLT[0];
    nLT[0]->face = nF;
    nLT[1]->face = nF;
    nLT[0]->edgeT = nET0[0];
    nLT[1]->edgeT = nET1[0];
    nLT[0]->parent = eShell;
    nLT[1]->parent = eShell;
    /* Update shell */
    (void )WlzGMShellUpdateG3D(eShell, pos[2]);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Joins two existing shells within the model by adding:
*		1 face and 2 edges (with topological elements).
*
*		Joins two existing shells within the model by adding:
*		1 face and 2 edges (with topological elements).
*		Need to create: 1 face (nF), 2 loop topology elements
*		(nLT[0,1]), 2 edges (nE[0,1]), 6 edge topology elements
*		(nET0[0,1,2], nET1[0,1,2]), 3 disk topology element
*		(nDT[0,1,2]) and 6 vertex topology elements (nVT[0,1,2]
*		and nVT1[0,1,2]).
*		Need to delete 1 shell (dShell).
* \verbatim
*                           nVT00        nET00
*                         O ------------------------------->
*                   eV0 @- - - - - - - - eE  - - - - - - - - -@ eV1
*                        \   <--------------------------- O  /
*                       ^   O nVT10      nET11     nVT11  ^   O nVT01
*                        \ \ \                           / / /
*                         \   \                         /   /
*                  nE1 ------\ \                       / / /
*                           \   \ nET10               /   /
*                            \ \ \                   / /---- nE0
*                             \   \                 /   /
*                        nET02 \ \ \         nET12 / / / nET01
*                               \   \             /   /
*                                \ \ \           / / /
*                                 \   \ nVT12   /   /
*                                  \ \ \    |  / / /
*                                   \   \   | /   /
*                                    \ \ \  |/ / /
*                               nVT02 O   v O   v
*                                        \   /
*                                          @
*                                            eV2 = sV
*
*                   nDT = {nVT02, nVT12}
*                   nLT0 = {nET00, nET01, nET02}
*                   nLT1 = {nET10, nET11, nET12}
* \endverbatim
* \param	model			The model to add the segment to.
* \param	eE			The shared edge.
* \param	sV			The shared vertex (not on the
*                                       shared edge).
*/
static WlzErrorNum WlzGMModelJoin3V1E2S3D(WlzGMModel *model,
				          WlzGMEdge *eE, WlzGMVertex *sV)
{
  int		idx,
  		nIdx,
		pIdx;
  WlzGMShell	*dShell,
  		*eShell;
  WlzGMFace	*nF;
  WlzGMLoopT	*nLT[2];
  WlzGMEdge	*nE[2];
  WlzGMEdgeT	*nET0[3],
  		*nET1[3];
  WlzGMDiskT	*nDT;
  WlzGMVertex	*eV[3];
  WlzGMVertexT	*nVT0[3],
  		*nVT1[3];
  WlzDVertex3	pos[3];
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model &&
     ((nF = WlzGMModelNewF(model, &errNum)) != NULL) &&
     ((nLT[0] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nLT[1] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nE[0] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nE[1] = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nET0[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nDT = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nVT0[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[2] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[2] = WlzGMModelNewVT(model, &errNum)) != NULL))
  {
    /* Get shells that are to be extended and deleted. */
    eShell = WlzGMEdgeGetShell(eE);
    dShell = WlzGMVertexGetShell(sV);
    /* Get verticies. */
    eV[0] = eE->edgeT->vertexT->diskT->vertex;
    eV[1] = eE->edgeT->opp->vertexT->diskT->vertex;
    eV[2] = sV;
    /* Get geometry of all 3 verticies. */
    for(idx = 0; idx < 3; ++idx)
    {
      (void )WlzGMVertexGetG3D(*(eV + idx), pos + idx);
    }
    /* New vertex topology elements. */
    WlzGMVertexTAppend(eE->edgeT->vertexT, nVT0[0]);
    WlzGMVertexTAppend(eE->edgeT->vertexT, nVT1[0]);
    nVT0[0]->diskT = nVT1[0]->diskT = eE->edgeT->vertexT->diskT;
    WlzGMVertexTAppend(eE->edgeT->opp->vertexT, nVT0[1]);
    WlzGMVertexTAppend(eE->edgeT->opp->vertexT, nVT1[1]);
    nVT0[1]->diskT = nVT1[1]->diskT = eE->edgeT->opp->vertexT->diskT;
    nVT0[2]->next = nVT0[2]->prev = nVT1[2];
    nVT1[2]->next = nVT1[2]->prev = nVT0[2];
    nVT0[2]->diskT = nVT1[2]->diskT = nDT;
    for(idx = 0; idx < 3; ++idx)
    {
      nVT0[idx]->parent = nET0[idx];
      nVT1[idx]->parent = nET1[idx];
    }
    /* New disk topology element. */
    nDT->next = nDT->prev = nDT;
    nDT->vertex = eV[2];
    nDT->vertexT = nVT0[2];
    /* New edges. */
    nE[0]->edgeT = nET0[1];
    nE[1]->edgeT = nET0[2];
    /* New edge topology elements */
    for(idx = 0; idx < 3; ++idx)
    {
      nIdx = (idx + 1) % 3;
      pIdx = (idx + 3 - 1) % 3;
      nET0[idx]->next = nET0[nIdx];
      nET0[idx]->prev = nET0[pIdx];
      nET0[idx]->opp = nET1[nIdx];
      nET0[idx]->rad = nET0[idx];
      nET0[idx]->vertexT = nVT0[idx];
      nET0[idx]->parent = nLT[0];
      nET1[idx]->next = nET1[pIdx];   /* Previous because reverse direction. */
      nET1[idx]->prev = nET1[nIdx];       /* Next because reverse direction. */
      nET1[idx]->opp = nET0[pIdx];
      nET1[idx]->rad = nET1[idx];
      nET1[idx]->vertexT = nVT1[idx];
      nET1[idx]->parent = nLT[1];
    }
    nET1[1]->edge = nET0[0]->edge = eE;
    nET1[2]->edge = nET0[1]->edge = nE[0];
    nET1[0]->edge = nET0[2]->edge = nE[1];
    /* Need to set radial edges for nET0[0], nET1[1]. */
    WlzGMEdgeTInsertRadial(nET0[0]);
    WlzGMEdgeTInsertRadial(nET1[1]);
    /* New face and loop topology elements. */
    nF->loopT = nLT[0];
    WlzGMLoopTAppend(eShell->child, nLT[0]);
    WlzGMLoopTAppend(eShell->child, nLT[1]);
    nLT[0]->opp = nLT[1];
    nLT[1]->opp = nLT[0];
    nLT[0]->face = nF;
    nLT[1]->face = nF;
    nLT[0]->edgeT = nET0[0];
    nLT[1]->edgeT = nET1[0];
    nLT[0]->parent = eShell;
    nLT[1]->parent = eShell;
    /* Update extended shell and delete the joined shell. */
    (void )WlzGMShellUpdateG3D(eShell, pos[2]);
    if(model->child->idx == dShell->idx)
    {
      model->child = eShell;
    }
    WlzGMShellJoinAndUnlink(eShell, dShell);
    WlzGMModelFreeS(model, dShell);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Extends a shell within the model by adding:
*		1 face and 1 edge (with topological elements).
* 
*		Extends a shell within the model by adding:
*		1 face and 1 edge (with topological elements).
*		Need to create: 1 face (nF), 2 loop topology elements
*		(nLT[0,1]), 1 edge (nE), 6 edge topology elements
*		(nET0[0,1,2], nET1[0,1,2]) and 6 vertex topology elements
*		(nVT[0,1,2] and nVT1[0,1,2]).
* \endverbatim
*                           nVT02        nET02
*                         O ------------------------------->
*                   eV2 @- - - - - - - - nE  - - - - - - - - -@ eV0
*                        \   <--------------------------- O  /
*                       ^   O nVT12      nET10     nVT10  ^   O nVT00
*                        \ \ \                           / / /
*                         \   \                         /   /
*                  eE1 ------\ \                       / / /
*                           \   \ nET12               /   /
*                            \ \ \                   / /---- eE0
*                             \   \                 /   /
*                        nET01 \ \ \         nET11 / / / nET00
*                               \   \             /   /
*                                \ \ \           / / /
*                                 \   \ nVT11   /   /
*                                  \ \ \    |  / / /
*                                   \   \   | /   /
*                                    \ \ \  |/ / /
*                               nVT12 O   v O   v
*                                        \   /
*                                          @
*                                            eV1 
*
*                   nLT0 = {nET00, nET01, nET02}
*                   nLT1 = {nET10, nET11, nET12}
* \endverbatim
* \param	model			The model.
* \param	eE0			First shared edge.
* \param	eE1			Second shared edge.
*/
static WlzErrorNum WlzGMModelExtend3V2E1S3D(WlzGMModel *model,
					    WlzGMEdge *eE0, WlzGMEdge *eE1)
{
  int		idx,
  		nIdx,
		pIdx;
  WlzGMShell	*eShell;
  WlzGMFace	*nF;
  WlzGMLoopT	*nLT[2];
  WlzGMEdge	*nE;
  WlzGMEdgeT	*nET0[3],
  		*nET1[3];
  WlzGMDiskT	*eDT[2];
  WlzGMVertex	*eV[3];
  WlzGMVertexT	*eVT[3],
  		*nVT0[3],
  		*nVT1[3];
  WlzDVertex3	pos[3];
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model &&
     ((nF = WlzGMModelNewF(model, &errNum)) != NULL) &&
     ((nLT[0] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nLT[1] = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nE = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nET0[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET0[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1[2] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nVT0[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT0[2] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[1] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1[2] = WlzGMModelNewVT(model, &errNum)) != NULL))
  {
    /* Get shell that's to be extended. */
    eShell = WlzGMEdgeGetShell(eE0);
    /* Get verticies. */
    eV[1] = WlzGMEdgeCommonVertex(eE0, eE1);
    eV[0] = (eVT[0] = eE0->edgeT->vertexT)->diskT->vertex;
    if(eV[0]->idx == eV[1]->idx)
    {
      eV[0] = (eVT[0] = eE0->edgeT->opp->vertexT)->diskT->vertex;
      eVT[1] = eE0->edgeT->vertexT;
    }
    else
    {
      eVT[1] = eE0->edgeT->opp->vertexT;
    }
    eV[2] = (eVT[2] = eE1->edgeT->vertexT)->diskT->vertex;
    if(eV[2]->idx == eV[1]->idx)
    {
      eV[2] = (eVT[2] = eE1->edgeT->opp->vertexT)->diskT->vertex;
    }
    /* Get geometry of all 3 verticies. */
    for(idx = 0; idx < 3; ++idx)
    {
      (void )WlzGMVertexGetG3D(*(eV + idx), pos + idx);
    }
    /* New vertex topology elements. */
    for(idx = 0; idx < 3; ++idx)
    {
      WlzGMVertexTAppend(eVT[idx], nVT0[idx]);
      WlzGMVertexTAppend(eVT[idx], nVT1[idx]);
      nVT0[idx]->diskT = nVT1[idx]->diskT = eVT[idx]->diskT;
      nVT0[idx]->parent = nET0[idx];
      nVT1[idx]->parent = nET1[idx];
    }
    /* New edge. */
    nE->edgeT = nET0[2];
    /* New edge topology elements */
    for(idx = 0; idx < 3; ++idx)
    {
      nIdx = (idx + 1) % 3;
      pIdx = (idx + 3 - 1) % 3;
      nET0[idx]->next = nET0[nIdx];
      nET0[idx]->prev = nET0[pIdx];
      nET0[idx]->opp = nET1[nIdx];
      nET0[idx]->rad = nET0[idx];
      nET0[idx]->vertexT = nVT0[idx];
      nET0[idx]->parent = nLT[0];
      nET1[idx]->next = nET1[pIdx];   /* Previous because reverse direction. */
      nET1[idx]->prev = nET1[nIdx];       /* Next because reverse direction. */
      nET1[idx]->opp = nET0[pIdx];
      nET1[idx]->rad = nET1[idx];
      nET1[idx]->vertexT = nVT1[idx];
      nET1[idx]->parent = nLT[1];
    }
    nET1[1]->edge = nET0[0]->edge = eE0;
    nET1[2]->edge = nET0[1]->edge = eE1;
    nET1[0]->edge = nET0[2]->edge = nE;
    /* Need to set radial edges. */
    WlzGMEdgeTInsertRadial(nET0[0]);
    WlzGMEdgeTInsertRadial(nET1[1]);
    WlzGMEdgeTInsertRadial(nET0[1]);
    WlzGMEdgeTInsertRadial(nET1[2]);
    /* New face and loop topology elements. */
    nF->loopT = nLT[0];
    WlzGMLoopTAppend(eShell->child, nLT[0]);
    WlzGMLoopTAppend(eShell->child, nLT[1]);
    nLT[0]->opp = nLT[1];
    nLT[1]->opp = nLT[0];
    nLT[0]->face = nF;
    nLT[1]->face = nF;
    nLT[0]->edgeT = nET0[0];
    nLT[1]->edgeT = nET1[0];
    nLT[0]->parent = eShell;
    nLT[1]->parent = eShell;
    /* No change to shell */
    /* No new disk topology elements, but may need to delete one. */
    eDT[0] = nET0[1]->rad->vertexT->diskT;
    eDT[1] = nET1[1]->rad->vertexT->diskT;
    if(eDT[0]->idx != eDT[1]->idx)
    {
      eVT[0] = eVT[1] = eDT[1]->vertexT;
      do
      {
        eVT[2] = eVT[1]->next;
	WlzGMVertexTAppend(eDT[0]->vertexT, eVT[1]);
	eVT[1]->diskT = eDT[0];
	eVT[1] = eVT[2];
      } while(eVT[1]->idx != eVT[0]->idx);
      if(eV[1]->diskT->idx == eDT[1]->idx)
      {
        eV[1]->diskT = eDT[0];
      }
      WlzGMModelFreeDT(model, eDT[1]);
    }
  }
  return(errNum);
}


/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Constructs a new shell, edge (with topological
*		elements) and adds them to the model.
*
*		Constructs a new shell, edge (with topological
*		elements) and adds them to the model.
*		Given a pair of 2D double precision edge end points which
*		are known not to be in the model.
*		Neither of the verticies already exists within the shell.
*		Need to create a new shell with: 1 loop
*		topology element (nLT), 1 edge (nE) 2 edge
*		topology elements (nET[0,1]), 2 disk topology elements,
*		(nDT[0,1]), 2 verticies (nV[0,1] with geometry
*		pos[0,1]) and 2 vertex topology elements (nVT[0,1]).
* \verbatim
*		     / nV0
*		    /
*		   |  / nVT0    / nET0          / nE
*		   | /         /               /
*		   | O -----------------------/---->
*		    @- - - - - - - - - - - - - - - -@
*		     <---------------------------- O|
*		                      /           / |
*		                nET1 /      nVT1 /  |
*		                                    /
*		                               nV1 /
* \endverbatim
* \param	model			The model to add the segment to.
* \param	pos			Ptr to 1st then 2nd point.
*/
static WlzErrorNum WlzGMModelConstructNewS2D(WlzGMModel *model,
					     WlzDVertex2 *pos)
{
  int		idx;
  WlzGMShell	*eShell,
  		*nShell;
  WlzGMLoopT	*nLT;
  WlzGMEdge	*nE;
  WlzGMEdgeT	*nET[2];
  WlzGMDiskT	*nDT[2];
  WlzGMVertex	*nV[2];
  WlzGMVertexT	*nVT[2];
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model &&
     ((nShell = WlzGMModelNewS(model, &errNum)) != NULL) &&
     ((nLT = WlzGMModelNewLT(model, &errNum)) != NULL) &&
     ((nE = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nET[0] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET[1] = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nDT[0] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nDT[1] = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nV[0] = WlzGMModelNewV(model, &errNum)) != NULL) &&
     ((nV[1] = WlzGMModelNewV(model, &errNum)) != NULL) &&
     ((nVT[0] = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT[1] = WlzGMModelNewVT(model, &errNum)) != NULL))
  {
    for(idx = 0; idx < 2; ++idx)
    {
      /* New vertcies */
      nV[idx]->diskT = nDT[idx];
      (void )WlzGMVertexSetG2D(nV[idx], *(pos + idx));
      WlzGMModelAddVertex(model, nV[idx]);
      /* New vertex topology elements */
      nVT[idx]->next = nVT[idx]->prev = nVT[idx];
      nVT[idx]->diskT = nDT[idx];
      nVT[idx]->parent = nET[idx];
      /* New disk topology elements */
      nDT[idx]->next = nDT[idx]->prev = nDT[idx];
      nDT[idx]->vertex = nV[idx];
      nDT[idx]->vertexT = nVT[idx];
    }
    /* New Edges and their topology elements */
    nE->edgeT = nET[0];
    nET[0]->edge = nET[1]->edge = nE;
    nET[0]->rad = nET[0];
    nET[0]->next = nET[0]->prev = nET[0]->opp = nET[1];
    nET[0]->vertexT = nVT[0];
    nET[0]->parent = nLT;
    nET[1]->rad = nET[1];
    nET[1]->next = nET[1]->prev = nET[1]->opp = nET[0];
    nET[1]->vertexT = nVT[1];
    nET[1]->parent = nLT;
    /* New loop topology elements */
    nLT->next = nLT->prev = nLT->opp = nLT;
    nLT->face = NULL;
    nLT->edgeT = nET[0];
    nLT->parent = nShell;
    /* New shell in model */
    (void )WlzGMShellSetG2D(nShell, 2, pos);
    nShell->child = nLT;
    nShell->parent = model;
    nShell->next = nShell->prev = nShell;
    /* Is this the first shell of the model? */
    if((eShell = model->child) == NULL)
    {
      /* First shell in model */
      model->child = nShell;
    }
    else
    {
      /* Append new shell onto end of model's list of shells. */
      WlzGMShellAppend(eShell, nShell);
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Constructs a new edge (with topological elements) and
*		adds them to the model.
*
*		Constructs a new edge (with topological elements) and
*		adds them to the model.
*		Given an existing edge topology element at one end
*		of the new edge and a 2D double precision end points
*		at the other end of the new edge, where the new end
*		point is known not to be in the model.
*		Only one vertex already exists within the model so no new
*		shell is required, instead an existing shell is
*		extended using: 1 edge (nE), 2 edge topology elements
*		(nET0, nET1), 1 disk topology element (nDT), 1 vertex
*		(nV0 with geometry (nPos) and 2 vertex topology elements
*		(nVT0, nVT1).
* \verbatim
*		    / nV0                    nE
*		   /                             /
*		   |   /nVT0    / nET0          /         /eET0
*		   |  /        /               /         /
*		   |  O ----------------------/--->   O----
*		    @- - - - - - - - - - - - - - - -@- - - -
*		     <--------------------------- O |<------
*		                      /          /  |    \
*		                nET1 /      nVT1/   |     \eET1
*		                                   /
*		                               nV1/
* \endverbatim
* \param	model			The model to add the segment to.
* \param	eET0			Next edge topology element for the
*                                       new edge topology element directed
*                                       away from the new end point.
* \param	nPos			The new end point.
*/
static WlzErrorNum WlzGMModelExtendL2D(WlzGMModel *model, WlzGMEdgeT *eET0,
				       WlzDVertex2 nPos)
{
  WlzGMVertex	*nV;
  WlzGMVertexT	*nVT0,
  		*nVT1;
  WlzGMDiskT	*nDT;
  WlzGMEdge	*nE;
  WlzGMEdgeT	*eET1,
		*nET0,
  		*nET1;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model && eET0 &&
     ((nE = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nET0 = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1 = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nDT = WlzGMModelNewDT(model, &errNum)) != NULL) &&
     ((nV = WlzGMModelNewV(model, &errNum)) != NULL) &&
     ((nVT0 = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1 = WlzGMModelNewVT(model, &errNum)) != NULL))
  {
    eET1 = eET0->prev;
    /* New vertex */
    nV->diskT = nDT;
    (void )WlzGMVertexSetG2D(nV, nPos);
    WlzGMModelAddVertex(model, nV);
    /* Set vertex topology elements */
    nVT0->prev = nVT0->next = nVT0;
    nVT0->diskT = nDT;
    nVT0->parent = nET0;
    WlzGMVertexTAppend(eET0->vertexT, nVT1);
    nVT1->diskT = eET0->vertexT->diskT;
    nVT1->parent = nET1;
    /* New disk topology element */
    nDT->next = nDT->prev = nDT;
    nDT->vertex = nV;
    nDT->vertexT = nVT0;
    /* Edges and their topology elements */
    nE->edgeT = nET0;
    nET0->prev = nET1;
    nET0->next = eET0;
    eET0->prev = nET0;
    nET1->prev = eET1;
    eET1->next = nET1;
    nET1->next = nET0;
    nET0->opp = nET1;
    nET0->rad = nET0;
    nET0->edge = nE;
    nET0->vertexT = nVT0;
    nET0->parent = eET0->parent;
    nET1->opp = nET0;
    nET1->rad = nET1;
    nET1->edge = nE;
    nET1->vertexT = nVT1;
    nET1->parent = eET1->parent;
    /* Update the shell's geometry. */
    (void )WlzGMShellUpdateG2D(nET0->parent->parent, nPos);
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Splits a loop by adding a new edge.
*
*		Splits a loop by adding a new edge.
*		First of all checks that the edge segment doesn't
*		already exist! If it doesn't then a new edge is
*		constructed which splits and existing loop
*		within the model. Given the loop topology element
*		to be split and the existing edge topology elements at
*		new edges end points.
*		Split a loop within a shell by adding a new edge between the
*		two matched verticies.
*		Create: 1 edge (nE), 2 edge topology elements (nET0,
*		nET1), 2 vertex topology elements (nVT0, nVT1) and
*		a loop topology element (nLT).
* \verbatim
*		      O ------------->     O --------------->
*		    @- - - - - - - - - -@- - - - - - - - - - -@
*		    |  <------------ O  |  <--------------- O | O
*		  ^   O        /      ^   O                 ^
*		  | |    eET0 /       | |  \nVT1            | | |
*		  |   |               |   |                 |   |
*		  | | |        nET0 \ | | |/nE              | | |
*		  |   |              \|   /                 |   |
*		  | | |    eL         | |/| /nET1   nL      | | |
*		  |   |               |   |/                |   |
*		  | | |         nVT0 \  | |     / eET1        | |
*		  |   V               O   V    /            O   V
*		    |  O ------------>  |  O -------------->  |
*		    @- - - - - - - - - -@- - - - - - - - - - -@
*		      <-------------- O    <--------------- O
* \endverbatim
* \param	model			The model to add the segment to.
* \param	eET0			Next edge topology element for the
*                                       first end point.
* \param	eET1			Next edge topology element for the
*                                       other end point.
*/
static WlzErrorNum WlzGMModelConstructSplitL2D(WlzGMModel *model,
					       WlzGMEdgeT *eET0,
					       WlzGMEdgeT *eET1)
{
  WlzGMVertexT	*nVT0,
  		*nVT1;
  WlzGMEdge	*nE;
  WlzGMEdgeT	*nET0,
  		*nET1;
  WlzGMLoopT	*eLT,
  		*nLT;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  /* Check for duplicate edge. */
  if(eET0->prev->idx != eET1->prev->opp->idx)
  {
    if(model && eET0 &&
       ((nE = WlzGMModelNewE(model, &errNum)) != NULL) &&
       ((nET0 = WlzGMModelNewET(model, &errNum)) != NULL) &&
       ((nET1 = WlzGMModelNewET(model, &errNum)) != NULL) &&
       ((nVT0 = WlzGMModelNewVT(model, &errNum)) != NULL) &&
       ((nVT1 = WlzGMModelNewVT(model, &errNum)) != NULL) &&
       ((nLT = WlzGMModelNewLT(model, &errNum)) != NULL))
    {
      /* Vertex topology elements */
      WlzGMVertexTAppend(eET1->vertexT, nVT0);
      nVT0->diskT = eET1->vertexT->diskT;
      nVT0->parent = nET0;
      WlzGMVertexTAppend(eET0->vertexT, nVT1);
      nVT1->diskT = eET0->vertexT->diskT;
      nVT1->parent = nET1;
      /* Edges and their topology elements */
      nE->edgeT = nET0;
      nET0->next = eET0;
      nET0->prev = eET1->prev;
      nET0->opp = nET1;
      nET0->rad = nET0;
      nET0->edge = nE;
      nET0->vertexT = nVT0;
      nET0->parent = eET0->parent;
      nET1->next = eET1;
      nET1->prev = eET0->prev;
      nET1->opp = nET0;
      nET1->rad = nET1;
      nET1->edge = nE;
      nET1->vertexT = nVT1;
      nET1->parent = eET1->parent;
      nET0->next->prev = nET0;
      nET0->prev->next = nET0;
      nET1->next->prev = nET1;
      nET1->prev->next = nET1;
      /* Loop topology element */
      eLT = eET0->parent;
      eLT->edgeT = nET0;
      WlzGMLoopTAppend(eET0->parent, nLT);	       /* Add loopT to shell */
      nLT->opp = nLT;
      nLT->face = NULL;
      nLT->parent = eLT->parent;
      /* Set/update the loops by walking around the edge topology elements
       * setting their parents. */
      eLT->edgeT = nET0;
      nLT->edgeT = nET1;
      WlzGMLoopTSetT(eLT);
      WlzGMLoopTSetT(nLT);
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Constructs a new edge which joins two (different)
*		existing loops within the model.
*
*		brief	Constructs a new edge which joins two (different)
*		existing loops within the model.
*		Join two loops within two possibly different shells by
*		adding a new edge between the two matched verticies.
*		Create: 1 edge (nE), 2 edge topology elements (nET0,
*		nET1) and 2 vertex topology elements (nVT0, nVT1).
*		Delete: 1 loop topology element (dLT)
*		and 1 shell (dShell). If edges were (they're not)
*		allowed to cross then it would be possible to have a
*		pair of verticies within the same shell but NOT within
*		a common loop. Check for this disallowed case and return
*		an error if it's found.
* \verbatim
*	                                 eV1\
*	                                     \
*	                               nET0\  \
*	                       nE\          \  \          /eET1
*	                          \   /nVT0  \  \        /
*	             O -------->   \ O --------->\   O -------->
*	          @- - - - - - - -@- - - - - - - -@- - - - - - - -@
*	             <-------- O  \  <--------- O    <-------- O
*	                  /        \       \    \
*	             eET0/          \eV0    \    \nVT1
*	                                     \
*	                                      \nET1
* \endverbatim
* \param	model			The model to add the segment to.
* \param	eET0			Next edge topology element for the
*                                       first end point.
* \param	eET1			Next edge topology element for the
*                                       other end point.
*/
static WlzErrorNum WlzGMModelJoinL2D(WlzGMModel *model,
				     WlzGMEdgeT *eET0, WlzGMEdgeT *eET1)
{
  WlzGMVertexT	*nVT0,
  		*nVT1;
  WlzGMEdge	*nE;
  WlzGMEdgeT	*nET0,
  		*nET1;
  WlzGMLoopT	*eLT,
  		*dLT;
  WlzGMShell	*dS;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  if(model && eET0 &&
     ((nE = WlzGMModelNewE(model, &errNum)) != NULL) &&
     ((nET0 = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nET1 = WlzGMModelNewET(model, &errNum)) != NULL) &&
     ((nVT0 = WlzGMModelNewVT(model, &errNum)) != NULL) &&
     ((nVT1 = WlzGMModelNewVT(model, &errNum)) != NULL))
  {
    /* Vertex topology elements */
    WlzGMVertexTAppend(eET0->vertexT, nVT0);
    nVT0->diskT = eET0->vertexT->diskT; /* eV0 */
    nVT0->parent = nET0;
    WlzGMVertexTAppend(eET1->vertexT, nVT1);
    nVT1->diskT = eET1->vertexT->diskT; /* eV1 */
    nVT1->parent = nET1;
    /* Edges and their topology elements */
    nE->edgeT = nET0;
    nET0->next = eET1;
    nET0->prev = eET0->prev;
    nET0->opp = nET1;
    nET0->rad = nET0;
    nET0->edge = nE;
    nET0->vertexT = nVT0;
    nET0->parent = eET0->parent;
    nET1->next = eET0;
    nET1->prev = eET1->prev;
    nET1->opp = nET0;
    nET1->rad = nET1;
    nET1->edge = nE;
    nET1->vertexT = nVT1;
    nET1->parent = eET0->parent;
    nET0->next->prev = nET0;
    nET0->prev->next = nET0;
    nET1->next->prev = nET1;
    nET1->prev->next = nET1;
    /* Loop topology elements */
    eLT = eET0->parent;
    dLT = eET1->parent;
    dS = dLT->parent;
    /* Look to see if loops share the same shell. */
    if((dS = dLT->parent)->idx == eLT->parent->idx)
    {
      dS = NULL;
    }
    else
    {
      /* Merge the two shell's geometries. */
      (void )WlzGMShellMergeG(eLT->parent, dS);
    }
    /* Set/update the retained loop topology element by walking around the
     * edge topology  elements setting their parents. */
    WlzGMLoopTSetT(eLT);
    /* Unlink and free the deleted loop topology element. */
    WlzGMLoopTUnlink(dLT);
    (void )WlzGMModelFreeLT(model, dLT);
    /* If deleted loop had a different parent shell then unlink it and
     * free the deleted loops parent shell if it's childless. */
    if(dS)
    {
      WlzGMShellUnlink(dS);
      (void )WlzGMModelFreeS(model, dS);
    }
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Constructs a 2D simplex (edge) defined by two double
*               precision end points. Either of the two points may
*               already exist within the model.
*               See WlzGMShellMatchVtxG2D() for the meaning of the
*               backwards, forwards and distance search parameters.
* \param	model			The model to add the segment to.
* \param	pos			Pointer to first then second
*                                       positions.
*/
WlzErrorNum	WlzGMModelConstructSimplex2D(WlzGMModel *model,
					     WlzDVertex2 *pos)
{
  unsigned int	matchCode;
  WlzGMEdgeT	*matchEdgeT[2];
  WlzErrorNum	errNum = WLZ_ERR_NONE;

#ifdef WLZ_GEOMODEL_DEBUG
  (void )fprintf(stderr, "%g %g %g %g\n",
		 (pos + 0)->vtX, (pos + 0)->vtY,
		 (pos + 1)->vtX, (pos + 1)->vtY);
  (void )fflush(stderr);
#endif /* WLZ_GEOMODEL_DEBUG */
  WlzGMModelMatchEdgeTG2D(model, matchEdgeT, pos);
  matchCode = ((matchEdgeT[1] != NULL) << 1) | (matchEdgeT[0] != NULL);
  switch(matchCode)
  {
    case 0:
      /* Edge makes a new shell. */
      errNum = WlzGMModelConstructNewS2D(model, pos);
      break;
    case 1:
      /* Edge extends a loop and shell. */
      errNum = WlzGMModelExtendL2D(model, matchEdgeT[0], *(pos + 1));
      break;
    case 2:
      /* Edge extends a loop and shell. */
      errNum = WlzGMModelExtendL2D(model, matchEdgeT[1], *(pos + 0));
      break;
    case 3:
      /* Both verticies already exist within the model.
         Don't insert this simplex if it is already in the model. */
      if(matchEdgeT[0]->edge->idx != matchEdgeT[1]->edge->idx)
      {
	/* Both verticies already exist within the model so adding another
	 * edge between the two verticies will change the number of loops
	 * and shells. */
	if(WlzGMEdgeTCommonLoopT(matchEdgeT[0], matchEdgeT[1]) != NULL)
	{
	  /* Both of the verticies lie on a common loop then that loop needs
	   * to be SPLIT by a new edge. */
	  errNum = WlzGMModelConstructSplitL2D(model,
						matchEdgeT[0], matchEdgeT[1]);
	}
	else
	{
	  /* The two verticies do NOT share a common loop so loops will be
	   * joined by a new edge, destroying a loop.  */
	  errNum = WlzGMModelJoinL2D(model, matchEdgeT[0], matchEdgeT[1]);
	  
	}
      }
      break;
  }
  return(errNum);
}

/*!
* \return				Woolz error code.
* \ingroup      WlzGeoModel
* \brief	Constructs a 3D simplex (triangle) defined by three
*               double precision verticies, any of which may already
*               exist within the model.
* \param	model			The model to add the segment to.
* \param	pos			Pointer to triangle verticies.
*/
WlzErrorNum	WlzGMModelConstructSimplex3D(WlzGMModel *model,
				       	     WlzDVertex3 *pos)
{
  int		idx0,
  		idx1,
		idx2,
		edgeCnt,
		shellCnt;
  WlzGMEdge	*cE[3];
  unsigned int	matchCode;
  WlzGMVertex	*matchV[3];
  WlzErrorNum	errNum = WLZ_ERR_NONE;
  const int 	bitCntTb[8] =	     /* Number of verticies matched for a given
  				      * match code index */
  {
    0, /* 0 */
    1, /* 1 */
    1, /* 2 */
    2, /* 3 */
    1, /* 4 */
    2, /* 5 */
    2, /* 6 */
    3  /* 7 */
  },
  		firstCtgBitTb[8] = /* First of contigous bits set, bits wrap
				    * around */
  {
    0, /* 0 */
    0, /* 1 */
    1, /* 2 */
    0, /* 3 */
    2, /* 4 */
    2, /* 5 */
    1, /* 6 */
    0  /* 7 */
  };
  const int 	matchVtxIdx[8][3] =  /* Indicies of the given simplex verticies
  				      * that have been matched for a given
				      * match code index */
  {
    {0, 0, 0}, /* 0 */
    {0, 1, 2}, /* 1 */
    {1, 2, 0}, /* 2 */
    {0, 1, 2}, /* 3 */
    {2, 0, 1}, /* 4 */
    {2, 0, 1}, /* 5 */
    {1, 2, 0}, /* 6 */
    {0, 1, 2}, /* 7 */
  };

  for(idx0 = 0; idx0 < 3; ++idx0)
  {
    matchV[idx0] = WlzGMModelMatchVertexG3D(model, *(pos + idx0));
  }
  matchCode = ((matchV[2] != NULL) << 2) |
  	      ((matchV[1] != NULL) << 1) | (matchV[0] != NULL);
  switch(bitCntTb[matchCode])
  {
    case 0:
      /* No verticies matched, construct a new shell. */
      errNum = WlzGMModelConstructNewS3D(model, pos);
      break;
    case 1:
      /* Single vertex matched, extend existing shell. */
      idx0 = matchVtxIdx[matchCode][0];
      idx1 = matchVtxIdx[matchCode][1];
      idx2 = matchVtxIdx[matchCode][2];
      errNum = WlzGMModelExtend1V0E1S3D(model, matchV[idx0],
				        *(pos + idx1), *(pos + idx2));
      break;
    case 2:
      /* Two verticies matched, check for existing edges and shells. */
      idx0 = matchVtxIdx[matchCode][0];
      idx1 = matchVtxIdx[matchCode][1];
      idx2 = matchVtxIdx[matchCode][2];
      if(WlzGMVertexCommonShell(matchV[idx0], matchV[idx1]) != NULL)
      {
        if((cE[0] = WlzGMVertexCommonEdge(matchV[idx0],
				          matchV[idx1])) != NULL)
	{
	  /* Matched 2 verticies connected by an edge. */
	  errNum = WlzGMModelExtend2V1E1S3D(model, cE[0], *(pos + idx2));
	}
	else
	{
	  /* Matched 2 verticies in same shell but not connected by an edge. */
	  errNum = WlzGMModelExtend2V0E1S3D(model, matchV[idx0], matchV[idx1],
					    *(pos + idx2));
	}
      }
      else
      {
	/* Matched 2 verticies in different shells. */
	errNum = WlzGMModelJoin2V0E0S3D(model, matchV[idx0], matchV[idx1],
					*(pos + idx2));
      }
      break;
    case 3:
      /* Three verticies matched, count existing edges and shells which use
       * these verticies. */
      edgeCnt = ((cE[0] = WlzGMVertexCommonEdge(matchV[0],
      						matchV[1])) != NULL) +
		((cE[1] = WlzGMVertexCommonEdge(matchV[1],
						matchV[2])) != NULL) +
		((cE[2] = WlzGMVertexCommonEdge(matchV[2],
						matchV[0])) != NULL);
      idx0 = (WlzGMVertexGetShell(matchV[0]))->idx;
      idx1 = (WlzGMVertexGetShell(matchV[1]))->idx;
      idx2 = (WlzGMVertexGetShell(matchV[2]))->idx;
      if((idx0 != idx1) && (idx1 != idx2) && (idx2 != idx0))
      {
        shellCnt = 3;
      }
      else if((idx0 == idx1) && (idx1 == idx2) && (idx2 == idx0))
      {
        shellCnt = 1;
      }
      else
      {
        shellCnt = 2;
      }
      switch(edgeCnt)
      {
        case 0:
	  switch(shellCnt)
	  {
	    case 1:
	      /* All 3 matched verticies share the same shell but there
	       * are no common edges between the 3 verticies. */
	      errNum = WlzGMModelExtend3V0E1S3D(model, matchV);
	      break;
	    case 2: /* 3V0E2S */
	      /* Two of the 3 matched verticies share the same shell but
	       * there is no common edge between any of the matched
	       * verticies. */
	      errNum = WlzGMModelJoin3V0E2S3D(model, matchV);
	      break;
	    case 3: /* 3V0E3S */
	      /* All 3 verticies are in different shells and there are
	       * no common edges between any of the matched verticies. */
	      errNum = WlzGMModelJoin3V0E3S3D(model, matchV);
	      break;
	  }
	  break;
	case 1:
	  idx2 = (cE[0] != NULL) | ((cE[1] != NULL) << 1) |
	  	 ((cE[2] != NULL) << 2);
	  idx0 = firstCtgBitTb[idx2];
	  idx1 = (idx0 + 2) % 3;
	  if(shellCnt == 1)
	  {
	    /* Two of the 3 matched verticies are connected by a single edge,
	     * all of the 3 are in a single shell. */
	     errNum = WlzGMModelExtend3V1E1S3D(model, cE[idx0], matchV[idx1]);
	  }
	  else
	  {
	    /* Two of the 3 matched verticies are connected by a single edge,
	     * the other vertex is in a different shell. */
	     errNum = WlzGMModelJoin3V1E2S3D(model, cE[idx0], matchV[idx1]);
	  }
	  break;
	case 2:
	  /* All 3 verticies share the same shell, and two pairs of verticies
	   * are connected by edges. */
	  idx2 = (cE[0] != NULL) | ((cE[1] != NULL) << 1) |
	  	 ((cE[2] != NULL) << 2);
	  idx0 = firstCtgBitTb[idx2];
	  idx1 = (idx0 + 1) % 3;
	  errNum = WlzGMModelExtend3V2E1S3D(model, cE[idx0], cE[idx1]);
	  break;
	case 3:
          /* All three verticies and all three edges are within the model,
	   * need to check if the three verticies are in a common loop and then
	   * if ther're not add a new loop, 2 loopT's, 6 edgeT's and 6
	   * vertexT's. */
	  /* TODO Write code for this! */
	  break;
      }
  }
  return(errNum);
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Walks around a loop topology element's child edge
*               topology elements setting their parent and making
*		sure that the loop topology element's have the
*		correct parent too.
* \param	gLT			The loop topology element.
*/
static void	WlzGMLoopTSetT(WlzGMLoopT *gLT)
{
  WlzGMShell	*gS;
  WlzGMEdgeT	*fET,
  		*tET;
  
  if(gLT && ((tET = fET = gLT->edgeT) != NULL))
  {
    gS = gLT->parent;
    do
    {
      tET->parent = gLT;
      tET->opp->parent->parent = gS;
    } while((tET = tET->next)->idx != fET->idx);
  }
}

/*!
* \return				The parent shell or NULL if no
*					parent shell was found.
* \brief	Walks around a loop topology element looking for another
*		loop topology element connected to the given one. If
*		such a loop topology element exists this function returns
*		it's shell.
* \param	gLT			The given loopT.
*/
static WlzGMShell *WlzGMLoopTFindShell(WlzGMLoopT *gLT)
{
  WlzGMEdgeT	*fET,
  		*tET;
  WlzGMShell	*fS = NULL;

  tET = fET = gLT->edgeT;
  do
  {
    tET = tET->next;
    if(tET->opp->parent != gLT)
    {
      /* LoopTs connected by an edge. */
      fS = tET->opp->parent->parent;
    }
    else if(tET->vertexT->diskT != tET->vertexT->diskT->next)
    {
      /* loopTs connected by a vertex. */
      /* TODO not implemented yet, needed for 3D. */
    }
  } while((fS == NULL) && (tET != fET));
  return(fS);
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Removes a vertex from the models vertex hash table.
*               The vertex's geometry must have been set.
* \param	model			The model.
* \param	dV			The vertex to remove from the
*                                       models hash table.
*/
static void	WlzGMModelRemVertex(WlzGMModel *model, WlzGMVertex *dV)
{
  WlzDVertex3	nPos;
  unsigned int 	hVal;
  WlzGMVertex  	**hdV;
  WlzGMVertex	*tV,
		*pV;

  (void )WlzGMVertexGetG3D(dV, &nPos);
  hVal = WlzGMHashPos3D(nPos);
  hdV = model->vertexHT + (hVal % model->vertexHTSz);
  if(*hdV)
  {
    /* Need to search for position in the linked list. */
    tV = *hdV;
    pV = NULL;
    while(tV && (WlzGMVertexCmpSign3D(tV, nPos) < 0))
    {
      pV = tV;
      tV = tV->next;
    }
    /* tV is now dV. */
    if(pV == NULL)
    {
      *hdV = dV->next;
    }
    else
    {
      pV->next = dV->next;
    }
    dV->next = NULL;
  }
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Adds a new vertex into the models vertex hash table.
*               The vertex's geometry must have been set.
* \param	model			The model.
* \param	nV			New vertex to insert into the
*                                       models hash table.
*/
static void	WlzGMModelAddVertex(WlzGMModel *model, WlzGMVertex *nV)
{
  WlzDVertex3	nPos;
  unsigned int 	hVal;
  WlzGMVertex  	**hdV;
  WlzGMVertex	*tV,
		*pV;

  (void )WlzGMVertexGetG3D(nV, &nPos);
  hVal = WlzGMHashPos3D(nPos);
  hdV = model->vertexHT + (hVal % model->vertexHTSz);
  if(*hdV == NULL)
  {
    /* No vertex at this position in the hash table yet. */
    *hdV = nV;
    nV->next = NULL;
  }
  else
  {
    /* Need to search for position in the linked list. */
    tV = *hdV;
    pV = NULL;
    while(tV && (WlzGMVertexCmpSign3D(tV, nPos) < 0))
    {
      pV = tV;
      tV = tV->next;
    }
    if(pV == NULL)
    {
      nV->next = *hdV;
      *hdV = nV;
    }
    else if(tV == NULL)
    {
      nV->next = NULL;
      pV->next = nV;
    }
    else
    {
      nV->next = pV->next;
      pV->next = nV;
    }
  }
}

/*!
* \return				<void>
* \ingroup      WlzGeoModel
* \brief	Attempts to find verticies which match the two given
*               double precision positions.
*               The linking/matching workspace makes the matching much
*               more efficient, but it's only valid if the verticies
*               are maintained with indicies which increase with 
*               increasing row, column.
* \param	model			Model with resources.
* \param	matchET			Array for return of matched
*                                       edge topology element pointers.
* \param	pos			Pointer to first then second
*                                       positions.
*/
static void	WlzGMModelMatchEdgeTG2D(WlzGMModel *model,
					WlzGMEdgeT **matchET,
				        WlzDVertex2 *pos)
{
  int		idD;
  double	tD0,
  		tD1,
		trX,
		trY,
		trCos,
		trSin,
		trScale;
  WlzDVertex2	tPosB,
  		tPosN,
		mPos,
  		oPos,
		tPos;
  WlzGMEdgeT	*eTB,
  		*eTL,
		*eTN,
		*eTS;
  WlzGMVertex	*vertex;
  WlzGMVertexT	*vertexT;
  WlzGMVertex	*matchV[2];
  const double	tol = 1.0e-06;

  *(matchET + 0) = *(matchET + 1) = NULL;
  /* First find any verticies that have geometry element equal one of
   * the given positions. */
  matchV[0] = WlzGMModelMatchVertexG2D(model, *(pos + 0));
  matchV[1] = WlzGMModelMatchVertexG2D(model, *(pos + 1));
  /* Now find the edge topology element(s) that are directed away from these
   * verticies. */
  for(idD = 0; idD < 2; ++idD)			 /* For each matched vertex. */
  {
    /* Check to see if this vertex has more than one vertex topology element.
     * If it does then more than one edge shares this vertex and the search
     * for the next edge topology elements becomes more complicated. */
    if((vertex = *(matchV + idD)) != NULL)
    {
      vertexT = vertex->diskT->vertexT;
      eTS = vertexT->parent;
      if(vertexT->idx == vertexT->next->idx)
      {
	/* The simple case just one edge uses this vertex. */
	*(matchET + idD) = eTS;
      }
      else
      {
	/* More than one edge uses this vertex. Oh dear! Now need to search
	 * for next edge, where the next edge is directed towards *(pos + idD)
	 * and away from *(pos + idD? 0: 1).
	 */
	mPos = *(pos + idD);
	oPos = *(pos + !idD);
	/* Compute the rigid body affine transform which takes the
	 * line segment from the matched vertex at mPos to the new vertex at
	 * oPos onto the x-axis and oriented from the origin.
	 * The transform is of the form:
	 *   x' = scale * (x*cos(theta) - y*sin(theta) + dx)
	 *   y' = scale * (x*sin(theta) + y*cos(theta) + dy) */
	tD0 = mPos.vtX - oPos.vtX;
	tD1 = mPos.vtY - oPos.vtY;
	trScale = (tD0 * tD0) + (tD1 * tD1);
	if(trScale > tol)
	{
	  trScale = 1.0 / sqrt(trScale);
	  trCos = (oPos.vtX - mPos.vtX);
	  trSin = (mPos.vtY - oPos.vtY);
	  trX = (mPos.vtX * mPos.vtX) + (mPos.vtY * mPos.vtY) -
		(mPos.vtX * oPos.vtX) - (mPos.vtY * oPos.vtY);
	  trY = (mPos.vtX * oPos.vtY) - (mPos.vtY * oPos.vtX);
	  /* Search for next edge by walking around the matched vertex CCW,
	   * using the vertex topology elements, comparing the relative
	   * polar angles of the line segments between the verticies.
	   * The next edge topology element is the one with the smallest
	   * CCW polar angle relative to the new, as yet unmade, edge. */
	  eTB = eTS;	  /* Initialize the search, best (eTB) = start (eTS) */
	  (void )WlzGMVertexGetG2D(eTB->vertexT->diskT->vertex, &tPos);
	  tPosB.vtX = trScale *
		      ((tPos.vtX * trCos) - (tPos.vtY * trSin) + trX);
	  tPosB.vtY = trScale *
		      ((tPos.vtX * trSin) - (tPos.vtY * trCos) + trY);
	  /* Find next edge topology element (eTN) directed from the vertex.
	   * Because the edge topology elements are unordered need to
	   * search them all. */
	  eTN = eTB->opp->next;
	  /* For each edge topology element directed away from the vertex */
	  while(eTS->idx != eTN->idx)
	  {
	    (void )WlzGMVertexGetG2D(eTN->vertexT->diskT->vertex, &tPos);
	    tPosN.vtX = trScale *
			((tPos.vtX * trCos) - (tPos.vtY * trSin) + trX);
	    tPosN.vtY = trScale *
			((tPos.vtX * trSin) - (tPos.vtY * trCos) + trY);
	    /* Compare the best and next transformed vertex positions */
	    eTL = eTN;
	    if(WlzGeomCmpAngle(tPosN, tPosB) > 0)
	    {
	      /* Update the best (minimum angle) so far. */
	      tPosB = tPosN;
	      eTB = eTN;
	    }
	    eTN = eTL->opp->next;
	  }
	  *(matchET + idD) = eTB;
	}
      }
    }
  }
}

/* Model Features */

/*!
* \return				Number of simplicies in given shell.
* \ingroup      WlzGeoModel
* \brief	Counts the number of simplicies in the given shell.
*               For 2D models the simplicies are edges and for 3D models
*               they are loops.
* \param	shell			The given shell.
*/
int		WlzGMShellSimplexCnt(WlzGMShell *shell)
{
  int		cnt = 0;
  WlzGMEdgeT	*cET,
  		*fET;
  WlzGMLoopT	*cLT,
  		*fLT;
  WlzGMModel	*model;

  if(((model = shell->parent) != NULL) && (shell != NULL))
  {
    if((cLT = fLT = shell->child) != NULL)
    {
      do
      {
	if((model->type == WLZ_GMMOD_2I) || (model->type == WLZ_GMMOD_2D))
	{
	  cET = fET = cLT->edgeT;
	  do
	  {
	   ++cnt;
	   cET = cET->next;
	  } while(cET != fET);
	}
	else /* model->type == WLZ_GMMOD_3I || model->type == WLZ_GMMOD_3D */
	{
	  ++cnt;
	}
	cLT = cLT->next;
      } while(cLT != fLT);
      cnt /= 2;
    }
  }
  return(cnt);
}
