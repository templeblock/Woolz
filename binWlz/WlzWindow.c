#pragma ident "MRC HGU $Id$"
/***********************************************************************
* Project:      Woolz
* Title:        WlzWindow.c
* Date:         March 1999
* Author:       Bill Hill
* Copyright:	1999 Medical Research Council, UK.
*		All rights reserved.
* Address:	MRC Human Genetics Unit,
*		Western General Hospital,
*		Edinburgh, EH4 2XU, UK.
* Purpose:      Woolz filter which applies a window function to the
*		given 2D object.
* $Revision$
* Maintenance:	Log changes below, with most recent at top of list.
************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <Wlz.h>

extern int      getopt(int argc, char * const *argv, const char *optstring);
 
extern char     *optarg;
extern int      optind,
                opterr,
                optopt;

int             main(int argc, char **argv)
{
  int		option,
		ok = 1,
		usage = 0,
  		orgDef = 1,
		radDef = 1;
  WlzIVertex2    org,
		rad;
  WlzWindowFnType winFn = WLZ_WINDOWFN_WELCH;
  FILE		*fP = NULL;
  WlzObject     *inObj = NULL,
                *outObj = NULL;
  char		*outObjFileStr,
  		*inObjFileStr;
  WlzErrorNum	errNum = WLZ_ERR_NONE;
  const char    *errMsg;
  static char	optList[] = "c:o:r:w:h",
		outObjFileStrDef[] = "-",
		inObjFileStrDef[] = "-";
 
  opterr = 0;
  outObjFileStr = outObjFileStrDef;
  inObjFileStr = inObjFileStrDef;
  while(ok && ((option = getopt(argc, argv, optList)) != -1))
  {
    switch(option)
    {
      case 'c':
	orgDef = 0;
	if(sscanf(optarg, "%d,%d", &(org.vtX), &(org.vtY)) != 2)
	{
	  usage = 1;
	  ok = 0;
	}
	break;
      case 'r':
	radDef = 0;
	if(sscanf(optarg, "%d,%d", &(rad.vtX), &(rad.vtY)) != 2)
	{
	  usage = 1;
	  ok = 0;
	}
	break;
      case 'w':
	if((winFn = WlzWindowFnValue(optarg)) == WLZ_WINDOWFN_UNSPECIFIED)
	{
	  usage = 1;
	  ok = 0;
	}
	break;
      case 'o':
        outObjFileStr = optarg;
	break;
      case 'h':
      default:
	usage = 1;
	ok = 0;
	break;
    }
  }
  if((inObjFileStr == NULL) || (*inObjFileStr == '\0') ||
     (outObjFileStr == NULL) || (*outObjFileStr == '\0'))
  {
    ok = 0;
    usage = 1;
  }
  if(ok && (optind < argc))
  {
    if((optind + 1) != argc)
    {
      usage = 1;
      ok = 0;
    }
    else
    {
      inObjFileStr = *(argv + optind);
    }
  }
  if(ok)
  {
    errNum = WLZ_ERR_READ_EOF;
    if((inObjFileStr == NULL) ||
       (*inObjFileStr == '\0') ||
       ((fP = (strcmp(inObjFileStr, "-")?
              fopen(inObjFileStr, "r"): stdin)) == NULL) ||
       ((inObj= WlzAssignObject(WlzReadObj(fP, &errNum), NULL)) == NULL))
    {
      ok = 0;
      (void )WlzStringFromErrorNum(errNum, &errMsg);
      (void )fprintf(stderr,
                     "%s: failed to read object from file %s (%s)\n",
                     *argv, inObjFileStr, errMsg);
    }
    if(fP && strcmp(inObjFileStr, "-"))
    {
      fclose(fP);
    }
  }
  if(ok)
  {
    if(inObj->type != WLZ_2D_DOMAINOBJ)
    {
      (void )fprintf(stderr,
      		     "%s: object read from %s has invalid type(%d)\n",
		     *argv, inObjFileStr, inObj->type);
      ok = 0;
    }
    else if(inObj->domain.core == NULL)
    {
      (void )fprintf(stderr,
      		     "%s: object read from %s has null domain\n",
		     *argv, inObjFileStr);
      ok = 0;
    }
    else if(inObj->values.core == NULL)
    {
      (void )fprintf(stderr,
      		     "%s: object read from %s has null values\n",
		     *argv, inObjFileStr);
      ok = 0;
    }
    else
    {
      if(orgDef)
      {
        org.vtX = (inObj->domain.i->lastkl + inObj->domain.i->kol1) / 2;
	org.vtY = (inObj->domain.i->lastln + inObj->domain.i->line1) / 2;
      }
      if(radDef)
      {
        rad.vtX = (inObj->domain.i->lastkl - inObj->domain.i->kol1 - 1) / 2;
	rad.vtY = (inObj->domain.i->lastln - inObj->domain.i->line1 - 1) / 2;
      }
    }
  }
  if(ok)
  {
    if((outObj = WlzWindow(inObj, winFn, org, rad, &errNum)) == NULL)
    {
      (void )WlzStringFromErrorNum(errNum, &errMsg);
      (void )fprintf(stderr,
      		     "%s: failed to windowed object (%s).\n",
		     *argv, errMsg);
      ok = 0;
    }
  }
  if(ok)
  {
    errNum = WLZ_ERR_WRITE_EOF;
    if(((fP = (strcmp(outObjFileStr, "-")?
              fopen(outObjFileStr, "w"):
              stdout)) == NULL) ||
       ((errNum = WlzWriteObj(fP, outObj)) != WLZ_ERR_NONE))
    {
      ok = 0;
      (void )WlzStringFromErrorNum(errNum, &errMsg);
      (void )fprintf(stderr,
                     "%s: failed to write windowed object (%s).\n",
                     *argv, errMsg);
    }
    if(fP && strcmp(outObjFileStr, "-"))
    {
      fclose(fP);
    }
  }
  if(inObj)
  {
    WlzFreeObj(inObj);
  }
  if(outObj)
  {
    WlzFreeObj(outObj);
  }
  if(!ok)
  {
    if(usage)
    {
      (void )fprintf(stderr,
      "Usage: %s %s",
      *argv,
      "        [-h] [-o<out object>]\n"
      "        [-c#,#] [-r#,#] [-w<window function>] [<in object>)]\n"
      "Applies a window function to a Woolz object.\n"
      "Options are:\n"
      "  -h         Help, prints this usage information.\n"
      "  -c#,#      Window function center in object's coordinates. First\n"
      "             coordinate is the column, second the line. If neither is\n"
      "             specified then the window is centered within the object.\n"
      "  -r#,#      Window function radii. First is the number of columns,\n"
      "             second the number of lines. If neither is specified then\n"
      "             the window radii are set to the maximum values st the\n"
      "             window is enclosed within the object.\n"
      " -w<win fn>  Window function name this must be one of: blackman,\n"
      "             hamming, hanning, parzen, rectangle or welch.\n");
    }
  }
  return(!ok);
}