
// Copyright (c) 2017-2024 King Abdullah University of Science and Technology,
// All rights reserved.
// ExaGeoStat is a software package, provided by King Abdullah University of Science and Technology (KAUST).

/**
 * @file HicmaDescriptor.cpp
 * @brief Example file for the Hicma descriptor.
 * @version 1.1.0
 * @author Mahmoud ElKarargy
 * @date 2024-02-14
**/

#include <kernels/Kernel.hpp>
#include <data-units/DescriptorData.hpp>
#include <utilities/Logger.hpp>

using namespace std;

using namespace exageostat::common;
using namespace exageostat::kernels;
using namespace exageostat::dataunits;
using namespace exageostat::configurations;

/**
 * @brief Main entry point for the Hicma descriptor example in ExaGeoStat.
 * @details Initializes configurations tailored for Hicma usage, sets up hardware, creates a kernel, and demonstrates the creation and initialization of a Hicma descriptor.
 * The process includes checking for TILE_LOW_RANK computation mode, generating a data matrix, and setting up the Hicma descriptor with this matrix, showcasing how ExaGeoStat
 * handles hierarchical matrix representations for efficient large-scale computations.
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return An integer indicating the success or failure of the program. A return value of 0 indicates success, while any non-zero value indicates failure.
 *
 */

int main(int argc, char **argv) {

    LOGGER("** Example of Hicma Descriptor **")

    // Initialize  Synthetic Configuration
    Configurations configuration;
    configuration.InitializeArguments(argc, argv);

    //Check for TLR computation specific for HICMA descriptor
    if (configuration.GetComputation() != TILE_LOW_RANK) {
        LOGGER("You must provide TILE_LOW_RANK computation to initialize HICMA descriptor.")
        LOGGER("Consider adding \"--computation=tlr\" to the arguments")
        return 0;
    }
    // Initialize Hardware and Data
    auto hardware = ExaGeoStatHardware(configuration.GetComputation(), configuration.GetCoresNumber(),
                                       configuration.GetGPUsNumbers(), configuration.GetPGrid(),
                                       configuration.GetQGrid());

    unique_ptr<DescriptorData<double>> data = make_unique<DescriptorData<double>>();
    Kernel<double> *kernel = exageostat::plugins::PluginRegistry<Kernel<double>>::Create(
            configuration.GetKernelName(), configuration.GetTimeSlot());

    // Get arguments for Descriptors Initialization
    int kernel_variables_number = kernel->GetVariablesNumber();
    int config_problem_size = configuration.GetProblemSize();
    int config_full_problem_size = config_problem_size * kernel_variables_number;
    int config_dts = configuration.GetDenseTileSize();
    int config_p_grid = ExaGeoStatHardware::GetPGrid();
    int config_q_grid = ExaGeoStatHardware::GetQGrid();
    bool config_is_OOC = configuration.GetIsOOC();

    // Randomly Initialized Matrix of Data
    std::vector<double> matrix(config_problem_size * config_problem_size);
    for (int i = 0; i < config_problem_size; ++i) {
        matrix[i] = i;
    }

    // Set Data Descriptor
    data->SetDescriptor(HICMA_DESCRIPTOR, DESCRIPTOR_C, config_is_OOC, matrix.data(), EXAGEOSTAT_REAL_DOUBLE,
                        config_dts,
                        config_dts,
                        config_dts * config_dts, config_full_problem_size, config_full_problem_size, 0, 0,
                        config_full_problem_size,
                        config_full_problem_size, config_p_grid, config_q_grid);

    auto *HICMA_descriptorC = data->GetDescriptor(HICMA_DESCRIPTOR, DESCRIPTOR_C).hicma_desc;

    //Print Descriptor Attributes
    LOGGER(" Problem Size: " << HICMA_descriptorC->m)
    LOGGER(" Dense Tile Size: " << HICMA_descriptorC->mb)

    LOGGER(" Entire Number of Rows :" << HICMA_descriptorC->lm)
    LOGGER(" Entire Number of Columns :" << HICMA_descriptorC->ln)

    LOGGER(" Number of Sub-matrix Tile Rows: " << HICMA_descriptorC->mt)
    LOGGER(" Number of Sub-matrix Tile Columns: " << HICMA_descriptorC->nt)

    LOGGER(" Number of Rows of 2D distribution grid: " << HICMA_descriptorC->p)
    LOGGER(" Number of Rows of 2D distribution grid: " << HICMA_descriptorC->q)

    LOGGER(" Is Matrix Not Fit in Memory: " << HICMA_descriptorC->ooc)
    LOGGER(" Size including Padding: " << HICMA_descriptorC->bsiz)

    // Print Data Matrix of Descriptor
    LOGGER("** Data in Matrix:")
    LOGGER_2("", 0)
    auto *data_mat = (double *) HICMA_descriptorC->mat;
    for (int i = 0; i < config_problem_size; ++i) {
        LOGGER_PRECISION_1(" " << data_mat[i], 0)
    }

    delete kernel;
    return 0;
}
