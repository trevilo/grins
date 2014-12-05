//-----------------------------------------------------------------------bl-
//--------------------------------------------------------------------------
// 
// GRINS - General Reacting Incompressible Navier-Stokes 
//
// Copyright (C) 2014 Paul T. Bauman, Roy H. Stogner
// Copyright (C) 2010-2013 The PECOS Development Team
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the Version 2.1 GNU Lesser General
// Public License as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc. 51 Franklin Street, Fifth Floor,
// Boston, MA  02110-1301  USA
//
//-----------------------------------------------------------------------el-


#ifndef GRINS_SPALART_ALLMARAS_H
#define GRINS_SPALART_ALLMARAS_H

//GRINS
#include "grins/physics.h"
#include "grins/turbulence_fe_variables.h"

namespace GRINS
{

  //! Physics class for Incompressible Navier-Stokes
  /*!
    This physics class implements the classical Incompressible Navier-Stokes equations.
    This is a templated class, the class Viscosity can be instantiated as a specific type
    (right now:ConstantViscosity or SpatiallyVaryingViscosity) to allow the user
    to specify a constant or spatially varying viscosity in the input file
   */
  template<class Viscosity>
  class SpalartAllmaras : public TurbulenceModels<Viscosity>
  {
  public:

    SpalartAllmaras(const std::string& physics_name, const GetPot& input);

    ~SpalartAllmaras();
        
    virtual void init_variables( libMesh::FEMSystem* system );

    //! Sets velocity variables to be time-evolving
    virtual void set_time_evolving_vars( libMesh::FEMSystem* system );

    // Context initialization
    virtual void init_context( AssemblyContext& context );    

    // The source function \tilde{S}
    Real _source_fn( libMesh::Number nu, Real wall_distance);

    // The vorticity function
    Real _vorticity( FlowVars, qp);

    // The destruction function f_w(nu)
    Real _destruction_fn(libMesh::Number nu, Real wall_distance, Real _S_tilde);

  protected:

    //! Spalart Allmaras model constants
    libMesh::Number _cb1, _sigma, _cb2, _cw1;

    //! Constants specific to the calculation of the source function
    libMesh::Number _kappa, _cv1, _cv2, _cv3;

    //! Constants specific to the calculation of the destruction function
    libMesh::Number _r_lin, _c_w2, _c_w3;

    //! Physical dimension of problem
    /*! \todo Do we really need to cache this? */
    unsigned int _dim;

    // This should be declared in TurbulentViscosity
    TurbulenceFEVariables _turbulence_vars;
 
    //! Viscosity object
    Viscosity _mu;
    
  private:
    SpalartAllmaras();

  };

} //End namespace block

#endif // GRINS_SPALART_ALLMARAS_H