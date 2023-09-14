
// Copyright (c) 2017-2023 King Abdullah University of Science and Technology,
// All rights reserved.
// ExaGeoStat is a software package, provided by King Abdullah University of Science and Technology (KAUST).

/**
 * @file HicmaImplementation.cpp
 * @brief Sets up the HiCMA descriptors needed for the tile low rank computations in ExaGeoStat.
 * @version 1.0.0
 * @author Mahmoud ElKarargy
 * @author Sameh Abdulah
 * @date 2023-03-26
**/

#include <lapacke.h>

#include <linear-algebra-solvers/concrete/tile-low-rank/HicmaImplementation.hpp>
#include <data-units/DescriptorData.hpp>

using namespace std;

using namespace exageostat::linearAlgebra::tileLowRank;
using namespace exageostat::common;
using namespace exageostat::dataunits;
using namespace exageostat::kernels;
using namespace exageostat::helpers;
using namespace exageostat::configurations;

template<typename T>
void
HicmaImplementation<T>::InitiateDescriptors(Configurations &aConfigurations, DescriptorData <T> &aDescriptorData,
                                            T *apMeasurementsMatrix) {

    // Check for initialize the Hicma context.
    if (!this->mpContext) {
        throw std::runtime_error(
                "ExaGeoStat hardware is not initialized, please use 'ExaGeoStat<double/float>::ExaGeoStatInitializeHardware(configurations)'.");
    }

    // Create a Hicma sequence
    HICMA_sequence_t *pSequence;
    HICMA_request_t request[2] = {HICMA_SUCCESS, HICMA_SUCCESS};
    HICMA_Sequence_Create(&pSequence);

    int n = aConfigurations.GetProblemSize();
    int lts = aConfigurations.GetLowTileSize();
    int p_grid = aConfigurations.GetPGrid();
    int q_grid = aConfigurations.GetQGrid();
    bool is_OOC = aConfigurations.GetIsOOC();
    int max_rank = aConfigurations.GetMaxRank();
    int nZmiss = aConfigurations.GetUnknownObservationsNb();
    T mean_square_error = aConfigurations.GetMeanSquareError();
    int approximation_mode = aConfigurations.GetApproximationMode();
    string actual_observations_path = aConfigurations.GetActualObservationsFilePath();

    int z_obs_number_value;
    if (actual_observations_path.empty()) {
        z_obs_number_value = n - nZmiss;
    } else {
        z_obs_number_value = n;
    }

    int nZobs = aConfigurations.CalculateZObsNumber();

    // For distributed system and should be removed
    T *Zcpy = new T[n];

    int MBC, NBC, MC, NC;
    int MBD, NBD, MD, ND;
    int MBUV, NBUV, MUV, NUV;
    int MBrk, NBrk, Mrk, Nrk;

    FloatPoint float_point;
    if (sizeof(T) == SIZE_OF_FLOAT) {
        float_point = EXAGEOSTAT_REAL_FLOAT;
    } else {
        float_point = EXAGEOSTAT_REAL_DOUBLE;
    }

    //CDense Descriptor
    if (approximation_mode == 1) {
        MBC = lts;
        NBC = lts;
        MC = n;
        NC = n;
    } else {
        MBC = 1;
        NBC = 1;
        MC = lts;
        NC = lts;
    }

    aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_C, is_OOC, nullptr, float_point, MBC, NBC,
                                  MBC * NBC, MC, NC, 0, 0, MC, NC, p_grid, q_grid);
    //CAD Descriptor
    MBD = lts;
    NBD = lts;
    MD = n;
    ND = MBD;
    aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_CD, is_OOC, nullptr, float_point, MBD, NBD,
                                  MBD * NBD, MD, ND, 0, 0, MD, ND, p_grid, q_grid);

    //CUV Descriptor
    MBUV = lts;
    NBUV = 2 * max_rank;
    int N_over_lts_times_lts = n / lts * lts;
    if (N_over_lts_times_lts < n) {
        MUV = N_over_lts_times_lts + lts;
    } else if (N_over_lts_times_lts == n) {
        MUV = N_over_lts_times_lts;
    } else {
        throw range_error("Invalid value. This case should not happen, Please make sure of N and lts values.");
    }

    T expr = (T) MUV / (T) lts;
    NUV = 2 * expr * max_rank;
    aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_CUV, is_OOC, nullptr, float_point, MBUV, NBUV,
                                  MBUV * NBUV, MUV, NUV, 0, 0, MUV, NUV, p_grid, q_grid);

    //Crk Descriptor
    MBrk = 1;
    NBrk = 1;
    auto *desc_cuv = aDescriptorData.GetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_CUV).hicma_desc;
    Mrk = desc_cuv->mt;
    Nrk = desc_cuv->mt;
    aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_CRK, is_OOC, nullptr, float_point, MBrk, NBrk,
                                  MBrk * NBrk, Mrk, Nrk, 0, 0, Mrk, Nrk, p_grid, q_grid);
    aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_Z, is_OOC, nullptr, float_point, lts, lts,
                                  lts * lts, n, 1, 0, 0, n, 1, p_grid, q_grid);
    aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_Z_COPY, is_OOC, nullptr, float_point, lts, lts,
                                  lts * lts, n, 1, 0, 0, n, 1, p_grid, q_grid);
    aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_DETERMINANT, is_OOC, nullptr, float_point, lts,
                                  lts, lts * lts, 1, 1, 0, 0, 1, 1, p_grid, q_grid);

    if (nZmiss != 0) {
        if (actual_observations_path.empty()) {
            aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_Z_OBSERVATIONS, is_OOC, &Zcpy[nZmiss],
                                          float_point, lts, lts, lts * lts, nZobs, 1, 0, 0, nZobs, 1, p_grid, q_grid);
        } else {
            aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_Z_OBSERVATIONS, is_OOC, nullptr,
                                          float_point, lts, lts, lts * lts, nZmiss, 1, 0, 0, nZmiss, 1, p_grid, q_grid);
        }
        //C12AD Descriptor
        MBD = lts;
        NBD = lts;
        MD = nZmiss;
        ND = MBD;
        aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_Z_Actual, is_OOC, nullptr, float_point, lts,
                                      lts, lts * lts, nZmiss, 1, 0, 0, nZmiss, 1, p_grid, q_grid);
        aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_C12D, is_OOC, nullptr, float_point, MBD, NBD,
                                      MBD * NBD, MD, ND, 0, 0, MD, ND, p_grid, q_grid);
        //C12UV Descriptor
        MBUV = lts;
        NBUV = 2 * max_rank;
        aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_C12UV, is_OOC, nullptr, float_point, MBUV,
                                      NBUV, MBUV * NBUV, MBUV, NBUV, 0, 0, MBUV, NBUV, p_grid, q_grid);
        //C12Ark Descriptor
        MBrk = 1;
        NBrk = 1;
        auto *desc_c12uv = aDescriptorData.GetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_C12UV).hicma_desc;
        Mrk = desc_c12uv->mt;
        Nrk = desc_c12uv->mt;
        aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_C12RK, is_OOC, nullptr, float_point, MBrk,
                                      NBrk, MBrk * NBrk, Mrk, Nrk, 0, 0, Mrk, Nrk, p_grid, q_grid);
        //C22D Descriptor
        MBD = lts;
        NBD = lts;
        MD = nZobs;
        ND = MBD;
        aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_C22D, is_OOC, nullptr, float_point, MBD, NBD,
                                      MBD * NBD, MD, ND, 0, 0, MD, ND, p_grid, q_grid);
        //C22UV Descriptor
        MBUV = lts;
        NBUV = 2 * max_rank;
        aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_C22UV, is_OOC, nullptr, float_point, MBUV,
                                      NBUV, MBUV * NBUV, MBUV, NBUV, 0, 0, MBUV, NBUV, p_grid, q_grid);
        //C22Ark Descriptor
        MBrk = 1;
        NBrk = 1;
        auto *desc_c22uv = aDescriptorData.GetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_C22UV).hicma_desc;
        Mrk = desc_c22uv->mt;
        Nrk = desc_c22uv->mt;
        aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_C22RK, is_OOC, nullptr, float_point, MBrk,
                                      NBrk, MBrk * NBrk, Mrk, Nrk, 0, 0, Mrk, Nrk, p_grid, q_grid);
        //Other descriptors
        aDescriptorData.SetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_MSE, is_OOC, nullptr, float_point, lts, lts,
                                      lts * lts, 1, 1, 0, 0, 1, 1, p_grid, q_grid);
    }

    aDescriptorData.SetSequence(pSequence);
    aDescriptorData.SetRequest(request);

    //stop gsl error handler
    gsl_set_error_handler_off();
    delete[] Zcpy;
    aDescriptorData.SetIsDescriptorInitiated(true);
}

template<typename T>
void HicmaImplementation<T>::InitiatePredictionDescriptors(Configurations &aConfigurations, ExaGeoStatData <T> &aData) {
    throw std::runtime_error("unimplemented for now");
}

template<typename T>
void
HicmaImplementation<T>::ExaGeoStatGaussianToNonTileAsync(DescriptorData <T> &aDescriptorData, void *apDesc,
                                                         T *apTheta) {
    throw std::runtime_error("unimplemented for now");
}

template<typename T>
void
HicmaImplementation<T>::CovarianceMatrixCodelet(DescriptorData <T> &aDescriptorData, void *apDescriptor,
                                                const int &aTriangularPart, Locations <T> *apLocation1,
                                                Locations <T> *apLocation2, Locations <T> *apLocation3, T *aLocalTheta,
                                                const int &aDistanceMetric, const string &aKernelName) {

    // Check for initialize the Hicma context.
    if (!this->mpContext) {
        throw std::runtime_error(
                "ExaGeoStat hardware is not initialized, please use 'ExaGeoStat<double/float>::ExaGeoStatInitializeHardware(configurations)'.");
    }

    HICMA_option_t options;
    HICMA_RUNTIME_options_init(&options, (HICMA_context_t * )
    this->mpContext,
            (HICMA_sequence_t *) aDescriptorData.GetSequence(),
            (HICMA_request_t *) aDescriptorData.GetRequest());
    int tempmm, tempnn;
    kernels::Kernel <T> *pKernel = exageostat::plugins::PluginRegistry < kernels::Kernel < T >> ::Create(aKernelName);

    auto *HICMA_apDescriptor = (HICMA_desc_t *) apDescriptor;
    HICMA_desc_t A = *HICMA_apDescriptor;
    struct starpu_codelet *cl = &this->cl_dcmg;
    int m, n, m0 = 0, n0 = 0;

    for (n = 0; n < A.nt; n++) {
        tempnn = n == A.nt - 1 ? A.n - n * A.nb : A.nb;
        if (aTriangularPart == HicmaUpperLower) {
            m = 0;
        } else {
            m = A.m == A.n ? n : 0;
        }
        for (; m < A.mt; m++) {

            tempmm = m == A.mt - 1 ? A.m - m * A.mb : A.mb;
            m0 = m * A.mb;
            n0 = n * A.nb;

            // Register the data with StarPU
            starpu_insert_task(cl,
                               STARPU_VALUE, &tempmm, sizeof(int),
                               STARPU_VALUE, &tempnn, sizeof(int),
                               STARPU_VALUE, &m0, sizeof(int),
                               STARPU_VALUE, &n0, sizeof(int),
                               STARPU_W, (starpu_data_handle_t) HICMA_RUNTIME_data_getaddr(HICMA_apDescriptor, m, n),
                               STARPU_VALUE, &apLocation1, sizeof(dataunits::Locations < T > *),
                               STARPU_VALUE, &apLocation2, sizeof(dataunits::Locations < T > *),
                               STARPU_VALUE, &apLocation3, sizeof(dataunits::Locations < T > *),
                               STARPU_VALUE, &aLocalTheta, sizeof(double *),
                               STARPU_VALUE, &aDistanceMetric, sizeof(int),
                               STARPU_VALUE, &pKernel, sizeof(exageostat::kernels::Kernel < T > *),
                               0);
        }
    }
    HICMA_RUNTIME_options_ws_free(&options);
    HICMA_RUNTIME_options_finalize(&options, (HICMA_context_t * )
    this->mpContext);
    HICMA_Sequence_Wait((HICMA_sequence_t *) aDescriptorData.GetSequence());
    delete pKernel;
}

template<typename T>
void
HicmaImplementation<T>::GenerateObservationsVector(Configurations &aConfigurations, DescriptorData <T> &aDescriptorData,
                                                   const BaseDescriptor &aDescriptor, Locations <T> *apLocation1,
                                                   Locations <T> *apLocation2, Locations <T> *apLocation3,
                                                   const int &aDistanceMetric) {

    // Check for initialize the Hicma context.
    if (!this->mpContext) {
        throw std::runtime_error(
                "ExaGeoStat hardware is not initialized, please use 'ExaGeoStat<double/float>::ExaGeoStatInitializeHardware(configurations)'.");
    }
    int n = aConfigurations.GetProblemSize();
    int seed = aConfigurations.GetSeed();
    int iseed[4] = {seed, seed, seed, 1};
    auto *pDescriptor = aDescriptor.hicma_desc;
    //nomral random generation of e -- ei~N(0, 1) to generate Z
    auto *Nrand = new T[n];
    LAPACKE_dlarnv(3, iseed, n, (double *) Nrand);

    //Generate the co-variance matrix C
    auto *theta = new T[aConfigurations.GetInitialTheta().size()];
    for (int i = 0; i < aConfigurations.GetInitialTheta().size(); i++) {
        theta[i] = aConfigurations.GetInitialTheta()[i];
    }

    VERBOSE("Initializing Covariance Matrix (Synthetic Dataset Generation Phase).....")
    int upper_lower = EXAGEOSTAT_LOWER;
    this->CovarianceMatrixCodelet(aDescriptorData, pDescriptor, upper_lower, apLocation1, apLocation2, apLocation3,
                                  theta, aDistanceMetric, aConfigurations.GetKernelName());
    delete[] theta;
    VERBOSE("Done.")

    //Copy Nrand to Z
    VERBOSE("Generate Normal Random Distribution Vector Z (Synthetic Dataset Generation Phase) .....")
    auto *HICMA_descriptorZ = aDescriptorData.GetDescriptor(common::HICMA_DESCRIPTOR, DESCRIPTOR_Z).hicma_desc;
    CopyDescriptorZ(aDescriptorData, HICMA_descriptorZ, Nrand);
    VERBOSE("Done.")
    delete[] Nrand;
    //// RESET OF THE IMPLEMENTATION WILL BE ADDED AFTER FINALIZING ALL MODULES WITH EXACT.
}

template<typename T>
void
HicmaImplementation<T>::CopyDescriptorZ(DescriptorData <T> &aDescriptorData, void *apDescriptor, T *apDoubleVector) {
    throw std::runtime_error("unimplemented for now");
}


template<typename T>
T HicmaImplementation<T>::ExaGeoStatMLETile(const hardware::ExaGeoStatHardware &apHardware, ExaGeoStatData <T> &aData,
                                            Configurations &aConfigurations, const double *theta,
                                            T *apMeasurementsMatrix) {

    throw std::runtime_error("unimplemented for now");
}

template<typename T>
T *HicmaImplementation<T>::ExaGeoStatMLEPredictTile(ExaGeoStatData <T> &aData, T *apTheta, const int &aZMissNumber,
                                                    const int &aZObsNumber, T *apZObs, T *apZActual, T *apZMiss,
                                                    const hardware::ExaGeoStatHardware &aHardware,
                                                    Configurations &aConfiguration, Locations <T> &aMissLocations,
                                                    Locations <T> &aObsLocations) {
    throw std::runtime_error("unimplemented for now");
}

template<typename T>
int HicmaImplementation<T>::ExaGeoStatLapackCopyTile(const UpperLower &aUpperLower, void *apA, void *apB) {
    throw std::runtime_error("unimplemented for now");

}

template<typename T>
int
HicmaImplementation<T>::ExaGeoStatLapackToDescriptor(const UpperLower &aUpperLower, void *apAf77, const int &aLDA,
                                                     void *apA) {
    throw std::runtime_error("unimplemented for now");
}

template<typename T>
int HicmaImplementation<T>::ExaGeoStatSequenceWait(void *apSequence) {
    throw std::runtime_error("unimplemented for now");
}

template<typename T>
int HicmaImplementation<T>::ExaGeoStatPotrfTile(const UpperLower &aUpperLower, void *apA) {
    throw std::runtime_error("unimplemented for now");
}

template<typename T>
int HicmaImplementation<T>::ExaGeoStatTrsmTile(const Side &aSide, const UpperLower &aUpperLower, const Trans &aTrans,
                                               const Diag &aDiag, const T &aAlpha,
                                               void *apA, void *apB) {
    throw std::runtime_error("unimplemented for now");
}

template<typename T>
int
HicmaImplementation<T>::ExaGeoStatGemmTile(const Trans &aTransA, const Trans &aTransB, const T &aAlpha, void *apA,
                                           void *apB, const T &aBeta,
                                           void *apC) {
    throw std::runtime_error("unimplemented for now");
}

template<typename T>
int
HicmaImplementation<T>::ExaGeoStaStrideVectorTileAsync(void *apDescA, void *apDescB, void *apDescC, void *apSequence,
                                                       void *apRequest) {
    throw std::runtime_error("unimplemented for now");
}

template<typename T>
int HicmaImplementation<T>::ExaGeoStatMeasureDetTileAsync(void *apDescA, void *apSequence, void *apRequest,
                                                          void *apDescDet) {
    throw std::runtime_error("unimplemented for now");
}

template<typename T>
int HicmaImplementation<T>::ExaGeoStatMLEMseTileAsync(void *apDescZPredict, void *apDescZMiss, void *apDescError,
                                                      void *apSequence, void *apRequest) {
    throw std::runtime_error("unimplemented for now");
}

template<typename T>
int
HicmaImplementation<T>::ExaGeoStatPosvTile(const UpperLower &aUpperLower, void *apA, void *apB) {
    throw std::runtime_error("unimplemented for now");
}


template<typename T>
void HicmaImplementation<T>::ExaGeoStatLap2Desc(T *apA, const int &aLDA, void *apDescA, const UpperLower &aUpperLower) {
    throw std::runtime_error("unimplemented for now");
}

template<typename T>
void HicmaImplementation<T>::ExaGeoStatDesc2Lap(T *apA, const int &aLDA, void *apDescA, const UpperLower &aUpperLower) {
    throw std::runtime_error("unimplemented for now");
}

template<typename T>
void
HicmaImplementation<T>::ExaGeoStatGetZObs(Configurations &aConfigurations, T *apZ, const int &aSize,
                                          DescriptorData <T> &aDescData, T *apMeasurementsMatrix) {
    throw std::runtime_error("unimplemented for now");
}


template<typename T>
void
HicmaImplementation<T>::InitiateMloeMmomDescriptors(Configurations &aConfigurations, ExaGeoStatData <T> &aData) {
    throw std::runtime_error("unimplemented for now");

}


template<typename T>
void
HicmaImplementation<T>::ExaGeoStatMLEMloeMmomTile(Configurations &aConfigurations, ExaGeoStatData <T> &aData,
                                                  const hardware::ExaGeoStatHardware &aHardware, T *apTruthTheta,
                                                  T *apEstimatedTheta, Locations <T> &aMissLocations,
                                                  Locations <T> &aObsLocations) {
    throw std::runtime_error("unimplemented for now");
}

template<typename T>
int HicmaImplementation<T>::ExaGeoStatMLEMloeMmomTileAsync(void *apDescExpr2, void *apDescExpr3, void *apDescExpr4,
                                                           void *apDescMloe, void *apDescMmom, void *apSequence,
                                                           void *apRequest) {
    throw std::runtime_error("unimplemented for now");
}

template<typename T>
int HicmaImplementation<T>::ExaGeoStatGeaddTile(const Trans &aTrans, const T &aAlpha, void *apDescA, const T &aBeta,
                                                void *apDescB) {
    throw std::runtime_error("unimplemented for now");
}

template<typename T>
void HicmaImplementation<T>::ExaGeoStatTrmmTile(const Side &aSide, const UpperLower &aUpperLower, const Trans &aTrans,
                                                const Diag &aDiag, const T &alpha,
                                                void *apDescA, void *apDescB) {
    throw std::runtime_error("unimplemented for now");
}