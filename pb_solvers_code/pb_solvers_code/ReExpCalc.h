//
//  ReExpCalc.h
//  pb_solvers_code
//
//  Created by David Brookes on 10/5/15.
//  Copyright © 2015 David Brookes. All rights reserved.
//

#ifndef ReExpCalc_h
#define ReExpCalc_h

#include <stdio.h>
#include "MyMatrix.h"
#include "Constants.h"
#include "util.h"

using namespace std;

/*
 Class for pre-computing the constants in the re-expansion coefficne
 */
class ReExpCoeffsConstants
{
protected:
  vector<vector<double> > a_;  // first index is m, second is n
  vector<vector<double> > b_;  // first index is m, second is n
  
  ReExpCoeffsConstants(int p=Constants::MAX_NUM_POLES)
  :a_(p -1), b_(p-1)
  {
    int m, n, inner_size, sign;
    double a_val, b_val;
    vector<double> a_m;
    vector<double> b_m;
    for (m = 0; m < p -1; m++)
    {
      inner_size = (2*p-m-1);
      a_m.reserve(inner_size);
      b_m.reserve(inner_size);
      for (n = 0; n < 2*p-m-1; n++)
      {
        if (n < (m-2))
        {
          a_val = 0.0;
          b_val = 0.0;
        }
        else
        {
          a_val = sqrt(((n+m+1) * (n-m+1)) / ((2*n+1) * (2*n+3)));
          if (m < 0)        sign = -1.0;
          else if (m == 0)  sign = 0.0;
          else              sign = 1.0;
          b_val = sign * sqrt(((n-m-1) * (n-m)) / ((2*n-1) * (2*n+1)));
        }
        a_m.push_back(a_val);
        b_m.push_back(b_val);
      }
      a_.push_back(a_m);
      b_.push_back(b_m);
    }
  }
  
};

/*
 Class containing necessary information for calculating re-expansion
 coefficients (labeled T matrix in Lotan 2006)
 */
class ReExpCoeffs
{
protected:
  vector<MyMatrix<cmplx> >  R_;  // Rotation coefficient matrices (labeled R '
                                // matrix in Lotan 2006)
  vector<MyMatrix<cmplx> >  S_;  // From Lotan 2006 Eq 34
  ReExpCoeffsConstants*     _consts_;
  
  
  
};

#endif /* ReExpCalc_hpp */
