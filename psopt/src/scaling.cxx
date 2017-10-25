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


void determine_scaling_factors_for_variables(Sol& solution, Prob& problem, Alg& algorithm)
{
     // Scaling factors  for variables computed automatically given the bound information
     // supplied by the user, such that the scaled variables are mostly in the interval [-1,1].
     // If, however, any of the variable bounds is 'inf' in magnitude, then the scaled range
     // will be [-1, inf], [-inf,1],  or [-1, 1]

     int i;

     for(i=0; i<problem.nphases; i++)
     {

	int norder    = problem.phase[i].current_number_of_intervals;
	int ncontrols = problem.phase[i].ncontrols;
	int npath     = problem.phase[i].npath;
	int nstates   = problem.phase[i].nstates;
	int nevents   = problem.phase[i].nevents;
        int nparam    = problem.phase[i].nparameters;

	DMatrix& control_scaling = problem.phase[i].scale.controls;
	DMatrix& state_scaling   = problem.phase[i].scale.states;

        DMatrix& param_scaling   = problem.phase[i].scale.parameters;


	DMatrix PathJac(npath, nstates+ncontrols);
	DMatrix h1(npath,norder+1);
	DMatrix h2(npath,norder+1);
	DMatrix e1(nevents,1);
	DMatrix e2(nevents,2);
	DMatrix EventJac1(nevents, nstates+ncontrols);
	DMatrix EventJac2(nevents, nstates+ncontrols);

	double zlower;
	double zupper;

	int ii;



	// Control scaling:

	if ( algorithm.scaling=="automatic" || algorithm.scaling!="user" )
	{
	   control_scaling = ones(ncontrols,1);

	   for(ii=1;ii<=ncontrols;ii++)
	   {
		zlower = (problem.phase[i].bounds.lower.controls)(ii);
		zupper = (problem.phase[i].bounds.upper.controls)(ii);
		if ( zlower!=-INF && zupper!= INF ) {
		   if (zlower !=0.0 || zupper!=0.0)
		      control_scaling(ii) = 1.0/MAX( fabs(zlower), fabs(zupper));
		}
		else if (zlower==-INF && zupper!=INF && zupper!=0.0)
		      control_scaling(ii) = 1.0/fabs(zupper);
		else if (zupper==INF && zlower!=-INF && zlower!=0.0)
		       control_scaling(ii) = 1.0/fabs(zlower);
	   }
	}




	// State scaling:

	if ( algorithm.scaling=="automatic" || algorithm.scaling!="user" )
	{

		state_scaling = ones(nstates,1);

		for(ii=1;ii<=nstates;ii++)
		{
			zlower = (problem.phase[i].bounds.lower.states)(ii);
			zupper = (problem.phase[i].bounds.upper.states)(ii);
			if ( zlower!=-INF && zupper!= INF ) {
			    if (zlower !=0.0 || zupper!=0.0)
			        state_scaling(ii) = 1.0/MAX( fabs(zlower), fabs(zupper));
			}
			else if (zlower==-INF && zupper!=INF && zupper!=0.0)
			     state_scaling(ii) = 1.0/fabs(zupper);
			else if (zupper==INF && zlower!=-INF && zlower!=0.0)
			     state_scaling(ii) = 1.0/fabs(zlower);
		}
	}



	// Parameter scaling

	if ( algorithm.scaling=="automatic" || algorithm.scaling!="user" )
	{
		param_scaling = ones(nparam,1);

		for(ii=1;ii<=nparam;ii++)
		{
			zlower = (problem.phase[i].bounds.lower.parameters)(ii);
			zupper = (problem.phase[i].bounds.upper.parameters)(ii);
			if ( zlower!=-INF && zupper!= INF ) {
				if (zlower !=0.0 || zupper!=0.0)
				param_scaling(ii) = 1.0/MAX( fabs(zlower), fabs(zupper));
			}
			else if (zlower==-INF && zupper!=INF && zupper!=0.0)
				param_scaling(ii) = 1.0/fabs(zupper);
			else if (zupper==INF && zlower!=-INF && zlower!=0.0)
				param_scaling(ii) = 1.0/fabs(zlower);


		}
	}




	// Time scaling

	if ( algorithm.scaling=="automatic" || algorithm.scaling!="user" )
	{
		problem.phase[i].scale.time = 1.0;

		zlower = (problem.phase[i].bounds.lower.StartTime);
		zupper = (problem.phase[i].bounds.upper.EndTime);
		if ( zlower!=-INF && zupper!= INF ) {
				if (zlower !=0.0 || zupper!=0.0)
				problem.phase[i].scale.time = 1.0/MAX( fabs(zlower), fabs(zupper));
		}
		else if (zlower==-INF && zupper!=INF && zupper!=0.0)
				problem.phase[i].scale.time = 1.0/fabs(zupper);
		else if (zupper==INF && zlower!=-INF && zlower!=0.0)
				problem.phase[i].scale.time = 1.0/fabs(zlower);

	}

  }


}

void determine_objective_scaling(DMatrix& X,Sol& solution, Prob& problem, Alg& algorithm, Workspace* workspace )
{

  // The scaling factor for the objective function is computed such that the
  // scaled gradient at the initial guess has an Euclidean norm of 1.0.

  double nrm_g;
  DMatrix& GF = *workspace->GFip;
  GF.Resize(get_number_nlp_vars(problem, workspace), 1);


  if ( algorithm.scaling=="automatic" || algorithm.scaling!="user" )
  {

	if ( (algorithm.derivatives=="automatic") ) {
	    int      n = length(X);
	    int i, itag=workspace->tag_f;
	    double  yp = 0.0;
	    adouble *xad = workspace->xad;
	    adouble  yad;
	    DMatrix&  GF = *workspace->GFip;
	    problem.scale.objective = -1.0;
	    trace_on(itag);
	    for(i=0;i<n;i++) {
			xad[i] <<= (X.GetPr())[i];
	    }
	    yad = ff_ad(xad, workspace);
	    yad >>= yp;
	    trace_off();

	    gradient(itag,n,X.GetPr(),GF.GetPr());

	}

	else {
	  problem.scale.objective = -1.0;
	  ScalarGradient( ff_num, X, &GF , workspace->grw, workspace );
	}

        nrm_g = enorm(GF);

        if ( nrm_g != 0.0 && nrm_g < INF)
	      problem.scale.objective = 1/nrm_g;
	else
	      problem.scale.objective = 1.0;

  }



}



void determine_constraint_scaling_factors(DMatrix & X, Sol& solution, Prob& problem, Alg& algorithm, Workspace* workspace)
{
// For the differential defect constraints there are two options: The default is to use the
// same scaling factors as those used for the corresponding state. Alternatively, the user
// may specify that the scaling factors for the differential defect constraints be calculated
// based on the Jacobian (see below).
// For all other constraints, the scaling factors are computed such that the corresponding row of
// the scaled Jacobian matrix of the constraints has an Euclidean norm of 1.0.


  if ( algorithm.scaling=="automatic" || algorithm.scaling!="user" )
  {
    int i, j, l, k;

    int nvars = get_number_nlp_vars(problem, workspace);

    int ncons = get_number_nlp_constraints(problem, workspace);

    DMatrix& JacCol1 = *workspace->JacCol1;
    DMatrix& xlb     = *workspace->xlb;
    DMatrix& xub     = *workspace->xub;
    DMatrix& xp      = *workspace->xp;
    DMatrix& jac_row_norm = *workspace->JacCol2;
    DMatrix jtemp;

    workspace->use_constraint_scaling = 0;

    jac_row_norm.Resize(ncons,1);


     xp = X;
//     clip_vector_given_bounds( xp, xlb, xub);

     if ( useAutomaticDifferentiation(algorithm) && algorithm.constraint_scaling=="automatic") {
        // NOTE, THIS OPTION HAD BEEN DESACTIVATED AS MATTEO CERIOTTI REPORTED A PROBLEM WITH A CASE
        // WITH RELEASE 2.
        // ACTIVATED AGAIN ON 27.11.2012
        // EXTRA PARAMETER .constraint_scaling ADDED TO ALGORITHM STRUCTURE 27.11.2012.

        jac_row_norm.FillWithZeros();


    unsigned int *jac_rind  = NULL;
	unsigned int *jac_cind  = NULL;
	double       *jac_values = NULL;
	int           nnz;

	adouble *xad = workspace->xad;
	adouble *gad = workspace->gad;
	double  *g   = workspace->fg;
	double  *x   = xp.GetPr();

	/* Tracing of function gg() */
	trace_on(workspace->tag_gc);
	for(i=0;i<nvars;i++)
		xad[i] <<= x[i];

	gg_ad(xad, gad, workspace);

	for(i=0;i<ncons;i++)
		gad[i] >>= g[i];
	trace_off();

#ifdef ADOLC_VERSION_1
	sparse_jac(workspace->tag_gc, ncons, nvars, 0, x, &nnz, &jac_rind, &jac_cind, &jac_values);
#endif

#ifdef ADOLC_VERSION_2
    int options[4];
    options[0]=0; options[1]=0; options[2]=0;options[3]=0;
	sparse_jac(workspace->tag_gc, ncons, nvars, 0, x, &nnz, &jac_rind, &jac_cind, &jac_values, options);
#endif

	for (i=0;i<nnz;i++) {
	    jac_row_norm( jac_rind[i] + 1  ) += pow( jac_values[i], 2.0);
	}


     }
     else {

        jac_row_norm.FillWithZeros();
	DMatrix& xlb = *(workspace->xlb);
	DMatrix& xub = *(workspace->xub);
	for(j=1;j<=nvars;j++) {
	    JacobianColumn( gg_num, xp, xlb, xub,j, &JacCol1, workspace->grw, workspace);
	    jac_row_norm+= (JacCol1^2);
	}

     } // end if-else


     jac_row_norm = Sqrt(jac_row_norm);



 for (i=1;i<=ncons;i++)
     {
        double sqeps = sqrt(DMatrix::GetEPS());
 //	if (jac_row_norm(i) != 0.0 && jac_row_norm(i) < 1.e7 ) {
        if ( jac_row_norm(i) < 1.e7 ) {
 //	  (*workspace->constraint_scaling)(i) = 1.0/jac_row_norm(i);
	  (*workspace->constraint_scaling)(i) = 1.0/(jac_row_norm(i)+sqeps);

	}
	else  {
//                fprintf(stderr,"\n>>>>> jac_row_norm(%i)=%e", i, jac_row_norm(i) );
                if ( jac_row_norm(i) > 1.e7 ) {
    		      (*workspace->constraint_scaling)(i) = 1.0/1.e7;
                }
                if ( jac_row_norm(i) == 0.0 ) {
    		      (*workspace->constraint_scaling)(i) = 1.0;
                }

	}
   }
//////////////////////////////////////////////////////////

// RELEASE 1 CODE BELOW
//     for (i=1;i<=ncons;i++)
//     {
//        if (jac_row_norm(i) != 0.0) {
//           (*workspace->constraint_scaling)(i) = 1.0/jac_row_norm(i);
//        }
//        else {
//            (*workspace->constraint_scaling)(i) = 1.0;
//        }
//    }

//////////////////////////////////////////////////////

     workspace->use_constraint_scaling = 1;


     if ( algorithm.defect_scaling == "jacobian-based" )
          return;


     // By default, use the state scaling factors for the differential defects. See Betts (2001).


     for(i=0; i< problem.nphases; i++) {

        DMatrix& state_scaling = (problem.phase[i].scale.states);

     	int norder    = problem.phase[i].current_number_of_intervals;

	int nstates   = problem.phase[i].nstates;

	int nevents   = problem.phase[i].nevents;

	int npath     = problem.phase[i].npath;

	int offset;

        int ncons_phase_i = get_ncons_phase_i(problem,i, workspace);

        offset = 0;

  	for (k=1;k<=norder+1;k++) {

  		for( j=1;j<=nstates;j++) {

                	l = offset + (k-1)*nstates+j;

        		(*workspace->constraint_scaling)(l) = state_scaling(j);

        	}

   	}

        offset += ncons_phase_i;

     }

  }




}

