
// Copyright (c) 2017-2023 King Abdullah University of Science and Technology,
// All rights reserved.
// ExaGeoStat is a software package, provided by King Abdullah University of Science and Technology (KAUST).

/**
 * @file BivariateMaternFlexible.hpp
 * @brief Defines the BivariateMaternFlexible class, a Bivariate Matern Flexible kernel.
 * @version 1.0.0
 * @author Suhas Shankar
 * @author Mary Lai Salvana
 * @author Sameh Abdulah
 * @author Mahmoud ElKarargy
 * @date 2023-04-14
**/

#ifndef EXAGEOSTATCPP_BIVARIATEMATERNFLEXIBLE_HPP
#define EXAGEOSTATCPP_BIVARIATEMATERNFLEXIBLE_HPP

#include <kernels/Kernel.hpp>

namespace exageostat {
    namespace kernels {

        /**
         * @class BivariateMaternFlexible
         * @brief A class representing a Bivariate Matern Flexible kernel.
         * @details This class represents a Bivariate Matern Flexible, which is a subclass of the Kernel class.
         * It provides a method for generating a covariance matrix using a set of input locations and kernel parameters.
         *
         */
        template<typename T>
        class BivariateMaternFlexible : public Kernel<T> {

        public:

            /**
             * @brief Constructs a new BivariateMaternFlexible object.
             * @details Initializes a new BivariateMaternFlexible object with default values.
             */
            BivariateMaternFlexible();

            /**
             * @brief Virtual destructor to allow calls to the correct concrete destructor.
             *
             */
            ~BivariateMaternFlexible() override = default;

            /**
             * @brief Generates a covariance matrix using a set of locations and kernel parameters.
             * @copydoc Kernel::GenerateCovarianceMatrix()
             */
            void GenerateCovarianceMatrix(T *apMatrixA, int &aRowsNumber, int &aColumnsNumber, int &aRowOffset,
                                          int &aColumnOffset, dataunits::Locations<T> *apLocation1,
                                          dataunits::Locations<T> *apLocation2, dataunits::Locations<T> *apLocation3,
                                          T *aLocalTheta, int &aDistanceMetric) override;

            /**
             * @brief Creates a new BivariateMaternFlexible object.
             * @details This method creates a new BivariateMaternFlexible object and returns a pointer to it.
             * @return A pointer to the new BivariateMaternFlexible object.
             *
             */
            static Kernel<T> *Create();

        private:
            //// Used plugin name for static registration
            static bool plugin_name;
        };

        /**
         * @brief Instantiates the Data Generator class for float and double types.
         * @tparam T Data Type: float or double
         *
         */
        EXAGEOSTAT_INSTANTIATE_CLASS(BivariateMaternFlexible)
    }//namespace Kernels
}//namespace exageostat
#endif //EXAGEOSTATCPP_BIVARIATEMATERNFLEXIBLE_HPP
