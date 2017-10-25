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

int get_number_of_mesh_refinement_iterations(Prob& problem, Alg& algorithm)
{
  int number_of_mesh_refinement_iterations;

  if (algorithm.mesh_refinement == "manual") {
        number_of_mesh_refinement_iterations = problem.phases(1).nodes.GetNoCols();
  }
  else
  {
        number_of_mesh_refinement_iterations = algorithm.mr_max_iterations;
  }

  return number_of_mesh_refinement_iterations;
}


int get_number_nlp_vars(Prob& problem, Workspace* workspace)
{
   int i;
   int nlp_vars = 0;


   for (i=0;i< problem.nphases;i++)
   {
   	int    nstates   = problem.phase[i].nstates;
   	int    ncontrols = problem.phase[i].ncontrols;
        int    nparam    = problem.phase[i].nparameters;
   	int    nodes     = problem.phase[i].current_number_of_intervals;
      	nlp_vars += (ncontrols+nstates)*(nodes+1)+nparam+2;
        if ( need_midpoint_controls(*workspace->algorithm, workspace) ) {
            nlp_vars += (ncontrols)*(nodes);
        }
   }

   return nlp_vars;
}

int get_max_number_nlp_vars(Prob& problem, Alg& algorithm)
{
   int i;
   int nlp_vars = 0;


   for (i=0;i< problem.nphases;i++)
   {
   	int    nstates   = problem.phase[i].nstates;
   	int    ncontrols = problem.phase[i].ncontrols;
        int    nparam    = problem.phase[i].nparameters;
   	int    max_nodes = get_max_nodes( problem, i+1, &algorithm );
      	nlp_vars += (ncontrols+nstates)*(max_nodes+1)+nparam+2;
        nlp_vars += (ncontrols)*(max_nodes);

   }

   return nlp_vars;
}


int get_number_nlp_constraints(Prob& problem,Workspace* workspace)
{

    int i;
    int nlp_ncons = 0;

    for(i=0; i<problem.nphases; i++)
    {

       nlp_ncons  += get_ncons_phase_i(problem,i, workspace);

    }

   nlp_ncons +=  problem.nlinkages;


   return nlp_ncons;
}

int get_max_number_nlp_constraints(Prob& problem, Alg& algorithm)
{

    int i;
    int nlp_ncons = 0;

    for(i=0; i<problem.nphases; i++)
    {

       int nstates   = problem.phase[i].nstates;
       int nevents   = problem.phase[i].nevents;
       int npath     = problem.phase[i].npath;
       int max_nodes = get_max_nodes(problem, i+1, &algorithm);


       nlp_ncons  += nstates*(max_nodes+1)+ (nevents) + npath*(max_nodes+1) + 1;


       nlp_ncons += npath*(max_nodes);



   }

   nlp_ncons +=  problem.nlinkages;


   return nlp_ncons;
}


int get_max_nodes(Prob& problem,int iphase, Alg* algorithm)
{

    int retval, i;

    if (algorithm->mesh_refinement == "manual") {
         retval = problem.phase[iphase-1].nodes("end");
    }

    else if (algorithm->mesh_refinement == "automatic" && !use_local_collocation(*algorithm) ) {
         int max_increment = algorithm->mr_max_increment_factor*(problem.phase[iphase-1].current_number_of_intervals+1);
         retval = problem.phase[iphase-1].nodes(1) + (algorithm->mr_min_extrapolation_points-1)*(algorithm->mr_initial_increment);
         int count = retval;
         for (i=1;i<=(algorithm->mr_max_iterations-2);i++) {
                int increment = algorithm->mr_max_increment_factor*count;
 		count += increment;
	 }
	 retval += count;
    }

    else if (algorithm->mesh_refinement == "automatic" && use_local_collocation(*algorithm) ) {
         int M = problem.phase[iphase-1].nodes(1);
	 int mcount = M;
	 for (int i=1; i<= algorithm->mr_max_iterations;i++) {
	    mcount = mcount + algorithm->mr_max_increment_factor*(mcount-1);
	 }
	 retval = mcount;
    }

    return retval;
}



int get_max_nodes_in_all_phases(Prob& problem, Alg& algorithm)
{
    int iphase;

    int max_nodes = 0;

    for (iphase=1;iphase<=problem.nphases;iphase++) {
        if ( get_max_nodes(problem,iphase,&algorithm) > max_nodes )
	     max_nodes = get_max_nodes(problem,iphase, &algorithm);
    }

    return (max_nodes);
}




int get_nvars_phase_i(Prob& problem, int i, Workspace* workspace)
{
	int norder    = problem.phase[i].current_number_of_intervals;
	int ncontrols = problem.phase[i].ncontrols;
	int nstates   = problem.phase[i].nstates;
        int nparam    = problem.phase[i].nparameters;


	int nvars_phase_i = (nstates+ncontrols)*(norder+1)+nparam;

        if ( need_midpoint_controls(*workspace->algorithm, workspace) ) {
                    nvars_phase_i += ncontrols*norder;
        }

        nvars_phase_i += 2;

        return nvars_phase_i;

}

int get_ncons_phase_i(Prob& problem, int i, Workspace* workspace)
{
	int norder    = problem.phase[i].current_number_of_intervals;
	int ncontrols = problem.phase[i].ncontrols;
	int nstates   = problem.phase[i].nstates;
        int nparam    = problem.phase[i].nparameters;
        int nevents   = problem.phase[i].nevents;
        int npath     = problem.phase[i].npath;

        int ncons_phase_i = nstates*(norder+1) + nevents + npath*(norder+1)+1;

        if ( need_midpoint_controls(*workspace->algorithm, workspace) ) {
                    ncons_phase_i += npath*norder;
        }

        return ncons_phase_i;

}


int get_iphase_offset(Prob& problem, int iphase, Workspace* workspace)
{
        int ii;

        int iphase_offset=0;

        for(ii=0;ii< iphase-1;ii++) {

		int nvars_phase_i = get_nvars_phase_i(problem,ii, workspace);

        	iphase_offset +=nvars_phase_i;

       }

       return (iphase_offset);

}



int get_number_of_controls(Prob& problem, int iphase)
{

	return problem.phase[iphase-1].ncontrols;
}

int get_number_of_states(Prob& problem, int iphase)
{
	return problem.phase[iphase-1].nstates;
}

int get_number_of_events(Prob& problem, int iphase)
{
	return problem.phase[iphase-1].nevents;
}

int get_number_of_path_constraints(Prob& problem, int iphase)
{
	return  problem.phase[iphase-1].npath;
}

int get_number_of_nodes(Prob& problem, int iphase)
{
	return problem.phase[iphase-1].current_number_of_intervals;
}



int getNumberOfParameters(Prob& problem, int iphase)
{
	return problem.phase[iphase-1].nparameters;
}

int getNumberOfLinkages(Prob& problem)
{
	return problem.nlinkages;
}

int getNumberOfPhases(Prob& problem)
{
	return problem.nphases;
}



