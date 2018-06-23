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
void booleanUpdate(float w[],float intensity[], float baseline[],int L, float p){
    for(int i=0;i<L;i++){
        if(intensity[i]>baseline[i]){
            w[i]=p;
        }
        else if(intensity[i]<baseline[i]){
            w[i]=1-p;
        }
    }
}
int main(){
    float** myTempArr;
    myTempArr=new float* [3];
    for(int i=0;i<3;i++){
        myTempArr[i]=new float[3];
    }
    myTempArr[0][0]=1;
    myTempArr[0][1]=-1;
    myTempArr[0][2]=0;
    myTempArr[1][0]=-1;
    myTempArr[1][1]=2;
    myTempArr[1][2]=0;
    myTempArr[2][0]=0;
    myTempArr[2][1]=0;
    myTempArr[2][2]=3;
    SparseMatrix<double> eigenSparse = getSparseFromHash(sparseRepresentation(myTempArr, 3,3),3,3);
    ConjugateGradient<SparseMatrix<double>, Eigen::Upper> solver;
    // HouseholderQR<SparseMatrix<double>> solver(eigenSparse);
    VectorXd b(3);
    for(int i=0;i<3;i++){
        b[i]=(i+1)*5;
    }
    // b<< 5,10,15;
    cout << b<< endl;
    solver.compute(eigenSparse);
    VectorXd x=solver.solve(b);
    cout<<"change"<< endl;
    for(int i=0;i<x.size();i++){
        cout << x[i]<<" ";
    }
    // cout << x<< endl;
    cout << endl;
    float w[5]={0};
    float y[5]={7,5,12,69,40};
    float z[5]={50,70,99,1,45};
    booleanUpdate(w,y,z,5,0.01);
    for(int i=0;i<5;i++){
        cout << w[i]<< " ";
    }
    cout << endl;

    return 0;
}