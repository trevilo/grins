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
#include "grins/elastic_membrane_constant_pressure.h"

// GRINS
#include "grins_config.h"
#include "grins/assembly_context.h"

// libMesh
#include "libmesh/getpot.h"
#include "libmesh/quadrature.h"
#include "libmesh/boundary_info.h"
#include "libmesh/fem_system.h"

namespace GRINS
{
  ElasticMembraneConstantPressure::ElasticMembraneConstantPressure( const GRINS::PhysicsName& physics_name, const GetPot& input )
    : ElasticMembraneBase(physics_name,input),
      _pressure( input("Physics/ElasticMembraneConstantPressure/pressure", 0.0) )
  {
    if( !input.have_variable("Physics/ElasticMembraneConstantPressure/pressure") )
      {
        std::cerr << "Error: Must input pressure for ElasticMembraneConstantPressure." << std::endl
                  << "       Please set Physics/ElasticMembraneConstantPressure/pressure." << std::endl;
        libmesh_error();
      }

    return;
  }

  ElasticMembraneConstantPressure::~ElasticMembraneConstantPressure()
  {
    return;
  }

  void ElasticMembraneConstantPressure::element_time_derivative( bool compute_jacobian,
                                                                 AssemblyContext& context,
                                                                 CachedValues& /*cache*/ )
  {
    const unsigned int n_u_dofs = context.get_dof_indices(_disp_vars.u_var()).size();
    
    const std::vector<libMesh::Real> &JxW =
      context.get_element_fe(_disp_vars.u_var())->get_JxW();

    const std::vector<std::vector<libMesh::Real> >& u_phi =
      context.get_element_fe(_disp_vars.u_var())->get_phi();

    libMesh::DenseSubVector<libMesh::Number> &Fu = context.get_elem_residual(_disp_vars.u_var());
    libMesh::DenseSubVector<libMesh::Number> &Fv = context.get_elem_residual(_disp_vars.v_var());
    libMesh::DenseSubVector<libMesh::Number> &Fw = context.get_elem_residual(_disp_vars.w_var());

    unsigned int n_qpoints = context.get_element_qrule().n_points();

    // All shape function gradients are w.r.t. master element coordinates
    const std::vector<std::vector<libMesh::Real> >& dphi_dxi =
      context.get_element_fe(_disp_vars.u_var())->get_dphidxi();

    const std::vector<std::vector<libMesh::Real> >& dphi_deta =
      context.get_element_fe(_disp_vars.u_var())->get_dphideta();

    const libMesh::DenseSubVector<libMesh::Number>& u_coeffs = context.get_elem_solution( _disp_vars.u_var() );
    const libMesh::DenseSubVector<libMesh::Number>& v_coeffs = context.get_elem_solution( _disp_vars.v_var() );
    const libMesh::DenseSubVector<libMesh::Number>& w_coeffs = context.get_elem_solution( _disp_vars.w_var() );

    const std::vector<libMesh::RealGradient>& dxdxi  = context.get_element_fe(_disp_vars.u_var())->get_dxyzdxi();
    const std::vector<libMesh::RealGradient>& dxdeta = context.get_element_fe(_disp_vars.u_var())->get_dxyzdeta();

    for (unsigned int qp=0; qp != n_qpoints; qp++)
      {
        // sqrt(det(a_cov)), a_cov being the covariant metric tensor of undeformed body
        libMesh::Real sqrt_a = sqrt( dxdxi[qp]*dxdxi[qp]*dxdeta[qp]*dxdeta[qp]
                                     - dxdxi[qp]*dxdeta[qp]*dxdeta[qp]*dxdxi[qp] );

        // Gradients are w.r.t. master element coordinates
        libMesh::Gradient grad_u, grad_v, grad_w;
        for( unsigned int d = 0; d < n_u_dofs; d++ )
          {
            libMesh::RealGradient u_gradphi( dphi_dxi[d][qp], dphi_deta[d][qp] );
            grad_u += u_coeffs(d)*u_gradphi;
            grad_v += v_coeffs(d)*u_gradphi;
            grad_w += w_coeffs(d)*u_gradphi;
          }

        libMesh::RealGradient dudxi( grad_u(0), grad_v(0), grad_w(0) );
        libMesh::RealGradient dudeta( grad_u(1), grad_v(1), grad_w(1) );
        
        libMesh::RealGradient A_1 = dxdxi[qp] + dudxi;
        libMesh::RealGradient A_2 = dxdeta[qp] + dudeta;

        libMesh::RealGradient A_3 = A_1.cross(A_2);

        /* The formula here is actually
           P*\sqrt{\frac{A}{a}}*A_3, where A_3 is a unit vector
           But, |A_3| = \sqrt{A} so the normalizing part kills
           the \sqrt{A} in the numerator, so we can leave it out
           and *not* normalize A_3.
         */
        libMesh::RealGradient traction = _pressure/sqrt_a*A_3;

        libMesh::Real jac = JxW[qp];

        for (unsigned int i=0; i != n_u_dofs; i++)
	  {
            Fu(i) += traction(0)*u_phi[i][qp]*jac;
            
            Fv(i) += traction(1)*u_phi[i][qp]*jac;

            Fw(i) += traction(2)*u_phi[i][qp]*jac;

            if( compute_jacobian )
              {
                libmesh_not_implemented();
              }
          }
      }

    return;
  }

  void ElasticMembraneConstantPressure::reset_pressure( libMesh::Real pressure_in )
  {
    _pressure = pressure_in;
    return;
  }

} // end namespace GRINS
