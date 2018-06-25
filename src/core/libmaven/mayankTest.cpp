#include "../../../3rdparty/Eigen/Sparse"
#include <bits/stdc++.h>
using namespace std;
using namespace Eigen;


void printSparse(SparseMatrix<double> mat){
    cerr << "printSparse"<< endl;
    cerr << mat<< endl;
}

// 
SparseMatrix<double> sparseRepresentation(float** a,int row, int col){
    cerr << "sparseRepresentation" << endl;
    SparseMatrix<double> mat(row,col);
    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
            if(a[i][j]!=0){
                mat.coeffRef(i,j)=a[i][j];
            }
        }
    }
    return mat;
}

SparseMatrix<double> zeroDiagSparse(float a[], int n){
    cerr << "zeroDiagSparse" << endl;
    SparseMatrix<double> mat(n,n);
    for (int i=0;i<n;i++){
        mat.coeffRef(i,i)=a[i];
    }
    return mat;
}

void discreteMatrixDifference(float** a,int row, int column, int n){
    cerr << "discreteMatrixDifference" << endl;
    if(n==0){
        return;
    }
    // TODO: check for the number of columns if column < n
    // float returnMat[row][column-1];
    for(int i=0;i<row;i++){
        for(int j=0;j<column-1;j++){
            a[i][j]=a[i][j+1]-a[i][j];
        }
    }
    return discreteMatrixDifference(a, row, column-1, n-1);
}

SparseMatrix<double> matrixTranspose(SparseMatrix<double> D){
    cerr << "matrixTranspose" << endl;
    // SparseMatrix<double> Dt;
    // SparseMatrix<double>::iterator itr;
    // for(itr=D.begin();itr!=D.end();itr++){
    //     Dt.insert(make_pair(pair<int,int>(itr->first.second,itr->first.first),itr->second));
    // }
    // return Dt;
    return D.transpose();
}

// void printHashSparse(map<pair<int,int>,float> hSparse){
//     cerr << "printHashSparse" << endl;
//     map<pair<int,int>,float>::iterator itr;
//     for(itr=hSparse.begin();itr!=hSparse.end();itr++){
//         cerr << itr->first.first<<","<<itr->first.second << " " << itr->second<< endl;
//     }
// }

// Matrix square D*D.T
// Complexity O(mn*log(k!))
SparseMatrix<double> matrixSquare(SparseMatrix<double> mat1, SparseMatrix<double> mat2){
    cerr << "matrixSquare" << endl;
    // cout << "mat1"<< endl;
    // printHashSparse(hashMat1);
    // cout << "mat2"<< endl;
    // printHashSparse(hashMat2);
    // cout << "Prod"<< endl;
    // map<pair<int,int>,float> hashRes;
    // pair<int,int> x;
    // map<pair<int,int>,float>::iterator itr1,itr2,itr;
    // for(itr1=hashMat1.begin();itr1!=hashMat1.end();itr1++){
    //     for(itr2=hashMat2.begin();itr2!=hashMat2.end();itr2++){
    //         if(itr1->first.second==itr2->first.first){
    //             x=make_pair(itr1->first.first, itr2->first.second);
    //             itr=hashRes.find(x);
    //             if(itr==hashRes.end()){
    //                 hashRes.insert(make_pair(x,(float)itr1->second*itr2->second));
    //             }
    //             else{
    //                 itr->second+=(float)itr1->second*itr2->second;
    //             }
    //         }
    //     }
    // }
    // return hashRes;

    return mat1*mat2;
}

SparseMatrix<double> matrixSum(SparseMatrix<double> mat1, SparseMatrix<double> mat2){
    cerr << "matrixSum" << endl;
    // map<pair<int,int>,float>::iterator itr1, itr2, itr;
    // map<pair<int,int>,float> finalSum;
    // for(itr1=mat1.begin();itr1!=mat1.end();itr1++){
    //     finalSum.insert(make_pair(pair<int,int>(itr1->first.first,itr1->first.second),itr1->second));
    // }
    // for(itr2=mat2.begin();itr2!=mat2.end();itr2++){
    //     itr=finalSum.find(make_pair(itr2->first.first, itr2->first.second));
    //     if(itr==finalSum.end()){
    //         finalSum.insert(make_pair(pair<int,int>(itr2->first.first,itr2->first.second),itr2->second));
    //     }
    //     else{
    //         itr->second+=itr2->second;
    //     }
    // }
    // return finalSum;
    return mat1+mat2;
}

SparseMatrix<double> scalarMultiplication(float scalar, SparseMatrix<double> mat){
    cerr << "scalarMultiplication" << endl;
    // map<pair<int,int>,float>::iterator itr;
    // map<pair<int,int>,float> matc=mat;
    // for(itr=matc.begin();itr!=matc.end();itr++){
    //     itr->second*=scalar;
    // }
    // return matc;
    return mat*scalar;
}

void linearMulti(float a1[], vector<float> a2, float ans[], int n){
    cerr << "linearMulti" << endl;
    for(int i=0;i<n;i++){
        ans[i]=a1[i]*a2[i];
    }
}

// SparseMatrix<double> getSparseFromHash(map<pair<int,int>,float> hSparse, int n, int m){
//     cerr << "getSparseFromHash" << endl;
//     SparseMatrix<double> matA(n,m);
//     map<pair<int,int>,float>::iterator itr;
//     for(itr=hSparse.begin();itr!=hSparse.end();itr++){
//         matA.coeffRef(itr->first.first, itr->first.second)=itr->second;
//     }
//     // cerr << matA << endl;
//     return matA;
// }
vector<float> solveLinearSystem(SparseMatrix<double> hermitianMat, VectorXd b){
    cerr << "solveLinearSystem" << endl;
    ConjugateGradient<SparseMatrix<double>, Eigen::Upper> solver;
    solver.compute(hermitianMat);
    VectorXd x=solver.solve(b);
    vector<float> retVec;
    for(int i=0;i<x.size();i++){
        retVec.push_back((float)x[i]);
    }
    return retVec;
}
// void updateBaselineViaVector(vector<float> x){
//     cerr << "updateBaselineViaVector" << endl;
//     for(int i=0;i<x.size();i++){
//         baseline[i]=x[i];
//     }
// }
    // void booleanUpdate(float w[],int L, float p){
    //     cerr << "booleanUpdate" << endl;
    //     for(int i=0;i<L;i++){
    //         if(intensity[i]>baseline[i]){
    //             w[i]=p;
    //         }
    //         else if(intensity[i]<baseline[i]){
    //             w[i]=1-p;
    //         }
    //     }
    // }
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
    float vec[3]={1,2,3};
    SparseMatrix<double> eigenSparse = sparseRepresentation(myTempArr, 3,3);
    printSparse(eigenSparse);
    printSparse(zeroDiagSparse(vec, 3));
    printSparse(matrixTranspose(eigenSparse));
    printSparse(matrixSquare(eigenSparse,matrixTranspose(eigenSparse)));
    printSparse(eigenSparse+eigenSparse);
    printSparse(scalarMultiplication(0.1,eigenSparse));



    /*
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
    */
    return 0;
}