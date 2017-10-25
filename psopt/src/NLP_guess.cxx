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


void  define_initial_nlp_guess(DMatrix& x0, DMatrix& lambda, Sol& solution, Prob& problem, Alg& algorithm, Workspace* workspace)
{
   // This function determines the initial guess for the NLP decision vector x0

   int i;
   int x_phase_offset =0;

   for(i=0; i<problem.nphases; i++)
   {

	DMatrix& control_scaling = problem.phase[i].scale.controls;
	DMatrix& state_scaling   = problem.phase[i].scale.states;
        DMatrix& param_scaling   = problem.phase[i].scale.parameters;
        double&   time_scaling   = problem.phase[i].scale.time;

	int norder    = problem.phase[i].current_number_of_intervals;
	int ncontrols = problem.phase[i].ncontrols;
	int nstates   = problem.phase[i].nstates;
        int nparam    = problem.phase[i].nparameters;
	int offset1   = ncontrols*(norder+1);
        int offset2   = (ncontrols+nstates)*(norder+1);
	int k;
	double t00, tf0;

	DMatrix umean, xmean;
	DMatrix Xdot(nstates,norder+1);
	DMatrix up, xp, un, xn;
	DMatrix time_guess;
	//   DMatrix w, P, D;

	int nvars_phase_i = get_nvars_phase_i(problem,i, workspace);

	if ( !problem.phase[i].guess.time.isEmpty() ) {
	  t00 = problem.phase[i].guess.time(1);
	  tf0 = problem.phase[i].guess.time("end");
	}
	else {
		t00 = (problem.phase[i].bounds.lower.StartTime + problem.phase[i].bounds.upper.StartTime)/2.0;
		tf0 = (problem.phase[i].bounds.lower.EndTime + problem.phase[i].bounds.upper.EndTime)/2.0;
	}

        if (nparam > 0) {
               if( !problem.phase[i].guess.parameters.isEmpty() )  {
                  solution.parameters[i] = problem.phase[i].guess.parameters;
               }
               else {
                  solution.parameters[i] = 0.5*((problem.phase[i].bounds.upper.parameters) + (problem.phase[i].bounds.lower.parameters));
               }
        }


	if ( !problem.phase[i].guess.time.isEmpty()) {
		time_guess = problem.phase[i].guess.time;
	} else {
		time_guess = linspace(t00,tf0,norder+1);
	}

	for (k=1; k<=norder+1; k++) {
		(solution.nodes[i])(1,k)  =  convert_to_original_time( (workspace->snodes[i])(k), t00, tf0 );
	}


	if ( !problem.phase[i].guess.controls.isEmpty() ) {

		for (k=1;k<=ncontrols;k++) {
			up = problem.phase[i].guess.controls(k,colon());
//			lagrange_interpolation(un,solution.nodes[i],time_guess, up);
			linear_interpolation(un,solution.nodes[i],time_guess, up, length(up) );

			(solution.controls[i])(k,colon())  = un;
		}


	}
	else {
		umean = ((problem.phase[i].bounds.lower.controls)+(problem.phase[i].bounds.upper.controls))/2.0;
	for(k=1;k<=norder+1;k++)
	{
		(solution.controls[i])(colon(),k) = umean;
	}
	}


	if ( !problem.phase[i].guess.states.isEmpty() ) {
		for (k=1;k<=nstates;k++) {
			xp = problem.phase[i].guess.states(k,colon());
//			lagrange_interpolation(xn,solution.nodes[i],time_guess, xp);
            linear_interpolation(xn,solution.nodes[i],time_guess, xp, length(xp));
			(solution.states[i])(k,colon())  = xn;
		}

	}

	// Set initial states at mean of feasible region for the moment...

	if ( problem.phase[i].guess.states.isEmpty() )
	{
		xmean = ((problem.phase[i].bounds.lower.states)+(problem.phase[i].bounds.upper.states))/2.0;
		(solution.states[i])(colon(),1)= xmean;
	}

	// Define the nodes in original time scale:

	for (k=1;k<=norder+1;k++)
	{
	(solution.nodes[i])(k) = convert_to_original_time( (workspace->snodes[i])(k), t00, tf0 );
	}

	// Take the state trajectory as constant and equal to the initial values found above.

	if ( problem.phase[i].guess.states.isEmpty() ) {
		for (k=1;k<=norder;k++)
		{
			(solution.states[i])(colon(),k+1)=(solution.states[i])(colon(),1);
		}
	}

	// Determine the scaling factors to be used

	determine_scaling_factors_for_variables(solution,problem,algorithm);

	// Now assign the (scaled) variables to the initial sqp decision vector:

	x0(x_phase_offset+nvars_phase_i-1) = t00*time_scaling;
	x0(x_phase_offset+nvars_phase_i)   = tf0*time_scaling;


	for (k=1; k<=norder+1; k++) {
           if (ncontrols>0) {
	        x0(colon(x_phase_offset+(k-1)*ncontrols+1, x_phase_offset+k*ncontrols) ) = elemProduct((solution.controls[i])(colon(),k), control_scaling);
           }
	   x0(colon(x_phase_offset+(k-1)*nstates+1+offset1,x_phase_offset+ k*nstates+offset1))=elemProduct((solution.states[i])(colon(),k), state_scaling);
	}

        if (nparam >= 1) {
           x0(colon(x_phase_offset+offset2 +1, x_phase_offset+offset2+nparam))=elemProduct(solution.parameters[i],param_scaling);
        }

       if (need_midpoint_controls(*workspace->algorithm, workspace)) {
	  for (k=1; k<=norder; k++) {
            if (ncontrols>0) {
	          x0(colon(x_phase_offset+offset2+nparam+(k-1)*ncontrols+1, x_phase_offset+offset2+nparam+k*ncontrols) ) = elemProduct((solution.controls[i])(colon(),k), control_scaling);
            }

	  }
       }

        x_phase_offset += nvars_phase_i;

  }

  determine_objective_scaling(x0,solution,problem,algorithm, workspace);

  determine_constraint_scaling_factors(x0, solution, problem, algorithm, workspace);

  // Assign zeros to the vector of lagrange multipliers:

  lambda.Resize(workspace->ncons, 1);
  lambda.FillWithZeros();


}



void hot_start_nlp_guess(DMatrix& x0,DMatrix& lambda, Sol& solution,Prob& problem,Alg& algorithm, DMatrix* prev_states, DMatrix* prev_controls, DMatrix* prev_costates, DMatrix* prev_path, DMatrix* prev_nodes, DMatrix* prev_param, DMatrix& prev_t0, DMatrix& prev_tf, Workspace* workspace )
{

     int i, k;

     int x_phase_offset   =0;
     int lam_phase_offset = 0;


     sprintf(workspace->text,"\nHot starting solution\n");
     psopt_print(workspace,workspace->text);

     x0.Resize(workspace->nvars,1);

     x0.FillWithZeros();

     lambda.Resize(workspace->ncons,1);

     lambda.FillWithZeros();


     for(i=0; i<problem.nphases;i++)
     {

	DMatrix& control_scaling = problem.phase[i].scale.controls;
	DMatrix& state_scaling   = problem.phase[i].scale.states;
        DMatrix& param_scaling   = problem.phase[i].scale.parameters;
        double   time_scaling    =  problem.phase[i].scale.time;


	int norder    = problem.phase[i].current_number_of_intervals;
	int ncontrols = problem.phase[i].ncontrols;
	int npath     = problem.phase[i].npath;
	int nstates   = problem.phase[i].nstates;
        int nparam    = problem.phase[i].nparameters;
	int nevents   = problem.phase[i].nevents;
	int offset1   = ncontrols*(norder+1);
        int offset2   = (ncontrols+nstates)*(norder+1);
	int k;
	int offset;
	DMatrix xn, xp;
	DMatrix un, up;
	DMatrix pn, pp;

	int nvars_phase_i = get_nvars_phase_i(problem,i, workspace);

        int ncons_phase_i = get_ncons_phase_i(problem,i, workspace);


	//     prev_nodes.Save("prev_nodes.dat");

	for (k=1; k<=norder+1; k++) {
		(solution.nodes[i])(1,k)  =  convert_to_original_time( (workspace->snodes[i])(k), prev_t0(i+1), prev_tf(i+1) );
	}


	// Interpolate states into new nodes
	for (k=1;k<=nstates;k++) {

		xp = (prev_states[i])(k,colon());
		if (!use_local_collocation(algorithm) ) {
		    lagrange_interpolation(xn,solution.nodes[i],prev_nodes[i], xp);
		}
		else {
		    linear_interpolation(xn,solution.nodes[i],prev_nodes[i], xp, length(xp));
		}
		(solution.states[i])(k,colon())  = xn;
	}

	// Interpolate costates into new nodes
	(workspace->dual_costates[i]).Resize(nstates,norder+1);
	for (k=1;k<=nstates;k++) {
		xp = (prev_costates[i])(k,colon());
		if (!use_local_collocation(algorithm)) {
		    lagrange_interpolation(xn,solution.nodes[i],prev_nodes[i], xp);
		}
		else {
		    linear_interpolation(xn,solution.nodes[i],prev_nodes[i], xp, length(xp));
		}
		(workspace->dual_costates[i])(k,colon())  = xn;
	}

	// Interpolate controls into new nodes
	for (k=1;k<=ncontrols;k++) {
		up = (prev_controls[i])(k,colon());
		linear_interpolation(un,solution.nodes[i],prev_nodes[i], up, length(up) );
		(solution.controls[i])(k,colon())  = un;
	}

	// Interpolate path constraint multipliers into new nodes
	if (npath) {
		(workspace->dual_path[i]).Resize(npath,norder+1);
		for (k=1;k<=npath;k++) {
			pp = (prev_path[i])(k,colon());
			if (!use_local_collocation(algorithm)) {
			    lagrange_interpolation(pn,solution.nodes[i],prev_nodes[i], pp);
			}
			else {
			    linear_interpolation(pn,solution.nodes[i],prev_nodes[i], pp, length(pp));
			}
			(workspace->dual_path[i])(k,colon())  = pn;
		}
	}

	// Now copy relevant variables into the decision vector

	for (k=1; k<=norder+1; k++) {
          if(ncontrols>0) {
	     x0(colon(x_phase_offset+(k-1)*ncontrols+1,x_phase_offset+k*ncontrols) ) = elemProduct( (solution.controls[i])(colon(),k), control_scaling);
          }
	  x0(colon(x_phase_offset+(k-1)*nstates+1+offset1, x_phase_offset+k*nstates+offset1))=elemProduct((solution.states[i])(colon(),k), state_scaling);
	}

        if (nparam>0) {
           x0( colon(x_phase_offset+offset2+1, x_phase_offset+offset2+nparam) ) = elemProduct(prev_param[i], param_scaling);
        }

        if ( need_midpoint_controls(*workspace->algorithm, workspace) ) {

	  for (k=1; k<=norder; k++) {
             if(ncontrols>0) {
	       x0(colon(x_phase_offset+offset2+nparam+(k-1)*ncontrols+1,x_phase_offset+offset2+nparam+k*ncontrols) ) =   elemProduct( (solution.controls[i])(colon(),k), control_scaling);
             }
	  }
        }

	x0(x_phase_offset+ nvars_phase_i-1) = prev_t0(i+1)*time_scaling;
	x0(x_phase_offset+ nvars_phase_i)   = prev_tf(i+1)*time_scaling;


	// And finally copy the lagrange multiplier variables into vector lambda
	lambda(colon(lam_phase_offset+1,lam_phase_offset+nstates*(norder+1)) ) = (*workspace->dual_costates)(colon(1, nstates*(norder+1)));
	offset = lam_phase_offset+nstates*(norder+1);

	if (nevents>0)
        {
               lambda(colon(offset+1,offset+nevents)) = (workspace->dual_events[i]);
	       offset = offset + nevents;
        }
	if (npath>0) lambda(colon(offset+1, offset+npath*(norder+1))) = (workspace->dual_path[i])(colon(1, npath*(norder+1)));

        x_phase_offset += nvars_phase_i;
        lam_phase_offset += ncons_phase_i;
  }

  // Now deal with the Lagrange multipliers of the linkage constraints

  if (problem.nlinkages)
  {
     for(k=1; k<=problem.nlinkages; k++)
     {
          lambda(lam_phase_offset + k) = -(*solution.dual.linkages)(k);
     }

  }

  // Recalculate the scaling factors for objective and constraints

  determine_objective_scaling(x0,solution,problem,algorithm, workspace);

  determine_constraint_scaling_factors(x0, solution, problem, algorithm, workspace);

}


