// Copyright (c) 2017-2023 King Abdullah University of Science and Technology,
// Copyright (C) 2023 by Brightskies inc,
// All rights reserved.
// ExaGeoStat is a software package, provided by King Abdullah University of Science and Technology (KAUST).

/**
 * @file TestBivariateMaternFlexible.cpp
 * @brief 
 * @version 1.0.0
 * @author Sameh Abdulah
 * @date 2023-05-09
**/

#include <libraries/catch/catch.hpp>
#include <configurations/data-generation/concrete/SyntheticDataConfigurations.hpp>
#include <data-generators/DataGenerator.hpp>

using namespace exageostat::configurations::data_configurations;
using namespace exageostat::linearAlgebra;
using namespace exageostat::common;
using namespace exageostat::generators;

void TEST_KERNEL_GENERATION_BivariateMaternFlexible() {

    // Create a unique pointer to a DataGenerator object
    std::unique_ptr<DataGenerator> synthetic_generator;

    // Create a new synthetic_data_configurations object with the provided command line arguments
    SyntheticDataConfigurations* synthetic_data_configurations = SyntheticDataConfigurations::GetInstance();

    synthetic_data_configurations->SetProblemSize(9);
    synthetic_data_configurations->SetKernel("BivariateMaternFlexible");
#ifdef EXAGEOSTAT_USE_CHAMELEON
    synthetic_data_configurations->SetDenseTileSize(5);
    synthetic_data_configurations->SetComputation(EXACT_DENSE);
#endif
#ifdef EXAGEOSTAT_USE_HiCMA
    synthetic_data_configurations->SetLowTileSize(5);
    synthetic_data_configurations->SetComputation(TILE_LOW_RANK);
#endif
    synthetic_data_configurations->SetDimension(Dimension2D);
    synthetic_data_configurations->SetIsSynthetic(true);
    synthetic_data_configurations->SetPrecision(DOUBLE);

    // Create the DataGenerator object
    synthetic_generator = synthetic_generator->CreateGenerator(synthetic_data_configurations);

    // Initialize the locations of the generated data
    synthetic_generator->GenerateLocations();
    synthetic_generator->GenerateDescriptors();

    auto descriptorC = synthetic_data_configurations->GetDescriptorC()[0];

    exageostat::dataunits::Locations *l1 = synthetic_generator->GetLocations();

    auto *initial_theta = (double *) malloc(3 * sizeof(double));

    initial_theta[0] = 1.0;
    initial_theta[1] = 0.1;
    initial_theta[2] = 0.5;

    // Set the dimensions of the covariance matrix
    int m = 5;
    int n = 5;

    auto linearAlgebraSolver = LinearAlgebraFactory<float>::CreateLinearAlgebraSolver(
            synthetic_data_configurations->GetComputation());
    linearAlgebraSolver->SetConfigurations(synthetic_data_configurations);
    auto *A = (double *) (starpu_data_handle_t) linearAlgebraSolver->EXAGEOSTAT_DATA_GET_ADDRESS((descriptorC), 0, 0);
    synthetic_generator->GetKernel()->GenerateCovarianceMatrix(A, m, n, 0, 0, l1, l1, nullptr, initial_theta, 0);

    // Define the expected output
    double expected_output_data[] = {1.000000, 0.000000, 0.000000, 0.000000, 0.000000,
                                     0.000000, 0.100000, 0.000000, 0.000000, 0.000000,
                                     0.000000, 0.000000, 1.000000, 0.000000, 0.000000,
                                     0.000000, 0.000000, 0.000000, 0.100000, 0.000000,
                                     0.000000, 0.000000, 0.000000, 0.000000, 1.000000};
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            double diff = A[i * n + j] - expected_output_data[i * n + j];
            if (expected_output_data[i * n + j] == 0.000000){
                if(std::isnan(A[i * n + j])){
                    REQUIRE(true);
                }
            }
            else{
                REQUIRE(diff == Approx(0.0).margin(1e-6));
            }
        }
    }
}

TEST_CASE("Bivariate Matern Flexible kernel test") {
    TEST_KERNEL_GENERATION_BivariateMaternFlexible();
}