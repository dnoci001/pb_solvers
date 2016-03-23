//
//  ThreeBody.cpp
//  pb_solvers_code
//
//  Created by Lisa Felberg on 3/9/16.
//  Copyright © 2016 Lisa Felberg. All rights reserved.
//

#include "ThreeBody.h"

ThreeBody::ThreeBody( shared_ptr<ASolver> _asolver, double cutoff )
: N_(_asolver->get_N()), p_(_asolver->get_p()), cutoffTBD_(cutoff),
_besselCalc_(_asolver->get_bessel()),
_shCalc_(_asolver->get_sh()),
_consts_(_asolver->get_consts()),
_sys_(_asolver->get_sys())
{
  dimer_.reserve(N_*N_);
  trimer_.reserve(N_*N_*N_);
  
  generatePairsTrips();
}

void ThreeBody::generatePairsTrips()
{
  int i, j, k;
  Pt dist1, dist2, dist3;
  vector<int> temp2(2), temp3(3);
  int M2 = N_*(N_-1)/2; int M3 = N_*(N_-1)*(N_-2)/6;
  
  for( i = 0; i < N_; i++)
  {
    for( j = i+1; j < N_; j++)
    {
      dist1 = _sys_->get_pbc_dist_vec( i, j);
      if (cutoffTBD_ > dist1.norm())
      {
        temp2[0] = i; temp2[1] = j;
        dimer_.push_back(temp2);
      }
      for( k = j + 1; k < N_; k++)
      {
        dist2 = _sys_->get_pbc_dist_vec( i, k);
        dist3 = _sys_->get_pbc_dist_vec( j, k);
        if (cutoffTBD_*2.0 > (dist1.norm() + dist2.norm() + dist3.norm() ))
        {
          temp3[0] = i; temp3[1] = j; temp3[2] = k;
          trimer_.push_back(temp3);
        }
      }
    }
  }
  
  // Resizing vectors for saving energy/force vals
  energy_di_.resize( dimer_.size());
  force_di_.resize( dimer_.size());
  
  if ( cutoffTBD_ < 1e37 )
    cout << "Cutoffs implemented, using " << cutoffTBD_ << endl;
  cout << "Max # of di: "  << M2 << " Act used: " << dimer_.size() <<endl;
  cout << "Max # of tri: " << M3 << " Act used: " << trimer_.size() <<endl;
} //end cutoffTBD


shared_ptr<System> ThreeBody::make_subsystem(vector<int> mol_idx)
{
  vector<Molecule> sub_mols (mol_idx.size());
  for (int i = 0; i < mol_idx.size(); i++)
  {
    sub_mols[i] = _sys_->get_molecule(mol_idx[i]);
  }
  
  shared_ptr<System> _subsys = make_shared<System>(sub_mols,_sys_->get_cutoff(),
                                                  _sys_->get_boxlength());
  _subsys -> set_time(_sys_->get_time());
  return _subsys;
}

// Three body approximation computation
void ThreeBody::solveNmer( int num )
{
  int i, j;
  vector< vector<int> > nmer = ( num == 2 ) ? dimer_ : trimer_;
  vector< Molecule > mol_temp;

  shared_ptr<System> _sysTemp = make_subsystem(nmer[0]);
  shared_ptr<ASolver> _asolvTemp = make_shared<ASolver>(_besselCalc_, _shCalc_,
                                                        _sysTemp, _consts_, p_);
  shared_ptr<EnergyCalc> _enCalc = make_shared<EnergyCalc>(_asolvTemp);
  shared_ptr<ForceCalc> _fCalc = make_shared<ForceCalc>(_asolvTemp);
  shared_ptr<TorqueCalc> _tauCalc = make_shared<TorqueCalc>(_asolvTemp);
  
//  
  for( i = 0; i < nmer.size(); i++)
  {
    cout << "This nmer is: ";
    shared_ptr<System> _sysTemp = make_subsystem(nmer[i]);
    
    _asolvTemp->reset_all(_sysTemp);
    
    _asolvTemp->solve_A(1E-5);
    _asolvTemp->solve_gradA(1E-5);
    
    _enCalc->calc_energy();
    _fCalc->calc_force();
    _tauCalc->calc_tau();
    
    cout << "Pot, force for all: ";
    for ( j = 0; j < num; j++)
    {
      cout << _enCalc->get_omega_i_int(j) << "\t" << _fCalc->get_fi(i)[0];
      cout << ", " << _fCalc->get_fi(i)[1]<< ", ";
      cout << _fCalc->get_fi(i)[2]<< "\t";
      if ( num == 2 )
      {
        energy_di_[i][j] = _enCalc->get_omega_i_int(j);
        force_di_[i][j] = _fCalc->get_fi(i);
      } else {
        energy_tri_[i][j] = _enCalc->get_omega_i_int(j);
        force_tri_[i][j] = _fCalc->get_fi(i);
      }
    }
    cout << endl;
  }
  cout << num << "mers done " << endl;
  
}