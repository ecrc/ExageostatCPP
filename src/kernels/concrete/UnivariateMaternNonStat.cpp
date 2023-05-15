
// Copyright (c) 2017-2023 King Abdullah University of Science and Technology,
// Copyright (C) 2023 by Brightskies inc,
// All rights reserved.
// ExaGeoStat is a software package, provided by King Abdullah University of Science and Technology (KAUST).

/**
 * @file UnivariateMaternNonStat.cpp
 *
 * @version 1.0.0
 * @author Sameh Abdulah
 * @date 2023-04-14
**/

#include <kernels/concrete/UnivariateMaternNonStat.hpp>

using namespace exageostat::kernels;
using namespace exageostat::dataunits;
using namespace std;

UnivariateMaternNonStat::UnivariateMaternNonStat() {
    this->mP = 1;
    this->mParametersNumber = 8;
}

Kernel *UnivariateMaternNonStat::Create() {
    return new UnivariateMaternNonStat();
}

namespace exageostat::kernels {
    bool UnivariateMaternNonStat::plugin_name = plugins::PluginRegistry<exageostat::kernels::Kernel>::Add(
            "UnivariateMaternNonStat", UnivariateMaternNonStat::Create);
}

void UnivariateMaternNonStat::GenerateCovarianceMatrix(double *apMatrixA, int aRowsNumber, int aColumnsNumber,
                                                       int aRowOffset, int aColumnOffset, Locations *apLocation1,
                                                       Locations *apLocation2, Locations *apLocation3,
                                                       double *aLocalTheta, int aDistanceMetric) {
    double l1x, l1y, l2x, l2y;

    double expr = 0.0;
    double con, sigma_square, beta, nu;
    double a, b, c, d, e, f, g, h, ti, tj;

    a = aLocalTheta[0];
    b = aLocalTheta[1];
    d = aLocalTheta[2];
    e = aLocalTheta[3];
    f = aLocalTheta[4];
    g = aLocalTheta[5];
    h = aLocalTheta[6];
    ti = aLocalTheta[7];
    tj = 0;

    double nu_arr_1[aRowsNumber];
    double sigma_arr_1[aRowsNumber];
    double lambda_arr_1[aRowsNumber];
    double nu_arr_2[aColumnsNumber];
    double sigma_arr_2[aColumnsNumber];
    double lambda_arr_2[aColumnsNumber];

    for (int i = 0; i < aRowsNumber; i++) {
        l1x = apLocation1->GetLocationX()[i + aRowOffset];
        l1y = apLocation1->GetLocationY()[i + aRowOffset];
        nu_arr_1[i] = Neu(l1x, l1y, g, h, ti);
        sigma_arr_1[i] = Sigma(l1x, l1y, d, e, f);
        lambda_arr_1[i] = Lambda(l1x, l1y, a, b);
    }

    for (int j = 0; j < aColumnsNumber; j++) {
        l2x = apLocation2->GetLocationX()[j + aColumnOffset];
        l2y = apLocation2->GetLocationY()[j + aColumnOffset];

        nu_arr_2[j] = Neu(l2x, l2y, g, h, ti);
        sigma_arr_2[j] = Sigma(l2x, l2y, d, e, f);
        lambda_arr_2[j] = Lambda(l2x, l2y, a, b);
    }

    for (int i = 0; i < aRowsNumber; i++) {
        l1x = apLocation1->GetLocationX()[i + aRowOffset];
        l1y = apLocation1->GetLocationY()[i + aRowOffset];

        for (int j = 0; j < aColumnsNumber; j++) {
            l2x = apLocation2->GetLocationX()[j + aColumnOffset];
            l2y = apLocation2->GetLocationY()[j + aColumnOffset];

            double term1 = (sigma_arr_1[i]) * (sigma_arr_2[j]) * sqrt(lambda_arr_1[i]) * sqrt(lambda_arr_2[j]);
            double term2 = 2 / ((lambda_arr_1[i]) + (lambda_arr_2[j]));
            double neuij = ((nu_arr_1[i]) + (nu_arr_2[j])) / 2;
            double Qij = CalculateMahalanobisDistanceSquared(l1x, l1y, l2x, l2y, term2, 0, 0, term2);
            double prod1 = 2 * sqrt(neuij * Qij);
            double term3 = MaternUtil(1, neuij, prod1);
            apMatrixA[i + j * aRowsNumber] = term1 * term2 * term3;
        }
    }
}

double UnivariateMaternNonStat::Neu(double x, double y, double g, double h, double ti) {
    return (g * pow(POW_e, h * (x + y)) + ti);
}

double UnivariateMaternNonStat::Sigma(double x, double y, double d, double e, double f) {
    return (d * pow(POW_e, e * (x + y)) + f);
}

double UnivariateMaternNonStat::Lambda(double x, double y, double a, double b) {

    return (a * pow(POW_e, sin(b * x) + sin(b * y)));
}

double UnivariateMaternNonStat::CalculateMahalanobisDistanceSquared(double x1, double y1, double x2,
                                           double y2, double a11, double a12,
                                           double a21, double a22) {

    double diffx = x1 - x2;
    double diffy = y1 - y2;

    double el1 = a11 * diffx + a21 * diffy;
    double el2 = a12 * diffx + a22 * diffy;

    double ans = el1 * diffx + el2 * diffy;

    return ans;
}

double UnivariateMaternNonStat::MaternUtil(double range, double smoothness, double distance) {
    double con = 0.0;
    con = pow(2, (smoothness - 1)) * tgamma(smoothness);
    con = 1.0 / con;

    if (distance == 0)
        return 1;
    else
        return con * pow(distance / range, smoothness)
               * gsl_sf_bessel_Knu(smoothness, distance / range); // Matern Function
}