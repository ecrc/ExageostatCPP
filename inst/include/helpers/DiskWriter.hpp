// Copyright (c) 2017-2023 King Abdullah University of Science and Technology,
// All rights reserved.
// ExaGeoStat is a software package, provided by King Abdullah University of Science and Technology (KAUST).

/**
 * @file DiskWriter.hpp
 * @brief Contains the definition of the DiskWriter class for writing data to disk.
 * @version 1.0.0
 * @author Sameh Abdulah
 * @date 2023-06-08
**/

#ifndef EXAGEOSTATCPP_DISKWRITER_HPP
#define EXAGEOSTATCPP_DISKWRITER_HPP

#include <iostream>

#include <common/Definitions.hpp>
#include <data-units/Locations.hpp>

namespace exageostat {
    namespace helpers {

        /**
         * @class DiskWriter
         * @brief A class for writting data to disk.
         * @tparam T Data Type: float or double
         *
         */
        template<typename T>
        class DiskWriter {
        public:

            /**
             * @brief Writes a matrix of vectors to disk.
             * @param[in] apMatrixPointer A pointer to the matrix data.
             * @param[in] apProblemSize The size of the problem.
             * @param[in] apP The number of processes.
             * @param[in] apLoggerPath The path to the logger file.
             * @param[in] apLocations A pointer to the Locations object.
             * @return void
             *
             */
            void static
            WriteVectorsToDisk(T *apMatrixPointer, const int *apProblemSize, const int *apP, std::string aLoggerPath,
                               exageostat::dataunits::Locations<T> *apLocations);
        };

        /**
          * @brief Instantiates the DiskWriter class for float and double types.
          * @tparam T Data Type: float or double
          *
          */
        EXAGEOSTAT_INSTANTIATE_CLASS(DiskWriter)
    }
}
#endif //EXAGEOSTATCPP_DISKWRITER_HPP
