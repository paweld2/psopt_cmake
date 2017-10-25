/*********************************************************************************************

This file is part of the PSOPT library, a software tool for computational optimal control

Copyright (C) 2009-2015 Victor M. Becerra

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA,
or visit http://www.gnu.org/licenses/

Author:    Professor Victor M. Becerra
           University of Reading
           School of Systems Engineering
           P.O. Box 225, Reading RG6 6AY
           United Kingdom
           e-mail: vmbecerra99@gmail.com

**********************************************************************************************/


#include "psopt.h"



void get_individual_control_trajectory(adouble *control_traj, int control_index, int iphase, adouble* xad, Workspace* workspace)
{
    int k;
    int i = iphase-1;
    Prob& problem = *workspace->problem;
    int ncontrols = problem.phase[i].ncontrols;
    int norder    = problem.phase[i].current_number_of_intervals;
    int iphase_offset = get_iphase_offset(problem,iphase, workspace);
    DMatrix& control_scaling = problem.phase[i].scale.controls;

    for(k=1;k<=norder+1;k++) {
	  control_traj[k-1] = xad[iphase_offset+(k-1)*ncontrols+control_index-1]/control_scaling(control_index);
    }

}

void get_individual_state_trajectory(adouble *state_traj, int state_index, int iphase, adouble* xad, Workspace* workspace)
{
    int k;
    int i = iphase-1;
    Prob& problem = *workspace->problem;
    int ncontrols = problem.phase[i].ncontrols;
    int nstates   = problem.phase[i].nstates;
    int norder    = problem.phase[i].current_number_of_intervals;
    int iphase_offset = get_iphase_offset(problem,iphase, workspace);
    int offset1   = ncontrols*(norder+1);
    DMatrix& state_scaling = problem.phase[i].scale.states;

    for(k=1;k<=norder+1;k++) {
	  state_traj[k-1] = xad[iphase_offset+offset1+(k-1)*nstates+state_index-1]/state_scaling(state_index);
    }

}




void compute_derivatives_trajectory( DMatrix& Xdot, Prob& problem, Sol& solution,  int iphase, Workspace* workspace )
{

	adouble *derivatives;
    	adouble *controls;
    	adouble *parameters;
    	adouble *states;
        adouble *path;
	adouble time;

        int iph;

        int  i = iphase-1;

	if ( problem.multi_segment_flag || workspace->auto_linked_flag ) {
	  iph = 1;
	}
	else {
	  iph = iphase;
	}

	states        = workspace->states[i];
	derivatives   = workspace->derivatives[i];
        controls      = workspace->controls[i];
        parameters    = workspace->parameters[iph-1];
        path          = workspace->path[i];

	int j, k, l;

	int norder    = problem.phase[i].current_number_of_intervals;

	int nstates   = problem.phase[i].nstates;

        int ncontrols = problem.phase[i].ncontrols;

        int nparam    = problem.phase[i].nparameters;


        for(l=1;l<=nparam;  l++) parameters[l-1]  = (solution.parameters[i])(l);

	for(k=1; k<=norder+1; k++)
        {

            for(l=1;l<=ncontrols;l++) controls[l-1] = (solution.controls[i])(l,k);

            for(l=1;l<=nstates;  l++) states[l-1]   = (solution.states[i])(l,k);


            time = (solution.nodes[i])(k);

             problem.dae(derivatives, path,  states, controls, parameters, time, solution.xad, iphase,workspace);


            for(j=1; j<= nstates; j++) {
                 Xdot(j,k) = derivatives[j-1].value();
            }

        }

}



