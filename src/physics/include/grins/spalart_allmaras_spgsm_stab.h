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

#ifndef GRINS_SPALART_ALLMARAS_SPGSM_STAB_H
#define GRINS_SPALART_ALLMARAS_SPGSM_STAB_H

//GRINS
#include "grins/spalart_allmaras_stab_base.h"

namespace GRINS
{
  //! Adds SUPG stabilization to the SpalartAllmaras 'physics'
  template<class Viscosity>
  class SpalartAllmarasSPGSMStabilization : public SpalartAllmarasStabilizationBase<Viscosity>
  {

  public:

    SpalartAllmarasSPGSMStabilization( const GRINS::PhysicsName& physics_name, const GetPot& input );
    virtual ~SpalartAllmarasSPGSMStabilization();

    virtual void element_time_derivative( bool compute_jacobian,
					  AssemblyContext& context,
					  CachedValues& cache );

    virtual void element_constraint( bool compute_jacobian,
                                     AssemblyContext& context,
                                     CachedValues& cache );

    virtual void mass_residual( bool compute_jacobian,
				AssemblyContext& context,
				CachedValues& cache );
    
  private:

    SpalartAllmarasSPGSMStabilization();

  };

} // end namespace GRINS

#endif // GRINS_SPALART_ALLMARAS_SPGSM_STAB_H
