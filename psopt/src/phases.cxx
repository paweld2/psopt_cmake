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




void auto_link(adouble* linkages, int* index, adouble* xad, int iphase_a, int iphase_b, Workspace* workspace)
{

    Prob* problem = workspace->problem;

    adouble tf_a, ti_b;
    tf_a = get_final_time(xad, iphase_a, workspace);
    ti_b = get_initial_time(xad, iphase_b, workspace);
    int k;
    int nstates_a = problem->phase[iphase_a-1].nstates;
    int nstates_b = problem->phase[iphase_b-1].nstates;

    adouble* initial_states= workspace->initial_states[iphase_b-1];
    adouble* final_states  = workspace->final_states[iphase_a-1];


    if (nstates_a != nstates_b)
    {
        error_message("\nauto_link(): it is not possible to auto link phases with different number of states");
    }

    // state continuity

    get_final_states(final_states, xad, iphase_a, workspace);
    get_initial_states(initial_states, xad, iphase_b, workspace);
    for(k=0;k<nstates_a;k++) {

          linkages[*index + k ] = final_states[k]- initial_states[k];
    }

    // Time continuity
    linkages[*index + nstates_a] = tf_a - ti_b;

    *index += (nstates_a+1);

    workspace->auto_linked_flag = true;



}

void auto_link_2(adouble* linkages, int* index, adouble* xad, int iphase_a, int iphase_b, Workspace* workspace)
{

    Prob* problem = workspace->problem;

    adouble tf_a, ti_b;
    tf_a = get_final_time(xad, iphase_a, workspace);
    ti_b = get_initial_time(xad, iphase_b, workspace);

    int k;
    int nstates_a = problem->phase[iphase_a-1].nstates;
    int nstates_b = problem->phase[iphase_b-1].nstates;
    int ncontrols_a = problem->phase[iphase_a-1].ncontrols;
    int ncontrols_b = problem->phase[iphase_b-1].ncontrols;


    adouble* initial_states= workspace->initial_states[iphase_b-1];
    adouble* final_states  = workspace->final_states[iphase_a-1];

    adouble* initial_controls= workspace->initial_controls[iphase_b-1];
    adouble* final_controls  = workspace->final_controls[iphase_a-1];


    if (nstates_a != nstates_b)
    {
        error_message("\nauto_link(): it is not possible to auto link phases with different number of states");
    }

    if (ncontrols_a != ncontrols_b)
    {
        error_message("\nauto_link_2(): It is not possible to auto link phases with different number of controls");
    }


    // state continuity

    get_final_states(final_states, xad, iphase_a, workspace);
    get_initial_states(initial_states, xad, iphase_b, workspace);
    for(k=0;k<nstates_a;k++) {

          linkages[*index + k ] = final_states[k]- initial_states[k];
    }
    // Time continuity
    linkages[*index + nstates_a] = tf_a - ti_b;


    // control continuity

    get_final_controls(final_controls, xad, iphase_a, workspace);
    get_initial_controls(initial_controls, xad, iphase_b, workspace);
    for(k=0;k<ncontrols_a;k++) {

          linkages[*index + nstates_a + 1 + k ] = final_controls[k]- initial_controls[k];
    }


    *index += (nstates_a+1+ncontrols_a);

    workspace->auto_linked_flag = true;



}



Phases& Prob::phases(int iphase)
{
     if (iphase <1 || iphase > nphases)
          error_message("incorrect phase index in Prob::phases()");
     return phase[iphase-1];
}





void multi_segment_setup(Prob& problem, Alg& algorithm, MSdata& msdata)
{
     problem.nphases = msdata.nsegments;

     if (!msdata.continuous_controls) {
	  problem.nlinkages = auto_link_count(problem, msdata.nstates);
     }
     else {
       	  problem.nlinkages = auto_link2_count(problem, msdata.nstates, msdata.ncontrols);
     }

     if (msdata.nodes.GetNoRows() != 1 && msdata.nodes.GetNoRows() != problem.nphases) {
	      error_message("Incorrect dimensions of msdata.nodes matrix, its row dimension should\n  \
	                        be either 1 or equal to the number of segments");
     }

     psopt_level1_setup(problem);

     problem.phases(1).nstates = msdata.nstates;

     problem.phases(1).ncontrols = msdata.ncontrols;

     if (problem.nphases == 1) {
     	problem.phases(1).nevents = msdata.ninitial_events + msdata.nfinal_events;
     }
     else {
     	problem.phases(1).nevents = msdata.ninitial_events;
     }

     problem.phases(1).npath = msdata.npath;

     problem.phases(1).nodes = msdata.nodes(1,colon());

     problem.phases(1).nparameters = msdata.nparameters;

     if (problem.observation_function!=NULL) {

        problem.phases(1).nobserved = msdata.nobserved;

        if (msdata.nsegments>0) problem.phases(1).nsamples  = (msdata.nsamples-1)/msdata.nsegments + 1;

     }

     else {

        problem.phases(1).nobserved = 0;

        problem.phases(1).nsamples  = 0;

     }

     auto_phase_setup(problem, msdata.nfinal_events, msdata.nodes);

     psopt_level2_setup(problem, algorithm);

     problem.multi_segment_flag = true;

     problem.continuous_controls_flag = msdata.continuous_controls;

}

int auto_link_count(Prob& problem, int nstates)
{
   return (problem.nphases-1)*(nstates + 1);
}

int auto_link2_count(Prob& problem, int nstates, int ncontrols)
{
   return (problem.nphases-1)*(nstates+ncontrols+1);
}


void auto_phase_setup(Prob& problem,int n_final_events, DMatrix& nodes)
{
	int i, j;

        if (problem.nphases == 1) return;


	for(i=2;i<=problem.nphases;i++)
	{
	        if ( nodes.GetNoRows() == 1 ) j = 1;
		else if (nodes.GetNoRows() == problem.nphases) j=i;
		problem.phases(i).nstates   		= problem.phases(1).nstates;
		problem.phases(i).ncontrols 		= problem.phases(1).ncontrols;
    		problem.phases(i).nevents   		= 0;
    		problem.phases(i).npath     		= problem.phases(1).npath;
    		problem.phases(i).nodes                 = nodes(j,colon());
		problem.phases(i).nparameters           = 0;
                problem.phases(i).nobserved             = problem.phases(1).nobserved;
                problem.phases(i).nsamples              = problem.phases(1).nsamples;
	}

	problem.phases(problem.nphases).nevents  	= n_final_events;


}





void  auto_phase_bounds(Prob& problem)
{
	int i;
	double t0l, t0u, tfl, tfu;

        if (problem.nphases == 1) return;

	double dt, t0, tf;

	if ( problem.bounds.lower.times.isEmpty() ) {

           dt = problem.phases(problem.nphases).bounds.upper.EndTime - problem.phases(1).bounds.lower.StartTime;
           t0 = problem.phases(1).bounds.lower.StartTime;
           tf = problem.phases(problem.nphases).bounds.upper.EndTime;

	}

	else {

	   problem.phases(1).bounds.lower.StartTime = problem.bounds.lower.times(1);
	   problem.phases(1).bounds.upper.StartTime = problem.bounds.upper.times(1);
	   problem.phases(problem.nphases).bounds.lower.EndTime = problem.bounds.lower.times("end");
	   problem.phases(problem.nphases).bounds.upper.EndTime = problem.bounds.upper.times("end");

	   dt = problem.phases(problem.nphases).bounds.upper.EndTime - problem.phases(1).bounds.lower.StartTime;
           t0 = problem.phases(1).bounds.lower.StartTime;
           tf = problem.phases(problem.nphases).bounds.upper.EndTime;

	}

	for(i=2;i<=problem.nphases;i++)
	{
	    problem.phases(i).bounds.lower.states = problem.phases(1).bounds.lower.states;
    	    problem.phases(i).bounds.upper.states = problem.phases(1).bounds.upper.states;
	    problem.phases(i).bounds.lower.controls = problem.phases(1).bounds.lower.controls;
    	    problem.phases(i).bounds.upper.controls = problem.phases(1).bounds.upper.controls;

	    problem.phases(i).bounds.lower.path = problem.phases(1).bounds.lower.path;
    	    problem.phases(i).bounds.upper.path = problem.phases(1).bounds.upper.path;



	    if ( problem.bounds.lower.times.isEmpty() ) {
                if (problem.phases(i).nobserved==0) {
	            	problem.phases(i).bounds.lower.StartTime = t0;
	 	    	problem.phases(i).bounds.upper.StartTime = tf;
                }
	        else {
	            	problem.phases(i).bounds.lower.StartTime = problem.phases(i).observation_nodes(1);
	 	    	problem.phases(i).bounds.upper.StartTime = problem.phases(i).observation_nodes(1);
                }
	    }
	    else {
		problem.phases(i).bounds.lower.StartTime = problem.bounds.lower.times(i);
 	    	problem.phases(i).bounds.upper.StartTime = problem.bounds.upper.times(i);
            }

	}

	for(i=1;i<problem.nphases;i++)
	{
            if (problem.phases(i).nobserved==0) {
	            problem.phases(i).bounds.lower.EndTime = problem.phases(i+1).bounds.lower.StartTime;
		    problem.phases(i).bounds.upper.EndTime = problem.phases(i+1).bounds.upper.StartTime;
            }
            else {
	            problem.phases(i).bounds.lower.EndTime = problem.phases(i).observation_nodes("end");
		    problem.phases(i).bounds.upper.EndTime = problem.phases(i).observation_nodes("end");

            }
	}


 	for (i=1;i<=problem.nphases;i++) {
         	fprintf(stderr,"\n Phase %i lower start time = %f", i, problem.phases(i).bounds.lower.StartTime);
         	fprintf(stderr,"\n Phase %i upper start time = %f", i, problem.phases(i).bounds.upper.StartTime);

         	fprintf(stderr,"\n Phase %i lower end time = %f",  i, problem.phases(i).bounds.lower.EndTime);
         	fprintf(stderr,"\n Phase %i upper end time = %f",   i, problem.phases(i).bounds.upper.EndTime);
 	}

}


void  auto_phase_guess(Prob& problem, DMatrix& controls, DMatrix& states, DMatrix& param, DMatrix& time)
{
        int i,j;
        double time_min, time_max;

	for(i=1;i<=problem.nphases;i++)
	{
            int min_nodes = Min(problem.phases(i).nodes);
 	    problem.phases(i).guess.controls = zeros(problem.phases(i).ncontrols,min_nodes);
	    for(j=1; j<=problem.phases(i).ncontrols;j++) {
	         problem.phases(i).guess.controls(j,colon()) = linspace(controls(j,1), controls(j,"end"), min_nodes);
            }
	    problem.phases(i).guess.states = zeros(problem.phases(i).nstates,min_nodes);
	    for(j=1; j<=problem.phases(i).nstates;j++) {
    	    	problem.phases(i).guess.states(j,colon()) =linspace(states(j,1), states(j,"end"), min_nodes);
	    }
	    if (problem.phase[i-1].nparameters>0) {
		problem.phases(i).guess.parameters = zeros(problem.phases(i).nparameters,1);
		for(j=1; j<=problem.phases(i).nparameters;j++) {
		    problem.phases(i).guess.parameters(j)= param(j);
		}
	    }
            if (i==1) {
                time_min = time(1);
                time_max = time(1)+ (time(1)-time("end"))/problem.nphases;
            }
            else {
		time_min = problem.phases(i-1).guess.time("end");
                time_max = time_min + i*(time(1)-time("end"))/problem.nphases;
            }
    	    problem.phases(i).guess.time = linspace(time_min,time_max,min_nodes);
	}

}

void auto_link_multiple(adouble* linkages, adouble* xad,int nphases, Workspace* workspace)
{
  int index = 0;
  int i;

  if ( workspace->problem->continuous_controls_flag==false ) {
     for(i=1;i<=(nphases-1);i++) {
	auto_link(linkages, &index, xad, i,  i+1, workspace);
     }
  }

  else {
      for(i=1;i<=(nphases-1);i++) {
	    auto_link_2(linkages, &index, xad, i,  i+1, workspace);
      }
  }
}

