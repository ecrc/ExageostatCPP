
// Copyright (c) 2017-2023 King Abdullah University of Science and Technology,
// All rights reserved.
// ExaGeoStat is a software package, provided by King Abdullah University of Science and Technology (KAUST).

/**
 * @file FunctionAdapter.hpp
 * @brief Header file for function adapters in the ExaGeoStat software.
 * @details  It provides declarations for functions that adapt and initialize statistical models or algorithms based on user-defined configurations.
 * @version 1.1.0
 * @author Mahmoud ElKarargy
 * @date 2024-01-29
**/

#ifndef EXAGEOSTATCPP_FUNCTIONSADAPTER_HPP
#define EXAGEOSTATCPP_FUNCTIONSADAPTER_HPP

#include <iostream>
#include <string>
#include <vector>

#include <configurations/Configurations.hpp>
#include <hardware/ExaGeoStatHardware.hpp>

namespace exageostat::adapters {

    /**
     * @brief Initializes and configures the R arguments for ExaGeoStat computations.
     * @details This function prepares the necessary configurations required by ExaGeoStat to perform statistical computations.
     * It includes setting up problem sizes, computational kernels, grid configurations, and other parameters essential
     * for the execution of the ExaGeoStat algorithms.
     * @param[in] aProblemSize The size of the problem to be solved.
     * @param[in] aKernelName The name of the computational kernel to be used.
     * @param[in] aTileSize A vector specifying the size of each tile in the computation.
     * @param[in] aP_QGrid A vector defining the P x Q process grid.
     * @param[in] aTimeSlot The time slot allocated for the computation.
     * @param[in] aComputation The type of computation to be performed (e.g., estimation, prediction).
     * @param[in] aPrecision The precision (e.g., single, double) of the computation.
     * @param[in] aCoresGPUsNumber A vector specifying the number of CPU cores or GPUs to be used.
     * @param[in] aBand The bandwidth of the problem, relevant for band-limited computations.
     * @param[in] aMaxRank The maximum rank for low-rank approximations.
     * @param[in] aInitialTheta A vector of initial values for the model parameters (theta).
     * @param[in] aLowerUpperBounds A 2D vector specifying the lower and upper bounds for model parameters.
     * @param[in] aEstimatedTheta A vector of estimated values for the model parameters after computation.
     * @param[in] aVerbose A string indicating the verbosity level of the output.
     * @param[in] aDimension The dimensionality of the problem (e.g., 2D, 3D).
     * @param[in] aMaxMleIterations The maximum number of iterations for the Maximum Likelihood Estimation (MLE) algorithm.
     * @param[in] aTolerance The tolerance threshold for convergence in iterative algorithms.
     * @param[in] aPrediction A vector indicating prediction locations or settings.
     * @return A pointer to a Configurations object containing the initialized settings.
     *
     */
    Configurations *
    R_InitializeArguments(const int &aProblemSize, const std::string &aKernelName, const std::vector<int> &aTileSize,
                         const std::vector<int> &aP_QGrid, const int &aTimeSlot, const std::string &aComputation,
                         const std::string &aPrecision, const std::vector<int> &aCoresGPUsNumber, const int &aBand,
                         const int &aMaxRank, const std::vector<double> &aInitialTheta,
                         const std::vector<std::vector<double>> &aLowerUpperBounds,
                         const std::vector<double> &aEstimatedTheta, const std::string &aVerbose,
                         const std::string &aDimension, const int &aMaxMleIterations, const double &aTolerance,
                         const std::vector<int> &aPrediction
    );

    /**
     * @brief Executes the ExaGeoStat main API functions based on provided hardware and configurations.
     * @details This function serves as an entry point to the ExaGeoStat library, allowing users to execute statistical models and algorithms by passing in hardware configurations and computational settings.
     * It integrates the hardware layer with the statistical computations, ensuring that the ExaGeoStat algorithms can efficiently utilize the available resources.
     * This function is crucial for the seamless operation of the ExaGeoStat software, facilitating the execution of complex statistical tasks on diverse hardware configurations.
     * @param[in] apHardware A pointer to an ExaGeoStatHardware object representing the hardware configuration.
     * @param[in] apConfigurations A pointer to a Configurations object containing the computational settings.
     */
    void R_ExaGeoStatAPI(ExaGeoStatHardware *apHardware, Configurations *apConfigurations);

}
#endif //EXAGEOSTATCPP_FUNCTIONSADAPTER_HPP