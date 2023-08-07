
// Copyright (c) 2017-2023 King Abdullah University of Science and Technology,
// All rights reserved.
// ExaGeoStat is a software package, provided by King Abdullah University of Science and Technology (KAUST).

/**
 * @file UnivariateExpNonGaussian.cpp
 *
 * @version 1.0.0
 * @author Sameh Abdulah
 * @date 2023-04-14
**/

#include <kernels/concrete/UnivariateExpNonGaussian.hpp>

using namespace exageostat::kernels;
using namespace exageostat::dataunits;
using namespace std;

template<typename T>
UnivariateExpNonGaussian<T>::UnivariateExpNonGaussian() {
    this->mP = 1;
    this->mParametersNumber = 6;
}

template<typename T>
Kernel<T> *UnivariateExpNonGaussian<T>::Create() {
    return new UnivariateExpNonGaussian();
}

namespace exageostat::kernels {
    template<typename T> bool UnivariateExpNonGaussian<T>::plugin_name = plugins::PluginRegistry<exageostat::kernels::Kernel<T>>::Add(
            "UnivariateExpNonGaussian", UnivariateExpNonGaussian<T>::Create);
}

template<typename T>
void UnivariateExpNonGaussian<T>::GenerateCovarianceMatrix(T *apMatrixA, int &aRowsNumber, int &aColumnsNumber,
                                                           int &aRowOffset, int &aColumnOffset,
                                                           Locations<T> *apLocation1,
                                                           Locations<T> *apLocation2, Locations<T> *apLocation3,
                                                           T *aLocalTheta, int &aDistanceMetric) {

    int i, j;
    int i0 = aRowOffset;
    int j0 = aColumnOffset;
    double x0, y0, z0;
    double expr = 0.0;
    double sigma_square = 1;
    int flag = 0;

    for (i = 0; i < aRowsNumber; i++) {
        j0 = aColumnOffset;
        for (j = 0; j < aColumnsNumber; j++) {
            expr = this->CalculateDistance(apLocation1, apLocation2, i0, j0, aDistanceMetric, flag) / aLocalTheta[0];

            if (expr == 0)
                apMatrixA[i + j * aRowsNumber] = sigma_square /*+ 1e-4*/;
            else
                apMatrixA[i + j * aRowsNumber] = exp(-expr);

            j0++;
        }
        i0++;
    }
}