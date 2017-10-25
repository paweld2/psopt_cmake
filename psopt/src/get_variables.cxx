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


void get_controls(adouble* controls, adouble* xad, int iphase, int k, Workspace* workspace)
{
        int i = iphase-1;
        Prob& problem = *workspace->problem;
	DMatrix& control_scaling = problem.phase[i].scale.controls;

	int j;

        int iphase_offset= get_iphase_offset(problem,iphase, workspace);

        // get controls

       int ncontrols = problem.phase[i].ncontrols;



        for(j=0;j<ncontrols;j++) {
           controls[j] =  xad[iphase_offset+(k-1)*ncontrols+j]/control_scaling(j+1);
        }

}

void get_controls_bar(adouble* controls_bar, adouble* xad, int iphase, int k, Workspace* workspace)
{
        int i = iphase-1;
        Prob& problem = *workspace->problem;
	DMatrix& control_scaling = problem.phase[i].scale.controls;

	int j;

        int iphase_offset= get_iphase_offset(problem,iphase, workspace);

	int norder    = problem.phase[i].current_number_of_intervals;
	int ncontrols = problem.phase[i].ncontrols;
	int nstates   = problem.phase[i].nstates;
        int nparam    = problem.phase[i].nparameters;

        int offset = (nstates+ncontrols)*(norder+1)+nparam;

        for(j=0;j<ncontrols;j++) {
           controls_bar[j] =  xad[iphase_offset+offset+(k-1)*ncontrols+j]/control_scaling(j+1);
        }
}


void get_final_controls(adouble* controls, adouble* xad, int iphase, Workspace* workspace)
{
        int i = iphase-1;
        Prob& problem = *workspace->problem;
        int k = problem.phase[i].current_number_of_intervals+1;
        get_controls(controls, xad, iphase, k, workspace);
}

void get_initial_controls(adouble* controls, adouble* xad, int iphase, Workspace* workspace)
{
        get_controls(controls, xad, iphase, 1, workspace);
}

void get_states(adouble* states, adouble* xad, int iphase, int k, Workspace* workspace)
{
        int i = iphase-1;
        Prob& problem            = *workspace->problem;
	DMatrix& state_scaling   = problem.phase[i].scale.states;


	int j;

        int iphase_offset= get_iphase_offset(problem, iphase, workspace);


        int nstates = problem.phase[i].nstates;
        int ncontrols=problem.phase[i].ncontrols;
        int norder   =problem.phase[i].current_number_of_intervals;
	int offset1   = ncontrols*(norder+1);
        // get states
        for(j=0;j<nstates;j++) {
           states[j] =  xad[iphase_offset+offset1+(k-1)*nstates+j]/state_scaling(j+1);
        }

}

void get_final_states(adouble* states, adouble* xad, int iphase, Workspace* workspace)
{
        Prob& problem = *workspace->problem;
        int k = problem.phase[iphase-1].current_number_of_intervals + 1;
        get_states(states, xad, iphase, k, workspace);
}

void get_initial_states(adouble* states, adouble* xad, int iphase, Workspace* workspace)
{
        get_states(states, xad, iphase, 1, workspace);
}



void get_parameters(adouble* parameters, adouble* xad, int iphase, Workspace* workspace)
{
        Prob& problem = *workspace->problem;

	int iph;

	if ( problem.multi_segment_flag || workspace->auto_linked_flag ) {
	  iph = 1;
	}
	else {
	  iph = iphase;
	}


        int i = iph-1;
        DMatrix& param_scaling   = problem.phase[i].scale.parameters;


	int j;

	int norder    = problem.phase[i].current_number_of_intervals;
	int ncontrols = problem.phase[i].ncontrols;
	int nstates   = problem.phase[i].nstates;
        int nparam    = problem.phase[i].nparameters;
        int offset2   = (ncontrols+nstates)*(norder+1);



        int iphase_offset = get_iphase_offset(problem,iph, workspace);

        // get parameters
        for(j=0;j<nparam;j++) {
           parameters[j] =  xad[iphase_offset+offset2+j]/param_scaling(j+1);
        }

}

void get_times(adouble *t0, adouble *tf, adouble* xad, int iphase, Workspace* workspace)
{
        int i = iphase-1;
        Prob& problem = *workspace->problem;
        double   time_scaling    =  problem.phase[i].scale.time;

	int norder    = problem.phase[i].current_number_of_intervals;
	int ncontrols = problem.phase[i].ncontrols;
	int nstates   = problem.phase[i].nstates;
        int nparam    = problem.phase[i].nparameters;

	int nvars_phase_i = get_nvars_phase_i(problem,i, workspace);

        int iphase_offset = get_iphase_offset(problem, iphase, workspace);

	*t0  = xad[iphase_offset + nvars_phase_i-2]/time_scaling;
	*tf  = xad[iphase_offset + nvars_phase_i-1]/time_scaling;

}

adouble get_initial_time(adouble* xad, int iphase, Workspace* workspace)
{
        int i = iphase-1;
        Prob& problem = *workspace->problem;
        double   time_scaling    =  problem.phase[i].scale.time;
        adouble t0;

	int norder    = problem.phase[i].current_number_of_intervals;
	int ncontrols = problem.phase[i].ncontrols;
	int nstates   = problem.phase[i].nstates;
        int nparam    = problem.phase[i].nparameters;


	int nvars_phase_i = get_nvars_phase_i(problem,i, workspace);

        int iphase_offset = get_iphase_offset(problem,iphase, workspace);

	t0  = xad[iphase_offset + nvars_phase_i-2]/time_scaling;

        return (t0);
}

adouble get_final_time(adouble* xad, int iphase, Workspace* workspace)
{
        int i = iphase-1;
        Prob& problem = *workspace->problem;
        double   time_scaling    =  problem.phase[i].scale.time;
        adouble tf;

	int norder    = problem.phase[i].current_number_of_intervals;
	int ncontrols = problem.phase[i].ncontrols;
	int nstates   = problem.phase[i].nstates;
        int nparam    = problem.phase[i].nparameters;


	int nvars_phase_i = get_nvars_phase_i(problem,i, workspace);

        int iphase_offset = get_iphase_offset(problem,iphase, workspace);

	tf  = xad[iphase_offset + nvars_phase_i-1]/time_scaling;

        return (tf);
}


