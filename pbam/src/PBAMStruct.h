#ifndef __PBAMSTRUCT_H
#define __PBAMSTRUCT_H

#define CHR_MAX 8192
#define FIL_MAX 20

//
//  input
//
struct PBAMInput {

  double temp_;
  double salt_;
  double idiel_;
  double sdiel_;
  int nmol_;
  char runType_[CHR_MAX];
  char runName_[CHR_MAX];
  int randOrient_;
  double boxLen_;
  int pbcType_;

  char map3D_[CHR_MAX];
  
  int grid2Dct_;
  char grid2D_[FIL_MAX][CHR_MAX];
  char grid2Dax_[FIL_MAX][CHR_MAX];
  double grid2Dloc_[FIL_MAX];

  char dxname_[CHR_MAX];
  


#ifdef __cplusplus
PBAMInput() :
  temp_(298.15),
  salt_(0.01),
  idiel_(1.5), // Solute dielectric
  sdiel_(80.0),
  nmol_(1),
  runType_("energyforce"),
  runName_("tst"),
  randOrient_(0),
  boxLen_(1.4e18),
  pbcType_(0),
  map3D_("tst.map"),
  grid2Dct_(0)
	{}
#endif

} ;

//
//  output
//
struct PBAMOutput {

  double energies_[500];
  double forces_[500][3];

#ifdef __cplusplus
  PBAMOutput() {}
#endif

} ;

#endif
