//  Copyright (c) 2007-2011 Hartmut Kaiser
//  Copyright (c) 2009-2011 Matt Anderson
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <cmath>
#include <cstdio>

#include <boost/scoped_array.hpp>

#include <hpx/hpx.hpp>

#include "../stencil/stencil_data.hpp"
#include "../stencil/stencil_functions.hpp"
#include <examples/adaptive1d/parameter.hpp>

namespace hpx { namespace components { namespace adaptive1d 
{

///////////////////////////////////////////////////////////////////////////
int generate_initial_data(stencil_data* val, int item, int maxitems, int row,
    detail::parameter const& par)
{
    // provide initial data for the given data value 
    val->max_index_ = maxitems;
    val->index_ = item;
    val->timestep_ = 0;

    val->value_.resize(par.grain_size);
    for (std::size_t i=0;i<par.grain_size;i++) {
      for (std::size_t j=0;j<NUM_EQUATIONS;j++) {
        val->value_[i].phi[0][j] = 0.0;
      }
    }

    return 1;
}

int rkupdate(std::vector<access_memory_block<stencil_data> > const&val, 
             stencil_data* result, 
             std::vector<int> &src, std::vector<int> &vsrc,double dt,double dx,double t,
             int nx0, int ny0, int nz0,
             double minx0, double miny0, double minz0,
             detail::parameter const& par)
{
    return 1;
}

}}}

