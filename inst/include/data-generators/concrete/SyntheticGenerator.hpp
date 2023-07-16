
/*
 * Copyright (c) 2017-2023 King Abdullah University of Science and Technology,
 * All rights reserved.
 * ExaGeoStat is a software package, provided by King Abdullah University of Science and Technology (KAUST).
 */

/**
 * @file SyntheticGenerator.hpp
 * @brief A class for generating synthetic data.
 * @version 1.0.0
 * @author Sameh Abdulah
 * @date 2023-02-14
**/

#ifndef EXAGEOSTAT_CPP_SYNTHETICGENERATOR_HPP
#define EXAGEOSTAT_CPP_SYNTHETICGENERATOR_HPP

#include <data-generators/DataGenerator.hpp>

namespace exageostat {
    namespace generators {
        namespace synthetic {

            /**
             * @class SyntheticGenerator
             * @brief A class for generating synthetic data.
             * @tparam T Data Type: float or double
             * @details This class generates synthetic data for use in testing machine learning models.
             *
             */
            template<typename T>
            class SyntheticGenerator : public DataGenerator<T> {

            public:

                /**
                 * @brief Get a pointer to the singleton instance of the SyntheticGenerator class.
                 * @param[in] apConfigurations A pointer to the configurations for the synthetic data.
                 * @return A pointer to the instance of the SyntheticGenerator class.
                 *
                 */
                static SyntheticGenerator<T> *GetInstance();

                /**
                 * @brief Initialize a vector with a given size to contain zeros.
                 * @param[in] apTheta A reference to the vector to initialize.
                 * @param[in] aSize The size of the vector to initialize.
                 * @return A reference to the initialized vector.
                 *
                 */
                static std::vector<double>& InitTheta(std::vector<double> &apTheta, int &aSize);

                /**
                 * @brief Generates the data locations.
                 * @details This method generates the X, Y, and Z variables used to define the locations of the data points.
                 * @return void
                 *
                 */
                void
                GenerateLocations() override;

                /**
                 * @brief Generates the data descriptors.
                 * @details This method generates the descriptors used to define the properties of the data points.
                 * @return void
                 *
                 */
                void GenerateDescriptors() override;

                /**
                 * @brief Destroy the data descriptors.
                 * @details This method destroys the descriptors used to define the properties of the data points.
                 * @return void
                 *
                 */
                void DestroyDescriptors() override;

                /**
                 * @brief Generates the data observations.
                 * @details This method generates the observations of the data points, which are used to train and test the model.
                 * @return void
                 *
                 */
                void GenerateObservations() override;

                /**
                 * @brief Generate uniform distribution between rangeLow , rangeHigh.
                 * @param[in] aRangeLow The Lower range.
                 * @param[in] aRangeHigh The Higher range.
                 * @return The scaled uniform distribution between the two bounds.
                 *
                 */
                static double UniformDistribution(const double &aRangeLow, const double &aRangeHigh);

                /**
                 * @brief Sort locations in Morton order (input points must be in [0;1]x[0;1] square]).
                 * @param[in] aN The problem size divided by P-Grid.
                 * @return void
                 *
                 */
                void SortLocations(int &aN);

                /**
                 * @brief Spread bits by three spaces.
                 * @param[in] aInputByte The input 64 bit to be spread.
                 * @return The byte after being spread.
                 *
                 */
                static uint64_t SpreadBits(uint64_t aInputByte);

                /**
                 * @brief Reverse Spread bits operation.
                 * @param[in] aInputByte The input spread 64 bit to be compacted.
                 * @return The byte after being compacted.
                 *
                 */
                static uint64_t ReverseSpreadBits(uint64_t aInputByte);

                /**
                 * @brief Compares two Unit64 values
                 * @param[in] aFirstValue Constant reference to the first input 64 bit value.
                 * @param[in] aSecondValue Constant reference to the second input 64 bit value.
                 * @return True if the second value is bigger than the first value, false otherwise.
                 *
                 */
                static bool CompareUint64(const uint64_t &aFirstValue, const uint64_t &aSecondValue);

                /**
                 * @brief Release the singleton instance of the SyntheticGenerator class.
                 * @return void
                 *
                 */
                static void ReleaseInstance();

            private:
                /**
                 * @brief Constructor for the SyntheticGenerator class.
                 * @param[in] apConfigurations A pointer to the configurations for the synthetic data.
                 * @return void
                 *
                 */
                SyntheticGenerator();

                /**
                 * @brief Virtual destructor to allow calls to the correct concrete destructor.
                 *
                 */
                ~SyntheticGenerator() override;

                /**
                 * @brief Pointer to the singleton instance of the SyntheticGenerator class.
                 *
                 */
                static SyntheticGenerator<T> *mpInstance;

            };

            /**
             * @brief Instantiates the Synthetic Data Generator class for float and double types.
             * @tparam T Data Type: float or double
             *
             */
            EXAGEOSTAT_INSTANTIATE_CLASS(SyntheticGenerator)
        } // namespace synthetic
    } // namespace generators
} // namespace exageostat

#endif //EXAGEOSTAT_CPP_SYNTHETICGENERATOR_HPP