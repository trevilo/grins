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
#include "grins/spalart_allmaras_viscosity.h"
#include "grins/constant_viscosity.h"
#include "grins/parsed_viscosity.h"

//GRINS
#include "grins/grins_physics_names.h"
#include "grins/turbulent_viscosity_macro.h"

// libMesh
#include "libmesh/getpot.h"
#include "libmesh/parsed_function.h"

namespace GRINS
{
  template<class Mu>
  SpalartAllmarasViscosity<Mu>::SpalartAllmarasViscosity( const GetPot& input ):
    _turbulence_vars(input, spalart_allmaras),
    _mu(input)
  {
    if( !input.have_variable("Materials/Viscosity/mu") )
	{
	  std::cerr<<"No viscosity has been specified."<<std::endl;
	  
	  libmesh_error();
	}
    return;
  }
    

  template<class Mu>
  SpalartAllmarasViscosity<Mu>::~SpalartAllmarasViscosity()
  {
    return;
  }

} // namespace GRINS

INSTANTIATE_TURBULENT_VISCOSITY_SUBCLASS(SpalartAllmarasViscosity);
