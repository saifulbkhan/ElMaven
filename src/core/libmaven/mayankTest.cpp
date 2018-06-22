#include "../../../3rdparty/Eigen/Sparse"
#include <bits/stdc++.h>
using namespace std;
using namespace Eigen;


map<pair<int,int>,float> sparseRepresentation(float** a,int row, int col){
    map<pair<int,int>,float> sparseMatrix;
    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
            if(a[i][j]!=0){
                sparseMatrix.insert(make_pair(pair<int,int>(i,j),a[i][j]));
            }
        }
    }
    return sparseMatrix;
}

SparseMatrix<double> getSparseFromHash(map<pair<int,int>,float> hSparse, int n, int m){
    SparseMatrix<double> matA(n,m);
    map<pair<int,int>,float>::iterator itr;
    for(itr=hSparse.begin();itr!=hSparse.end();itr++){
        matA.coeffRef(itr->first.first, itr->first.second)=itr->second;
    }
    // cerr << matA << endl;
    return matA;
}
int main(){
    float** myTempArr;
    myTempArr=new float* [2];
    for(int i=0;i<2;i++){
        myTempArr[i]=new float[2];
    }
    myTempArr[0][0]=1;
    myTempArr[0][1]=-1;
    myTempArr[1][0]=1;
    myTempArr[1][1]=1;
    SparseMatrix<double> eigenSparse = getSparseFromHash(sparseRepresentation(myTempArr, 2,2),2,2);
    ConjugateGradient<SparseMatrix<double>, Eigen::Upper> solver;
    // HouseholderQR<SparseMatrix<double>> solver(eigenSparse);
    VectorXd b(2);
    b<< 1,2;
    cout << b<< endl;
    solver.compute(eigenSparse);
    VectorXd x=solver.solve(b);
    cout<<"change"<< endl;
    cout << x<< endl; 
    cout << endl;
    return 0;
}