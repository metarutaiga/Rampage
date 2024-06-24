/* -*-c++-*- */
/* $Header: edgeesc.c, 4, 7/17/00 4:44:43 PM PDT, Dan O'Connel$ */
/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** File name:   edgeesc.c
**
** Description: Catches escape calls made for Edge Tools functions.
**
** $Revision: 4$
** $Date: 7/17/00 4:44:43 PM PDT$
**
** $Log: 
**  4    3dfx      1.3         7/17/00  Dan O'Connel    Major changes ported 
**       from Napalm driver to support: Registry Controlled Modes, DFP, TvOut, 
**       read OEM config from BIOS, updated 3dfx Tools support, and other 
**       features and bug fixes.
**  3    3dfx      1.2         7/13/00  Ryan Bissell    Fix for PRS 14475,  
**       R2CSIMTOT: WHQL DCT 300 GDI Test returns a DibView2 Invalid page fault 
**       
**  2    3dfx      1.1         12/22/99 Ryan Bissell    New clut management code
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $ 
*/



#include "dfpapi.h"
#include "edgecaps.h"
#include "edgedefs.h"
#include "infoapi.h"
#include "funcapi.h"
#include "edgeapi.h" /* for guid definitions */
#include "colorapi.h" /* for gamma and colour functions */
#include "edgeesc.h"
#if 0
#include "monitorapi.h"
#endif
//will know the structure of lpszInData by the escape call

int EdgeEscape(int nEscape, void * lpszInData, void * lpszOutData)
{


	switch (nEscape)
	{
	case STB_QUERYINTERFACE:
		{	
		//	STB_InterfaceSupported(lpszInData, lpszOutData);

			int match = 0;	/* match used to check whether a GUID has been matched */
			int i =0;		/* used as a count to check for a matching guid */
		
			/* need to search structure of guids to find if one exists */
			/* have to look up guid to return enumerated value */
			/* loop while guid is not the null guid */

         if (0UL == (DWORD)lpszOutData)
            return STB_ESCAPE_NOT_SUPPORTED; /* error */

			do
			{
				if STB_GUIDMATCH(STB_INTERFACE_GUIDS[i].Guid,((STB_INTERFACESUPPORTED *)lpszInData)->Guid)
				{
					((STB_INTERFACESUPPORTED *)lpszOutData)->ulResult = STB_INTERFACE_GUIDS[i].Enum;
					((STB_INTERFACESUPPORTED *)lpszOutData)->Guid = STB_INTERFACE_GUIDS[i].Guid;
					match=1;
					return STB_ESCAPE_HANDLED;
				}
				i++;
			}
			while (!(STB_GUIDMATCH(STB_INTERFACE_GUIDS[i].Guid,ZeroGuid)));

         ((STB_INTERFACESUPPORTED *)lpszOutData)->ulResult = 0UL;
			return STB_ESCAPE_NOT_SUPPORTED; /* error */
		}
	case STB_GETPROPERTY:
		{

			STB_INTERFACESUPPORTED InterfaceSupported;

			InterfaceSupported.Guid=((STB_PROPERTY *)lpszInData)->Guid;
		
			EdgeEscape(STB_QUERYINTERFACE,&InterfaceSupported, &InterfaceSupported);
	
		/* check to see if interace supported, and only precede if it does */
		/* if interface is supported, pInterfaceSupported->ulResult will   */
		/* contain an identifier for the interface that can be switched on */
	
			if	(0!=(InterfaceSupported.ulResult))
			{
				switch (InterfaceSupported.ulResult)
				{
#ifdef STB_DFP_ENABLED
					case (STB_DFP_API):
					{
						return (DFPGetProperty(lpszInData,lpszOutData));
					}
#endif
					case (STB_FUNCTION_API):
					{
						return(FunctionAPIGetProperty(lpszInData,lpszOutData));
					}
					case (STB_INFO_API):
					{
						return(InfoAPIGetProperty(lpszInData,lpszOutData));
					}
#if 0
					case (STB_MONITOR_API):
					{
						return(MonitorAPIGetProperty(lpszInData,lpszOutData));
					}
#endif
                    case (STB_COLOUR_API):
					{
						return(ColourAPIGetProperty(lpszInData,lpszOutData));
					}
					default :
					{
						return (STB_ESCAPE_NOT_SUPPORTED);
					}
				}
			} 
			else
			{
            if (0UL != (DWORD)lpszOutData)
            {
				   ((STB_PROPERTY *)lpszOutData)->Guid =  ((STB_PROPERTY *)lpszInData)->Guid;
				   ((STB_PROPERTY *)lpszOutData)->ulPropertyId =  ((STB_PROPERTY *)lpszInData)->ulPropertyId;
				   ((STB_PROPERTY *)lpszOutData)->ulResult=(DWORD)STB_INTERFACE_NOT_SUPPORTED;
            }
				return (STB_ESCAPE_NOT_SUPPORTED);
			}
		}
      break;


	case STB_SETPROPERTY:
		{
			STB_INTERFACESUPPORTED InterfaceSupported;

			InterfaceSupported.Guid=((STB_PROPERTY *)lpszInData)->Guid;
		
			EdgeEscape(STB_QUERYINTERFACE,&InterfaceSupported, &InterfaceSupported);
	
		/* check to see if interace supported, and only precede if it does */
		/* if interface is supported, pInterfaceSupported->ulResult will   */
		/* contain an identifier for the interface that can be switched on */

			if	(0!=(InterfaceSupported.ulResult))
			{
				switch (InterfaceSupported.ulResult)
				{
#ifdef STB_DFP_ENABLED
					case (STB_DFP_API):
					{
						return (DFPSetProperty(lpszInData,lpszOutData));
					}
#endif /* STB_DFP_ENABLED */
					case (STB_FUNCTION_API):
					{
						return (FunctionAPISetProperty(lpszInData,lpszOutData));
					}
#if 0
					case (STB_MONITOR_API):
					{
						return (MonitorAPISetProperty(lpszInData,lpszOutData));
					}
#endif
					default :
					{
						return (STB_ESCAPE_NOT_SUPPORTED);
					}
				}
			} 
			else
			{
            if (0UL != (DWORD)lpszOutData)
            {
				   ((STB_PROPERTY *)lpszOutData)->Guid =  ((STB_PROPERTY *)lpszInData)->Guid;
				   ((STB_PROPERTY *)lpszOutData)->ulPropertyId =  ((STB_PROPERTY *)lpszInData)->ulPropertyId;
				   ((STB_PROPERTY *)lpszOutData)->ulResult=(DWORD)STB_INTERFACE_NOT_SUPPORTED;
            }
				return (STB_ESCAPE_NOT_SUPPORTED);
			}
		}
      break;

	case STB_GETGROUPPROPERTY:
		{

			STB_INTERFACESUPPORTED InterfaceSupported;

			InterfaceSupported.Guid=((STB_GROUPPROPERTY *)lpszInData)->Guid;
		
			EdgeEscape(STB_QUERYINTERFACE,&InterfaceSupported, &InterfaceSupported);
	
		/* check to see if interace supported, and only precede if it does */
		/* if interface is supported, pInterfaceSupported->ulResult will   */
		/* contain an identifier for the interface that can be switched on */
	
			if	(0!=(InterfaceSupported.ulResult))
			{
				switch (InterfaceSupported.ulResult)
				{
#ifdef STB_DFP_ENABLED
					case (STB_DFP_API):
					{
						return (DFPGetGroupProperty(lpszInData,lpszOutData));
					}
#endif /* STB_DFP_ENABLED */
					case (STB_FUNCTION_API):
					{
						return (FunctionAPIGetGroupProperty(lpszInData,lpszOutData));
					}
#ifdef STB_COLOUR_ENABLED
					case (STB_COLOUR_API):
					{
						return(ColourAPIGetGroupProperty(lpszInData,lpszOutData));
					}
#endif
					default :
					{
						return (STB_ESCAPE_NOT_SUPPORTED);
					}
				}
			} 
			else
			{
            if (0UL != (DWORD)lpszOutData)
            {
				   ((STB_GROUPPROPERTY *)lpszOutData)->Guid =  ((STB_GROUPPROPERTY *)lpszInData)->Guid;
				   ((STB_GROUPPROPERTY *)lpszOutData)->ulPropertyId =  ((STB_GROUPPROPERTY *)lpszInData)->ulPropertyId;
				   ((STB_GROUPPROPERTY *)lpszOutData)->ulResult=(DWORD)STB_INTERFACE_NOT_SUPPORTED;
            }
				return (STB_ESCAPE_NOT_SUPPORTED);
			}
		}
      break;


	case STB_SETGROUPPROPERTY:
		{
			STB_INTERFACESUPPORTED InterfaceSupported;

			InterfaceSupported.Guid=((STB_GROUPPROPERTY *)lpszInData)->Guid;
		
			EdgeEscape(STB_QUERYINTERFACE,&InterfaceSupported, &InterfaceSupported);
	
		/* check to see if interace supported, and only precede if it does */
		/* if interface is supported, pInterfaceSupported->ulResult will   */
		/* contain an identifier for the interface that can be switched on */

			if	(0!=(InterfaceSupported.ulResult))
			{
				switch (InterfaceSupported.ulResult)
				{
#ifdef STB_DFP_ENABLED
					case (STB_DFP_API):
					{
						return (DFPSetGroupProperty(lpszInData,lpszOutData));
					}
#endif /* STB_DFP_ENABLED */
					case (STB_FUNCTION_API):
					{
						return (FunctionAPISetGroupProperty(lpszInData,lpszOutData));
					}
#ifdef STB_COLOUR_ENABLED
					case (STB_COLOUR_API):
					{
					return (ColourAPISetGroupProperty(lpszInData,lpszOutData));
					}
#endif
					default :
					{
						return (STB_ESCAPE_NOT_SUPPORTED);
					}
				}
			} 
			else
			{
            if (0UL != (DWORD)lpszOutData)
            {
				   ((STB_GROUPPROPERTY *)lpszOutData)->Guid =  ((STB_GROUPPROPERTY *)lpszInData)->Guid;
				   ((STB_GROUPPROPERTY *)lpszOutData)->ulPropertyId =  ((STB_GROUPPROPERTY *)lpszInData)->ulPropertyId;
				   ((STB_GROUPPROPERTY *)lpszOutData)->ulResult=(DWORD)STB_INTERFACE_NOT_SUPPORTED;
            }
				return (STB_ESCAPE_NOT_SUPPORTED);
			}
		}
      break;



	default:
		{
			return (STB_ESCAPE_NOT_SUPPORTED); /* escape is not handled */
		}
      break;
	}

}

int EdgeInit()
{
#ifdef STB_DFP_ENABLED
   {
      STB_GROUPPROPERTY GroupProperty;

      GroupProperty.ulPropertyId = STB_DFP_DFPCONNECTED;

      DFPGetProperty(&GroupProperty, &GroupProperty);
   }
#endif

#ifdef STB_TV_ENABLED
#endif

#ifdef STB_MONITOR_ENABLED
#endif


#ifdef STB_COLOUR_ENABLED
	ColourAPIInit();
#endif

#ifdef STB_FUNCTION_ENABLED
	FunctionAPIInit();

#endif
	return ((int)STB_ESCAPE_HANDLED);
	
}


