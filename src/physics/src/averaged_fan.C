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
#include "grins/averaged_fan.h"

// GRINS
#include "grins/generic_ic_handler.h"
#include "grins/constant_viscosity.h"
#include "grins/parsed_viscosity.h"
#include "grins/spalart_allmaras_viscosity.h"
#include "grins/inc_nav_stokes_macro.h"

// libMesh
#include "libmesh/quadrature.h"
#include "libmesh/boundary_info.h"
#include "libmesh/parsed_function.h"
#include "libmesh/zero_function.h"

namespace GRINS
{

  template<class Mu>
  AveragedFan<Mu>::AveragedFan( const std::string& physics_name, const GetPot& input )
    : IncompressibleNavierStokesBase<Mu>(physics_name, input)
  {
    this->read_input_options(input);

    return;
  }

  template<class Mu>
  AveragedFan<Mu>::~AveragedFan()
  {
    return;
  }

  template<class Mu>
  void AveragedFan<Mu>::read_input_options( const GetPot& input )
  {
    std::string base_function =
      input("Physics/"+averaged_fan+"/base_velocity",
        std::string("0"));

    if (base_function == "0")
      libmesh_error_msg("Error! Zero AveragedFan specified!" <<
                        std::endl);

    if (base_function == "0")
      this->base_velocity_function.reset
        (new libMesh::ZeroFunction<libMesh::Number>());
    else
      this->base_velocity_function.reset
        (new libMesh::ParsedFunction<libMesh::Number>(base_function));

    std::string vertical_function =
      input("Physics/"+averaged_fan+"/local_vertical",
        std::string("0"));

    if (vertical_function == "0")
      libmesh_error_msg("Warning! Zero LocalVertical specified!" <<
                        std::endl);

    this->local_vertical_function.reset
      (new libMesh::ParsedFunction<libMesh::Number>(vertical_function));

    std::string lift_function_string =
      input("Physics/"+averaged_fan+"/lift",
        std::string("0"));

    if (lift_function_string == "0")
      std::cout << "Warning! Zero lift function specified!" << std::endl;

    this->lift_function.reset
      (new libMesh::ParsedFunction<libMesh::Number>(lift_function_string));

    std::string drag_function_string =
      input("Physics/"+averaged_fan+"/drag",
        std::string("0"));

    if (drag_function_string == "0")
      std::cout << "Warning! Zero drag function specified!" << std::endl;

    this->drag_function.reset
      (new libMesh::ParsedFunction<libMesh::Number>(drag_function_string));

    std::string chord_function_string =
      input("Physics/"+averaged_fan+"/chord_length",
        std::string("0"));

    if (chord_function_string == "0")
      libmesh_error_msg("Warning! Zero chord function specified!" <<
                        std::endl);

    this->chord_function.reset
      (new libMesh::ParsedFunction<libMesh::Number>(chord_function_string));

    std::string area_function_string =
      input("Physics/"+averaged_fan+"/area_swept",
        std::string("0"));

    if (area_function_string == "0")
      libmesh_error_msg("Warning! Zero area_swept_function specified!" <<
                        std::endl);

    this->area_swept_function.reset
      (new libMesh::ParsedFunction<libMesh::Number>(area_function_string));

    std::string aoa_function_string =
      input("Physics/"+averaged_fan+"/angle_of_attack",
        std::string("00000"));

    if (aoa_function_string == "00000")
      libmesh_error_msg("Warning! No angle-of-attack specified!" <<
                        std::endl);

    this->aoa_function.reset
      (new libMesh::ParsedFunction<libMesh::Number>(aoa_function_string));
  }

  template<class Mu>
  void AveragedFan<Mu>::element_time_derivative( bool compute_jacobian,
					        AssemblyContext& context,
					        CachedValues& /* cache */ )
  {
#ifdef GRINS_USE_GRVY_TIMERS
    this->_timer->BeginTimer("AveragedFan::element_time_derivative");
#endif

    // Element Jacobian * quadrature weights for interior integration
    const std::vector<libMesh::Real> &JxW = 
      context.get_element_fe(this->_flow_vars.u_var())->get_JxW();

    // The shape functions at interior quadrature points.
    const std::vector<std::vector<libMesh::Real> >& u_phi = 
      context.get_element_fe(this->_flow_vars.u_var())->get_phi();

    const std::vector<libMesh::Point>& u_qpoint = 
      context.get_element_fe(this->_flow_vars.u_var())->get_xyz();

    // The number of local degrees of freedom in each variable
    const unsigned int n_u_dofs = context.get_dof_indices(this->_flow_vars.u_var()).size();

    // The subvectors and submatrices we need to fill:
    libMesh::DenseSubMatrix<libMesh::Number> &Kuu = context.get_elem_jacobian(this->_flow_vars.u_var(), this->_flow_vars.u_var()); // R_{u},{u}
    libMesh::DenseSubMatrix<libMesh::Number> &Kuv = context.get_elem_jacobian(this->_flow_vars.u_var(), this->_flow_vars.v_var()); // R_{u},{v}
    libMesh::DenseSubMatrix<libMesh::Number> &Kvu = context.get_elem_jacobian(this->_flow_vars.v_var(), this->_flow_vars.u_var()); // R_{v},{u}
    libMesh::DenseSubMatrix<libMesh::Number> &Kvv = context.get_elem_jacobian(this->_flow_vars.v_var(), this->_flow_vars.v_var()); // R_{v},{v}

    libMesh::DenseSubMatrix<libMesh::Number>* Kwu = NULL;
    libMesh::DenseSubMatrix<libMesh::Number>* Kwv = NULL;
    libMesh::DenseSubMatrix<libMesh::Number>* Kww = NULL;
    libMesh::DenseSubMatrix<libMesh::Number>* Kuw = NULL;
    libMesh::DenseSubMatrix<libMesh::Number>* Kvw = NULL;

    libMesh::DenseSubVector<libMesh::Number> &Fu = context.get_elem_residual(this->_flow_vars.u_var()); // R_{u}
    libMesh::DenseSubVector<libMesh::Number> &Fv = context.get_elem_residual(this->_flow_vars.v_var()); // R_{v}
    libMesh::DenseSubVector<libMesh::Number>* Fw = NULL;

    if( this->_dim == 3 )
      {
        Kuw = &context.get_elem_jacobian(this->_flow_vars.u_var(), this->_flow_vars.w_var()); // R_{u},{w}
        Kvw = &context.get_elem_jacobian(this->_flow_vars.v_var(), this->_flow_vars.w_var()); // R_{v},{w}

        Kwu = &context.get_elem_jacobian(this->_flow_vars.w_var(), this->_flow_vars.u_var()); // R_{w},{u}
        Kwv = &context.get_elem_jacobian(this->_flow_vars.w_var(), this->_flow_vars.v_var()); // R_{w},{v}
        Kww = &context.get_elem_jacobian(this->_flow_vars.w_var(), this->_flow_vars.w_var()); // R_{w},{w}
        Fw  = &context.get_elem_residual(this->_flow_vars.w_var()); // R_{w}
      }

    unsigned int n_qpoints = context.get_element_qrule().n_points();

    for (unsigned int qp=0; qp != n_qpoints; qp++)
      {
        // Compute the solution & its gradient at the old Newton iterate.
        libMesh::Number u, v;
        u = context.interior_value(this->_flow_vars.u_var(), qp);
        v = context.interior_value(this->_flow_vars.v_var(), qp);

        libMesh::NumberVectorValue U(u,v);
        if (this->_dim == 3)
          U(2) = context.interior_value(this->_flow_vars.w_var(), qp); // w

        // Find base velocity of moving fan at this point
        libmesh_assert(base_velocity_function.get());

        libMesh::DenseVector<libMesh::Number> output_vec(3);

        (*base_velocity_function)(u_qpoint[qp], context.time,
                                  output_vec);

        const libMesh::NumberVectorValue U_B(output_vec(0),
                                             output_vec(1),
                                             output_vec(2));

        const libMesh::Number U_B_size = U_B.size();

        // Normal in fan velocity direction
        const libMesh::NumberVectorValue N_B = U_B_size ?
                libMesh::NumberVectorValue(U_B/U_B.size()) : U_B;

        (*local_vertical_function)(u_qpoint[qp], context.time,
                                   output_vec);

        // Normal in fan vertical direction
        const libMesh::NumberVectorValue N_V(output_vec(0),
                                             output_vec(1),
                                             output_vec(2));

        // Normal in radial direction (or opposite radial direction,
        // for fans turning clockwise!)
        const libMesh::NumberVectorValue N_R = N_B.cross(N_V);

        // Fan-wing-plane component of local relative velocity
        const libMesh::NumberVectorValue U_P = U - (U*N_R)*N_R - U_B;

        const libMesh::Number U_P_size = U_P.size();

        // Direction opposing drag
        const libMesh::NumberVectorValue N_drag = U_P_size ?
                libMesh::NumberVectorValue(-U_P/U_P_size) : U_P;

        // Direction opposing lift
        const libMesh::NumberVectorValue N_lift = N_drag.cross(N_R);

        // "Forward" velocity
        const libMesh::Number u_fwd = -(U_P * N_B);

        // "Upward" velocity
        const libMesh::Number u_up = U_P * N_V;

        // Angle WRT fan velocity direction
        const libMesh::Number part_angle = (u_up || u_fwd) ?
                std::atan2(u_up, u_fwd) : 0;

        // Angle WRT fan chord
        const libMesh::Number angle = part_angle +
          (*aoa_function)(u_qpoint[qp], context.time);

        const libMesh::Number C_lift  = (*lift_function)(u_qpoint[qp], angle);
        const libMesh::Number C_drag  = (*drag_function)(u_qpoint[qp], angle);

        const libMesh::Number chord = (*chord_function)(u_qpoint[qp], context.time);
        const libMesh::Number area  = (*area_swept_function)(u_qpoint[qp], context.time);

        const libMesh::Number v_sq = U_P*U_P;

        const libMesh::Number LDfactor = 0.5 * this->_rho * v_sq * chord / area;
        const libMesh::Number lift = C_lift * LDfactor;
        const libMesh::Number drag = C_drag * LDfactor;

        // Force 
        const libMesh::NumberVectorValue F = lift * N_lift + drag * N_drag;

        for (unsigned int i=0; i != n_u_dofs; i++)
          {
            Fu(i) += F(0)*u_phi[i][qp]*JxW[qp];
            Fv(i) += F(1)*u_phi[i][qp]*JxW[qp];

            if (this->_dim == 3)
              (*Fw)(i) += F(2)*u_phi[i][qp]*JxW[qp];

            if (compute_jacobian)
              {
                // FIXME: Jacobians here are very inexact!
                // Dropping all AoA dependence on U terms!
                for (unsigned int j=0; j != n_u_dofs; j++)
                  {
                    const libMesh::Number UPNR = U_P*N_R;
                    const libMesh::Number
                      dV2_du = 2 * u_phi[j][qp] *
                               (U_P(0) - N_R(0)*UPNR);
                    const libMesh::Number
                      dV2_dv = 2 * u_phi[j][qp] *
                               (U_P(1) - N_R(1)*UPNR);

                    const libMesh::NumberVectorValue
                      LDderivfactor = 
                        (N_lift*C_lift+N_drag*C_drag) *
                        0.5 * this->_rho * chord / area;

                    Kuu(i,j) += LDderivfactor(0) * dV2_du *
                                u_phi[i][qp]*JxW[qp] * context.get_elem_solution_derivative();

                    Kuv(i,j) += LDderivfactor(0) * dV2_dv *
                                u_phi[i][qp]*JxW[qp] * context.get_elem_solution_derivative();

                    Kvu(i,j) += LDderivfactor(1) * dV2_du *
                                u_phi[i][qp]*JxW[qp] * context.get_elem_solution_derivative();

                    Kvv(i,j) += LDderivfactor(1) * dV2_dv *
                                u_phi[i][qp]*JxW[qp] * context.get_elem_solution_derivative();

                    if (this->_dim == 3)
                      {
                        const libMesh::Number
                          dV2_dw = 2 * u_phi[j][qp] *
                                   (U_P(2) - N_R(2)*UPNR);

                        (*Kuw)(i,j) += LDderivfactor(0) * dV2_dw *
                                       u_phi[i][qp]*JxW[qp] * context.get_elem_solution_derivative();

                        (*Kvw)(i,j) += LDderivfactor(1) * dV2_dw *
                                       u_phi[i][qp]*JxW[qp] * context.get_elem_solution_derivative();

                        (*Kwu)(i,j) += LDderivfactor(2) * dV2_du *
                                       u_phi[i][qp]*JxW[qp] * context.get_elem_solution_derivative();

                        (*Kwv)(i,j) += LDderivfactor(2) * dV2_dv *
                                       u_phi[i][qp]*JxW[qp] * context.get_elem_solution_derivative();

                        (*Kww)(i,j) += LDderivfactor(2) * dV2_dw *
                                       u_phi[i][qp]*JxW[qp] * context.get_elem_solution_derivative();
                      }

                  } // End j dof loop
              } // End compute_jacobian check

          } // End i dof loop

      }

#ifdef GRINS_USE_GRVY_TIMERS
    this->_timer->EndTimer("AveragedFan::element_time_derivative");
#endif

    return;
  }

} // namespace GRINS

// Instantiate
INSTANTIATE_INC_NS_SUBCLASS(AveragedFan);
