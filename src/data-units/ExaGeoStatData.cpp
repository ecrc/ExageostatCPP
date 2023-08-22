
// Copyright (c) 2017-2023 King Abdullah University of Science and Technology,
// All rights reserved.
// ExaGeoStat is a software package, provided by King Abdullah University of Science and Technology (KAUST).

/**
 * @file ExaGeoStatData.cpp
 * @brief Contains the implementation of the ExaGeoStatData class.
 * @version 1.0.0
 * @author Sameh Abdulah
 * @author Mahmoud ElKarargy
 * @date 2023-07-21
**/

#include <data-units/ExaGeoStatData.hpp>

using namespace exageostat::dataunits;
using namespace exageostat::common;

template<typename T>
ExaGeoStatData<T>::ExaGeoStatData(int aSize, Dimension aDimension, hardware::ExaGeoStatHardware &apHardware) {
    this->mpLocations = new Locations<T>(aSize, aDimension);
    this->mpDescriptorData = new DescriptorData<T>(apHardware);
}

template<typename T>
ExaGeoStatData<T>::~ExaGeoStatData() {
    delete this->mpLocations;
    delete this->mpDescriptorData;
}

template<typename T>
Locations<T> *ExaGeoStatData<T>::GetLocations() {
    return this->mpLocations;
}

template<typename T>
void ExaGeoStatData<T>::SetLocations(Locations<T> &aLocation) {

    if (this->mpLocations) {
        delete this->mpLocations;
    }
    this->mpLocations = &aLocation;
    this->mpLocations->SetLocationX(*aLocation.GetLocationX());
    this->mpLocations->SetLocationY(*aLocation.GetLocationY());
    this->mpLocations->SetLocationZ(*aLocation.GetLocationZ());
}

template<typename T>
DescriptorData<T> *ExaGeoStatData<T>::GetDescriptorData() {
    return this->mpDescriptorData;
}

template<typename T>
void ExaGeoStatData<T>::SetMleIterations(int aMleIterations) {
    this->mMleIterations = aMleIterations;
}

template<typename T>
int ExaGeoStatData<T>::GetMleIterations() {
    return this->mMleIterations;
}

template<typename T>
void ExaGeoStatData<T>::CalculateMedianLocations(std::string &aKernelName, Locations<T> &apLocations) {

    if (aKernelName == "UnivariateMaternNonStationary") {

        T x_min = this->mpLocations->GetLocationX()[0], x_max = this->mpLocations->GetLocationX()[0],
                y_min = this->mpLocations->GetLocationY()[0], y_max = this->mpLocations->GetLocationY()[0],
                z_min, z_max;


        if (this->mpLocations->GetDimension() != common::Dimension2D) {
            z_min = this->mpLocations->GetLocationZ()[0];
            z_max = this->mpLocations->GetLocationZ()[0];
        }

        for (int i = 0; i < this->mpLocations->GetSize(); i++) {
            T x = this->mpLocations->GetLocationX()[i];
            T y = this->mpLocations->GetLocationY()[i];

            x_min = (x < x_min) ? x : x_min;
            x_max = (x > x_max) ? x : x_max;
            y_min = (y < y_min) ? y : y_min;
            y_max = (y > y_max) ? y : y_max;

            if (this->mpLocations->GetDimension() != common::Dimension2D) {
                T z = this->mpLocations->GetLocationX()[i];
                z_min = (z < z_min) ? z : z_min;
                z_max = (z > z_max) ? z : z_max;
            }
        }

        apLocations.GetLocationX()[0] = x_min + (x_max - x_min) / 2;
        apLocations.GetLocationY()[0] = y_min + (y_max - y_min) / 2;
        if (this->mpLocations->GetDimension() != common::Dimension2D) {
            apLocations.GetLocationZ()[0] = z_min + (z_max - z_min) / 2;
        }
    } else {
        apLocations.GetLocationX()[0] = 0.5;
        apLocations.GetLocationY()[0] = 0.5;
        if (this->mpLocations->GetDimension() != common::Dimension2D) {
            apLocations.GetLocationY()[0] = 0.5;
        }
    }
}