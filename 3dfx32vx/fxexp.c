/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** $Log: 
**  1    3dfx      1.0         9/11/99  StarTeam VTS Administrator 
** $
** 
** 5     6/03/99 11:13p Peterm
** changed to enable dd32 to build with h3 tot deltas
** 
** 5     5/13/99 1:57a Mconrad
** Branch Prior to Retirement in users folder.
 * 
 * 3     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/
#define EXP_THRESHOLD 1.0e-20
#define LN_2          0.6931471805599

double two_to_x(double x);
double fxExp(double x);

double two_to_x(double x)
{
  return(fxExp(x*LN_2));
} 

double fxExp(double x)
{
  double ratio=x, result=1.0,n=1.0,test;
  
  // some really small number close to zero
  if (x < -30)
    return(0.000000000001);
  
  do
    {
      result+=ratio;
      n+=1.0;
      ratio=ratio*x/n;
      test=ratio/result;
      if(test<0.0)
        test=-test;
    } while(test>EXP_THRESHOLD);
  return(result);
}


