
// Copyright (c) 2017-2023 King Abdullah University of Science and Technology,
// All rights reserved.
// ExaGeoStat is a software package, provided by King Abdullah University of Science and Technology (KAUST).

/**
 * @file stride-vec-codelet.hpp
 * @brief A class for starpu codelet stride-vec.
 * @version 1.1.0
 * @author Mahmoud ElKarargy
 * @author Sameh Abdulah
 * @date 2024-02-25
**/

#ifndef EXAGEOSTATCPP_STRIDE_VEC_CODELET_HPP
#define EXAGEOSTATCPP_STRIDE_VEC_CODELET_HPP

#include <common/Definitions.hpp>

namespace exageostat::runtime {

    /**
     * @class stride-vec Codelet
     * @brief A class for starpu codelet stride-vec.
     * @tparam T Data Type: float or double
     * @details This class encapsulates the struct cl_stride_vec and its CPU functions.
     *
     */
    template<typename T>
    class STRIDEVECCodelet {

    public:

        /**
         * @brief Default constructor
         *
         */
        STRIDEVECCodelet() = default;

        /**
         * @brief Default destructor
         *
         */
        ~STRIDEVECCodelet() = default;

        /**
         * @brief Inserts a task for STRIDE vector operation codelet processing.
         * @param[in] apDescA A pointer to the descriptor for the source vector.
         * @param[in,out] apDescB A pointer to the descriptor for the first destination vector.
         * @param[in,out] apDescC A pointer to the descriptor for the second destination vector.
         * @return void
         *
         */
        void InsertTask(const void *apDescA, void *apDescB, void *apDescC);

    private:

        /**
         * @brief Executes the STRIDE vector operation codelet function.
         * @param[in] apBuffers An array of pointers to the buffers.
         * @param[in] apCodeletArguments A pointer to the codelet arguments structure, which includes the vector size,
         * offset, and the stride factor.
         * @return void
         *
         */
        static void cl_stride_vec_function(void **apBuffers, void *apCodeletArguments);

        /// starpu_codelet struct
        static struct starpu_codelet cl_stride_vec;
    };

    /**
     * @brief Instantiates the stride-vec codelet class for float and double types.
     * @tparam T Data Type: float or double
     *
     */
    EXAGEOSTAT_INSTANTIATE_CLASS(STRIDEVECCodelet)

}//namespace exageostat

#endif //EXAGEOSTATCPP_STRIDE_VEC_CODELET_HPP
