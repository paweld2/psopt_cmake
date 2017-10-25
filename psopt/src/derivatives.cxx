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

// Numerical Gradient Functions


void JacobianColumn( void fun(DMatrix& x, DMatrix* f, Workspace* ), DMatrix& x, DMatrix& xlb, DMatrix& xub, int jCol,
                DMatrix* JacColumn, GRWORK* grw, Workspace* workspace )
{
  // Computes only one column of the Jacobian matrix

  int  j, k;
  double delj;
  double sqreps;
  double xs;
  long nf  = JacColumn->GetNoRows();

  DMatrix *dfdx_j = grw->dfdx_j;
  DMatrix *F1   = grw->F1;
  DMatrix *F2   = grw->F2;
  DMatrix *F3   = grw->F3;


  dfdx_j->Resize( nf, 1 );
  F1->Resize(   nf, 1 );
  F2->Resize(   nf, 1 );

  sqreps= sqrt( DMatrix::GetEPS() );

  if (any( x < (xub-sqreps))|| any( x>(xlb+sqreps)) )
  {
     F3->Resize( nf, 1);
     fun(x, F3, workspace);
  }

  j = jCol;
      delj = sqreps*(1.+fabs(x(j)));
//      delj = sqreps;
      xs   = x(j);
      if ((xs < xub(j)-delj && xs>xlb(j)+delj) || (xub(j)==xlb(j))) {
        // Use central difference formula
      	x(j)+= delj;
      	fun( x, F1, workspace );
      	x(j) = xs-delj;
      	fun( x, F2, workspace );
      	*dfdx_j=( (*F1)-(*F2) )/(2*delj);
      	x(j)=xs;
      }
      else if (xs >= xub(j)-delj) {
        // Variable at upper bound, use backward difference formula
      	x(j)= xs - delj;
      	fun( x, F1, workspace );
        x(j)= xs;
      	*dfdx_j=( (*F1)-(*F3) )/(-delj);
      }
      else if (xs <= xlb(j)+delj) {
        // Variable at lower bound, use forward difference formula
      	x(j)= xs + delj;
      	fun( x, F1, workspace );
        x(j)= xs;
      	*dfdx_j=( (*F1)-(*F3) )/(delj);
      }
      for(k=1;k<=nf;k++) { (*JacColumn)(k,1)=(*dfdx_j)(k,1); }

}




void JacobianRow( void fun(DMatrix& x, DMatrix* f, Workspace* ), DMatrix& x, int iRow, int nf,
                  DMatrix* JacRow, GRWORK* grw, Workspace* workspace )
{
  // Computes only one row of the Jacobian matrix

  int  j;
  double delj;
  double sqreps;
  double xs;
  long nvar= x.GetNoRows();

  DMatrix *dfdx_j = grw->dfdx_j;
  DMatrix *F1   = grw->F1;
  DMatrix *F2   = grw->F2;
  DMatrix *F3   = grw->F3;

  DMatrix& xlb    = *workspace->xlb;
  DMatrix& xub    = *workspace->xub;


  dfdx_j->Resize( nf, 1 );
  F1->Resize(   nf, 1 );
  F2->Resize(   nf, 1 );

  sqreps= sqrt( DMatrix::GetEPS() );

  if (any( x < (xub-sqreps))|| any( x>(xlb+sqreps)) )
  {
     F3->Resize( nf, 1);
     fun(x, F3, workspace);
  }

  for(j=1;j<=nvar;j++) {
      delj = sqreps;
      xs   = x(j);
      if ((xs < xub(j)-delj && xs>xlb(j)+delj) || (xub(j)==xlb(j))) {
        // Use central difference formula
      	x(j)+= delj;
      	fun( x, F1, workspace );
      	x(j) = xs-delj;
      	fun( x, F2, workspace );
      	*dfdx_j=( (*F1)-(*F2) )/(2*delj);
      	x(j)=xs;
      }
      else if (xs >= xub(j)-delj) {
        // Variable at upper bound, use backward difference formula
      	x(j)= xs - delj;
      	fun( x, F1, workspace );
        x(j)= xs;
      	*dfdx_j=( (*F1)-(*F3) )/(-delj);
      }
      else if (xs <= xlb(j)+delj) {
        // Variable at lower bound, use forward difference formula
      	x(j)= xs + delj;
      	fun( x, F1, workspace );
        x(j)= xs;
      	*dfdx_j=( (*F1)-(*F3) )/(delj);
      }
      (*JacRow)(1,j)=(*dfdx_j)(iRow,1);
  }
}



void ComputeJacobianNonZeros( void fun(DMatrix& x, DMatrix* f ), DMatrix& x,
                int nf, double *nzvalue, int nnz, int* iArow, int* jAcol, GRWORK* grw, Workspace* workspace )
{

  int nvar, I, i;
  double delj;
  double sqreps;
  double xs = 0.0;
  nvar= x.GetNoRows();
  int iflag=1;



  DMatrix *F1   = grw->F1;
  DMatrix *F2   = grw->F2;
  DMatrix *F3   = grw->F3;

  DMatrix& xlb    = *workspace->xlb;
  DMatrix& xub    = *workspace->xub;

  F1->Resize(   nf, 1 );
  F2->Resize(   nf, 1 );

  sqreps = sqrt( DMatrix::GetEPS() );



  if (any( x < (xub-sqreps))|| any( x>(xlb+sqreps)) )
  {
     F3->Resize( nf, 1);
     fun(x, F3);
  }


  for (i=1;i<=nvar;i++)
  {
     for(I=0; I<nnz; I++)
     {
         if( jAcol[I]==i ) {
            if (iflag) {
                      delj = sqreps;
                      xs   = x(i);
                      if (xs< xub(i)-delj  || (xub(i)==xlb(i)))
                      {
		          x(i) += delj;
      		          fun( x, F1 );
                      }
                      if (xs> xlb(i)+delj || (xub(i)==xlb(i)))
                      {
                         x(i)  = xs-delj;
                         fun( x, F2 );
                      }
                      x(i) = xs;
                      iflag=0;
            }
            if ((xs< (xub(i)-delj) && (xs> xlb(i)+delj))  || (xub(i)==xlb(i))) {
            	// Use central difference formula
            	nzvalue[I] = ((*F1)(iArow[I]) - (*F2)(iArow[I]))/(2*delj);
            }
            else if (xs>= (xub(i)-delj) ) {
            	// Use backward difference formula
            	nzvalue[I] = ((*F2)(iArow[I]) - (*F3)(iArow[I]))/(-delj);
            }
            else if (xs<= (xlb(i)+delj) ) {
            	// Use forward difference formula
            	nzvalue[I] = ((*F1)(iArow[I]) - (*F3)(iArow[I]))/(delj);
            }

         }
     }
     iflag=1;

  }
}

void deleteIndexGroups(IGroup* igroup, int ncols )
{
   int i;

   for(i=0;i< ncols;i++)
   {
         delete[] igroup->colindex[i];
   }

   delete[] igroup->size;

   delete[] igroup->colindex;
}


void getIndexGroups( IGroup* igroup, int nrows, int ncols, int nnz, int* iArow, int* jAcol, Workspace* workspace)
{
/* This function uses the method of Curtis, Powell and Reid (1974) to find groups of variables
 * to evaluate efficiently the sparse Jacobian by perturbing simultaneously groups of variables.
 * Reference:
 * A. R. Curtis, M.J.D. Powell and J.K. Reid
 * "On the estimation of Sparse Jacobian Matrices"
 * J Inst Maths Applics (1974) 13, 117-119
 *
 */

   int i, j, l, q, r;
   int group_index;
   DMatrix& C1 = *workspace->JacCol1;
   DMatrix& C2 = *workspace->JacCol2;

   double dotCols;

   // Form dummy Jacobian matrix with ones at the non-zero elements

   double* ones_pr = workspace->jac_Gij;


   for(i=0;i<nnz;i++)  ones_pr[i] = 1.0;

   SparseMatrix J(ones_pr, nrows, ncols, nnz, iArow, jAcol );

//   J.Save("J.dat");

//   J.SaveSparsityPattern("pattern.txt");

   // Now allocate the pointer to the groups

   igroup->colindex = new int*[ncols];
   int* col_done  = new int[ncols];


   for(i=0;i<ncols;i++) col_done[i]=0;

   igroup->size = new int[ncols];

   for(i=0;i< ncols;i++) {
      igroup->colindex[i] = new int[ncols];
      igroup->size[i]     = 0;
   }


// To form the first group we inspect the columns
//  in turn and include each that has no unknowns in common with those columns already
//  included.
   // Add the first column to the first group
   igroup->colindex[0][0] = 1;
   col_done[0]=1;
   int gcount   = 1;
   int colcount = 1;


   // Now form the first group
   bool ok;
   for(j=2;j<=ncols;j++) {
          ok = true;
	  for(l=0;l<gcount;l++) {
          	if (j== igroup->colindex[0][l]) {
                        ok=false;
               		break;
                }
                C1 = J.Column(igroup->colindex[0][l]);
                C2 = J.Column(j);
                dotCols = dot( C1, C2);
//                sprintf(workspace->text,"\nz=%d, j=%d, dotCols=%f",igroup->colindex[0][l],j,dotCols);
          	if  ( dotCols>0.0 ) {
                        ok=false;
          		break;
		}
          }
  	  if (ok) {
                	igroup->colindex[0][gcount]=j;
                	gcount++;
                	colcount++;
                        col_done[j-1]=1;
          }

   }


   igroup->size[0]= gcount;

// Now form the remaining groups.
//The other groups are formed successively by applying the same procedure to
//those columns not already included in a group.


   group_index=0;
   while (colcount<ncols) {
        group_index++;
        gcount = 0;
   	for(j=2;j<=ncols;j++) {
                  ok=true;
	 	  for(q = 0; q< group_index; q++)
                  {
			for(r=0; r< igroup->size[q]; r++) {

				if(j==igroup->colindex[q][r]) {
                                	ok=false;
                                        break;
                                }
			}
			if (ok==false)
				break;
                  }
		  for(l=0;l<gcount;l++) {

          		if (j== igroup->colindex[group_index][l]) {
                                ok=false;
               			break;
                        }
                        C1 = J.Column(igroup->colindex[group_index][l]);
                        C2 = J.Column(j);
                        dotCols = dot( C1, C2 );
          	  	if ( dotCols>0.0 ) {
                                ok = false;
                                break;
                  	}
                  }
                  if (ok) {
                	igroup->colindex[group_index][gcount]=j;
                	gcount++;
                        colcount++;
                        col_done[j-1]=1;
          	  }
   	}



        igroup->size[group_index] = gcount;

    }



    for(i=0;i<ncols;i++)
    {
         if (igroup->size[i]==0)
         {
		igroup->number=i;
                break;
         }
    }



    sprintf(workspace->text,"\nNumber of index sets for sparse finite differences = %i\n", igroup->number);
    psopt_print(workspace,workspace->text);
// Now, lets print our groupings to check
//    full(J).Print("Dummy J");
//     for (l=0; l<(igroup->number); l++) {
//          for(j=0; j< igroup->size[l]; j++ ) {
//               sprintf(workspace->text, "\n colindex[%i][%i] = %i", l, j, igroup->colindex[l][j]);
//          }
//      }
//      sprintf(workspace->text,"\n");
//     exit(0);



}

void EfficientlyComputeJacobianNonZeros( void fun(DMatrix& x, DMatrix* f, Workspace* ), DMatrix& x, int nf,
            double *nzvalue, int nnz, int* iArow, int* jAcol, IGroup* igroup, GRWORK* grw, Workspace* workspace )
{
/* This function uses the method of Curtis, Powell and Reid (1974) to
 * evaluate efficiently the sparse Jacobian by perturbing simultaneously groups of variables.
 * Reference:
 * A. R. Curtis, M.J.D. Powell and J.K. Reid
 * "On the estimation of Sparse Jacobian Matrices"
 * J Inst Maths Applics (1974) 13, 117-119
 *
 */

  int  j, k, i;
  double delj;
  double sqreps;
  long nvar= x.GetNoRows();

  DMatrix *F1   = grw->F1;
  DMatrix *F2   = grw->F2;


  DMatrix xp(nvar,1);

  F1->Resize(   nf, 1 );
  F2->Resize(   nf, 1 );

  sqreps = sqrt( DMatrix::GetEPS() );

  for (i=0;i<igroup->number;i++)
  {
        xp=x;
	for(j=0; j<igroup->size[i]; j++) {
                      delj = sqreps;
		      xp(igroup->colindex[i][j]) += delj;
        }
        fun( xp, F1, workspace );
        for(j=0; j<igroup->size[i]; j++) {
                      delj=sqreps;
                      xp(igroup->colindex[i][j]) -= 2*delj;
        }
        fun( xp, F2, workspace );
        for(j=0; j<igroup->size[i]; j++) {
             for(k=0;k<nnz;k++) {
              	if (jAcol[k] == igroup->colindex[i][j] )
                {
                     nzvalue[k] = ((*F1)(iArow[k]) - (*F2)(iArow[k]))/(2*delj);
                }
             }
        }
   }

}


void DetectJacobianSparsity(void fun(DMatrix& x, DMatrix* f, Workspace* ), DMatrix& x, int nf,
                           int* nnzA, int* iArow, int* jAcol, double* Aij,
                           int* nnzG, int* jGrow, int* jGcol,
                           GRWORK* grw, Workspace* workspace)
{



  long nvars = x.GetNoRows();
  long i,j;
  int nzcount_A=0;
  int nzcount_G=0;
  double s = 1.0e6*sqrt(DMatrix::GetEPS());
//  double tol  = pow( DMatrix::GetEPS(), 0.8)* MAX( 1.0, enorm(x) );
  double tol  = 1.e-16*pow( DMatrix::GetEPS(), 0.8)* MAX( 1.0, enorm(x) );





  DMatrix& JacCol1 = *workspace->JacCol1;
  DMatrix& JacCol2 = *workspace->JacCol2;
  DMatrix& JacCol3 = *workspace->JacCol3;
  DMatrix& xp      = *workspace->xp;
  DMatrix& xlb     = *workspace->xlb;
  DMatrix& xub     = *workspace->xub;

  for(j=1;j<=nvars;j++) {

     xp = x;
#ifndef TESTING_HESSIAN
     clip_vector_given_bounds( xp, xlb, xub);
#endif
     JacobianColumn( fun, xp, xlb, xub, j, &JacCol1,  grw, workspace);
     xp = x + 0.1*Abs(x) + s*ones(nvars,1);
#ifndef TESTING_HESSIAN
     clip_vector_given_bounds( xp, xlb, xub);
#endif
     JacobianColumn( fun, xp, xlb, xub, j, &JacCol2,  grw, workspace);
     xp = x - 0.15*Abs(x) - 1.1*s*ones(nvars,1);
#ifndef TESTING_HESSIAN
     clip_vector_given_bounds( xp, xlb, xub);
#endif
     JacobianColumn( fun, xp, xlb, xub,j, &JacCol3, grw, workspace);



      for(i=1; i<=nf; i++) {
            if ( ( fabs(JacCol1(i,1)) +  fabs(JacCol2(i,1)) + fabs(JacCol3(i,1)) )>=tol ) {
              if ( fabs(JacCol1(i,1)-JacCol2(i,1))<=tol && fabs(JacCol1(i,1)-JacCol3(i,1))<=tol ) {
//            if ( JacCol1(i,1)!=0.0 || JacCol2(i,1)!=0.0 || JacCol3(i,1)!=0.0 ) {
//              if ( JacCol1(i,1)==JacCol2(i,1) && JacCol2(i,1)==JacCol3(i,1) ) {
                        // Constant Jacobian element detected
              		iArow[nzcount_A]=(int) i;
              		jAcol[nzcount_A]=(int) j;
                        Aij[nzcount_A]    = JacCol1(i,1);

                        nzcount_A++;
              }
              else {
		       // Non-constant Jacobian element
			jGrow[nzcount_G]= (int) i;
                        jGcol[nzcount_G]= (int) j;
                        nzcount_G++;
              }
           }
      }

  }
  *nnzA=nzcount_A;
  *nnzG=nzcount_G;

  workspace->jac_nnz  = nzcount_A + nzcount_G;
  workspace->jac_nnzA = nzcount_A;
  workspace->jac_nnzG = nzcount_G;

}


void ScalarGradient( double (*fun)(DMatrix& x, Workspace* workspace), DMatrix& x,
                DMatrix* grad, GRWORK* grw, Workspace* workspace )
{

  int j = 0, nf;
  double delj;
  double sqreps;
  double xs = 0.0;
  long nvar= x.GetNoRows();
  nf  = 1;
  double F1 = 0.0;
  double F2 = 0.0;
  double F3 = 0.0;
  double dfdx = 0.0;;

  DMatrix& xlb    = *workspace->xlb;
  DMatrix& xub    = *workspace->xub;

  sqreps=sqrt( DMatrix::GetEPS() );

  if (any( x <= (xub-sqreps))|| any( x>=(xlb+sqreps)) )
  {
     F3 = fun(x, workspace);
  }

  for(j=1;j<=nvar;j++) {
      delj = sqreps*(1.0+fabs(x(j)));
      xs   = x(j);
      if (xs< xub(j)-delj || (xub(j)==xlb(j)))
      {
      	x(j) += delj;
      	F1 = fun( x, workspace);
      }
      if (xs> xlb(j)+delj || (xub(j)==xlb(j)))
      {
      	x(j) = xs-delj;
      	F2 = fun( x, workspace );
      }
      if (( (xs< (xub(j)-delj)) && (xs> (xlb(j)+delj)) ) || (xub(j)==xlb(j)) ) {
        // Use central difference formula
      	dfdx = ( F1 - F2 )/(2*delj);
      }
      else if (xs>= (xub(j)-delj)) {
        // Variable at upper bound, use backward difference formula
      	dfdx = ( F2 - F3 )/(-delj);
      }
      else if (xs<= (xlb(j)+delj)) {
        // Variable at lower bound, use forward difference formula
      	dfdx = ( F1 - F3 )/(delj);
      }
      x(j) = xs;
      (*grad)(j) = dfdx;
  }


}

void ScalarGradientAD( adouble (*fun)(adouble *, Workspace*), DMatrix& x, DMatrix* grad, bool* trace_done, int itag, Workspace* workspace )
{
    // Compute the gradient of a scalar function using automatic differentiation
    int      n = length(x);
    int i;
    double  yp = 0.0;
    adouble *xad = workspace->xad;
    adouble  yad;

    if( !(*trace_done) ) {
    	trace_on(itag);
    	for(i=0;i<n;i++) {
    		xad[i] <<= (x.GetPr())[i];
	}
    	yad = (*fun)(xad, workspace);
    	yad >>= yp;
    	trace_off();
        *trace_done = true;
    }

    gradient(itag,n,x.GetPr(),grad->GetPr());

}


void compute_jacobian_of_constraints_with_respect_to_variables(DMatrix& Jc, DMatrix& X, DMatrix& XL, DMatrix& XU, Workspace* workspace)
{

    Alg& algorithm = *workspace->algorithm;
    Prob& problem  = *workspace->problem;

    int i, j, l, k;

    int nvars = get_number_nlp_vars(problem, workspace);

    int ncons = get_number_nlp_constraints(problem, workspace);

    DMatrix Jctmp(ncons,nvars);

    Jc.Resize(ncons,nvars);

    DMatrix& JacCol1 = *workspace->JacCol1;
    DMatrix& xlb     = *workspace->xlb;
    DMatrix& xub     = *workspace->xub;
    DMatrix& xp      = *workspace->xp;
    DMatrix jtemp;

    workspace->use_constraint_scaling = 0;

     xp = X;
//     clip_vector_given_bounds( xp, xlb, xub);

     if ( 0*useAutomaticDifferentiation(algorithm) ) {

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

    for(j=0;j<nvars;j++)
    {
       Jctmp( jac_rind[j]+1, jac_cind[j]+1) = jac_values[j];
    }


   }
   else {

    	DMatrix& xlb = *(workspace->xlb);
	    DMatrix& xub = *(workspace->xub);
	    for(j=1;j<=nvars;j++) {
	      JacobianColumn( gg_num, xp, xlb, xub,j, &JacCol1, workspace->grw, workspace);
	      Jctmp(colon(),j) = JacCol1;
	    }

   } // end if-else


   DMatrix& lambda = *workspace->lambda;

   int icount=1;

   int lam_phase_offset=0;

   int iphase;

   for(iphase=1;iphase<=problem.nphases;iphase++) {
       int ncons_phase_i =  get_ncons_phase_i(problem,iphase-1, workspace);
       for(j=1;j<=ncons_phase_i;j++) {
           int nstates = problem.phases(iphase).nstates;
           int norder  = problem.phases(iphase).current_number_of_intervals;
           i = lam_phase_offset + j;
           if (j<= nstates*(norder+1)) {
           // Copy rows corresponding to differential defect constraints.
              for (k=1;k<=nvars;k++) {
                    Jc(icount,k) = Jctmp(i,k);
              }
              icount++;
           }
           else if( j< ncons_phase_i) { // discard constraint tf>=t0 for each phase
              if ( lambda(i)!=0.0 ) {
                // Only copy rows corresponding to active inequality constraints.
                 for (k=1;k<=nvars;k++) {
                    Jc(icount,k) = Jctmp(i,k);
                 }
                 icount++;
              }
           }
       }
       lam_phase_offset+= ncons_phase_i;
   }

   Jc = Jc(colon(1,icount-1), colon());

   workspace->use_constraint_scaling = 1;

}


void compute_jacobian_of_residual_vector_with_respect_to_variables(DMatrix& Jr, DMatrix& X, DMatrix& XL, DMatrix& XU, Workspace* workspace)
{
    DMatrix Jcol;
	Prob & problem = *(workspace->problem);
    int nvar, nr, iphase, pindex, j;

	nvar = get_number_nlp_vars(problem, workspace);

	nr    = 0;

	for(iphase=1; iphase<=problem.nphases;iphase++)
	{
	  nr    += problem.phases(iphase).nobserved*problem.phases(iphase).nsamples;
	}

	Jr.Resize(nr,nvar);
	Jcol.Resize(nr,1);


	for(j=1;j<=nvar;j++) {
	    JacobianColumn( rr_num, X, XL, XU, j, &Jcol, workspace->grw, workspace);
	    Jr(colon(),j) = Jcol;
	}
}
