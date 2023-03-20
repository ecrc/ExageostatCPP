
// Copyright (c) 2017-2023 King Abdullah University of Science and Technology,
// Copyright (C) 2023 by Brightskies inc,
// All rights reserved.
// ExaGeoStat is a software package, provided by King Abdullah University of Science and Technology (KAUST).

/**
 * @file ChameleonAllocateDescriptors.cpp
 *
 * @version 1.0.0
 * @author Sameh Abdulah
 * @date 2023-03-20
**/

#include <linear-algebra-solvers/concrete/dense/ChameleonAllocateDescriptors.hpp>
#include <iostream>

using namespace exageostat::linearAlgebra::dense;

void ChameleonAllocateDescriptors::InitiateDescriptors() {
    std::cout << "From Chameleon" << std::endl;
}

ChameleonAllocateDescriptors::ChameleonAllocateDescriptors() {
    this->InitiateDescriptors();
}
