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

// This class
#include "grins/mooney_rivlin.h"

// libMesh
#include "libmesh/getpot.h"

namespace GRINS
{
  MooneyRivlin::MooneyRivlin( const GetPot& input )
    : HyperelasticStrainEnergy<MooneyRivlin>(),
      _C1( input("Physics/MooneyRivlin/C1", -1.0) ),
      _C2( input("Physics/MooneyRivlin/C2", -1.0) )
  {
    //Force the user to specify C1 and C2
    if( !input.have_variable("Physics/MooneyRivlin/C1") ||
        !input.have_variable("Physics/MooneyRivlin/C2")    )
      {
        std::cerr << "Error: Must specify both C1 and C2 for Mooney-Rivlin material." << std::endl
                  << "       They can be specified in Physics/MooneyRivlin/C1 and Physics/MooneyRivlin/C2" << std::endl;
        libmesh_error();
      }

    return;
  }
   
  MooneyRivlin::~MooneyRivlin()
  {
    return;
  }

} // end namespace GRINS
