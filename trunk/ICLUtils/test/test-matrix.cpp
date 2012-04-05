/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/test/test-matrix.cpp                          **
** Module : ICLUtils                                               **
** Authors: Erik Weitnauer                                         **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <gtest/gtest.h>
#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/FixedMatrix.h>
#include <ICLUtils/TestAssertions.h>

using namespace icl;

TEST(Matrix, DecomposeQR) {
  FixedMatrix<icl32f,3,4> A(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
  FixedMatrix<icl32f,3,4> Q;
  FixedMatrix<icl32f,3,3> R;
  A.decompose_QR(Q, R);
  EXPECT_TRUE(isNear(A,Q*R,1e-6f));
}
/*
TEST(Matrix, DecomposeRQ) {
  FixedMatrix<icl32f,4,3> A(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
  FixedMatrix<icl32f,4,3> Q;
  FixedMatrix<icl32f,3,3> R;
  A.decompose_RQ(R, Q);
  EXPECT_TRUE(isNear(A,R*Q,1e-6f));
}
*/
TEST(Matrix, SVD) {
  FixedMatrix<icl32f,3,4> A(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
  FixedMatrix<icl32f,3,4> U;
  FixedColVector<icl32f,3> s;
  FixedMatrix<icl32f,3,3> V;
  A.svd(U,s,V);

  // singular values correctly sorted?
  EXPECT_GE(s[0],s[1]);
  EXPECT_GE(s[1],s[2]);
  // correct decomposition?
  FixedMatrix<icl32f,3,3> S(0.0);
  S(0,0) = s[0]; S(1,1) = s[1]; S(2,2) = s[2];
  EXPECT_TRUE(isNear(A,U*S*V.transp(),1e-6f));
  // orthogonal matrices?
  EXPECT_TRUE(isNear(V.transp()*V,FixedMatrix<icl32f,3,3>::id(),1e-6f));
}

TEST(Matrix, BigMatrixPseudoInverse) {
  srand ( 230880 ); int cols = 3; int rows = 5;
  DynMatrix<double> MatDbl( cols, rows );
  for ( unsigned int i(0); i < rows; ++i )
    for ( unsigned int j(0); j < cols; ++j )
        MatDbl(j,i) = rand() % 10;
  DynMatrix<double> IdentityDbl( cols, cols, 0.0 );
  for ( unsigned int i(0); i < cols; ++i )
    IdentityDbl( i, i ) = 1.0;
  double ErrorDbl = matrix_distance( MatDbl.big_matrix_pinv() * MatDbl, IdentityDbl );
  EXPECT_TRUE( ErrorDbl < 1e-14 );

  DynMatrix<float> MatFlt( cols, rows );
  for ( unsigned int i(0); i < rows; ++i )
    for ( unsigned int j(0); j < cols; ++j )
        MatFlt(j,i) = rand() % 10;
  DynMatrix<float> IdentityFlt( cols, cols, 0.0 );
  for ( unsigned int i(0); i < cols; ++i )
    IdentityFlt( i, i ) = 1.0;
  float ErrorFlt = matrix_distance( MatFlt.big_matrix_pinv() * MatFlt, IdentityFlt );
  EXPECT_TRUE( ErrorFlt < 1e-6f );
}
