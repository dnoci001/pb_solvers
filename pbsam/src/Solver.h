//
//  Solver.h
//  pbsam_xcode
//
//  Created by David Brookes on 5/17/16.
//  Copyright © 2016 David Brookes. All rights reserved.
//

#ifndef Solver_h
#define Solver_h

#include <stdio.h>
#include <iostream>
#include <memory>
#include "ReExpCalc.h"


// For calculating the n grid points on surface
int calc_n_grid_pts(int poles, double r);

// use Rakhmanov method
vector<Pt> make_uniform_sph_grid(int m_grid, double r);

/*
 Base class for coefficients of expansion
 */
class ExpansionConstants
{
protected:
  MyVector<double> expansionConst1_; // (2*l+1)/(4*pi)
  MyVector<double> expansionConst2_; // 1 / (2*l+1)
  vector<vector<int> > imatLoc_; // Locator for imat coeffs
  int p_;
  
  void compute_coeffs();
  
public:
  ExpansionConstants(int p);

  void set_const1_l(int l, double val) { expansionConst1_.set_val(l, val); }
  void set_const2_l(int l, double val) { expansionConst2_.set_val(l, val); }
  double get_const1_l(int l)        { return expansionConst1_[l]; }
  double get_const2_l(int l)        { return expansionConst2_[l]; }
  vector<int> get_imat_loc(int m)   { return imatLoc_[m]; }
  const int get_p() const           { return p_; }
};




/*
 Base class for matrices that will be re-expanded
 */
class ComplexMoleculeMatrix
{
protected:
  vector<MyMatrix<cmplx> > mat_;
  int p_;
  int I_;
    
public:
  ComplexMoleculeMatrix(int I, int ns, int p);
  
  cmplx get_mat_knm(int k, int n, int m) { return mat_[k](n, m+p_); }
  void set_mat_knm(int k, int n, int m, cmplx val)
  { mat_[k].set_val(n, m+p_, val); }
  MyMatrix<cmplx> get_mat_k(int k)       { return mat_[k]; }
  const int get_I() const   { return I_; }
  const int get_p() const   { return p_; }
  const int get_ns() const  { return (int) mat_.size(); }
  
  void reset_mat();
  
  friend ostream & operator<<(ostream & fout, ComplexMoleculeMatrix & M)
  {
    for (int k = 0; k < M.get_ns(); k++)
    {
      fout << "For sphere " << k << endl;
      for (int n = 0; n < M.get_p(); n++)
      {
        for (int m = 0; m <= n; m++)
        {
          double real = M.get_mat_knm( k, n, m).real();
          double imag = M.get_mat_knm( k, n, m).imag();
          if(abs(real) < 1e-15 ) real = 0.0;
          if(abs(imag) < 1e-15 ) imag = 0.0;
          fout << "(" << real << ", " << imag << ") ";
        }
        fout << endl;
      }
    }
    return fout;
  }

};

/*
 Pre-computed values representing fixed charges within each sphere. Each object
 of this class refers to one molecule. This is then a vector of matrices where
 the vector index, k, loops through each coarse-grained sphere in the molecule.
 The indices of the inner matrices are then n and m, which loop over the number
 of poles in the system. See eq. 8a in Yap 2010 for more info
 */
class EMatrix: public ComplexMoleculeMatrix
{
public:
  EMatrix(int I, int ns, int p);
  virtual void calc_vals(Molecule mol, shared_ptr<SHCalc> sh_calc,
                         double eps_in);
  
};

/*
 Pre-computed values representing fixed charges outside each sphere. Each object
 of this class refers to one molecule. This is then a vector of matrices where
 the vector index, k, loops through each coarse-grained sphere in the molecule.
 The indices of the inner matrices are then n and m, which loop over the number
 of poles in the system. See eq. 8b in Yap 2010 for more info
 */
class LEMatrix : public ComplexMoleculeMatrix
{
public:
  LEMatrix(int I, int ns, int p);
  void calc_vals(Molecule mol, shared_ptr<SHCalc> sh_calc,
                 double eps_in);
};


/*
 Class for pre-computing values of surface integral matrices I_E. Each object 
 of this class refers to one molecule. See equation 21 in Yap 2010 for more
 info.
 */
class IEMatrix
{
protected:
  
  // indices in order are k, (n, m), (l, s)
  vector<MatOfMats<cmplx>::type > IE_;
  vector<vector<double> > IE_orig_;
  shared_ptr<ExpansionConstants> _expConst_;
  int p_;
  int I_;
  int gridPts_; // grid point count for surface integrals
  
  void set_IE_k_nm_ls(int k, int n, int m, int l, int s, cmplx val)
  {
    IE_[k](n, m+p_).set_val(l, s+p_, val);
  }
  
public:
  IEMatrix(int I, int ns, int p, shared_ptr<ExpansionConstants> _expconst);

  IEMatrix(int I, shared_ptr<Molecule> _mol, shared_ptr<SHCalc> sh_calc, int p,
           shared_ptr<ExpansionConstants> _expconst,
           int npts = Constants::IMAT_GRID );
  
  cmplx get_IE_k_nm_ls(int k, int n, int m, int l, int s)
  {
    return IE_[k](n, m+p_)(l, s+p_);
  }
  double get_IE_k_ind(int k, int ind) { return IE_orig_[k][ind]; }
  
  void compute_grid_pts(shared_ptr<Molecule> _mol);
  vector<MatOfMats<cmplx>::type >compute_integral(shared_ptr<Molecule> _mol,
                                                  shared_ptr<SHCalc> sh_calc,
                                                  int k);
  void populate_mat(vector<MatOfMats<cmplx>::type > Ys, int k);
  void calc_vals(shared_ptr<Molecule> _mol, shared_ptr<SHCalc> sh_calc);
  void reset_mat();
};


/*
 Re-expansion coefficients
 */
class TMatrix
{
protected:
  int p_;
  double kappa_;
  vector<shared_ptr<ReExpCoeffs> > T_;
  // maps (I,k), (J,l) indices to a single index in T. If this returns -1
  // then the spheres are overlapping or less than 5A away and numerical
  // re-expansion is required
  map<vector<int>, int> idxMap_;
  shared_ptr<SHCalc>  _shCalc_;
  shared_ptr<BesselCalc>  _besselCalc_;
  
  int Nmol_;
  vector<int> Nsi_; // number of spheres in each molecule
  
public:
  
  TMatrix() { }
  
  TMatrix(int p, shared_ptr<System> _sys, shared_ptr<SHCalc> _shcalc,
          shared_ptr<Constants> _consts, shared_ptr<BesselCalc> _besselcalc,
          shared_ptr<ReExpCoeffsConstants> _reexpconsts);
  
  // if these spheres can be re-expanded analytically, return true
  bool is_analytic(int I, int k, int J, int l)
  {
    if (idxMap_[{I, k, J, l}] == -1 ) return false;
    else return true;
  }
  
  // get re-expansion of sphere (I, k) with respect to (J, l)
  shared_ptr<ReExpCoeffs> get_T_Ik_Jl(int I, int k, int J, int l)
  {
    return T_[idxMap_[{I, k, J, l}]];
  }
  
  void update_vals(shared_ptr<System> _sys, shared_ptr<SHCalc> _shcalc,
                   shared_ptr<BesselCalc> _besselcalc,
                   shared_ptr<ReExpCoeffsConstants> _reexpconsts);
  
  /*
   Re-expand a matrix X with respect to T(I,k)(J,l)
  */
  MyMatrix<cmplx> re_expand(int I, int k, int J, int l, MyMatrix<cmplx> X);
  
  int get_nmol() const { return Nmol_; }
  
  int get_nsi(int i)   { return Nsi_[i]; }
  
};

class HMatrix;
class FMatrix;

/*
 Equation 8c
 */
class LFMatrix : public ComplexMoleculeMatrix
{
public:
  LFMatrix(int I, int ns, int p);
  
  void calc_vals(shared_ptr<TMatrix> T, shared_ptr<FMatrix> F,
                 shared_ptr<SHCalc> shcalc, shared_ptr<System> sys);
  
  // analytic re-expansion (Equation 27a)
  MyMatrix<cmplx> analytic_reex(int I, int k, int j,
                                shared_ptr<FMatrix> F,
                                shared_ptr<SHCalc> shcalc,
                                shared_ptr<System> sys, int Mp=-1);
  
  /*
   Equation 15a. For analytic re expansion
   */
  cmplx make_fb_Ij(int I, int j, Pt rb,
                   shared_ptr<FMatrix> F,
                   shared_ptr<SHCalc> shcalc);
};

/*
 Equation 10b
 */
class LHMatrix : public ComplexMoleculeMatrix
{
protected:
  double kappa_;
  
public:
  LHMatrix(int I, int ns, int p, double kappa);
  
  void calc_vals(shared_ptr<TMatrix> T, shared_ptr<HMatrix> H,
                 shared_ptr<SHCalc> shcalc, shared_ptr<System> sys,
                 shared_ptr<BesselCalc> bcalc, int Mp=-1);
  
  // analytic re-expansion (Equation 27b)
  MyMatrix<cmplx> analytic_reex(int I, int k, int j,
                                shared_ptr<HMatrix> H,
                                shared_ptr<SHCalc> shcalc,
                                shared_ptr<System> sys,
                                shared_ptr<BesselCalc> bcalc,
                                int Mp=-1);
  
  /*
   Equation 15b. For analytic re expansion
   */
  cmplx make_hb_Ij(int I, int j, Pt rb,
                   shared_ptr<HMatrix> H,
                   shared_ptr<SHCalc> shcalc,
                   shared_ptr<BesselCalc> bcalc);
  
};

/*
 Equation 10c
 */
class LHNMatrix : public ComplexMoleculeMatrix
{
public:
  LHNMatrix(int I, int ns, int p);
  
  void calc_vals(shared_ptr<TMatrix> T, vector<shared_ptr<HMatrix> > H);
  
};

/*
 Equation 14a
 */
class XHMatrix : public ComplexMoleculeMatrix
{
public:
  XHMatrix(int I, int ns, int p);
  
  void calc_vals(Molecule mol, shared_ptr<BesselCalc> bcalc,
                 shared_ptr<EMatrix> E, shared_ptr<LEMatrix> LE,
                 shared_ptr<LHMatrix> LH, shared_ptr<LFMatrix> LF,
                 shared_ptr<LHNMatrix> LHN);

};

/*
 Equation 14b
 */
class XFMatrix : public ComplexMoleculeMatrix
{
protected:
  double eps_;
  
public:
  XFMatrix(int I, int ns, int p, double eps_in, double eps_out);
  
  void calc_vals(Molecule mol, shared_ptr<BesselCalc> bcalc,
                 shared_ptr<EMatrix> E, shared_ptr<LEMatrix> LE,
                 shared_ptr<LHMatrix> LH, shared_ptr<LFMatrix> LF,
                 shared_ptr<LHNMatrix> LHN, double kappa);
  
  const double get_eps() const { return eps_; }
};


class HMatrix: public ComplexMoleculeMatrix
{
protected:
  double kappa_;
  
public:
  HMatrix(int I, int ns, int p, double kappa);
  
  void calc_vals(Molecule mol, shared_ptr<HMatrix> prev,
                 shared_ptr<XHMatrix> XH,
                 shared_ptr<FMatrix> F,
                 shared_ptr<IEMatrix> IE,
                 shared_ptr<BesselCalc> bcalc);
  
  // calculate convergence criteria (Equation 23)
  static double calc_converge(shared_ptr<HMatrix> curr,
                              shared_ptr<HMatrix> prev);
  
};


class FMatrix: public ComplexMoleculeMatrix
{
protected:
  double kappa_;
  
public:
  FMatrix(int I, int ns, int p, double kappa);
  
  void calc_vals(Molecule mol, shared_ptr<FMatrix> prev,
                 shared_ptr<XFMatrix> XF,
                 shared_ptr<HMatrix> H,
                 shared_ptr<IEMatrix> IE,
                 shared_ptr<BesselCalc> bcalc);
  
};


/*
 Class the uses the above classes to iteratively solve for the F and H matrices
 */
class Solver
{
protected:
  int p_;
  double kappa_;
  
  vector<shared_ptr<EMatrix> >      _E_;
  vector<shared_ptr<LEMatrix> >     _LE_;
  vector<shared_ptr<IEMatrix> >     _IE_;
  
  vector<shared_ptr<LFMatrix> >     _LF_;
  vector<shared_ptr<LHMatrix> >     _LH_;
  vector<shared_ptr<LHNMatrix> >    _LHN_;
  vector<shared_ptr<XFMatrix> >     _XF_;
  vector<shared_ptr<XHMatrix> >     _XH_;
  
  vector<shared_ptr<HMatrix> >      _H_;
  vector<shared_ptr<HMatrix> >      _prevH_;
  
  vector<shared_ptr<FMatrix> >      _F_;
  vector<shared_ptr<FMatrix> >      _prevF_;
  
  shared_ptr<TMatrix>               _T_;
  
  shared_ptr<System>                _sys_;
  shared_ptr<SHCalc>                _shCalc_;
  shared_ptr<BesselCalc>            _bCalc_;
  shared_ptr<Constants>             _consts_;
  shared_ptr<ReExpCoeffsConstants>  _reExConsts_;
  
  // update prevH and prevF
  void update_prev();
  
  // run an iteration and return convergence value
  double iter();

public:
  Solver(shared_ptr<System> _sys, shared_ptr<Constants> _consts,
         shared_ptr<SHCalc> _shCalc, shared_ptr<BesselCalc> _bCalc,
         int p);
  
  
  void solve(double tol, int maxiter=10000);
  
  void reset_all();
  
};

#endif /* Solver_h */



