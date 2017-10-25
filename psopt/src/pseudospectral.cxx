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


double delta(long l, long N)
{
      double delta_l;
      if (l == 0 || l==N)
            delta_l = 0.5;
      else
            delta_l = 1.0;
      return delta_l;
}

void diffmat_central_differences(DMatrix& D, DMatrix & x )
{

int i;

double h1, h2, h;

int N1 = length(x);

int N = N1-1;

// D = zeros(N1,N1);
D.Resize(N1,N1);
D.FillWithZeros();

h = x(2)-x(1);
D(1,1)  = -1/h;
D(1,2)  =  1/h;

for (i=2;i<=N;i++) {
    h2 = x(i+1)-x(i);
    h1 = x(i)-x(i-1);
    D(i,i+1) = 1/(h1+h2);
    D(i,i-1) = -1/(h1+h2);
}
h = x(N)-x(N+1);
D(N+1,N)  =1/h;
D(N+1,N+1)=-1/h;

}

void diffmat_lagrange3pt(DMatrix& D, DMatrix & x )
{

int i;

double h1, h2, h;

int N1 = length(x);

int N = N1-1;

//D = zeros(N1,N1);
D.Resize(N1,N1);
D.FillWithZeros();

h = x(2)-x(1);
D(1,1)  = -1/h;
D(1,2)  =  1/h;

for (i=2;i<=N;i++) {
    // 3 Point differentiation based on lagrange polynomial interpolation
    D(i,i-1)   =  (x(i)-x(i+1))/((x(i-1)-x(i))*(x(i-1)-x(i+1)));
    D(i,i)     =  (2*x(i)-x(i-1)-x(i+1))/((x(i)-x(i-1))*(x(i)-x(i+1)));
    D(i,i+1)   =  (x(i)-x(i-1))/((x(i+1)-x(i-1))*(x(i+1)-x(i)));

}
h = x(N)-x(N+1);
D(N+1,N)  =1/h;
D(N+1,N+1)=-1/h;

}


void diffmat_lagrange5pt(DMatrix& D, DMatrix & x )
{

int i;

double h1, h2, h;

int N1 = length(x);

int N = N1-1;

// D = zeros(N1,N1);
D.Resize(N1,N1);
D.FillWithZeros();

h = x(2)-x(1);
D(1,1)  = -1/h;
D(1,2)  =  1/h;

    // 3 Point differentiation based on lagrange polynomial interpolation
i=2;
    D(i,i-1)   =  (x(i)-x(i+1))/((x(i-1)-x(i))*(x(i-1)-x(i+1)));
    D(i,i)     =  (2*x(i)-x(i-1)-x(i+1))/((x(i)-x(i-1))*(x(i)-x(i+1)));
    D(i,i+1)   =  (x(i)-x(i-1))/((x(i+1)-x(i-1))*(x(i+1)-x(i)));

i=N;
    D(i,i-1)   =  (x(i)-x(i+1))/((x(i-1)-x(i))*(x(i-1)-x(i+1)));
    D(i,i)     =  (2*x(i)-x(i-1)-x(i+1))/((x(i)-x(i-1))*(x(i)-x(i+1)));
    D(i,i+1)   =  (x(i)-x(i-1))/((x(i+1)-x(i-1))*(x(i+1)-x(i)));

for (i=3;i<=(N-1);i++) {
    // 5 Point central differentiation based on lagrange polynomial interpolation
    double x1 = x(i-2);
    double x2 = x(i-1);
    double x3 = x(i);
    double x4 = x(i+1);
    double x5 = x(i+2);
    D(i,i-2)   =  -((x2-x3)*(x3-x4)*(x3-x5))/((x1-x2)*(x1-x3)*(x1-x4)*(x1-x5));
    D(i,i-1)   =  ((x1-x3)*(x3-x4)*(x3-x5))/((x1-x2)*(x2-x3)*(x2-x4)*(x2-x5));
    D(i,i)     =  (x3*(4*x3*x3+2*x4*x5-3*x3*(x4+x5))+x2*(-3*x3*x3-x4*x5+2*x3*(x4+x5))+x1*(-3*x3*x3+2*x3*x4+2*x3*x5-x4*x5-x2*(-2*x3+x4+x5)))/((x1-x3)*(x2-x3)*(x3-x4)*(x3-x5));
    D(i,i+1)   =  ((x1-x3)*(-x2+x3)*(x3-x5))/((x1-x4)*(-x2+x4)*(-x3+x4)*(x4-x5));
    D(i,i+2)   =  ((-x1+x3)*(-x2+x3)*(x3-x4))/((-x1+x5)*(-x2+x5)*(-x3+x5)*(-x4+x5));

}
h = x(N)-x(N+1);
D(N+1,N)  =1/h;
D(N+1,N+1)=-1/h;

}

void legendre_points(int N, DMatrix& x, DMatrix& w)
{
// Finds the roots of the Legendre polynomials in (-1,1), also known as the Legendre points,
// together with the corresponding weights for quadrature.
//
      DMatrix beta;

      int N1 = N+1;

      int i;

      DMatrix T1(N1,N1), T2(N1,N1), T(N1,N1);

      DMatrix V(N1,N1);

      DMatrix indx(N1,1);

      T1.FillWithZeros();
      T2.FillWithZeros();

      beta = 0.5*elemDivision( ones(N,1) , Sqrt(ones(N,1)-((2*(colon(1,N1-1)))^(-2))) );

      for(i=1;i<N1-1;i++) {
  	    T1(i,i+1) = beta(i);
            T2(i+1,i) = beta(i);
      }
      T  = T1 + T2;
      x = eig(T, &V);
      x = x.Column(1);
      sort(x,indx);
      w = 2.0*(V(1,indx)^2);

}

void lglnodes(int N, DMatrix& x, DMatrix& w, DMatrix& P, DMatrix& D, Workspace* workspace)
{
// Computes the Legendre-Gauss-Lobatto nodes, weights and the LGL Vandermonde
// matrix. The LGL nodes are the zeros of (1-x^2)*P'_N(x).
//
// Reference on LGL nodes and weights:
//   C. Canuto, M. Y. Hussaini, A. Quarteroni, T. A. Tang, "Spectral Methods
//   in Fluid Dynamics," Section 2.3. Springer-Verlag 1987
//


  long N1 = N+1;
  long k;
  long l,i,j;

  DMatrix xold, X, Xdiff, L;
//  Use the Chebyshev-Gauss-Lobatto nodes as the first guess
  x = cos(pi*colon(0,N)/N);
//  The Legendre Vandermonde Matrix
//  P = zeros(N1,N1);
//  w = zeros(N1,1);
  P.Resize(N1,N1);
  w.Resize(N1,N1);
  P.FillWithZeros();
  w.FillWithZeros();
// Compute P_(N) using the recursion relation
// Compute its first and second derivatives and
// update x using the Newton-Raphson method.
  xold = 2*ones(N1,1);
  while ( Max( Abs( x-xold ) ) > DMatrix::GetEPS() )
  {


       xold = x;
       P(colon(),1)=ones(N1,1); P(colon(),2)=x;

       for ( k = 2; k<=N; k++ )
       {
	   double kd = (double) k;
           P(colon(),k+1) = ( elemProduct((2*kd-1)*x,P(colon(),k)) - (kd-1)*P(colon(),k-1) )/kd;

       }

       double N1d = (double) N1;
       x = xold-elemDivision(  elemProduct(x,P(colon(),N1))-P(colon(),N) , N1d*P(colon(),N1) );

  }

  for( k=1; k<=N1; k++)
  {
     w(k) = 2/((N*N1)*P(k,N1)*P(k,N1) );
  }

  if ( workspace->differential_defects == "standard") {

	// Compute now the differentiation matrix D using the standard formula
//	X = zeros(N1,N1);
        X.Resize(N1,N1);
        X.FillWithZeros();
	for(k=1;k<=N1;k++)
	{
	X(colon(),k) = x;
	}

//	Xdiff= X-tra(X)+eye(N1);
        Xdiff.Resize(N1,N1);

        for(i=1;i<=N1;i++) {
                for(j=1;j<=N1;j++) {
                         double identity;
                         if (i==j) identity = 1.0; else identity=0.0;
                         Xdiff(i,j) = X(i,j)-X(j,i) + identity;
                }
        }

//	L = zeros(N1,N1);
        L.Resize(N1,N1);
        L.FillWithZeros();
	for(k=1;k<=N1;k++)
	{
	L(colon(),k) =  P(colon(),N1);
	}

	for (k=1;k<=N1*N1;k=k+N1+1)
	{
	L(k)=1;
	}

	D = elemDivision(L, elemProduct(Xdiff, tra(L) ) );
        for(i=1;i<=N1;i++) {
                for(j=1;j<=N1;j++) {
//                     D(i,j) = L(i,j)/(Xdiff(i,j)*L(j,i));
                }
        }

	for (k=1; k<=N1*N1; k=k+N1+1)
	{
	D(k) = 0.0;
	}

	D(1) = ((double) N1*N)/4.0;

	D(N1*N1)=-((double) N1*N)/4.0;

//	D=-D;
        for(i=1;i<=N1;i++) {
                for(j=1;j<=N1;j++) {
                     D(i,j) = -D(i,j);
                }
        }

  }

  else if (workspace->differential_defects == "reduced-roundoff") {

  // Use equation 2_4_35 from the book Canuto et al (2006)
	for(j=0;j<=N;j++) {
		for(l=0;l<=N;l++)  {
		if(j!=l) {
			D(j+1,l+1) = delta(l,N)*pow(-1.0,(double) (j+l))/(delta(j,N)*(x(j+1)-x(l+1)));
		}
		else {
			double sum=0.0;
			long i;
			for(i=0;i<=N;i++) {
				if (i != j) {
					sum += delta(i,N)*pow(-1.0,(double) (i+j))/(delta(j,N)*(x(j+1)-x(i+1)));
				}
			}
			D(j+1,l+1) = -sum;
		}
		}
	}
//	D = -D;
        for(i=1;i<=N1;i++) {
                for(j=1;j<=N1;j++) {
                     D(i,j) = -D(i,j);
                }
        }

  }

  else if (workspace->differential_defects == "central-differences") {
      diffmat_central_differences( D, x );
//      diffmat_lagrange5pt( D, x );
//      D=-D;
        for(i=1;i<=N1;i++) {
                for(j=1;j<=N1;j++) {
                     D(i,j) = -D(i,j);
                }
        }
  }

  else if (workspace->differential_defects == "Lagrange-3pt") {
      diffmat_lagrange3pt( D, x );
//      D=-D;
        for(i=1;i<=N1;i++) {
                for(j=1;j<=N1;j++) {
                     D(i,j) = -D(i,j);
                }
        }
  }

  return;



}

double cbar(long j,long N)
{
    double retval;

    if (j==0 || j==N)
        retval = 2.0;
    else
        retval = 1.0;

    return retval;
}


void cglnodes(int N, DMatrix& x, DMatrix& w,  DMatrix& D, Workspace* workspace)
{
// Computes the Chebyshev-Gauss-Lobatto nodes, weights and the first order
// differentiation matrix
//
// Reference
//   C. Canuto, M. Y. Hussaini, A. Quarteroni, T. A. Tang, "Spectral Methods
//   in Fluid Dynamics," Springer-Verlag 1987
//


  long N1 = N+1;
  long k;
  long i,j,l;

//  compute the Chebyshev-Gauss-Lobatto nodes
  x = cos(pi*colon(0,N)/N);

  w = zeros(N1,1);


  w(1)=pi/(2*N);

  for( k=2; k<=N1-1; k++)
  {
     w(k) = pi/N;
  }

  w(N1)=pi/(2*N);

  // Compute now the differentiation matrix D

  if ( workspace->differential_defects == "standard" ) {

	for(j=0;j<=N;j++) {
		for(l=0;l<=N;l++)  {
		if(j!=l)
			D(j+1,l+1)= -cbar(j,N)/(2*cbar(l,N))*pow(-1.0,(int) (j+l))/( sin((j+l)*pi/(2*N))*sin((j-l)*pi/(2*N)));
			else if ( j==l && 1<=j && l <= N-1)
			D(j+1,l+1) =  -x(j+1)/(2*pow( sin(j*pi/N), 2));
			else if ( j==0 && l==0 )
			D(j+1,l+1)=(2*pow(N,2.0)+1.0)/6.0;
			else if ( j==N && l==N)
				D(j+1,l+1)=-(2*pow(N,2.0)+1.0)/6.0;
		}
	}

//	D=-D;
        for(i=1;i<=N1;i++) {
                for(j=1;j<=N1;j++) {
                     D(i,j) = -D(i,j);
                }
        }

  }

  else if ( workspace->differential_defects == "reduced-roundoff") {

  // Use equation No. 2_4_35 from Canuto et al (2006).
	for(j=0;j<=N;j++) {
		for(l=0;l<=N;l++)  {
		if(j!=l) {
			D(j+1,l+1) = delta(l,N)*pow(-1.0,(int) (j+l))/(delta(j,N)*(x(j+1)-x(l+1)));
		}
		else {
			double sum=0.0;
			long i;
			for(i=0;i<=N;i++) {
				if (i != j) {
					sum += delta(i,N)*pow(-1.0,(int) (i+j))/(delta(j,N)*(x(j+1)-x(i+1)));
				}
			}
			D(j+1,l+1) = -sum;
		}
		}
	}
//	D = -D;
        for(i=1;i<=N1;i++) {
                for(j=1;j<=N1;j++) {
                     D(i,j) = -D(i,j);
                }
        }

  }

  else if (workspace->differential_defects == "central-differences") {
      diffmat_central_differences( D, x );
//      diffmat_lagrange5pt( D, x );
//      D=-D;
        for(i=1;i<=N1;i++) {
                for(j=1;j<=N1;j++) {
                     D(i,j) = -D(i,j);
                }
        }
  }


  return;

}
